/******************************************************************************
 *
 *                               The NG Project
 *
 *                          Action queue and processor
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "parser.h"
#include "actions.h"
#include "actionsdk.h"
#include "socketinfo.h"
#include "ligHT.h"


/******************************************************************************
 * ActionSet definition
 *****************************************************************************/

static struct ActionTaskDefinition       taskSet[ATT_Number];
static struct ActionConditionDefinition  conditionSet[ACT_Number];

#define Action(action)    (taskSet[(action)->task.type])
#define Condition(action) (conditionSet[(action)->condition.type])

void ActionSet_init(void) {
  BUILD_INIT_AT(taskSet);
  BUILD_INIT_AC(conditionSet);
}


/******************************************************************************
 * Action
 *****************************************************************************/

/** Action mode.
 */
enum ActionMode {
  AM_Normal        = 0, /**< Do the action each time a match occured. */
  AM_Once          = 1, /**< Do the action only once (action is removed after it execution). */
  AM_OncePerCall   = 2, /**< Do the action only once per call. */
  AM_OncePerSocket = 3  /**< Do the action only once per socket. */
};

/** Action goto types.
 */
enum ActionGotoType {
  AGT_Continue, /**< continue execution starting next line. */
  AGT_Goto,     /**< continue execution starting on the given line. */
  AGT_Next,     /**< process the socket against the next rule even if it does not match the filter. */
  AGT_Do,       /**< process the socket against the rule at given line (same as AGT_Next). */
  AGT_Stop      /**< stop processing. */
};

/** Action goto.
 */
struct ActionGoto {
  enum ActionGotoType type; /**< The type of goto. */
  int  line;                /**< The next execution line (if needed by the type only). */
};

/** An action.
 */
struct Action {
  int          pos;   /**< Position of the action in the queue. */
  ActionQueue* queue; /**< Queue containing the action. */

  Proto       proto;  /**< IP transport protocole. */
  SocketInfoDirection direction; /**< Direction of the data. */
  HostAddress from;   /**< Source address. */
  HostAddress to;     /**< Destination address. */

  struct ActionCondition condition;  /**< Condition of matching. */
  enum ActionMode        mode;       /**< 'Do'-Mode. */
  struct ActionTask      task;       /**< The task to execute. */
  struct ActionGoto      next;       /**< Next action to execute. */
};

static bool Action_parse_host(const char** from, void* dest, const void* constraint) {
  static Parse_enumData specialHosts[] = {
    { "me",  AH_Me },
    { "any", AH_Any },
    { "dns", AH_DNS },
    { NULL,  AH_Address } };
  struct HostAddress* host = (struct HostAddress*)dest;
  bool ok;
  char* addr = NULL;
  char* service = NULL;
  const char* source = *from;
  Parser* parser;
  Parser* subparser;

  host->addr = 0;
  host->port = -1;

  parser = Parser_init();
  Parser_addSpacedConstant(parser, constraint);

  /* hostname is an special identifier OR an explicit address */
  subparser = Parser_newSubParser(parser, PB_first);
  Parser_add(subparser, Parse_enum, &host->type, specialHosts, NULL);
  Parser_add(subparser, Parse_word, &addr, NULL, NULL);

  /* port is optional an contains a keyword AND a port number */
  subparser = Parser_newSubParser(parser, PB_optional);
  Parser_addSpace(subparser);
  Parser_addSpacedConstant(subparser, "port");
  subparser = Parser_newSubParser(subparser, PB_first);
  Parser_addInt(subparser, &host->port);
  Parser_add(subparser, Parse_word, &service, NULL, NULL);

  /* A host is a hostname AND a port */
  ok = Parse_suite(&source, NULL, parser);
  Parser_destroy(parser);

  if (!ok) {
    return false;
  }
  if (ok && host->type == AH_Address) {
    struct hostent* hostaddr = NULL;
    hostaddr = gethostbyname2(addr, AF_INET);
    free(addr);
    if (hostaddr == NULL) {
      return false;
    }
    host->addr = ntohl(*(in_addr_t*)(hostaddr->h_addr));
  }
  if (ok && service != NULL) {
    struct servent* serv = NULL;
    serv = getservbyname(service, "tcp");
    free(service);
    if (serv == NULL) {
      return false;
    }
    host->port = serv->s_port;
    host->port = ntohs(host->port);
  }
  *from = source;
  return ok;
}

static bool Action_parse_with(const char** from, void* dest, const void* constraint) {
  Action* action = (Action*)dest;
  action->direction = Any_Dir;
  return true;
}

static bool Action_parse_connect(const char** from, void* dest, const void* constraint) {
  SocketInfoDirection* dir = (SocketInfoDirection*)dest;
  return Parse_word(from, NULL, (void*)"connect")
      && (*dir = Connecting);
}

static bool Action_parse_close(const char** from, void* dest, const void* constraint) {
  SocketInfoDirection* dir = (SocketInfoDirection*)dest;
  return Parse_word(from, NULL, (void*)"close")
      && (*dir = Closing);
}

static bool Action_parse_condition(const char** from, void* dest, const void* constraint) {
  const char* source = *from;
  char* action;
  struct ActionCondition* condition = (struct ActionCondition*)dest;
  int i;

  if (!Parse_word(&source, &action, NULL)) {
    return false;
  }
  for (i = 0 ; i < ACT_Number ; ++i) {
    if (strcmp(action, conditionSet[i].name) == 0) {
      bool ret = true;
      if (conditionSet[i].argument) {
        ret = Parse_space(&source, NULL, NULL) 
              && conditionSet[i].argument(&source, condition->data, NULL);
      }
      free(action);
      if (ret) {
        *from      = source;
        condition->type = i;
      }
      return ret;
    }
  }
  free(action);
  return false;
}

static bool Action_parse_action(const char** from, void* dest, const void* constraint) {
  const char* source = *from;
  char* action;
  struct ActionTask* task = (struct ActionTask*)dest;
  int i;

  if (!Parse_word(&source, &action, NULL)) {
    return false;
  }
  for (i = 0 ; i < ATT_Number ; ++i) {
    if (strcmp(action, taskSet[i].name) == 0) {
      bool ret = true;
      if (taskSet[i].argument) {
        ret = Parse_space(&source, NULL, NULL) && taskSet[i].argument(&source, task->data, NULL);
      }
      free(action);
      if (ret) {
        *from      = source;
        task->type = i;
      }
      return ret;
    }
  }
  free(action);
  return false;
}

static bool Action_parse_next(const char** from, void* dest, const void* constraint) {
  static Parse_enumData gotos[] = {
    { "continue", AGT_Continue },
    { "goto",     AGT_Goto },
    { "next",     AGT_Next },
    { "exec",     AGT_Do },
    { "stop",     AGT_Stop },
    { NULL,       AGT_Stop } };

  struct ActionGoto* next = (struct ActionGoto*)dest;
  const char* source = *from;
  next->line = 0;
  if (!Parse_enum(&source, &next->type, gotos)) {
    return false;
  }
  if (next->type == AGT_Goto || next->type == AGT_Do) {
    if (!Parse_space(&source, NULL, NULL)) {
      return false;
    }
    if (!Parse_int(&source, &next->line, NULL)) {
      return false;
    }
  }
  *from = source;
  return true;
}

static bool Action_parse(Action* action, const char* instruction) {
  /* Static stuff for parser */
  static Parse_enumData protos[] = {
    { "ip",  AP_IP },
    { "tcp", AP_TCP },
    { "udp", AP_UDP },
    { NULL,  0 } };
  static Parse_enumData domodes[] = {
    { "do",                 AM_Normal },
    { "do-once",            AM_Once },
    { "do-once-per-call",   AM_OncePerCall },
    { "do-once-per-socket", AM_OncePerSocket },
    { NULL,                 0 } };

  Parser* parser;
  Parser* subparser;
  Parser* subparser2;

  parser = Parser_init();
  /* line number */
  Parser_addSpacedInt(parser, &action->pos);
  /* on protocole */
  Parser_addSpacedConstant(parser, "on");
  Parser_addSpaced(parser, Parse_enum, &action->proto, protos, NULL);
  /* hosts */
  subparser = Parser_newSubParser(parser, PB_first);
  /*   with */
  subparser2 = Parser_newSubParser(subparser, PB_suite);
  Parser_addSpaced(subparser2, Action_parse_host, &action->from, (void*)"with", NULL);
  Parser_add(subparser2, Action_parse_with, action, NULL, NULL);
  /*   talk-with */
  Parser_addSpaced(subparser, Action_parse_host, &action->from, (void*)"talk-with", NULL);
  /*   or (connect|from) */
  subparser = Parser_newSubParser(subparser, PB_suite);
  subparser2 = Parser_newSubParser(subparser, PB_first);
  Parser_addSpaced(subparser2, Action_parse_connect, &action->direction, NULL, NULL);
  Parser_addSpaced(subparser2, Action_parse_close, &action->direction, NULL, NULL);
  Parser_addSpaced(subparser2, Action_parse_host, &action->from, (void*)"from", NULL);
  /*      and to */
  Parser_addSpaced(subparser, Action_parse_host, &action->to, (void*)"to", NULL);
  /* when condition */
  subparser = Parser_newSubParser(parser, PB_optional);
  Parser_addSpacedConstant(subparser, "when");
  Parser_addSpaced(subparser, Action_parse_condition, &action->condition, NULL, NULL);
  /* do action */
  subparser = Parser_newSubParser(parser, PB_optional);
  Parser_addSpaced(subparser, Parse_enum, &action->mode, domodes, NULL);
  Parser_addSpaced(subparser, Action_parse_action, &action->task, NULL, NULL);
  /* next */
  Parser_add(parser, Action_parse_next, &action->next, NULL, NULL);
  /* the instruction must then be finished */
  subparser = Parser_newSubParser(parser, PB_optional);
  Parser_addSpace(subparser);
  Parser_checkEOB(parser);

  /* Default values */
  action->direction      = Data;
  action->condition.type = ACT_Always;
  action->mode           = AM_Normal;
  action->task.type      = ATT_Nop;
  action->from.type      = AH_None;
  action->to.type        = AH_None;

  return Parser_run(parser, instruction, PB_suite, true);
}

Action* Action_init(const char* instruction) {
  Action* action;
  action = (Action*)malloc(sizeof(Action));
  if (!Action_parse(action, instruction)) {
    free(action);
    return NULL;
  }
  return action;
}

void Action_destroy(Action* action) {
  if (!action) {
    return;
  }
  if (Condition(action).close) {
    Condition(action).close(action->condition.data);
  }
  if (Action(action).close) {
    Action(action).close(action->task.data);
  }
  free(action);
}

static bool Action_addressMatch(const struct HostAddress* address,
                                const struct HostAddress* sockAddr) {
  const bool canBeMe  = sockAddr->type == AH_Me;
  const bool dns      = sockAddr->type == AH_DNS;
  const bool samePort = (address->port == -1 || address->port == sockAddr->port);
  const bool sameHost = (sockAddr->addr != address->addr);
  switch (address->type) {
   case AH_Me:
    return canBeMe && samePort;
   case AH_DNS:
    return dns;
   case AH_Address:
    return sameHost && samePort;
   case AH_Any:
    return samePort;
   default:
    return false;
  }
}

inline bool Action_match(Action* action, SocketInfo* si,
                         SocketInfoDirection direction, bool matched) {
  if (!(action->direction & direction)) {
    return false;
  }
  if (!Condition(action).match(action->condition.data, si, direction, matched)
       || !(si->proto & action->proto)) {
    return false;
  }
  switch (action->direction) {
   case Data: case Writing: case Reading:
    if (action->to.type != AH_None) {
      return Action_addressMatch(direction == Writing ? &action->from : &action->to, &si->local)
          && Action_addressMatch(direction == Reading ? &action->from : &action->to, &si->remote);
    }
   case Any_Dir:
    return Action_addressMatch(&action->from, &si->local)
        || Action_addressMatch(&action->from, &si->remote);
   case Connecting: case Closing:
    return Action_addressMatch(&action->to, &si->remote);
   default:
    return false;
  }
}

bool Action_process(Action* action, SocketInfo* si, ActionCallData* state) {
  struct ActionSocketData* socketState;
  socketState = ActionSocketData_get(si, state->queue);
  if (state->direction == Reading && socketState->hanging) {
    socketState->hanging = false;
    if (action->task.type == ATT_Hang) {
      return true;
    }
  }
  return Action(action).perform(action->pos, action->task.data, si, state);
}

static inline  void Action_show_address(const char* kw, struct HostAddress* addr, char** buffer) {
  switch (addr->type) {
   case AH_Me:      *buffer += sprintf(*buffer, "%s me ", kw); break;
   case AH_Any:     *buffer += sprintf(*buffer, "%s any ", kw); break;
   case AH_DNS:     *buffer += sprintf(*buffer, "%s dns ", kw); break;
   case AH_Address: *buffer += sprintf(*buffer, "%s %d.%d.%d.%d ", kw, SHOW_ADDR(addr->addr)); break;
   default:         *buffer += sprintf(*buffer, "%s ?none? ", kw); break;
  }
  if (addr->port != -1 && addr->type != AH_DNS) {
    *buffer += sprintf(*buffer, "port %d ", addr->port);
  }
}

void Action_show(Action* action, int fd) {
  char buffer[1024];
  char *ptr = buffer;
  switch (action->proto) {
   case AP_UDP: ptr += sprintf(ptr, "%d on udp ", action->pos); break;
   case AP_TCP: ptr += sprintf(ptr, "%d on tcp ", action->pos); break;
   case AP_IP:  ptr += sprintf(ptr, "%d on ip ", action->pos); break;
  }
  switch (action->direction) {
   case Any_Dir: Action_show_address("with", &action->from, &ptr); break;
   case Connecting: Action_show_address("connect to", &action->to, &ptr); break;
   case Closing: Action_show_address("close to", &action->to, &ptr); break;
   default:
    if (action->to.type == AH_None) {
      Action_show_address("talk-with", &action->from, &ptr);
    } else {
      Action_show_address("from", &action->from, &ptr);
      Action_show_address("to", &action->to, &ptr);
    }
    break;
  }
  if (Condition(action).write) {
    ptr += sprintf(ptr, "when ");
    Condition(action).write(&ptr, action->condition.data);
  } else {
    ptr += sprintf(ptr, "when %s ", Condition(action).name);
  }
  switch (action->mode) {
   case AM_Normal:        ptr += sprintf(ptr, "do "); break;
   case AM_Once:          ptr += sprintf(ptr, "do-once "); break;
   case AM_OncePerCall:   ptr += sprintf(ptr, "do-once-per-call "); break;
   case AM_OncePerSocket: ptr += sprintf(ptr, "do-once-per-socket "); break;
  }
  if (Action(action).write) {
    Action(action).write(&ptr, action->task.data);
  } else {
    ptr += sprintf(ptr, "%s ",  Action(action).name);
  }
  switch (action->next.type) {
   case AGT_Continue: ptr += sprintf(ptr, "continue"); break;
   case AGT_Goto:     ptr += sprintf(ptr, "goto %d", action->next.line); break;
   case AGT_Next:     ptr += sprintf(ptr, "next"); break;
   case AGT_Do:       ptr += sprintf(ptr, "exec %d", action->next.line); break;
   case AGT_Stop:     ptr += sprintf(ptr, "stop"); break;
  }
  ptr += sprintf(ptr, "\n");
  write(fd, buffer, ptr - buffer);
}

bool Action_error(const char* message) {
  fprintf(stderr, "ERR  %s\n", message);
  return false;
}


/******************************************************************************
 * ActionQueue
 *****************************************************************************/

#define ACQUIRE_READ                                                          \
  if (mt) {                                                                   \
    pthread_rwlock_rdlock(&queue->lock);                                      \
}
#define ACQUIRE_WRITE                                                         \
  if (mt) {                                                                   \
    pthread_rwlock_wrlock(&queue->lock);                                      \
  }
#define RELEASE                                                               \
  if (mt) {                                                                   \
    pthread_rwlock_unlock(&queue->lock);                                      \
  }

/** An action queue.
 */
struct ActionQueue {
  Action** queue;         /**< Table of Actions */
  size_t   capacity;      /**< Capacity of the table */

  pthread_rwlock_t lock;  /**< A lock for mt accesses */
};

ActionQueue* ActionQueue_init(size_t capacity) {
  ActionQueue* queue;

  queue = (ActionQueue*)malloc(sizeof(ActionQueue));
  if (queue) {
    queue->queue = (Action**)calloc(capacity, sizeof(Action*));
    if (!queue->queue) {
      free(queue);
      return NULL;
    }
    queue->capacity = capacity;
    pthread_rwlock_init(&queue->lock, NULL);
  }
  return queue;
}

void ActionQueue_destroy(ActionQueue* queue) {
  off_t i;
  for (i = 0 ; i < (off_t)queue->capacity ; ++i) {
    Action_destroy(queue->queue[i]);
  }
  free(queue->queue);
  pthread_rwlock_destroy(&queue->lock);
  free(queue);
}

bool ActionQueue_put(ActionQueue* queue, const char* instruction, bool replace, bool mt) {
  Action* action;
  action = Action_init(instruction);
  if (!action) {
    return false;
  }

  ACQUIRE_WRITE
  if (queue->queue[action->pos] == NULL || replace) {
    Action_destroy(queue->queue[action->pos]);
    queue->queue[action->pos] = action;
    action->queue             = queue;
    RELEASE
    return true;
  }
  RELEASE
  Action_destroy(action);
  return false;
}

Action* ActionQueue_get(ActionQueue* queue, off_t pos, bool mt) {
  if (pos >= (off_t)queue->capacity) {
    return NULL;
  } else {
    Action* action;
    ACQUIRE_READ
    action = queue->queue[pos];
    RELEASE
    return action;
  }
}

Action* ActionQueue_getFrom(ActionQueue* queue, off_t pos, bool mt) {
  Action* action = NULL;
  off_t   i;

  ACQUIRE_READ
  for (i = pos ; i < (off_t)queue->capacity ; ++i) {
    if (queue->queue[i]) {
      action = queue->queue[i];
      break;
    }
  }
  RELEASE
  return action;
}

Action* ActionQueue_getMatch(ActionQueue* queue, SocketInfo* si, SocketInfoDirection direction,
                             bool matched, off_t pos, bool mt) {
  Action* action = NULL;
  off_t   i;

  ACQUIRE_READ
  for (i = pos ; i < (off_t)queue->capacity ; ++i) {
    if (queue->queue[i] && Action_match(queue->queue[i], si, direction, matched)) {
      action = queue->queue[i];
      break;
    }
  }
  RELEASE
  return action;
}

ssize_t ActionQueue_process(ActionQueue* queue, SocketInfo* si,
                            SocketInfoDirection direction, Action_syscall callback,
                            void* buf, size_t len, int flags, void* data) {
  Action* action;
  ActionCallData state;
  ActionSocketData* socketState;

  action = ActionQueue_getFirstMatch(queue, si, direction, true);
  if (!action) {
    return callback(si->fd, buf, len, flags, data);
  }

  socketState = ActionSocketData_get(si, queue);
  if (direction == Reading && socketState->hanging && socketState->msec > getMSecTime()) {
    errno = EAGAIN;
    return -1;
  } else if (direction == Reading && socketState->hanging) {
    if (queue->queue[socketState->pos]) {
      action = queue->queue[socketState->pos];
    } else {
      action = ActionQueue_getMatch(queue, si, direction, socketState->pos, true, true);
    }
    if (!action) {
      socketState->hanging = false;
      return callback(si->fd, buf, len, flags, data);
    }
  }

  state.queue     = queue;
  state.callback  = callback;
  state.origBuf   = buf;
  state.origLen   = len;
  if (direction == Writing) {
    state.buf = NULL;
    state.len = 0;
  } else {
    state.buf = state.origBuf;
    state.len = state.origLen;
  }
  state.flags     = flags;
  state.data      = data;
  state.direction = direction;
  state.aborted   = false;
  state.done      = false;
  state.calledLines = LigHT_init(queue->capacity, NULL);
  state.keepError = false;
  state.err       = errno;
  state.result    = 0;

  while (action) {
    Action* next = NULL;
    if ((action->mode == AM_OncePerCall && LigHT_contains(state.calledLines, action->pos, false))
        || (action->mode == AM_OncePerSocket && LigHT_contains(socketState->calledLines, action->pos, true))) {
      action = ActionQueue_getMatch(queue, si, direction, true, action->pos + 1, true);
      continue;
    }
    if (!Action_process(action, si, &state) || socketState->hanging) {
      break;
    }
    (void)LigHT_put(state.calledLines, action->pos, (void*)"OK", true, false);
    (void)LigHT_put(socketState->calledLines, action->pos, (void*)"OK", true, true);
    switch (action->next.type) {
     case AGT_Continue:
      next = ActionQueue_getMatch(queue, si, direction, true, action->pos + 1, true);
      break;
     case AGT_Goto:
      next = ActionQueue_getMatch(queue, si, direction, true, action->next.line, true);
      break;
     case AGT_Next:
      next = ActionQueue_getFrom(queue, action->pos + 1, true);
      break;
     case AGT_Do:
      next = ActionQueue_getFrom(queue, action->next.line, true);
      break;
     default:
      next = NULL;
    }
    if (action->mode == AM_Once) {
      ActionQueue_remove(queue, action->pos, true);
    }
    action = next;
  }
  LigHT_destroy(state.calledLines);
  if (!state.done && !state.aborted) {
    (void)ActionSyscall_perform(-1, NULL, si, &state);
  }
  if (direction == Writing) {
    free(state.buf);
  }
  errno = state.err;
  return state.result;
}

void ActionQueue_remove(ActionQueue* queue, off_t pos, bool mt) {
  ACQUIRE_WRITE
  Action_destroy(queue->queue[pos]);
  queue->queue[pos] = NULL;
  RELEASE
}

void ActionQueue_list(ActionQueue* queue, int fd) {
  size_t i;
  for (i = 0 ; i < queue->capacity ; ++i) {
    if (queue->queue[i] != NULL) {
      Action_show(queue->queue[i], fd);
    }
  }
}


/******************************************************************************
 * ActionData
 *****************************************************************************/

static void ActionSocketData_destroy(SocketInfo* si) {
  if (si) {
    struct ActionSocketData* data = (struct ActionSocketData*)(si->data);
    LigHT_destroy(data->calledLines);
    free(data);
    si->data = NULL;
  }
}

struct ActionSocketData* ActionSocketData_get(SocketInfo* si, ActionQueue* queue) {
  if (!si->data) {
    ActionSocketData* data;
    data = (ActionSocketData*)malloc(sizeof(ActionSocketData));
    data->calledLines = LigHT_init(queue->capacity, NULL);
    SocketInfo_setData(si, data, ActionSocketData_destroy);
    data->hanging    = false;
    data->msec       = 0;
  }
  return (ActionSocketData*)si->data;
}

bool ActionCallData_prepareBuffer(ActionCallData* data) {
  if (data->buf) {
    return true;
  }
  data->buf = malloc(data->origLen);
  if (!data->buf) {
    return false;
  }
  (void)memcpy(data->buf, data->origBuf, data->origLen);
  data->len = data->origLen;
  return true;
}
