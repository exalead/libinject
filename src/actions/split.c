/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Split command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@   Split
 * @@DOC@@    <b>split [percent]</b> split the requested action in multiple
 * @@DOC@@    subactions. So, if you are reading, and you call split, you read
 * @@DOC@@    will be performed in only the given percentage of data.
 * @@DOC@@      eg: split 50 on a read of 200 bytes will require only 100 bytes
 * @@DOC@@    When writing, split perform 2 effective writes the first one
 * @@DOC@@    having the specified percentage of the global size.
 */

static bool ActionSplit_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  const char* source = *from;
  if (Parse_int(&source, &data[0].i, NULL) && data[0].i >= 0 && data[0].i <= 100) {
    *from = source;
    return true;
  }
  return false;
}

static bool ActionSplit_perform(int pos, ActionData* data, SocketInfo* si,
                                ActionCallData* state) {
  if (state->done || state->aborted) {
    return Action_error("Trying to perform split action while syscall already marked as done");
  }
  if (state->direction == Reading) {
    state->len = (data[0].i * state->len) / 100;
    if (state->len == 0) {
      state->len = 1;
    }
    return ActionSyscall_perform(pos, data, si, state);
  } else if (state->direction == Writing) {
    void* bufbackup = state->buf;
    size_t lenbackup = state->len;
    void* buf    = READ_BUFFER(state);
    size_t len   = READ_BUFFER_LENGTH(state);
    size_t first = (data[0].i * len) / 100;
    if (len == 1) {
      return ActionSyscall_perform(pos, data, si, state);
    } else if (first == 0) {
      first = 1;
    }
    state->buf = buf;
    state->len = first;
    ActionSyscall_perform(pos, data, si, state);
    if (state->result == (ssize_t)first) {
      state->done = false;
      state->buf = (void*)((uint8_t*)state->buf + first);
      state->len  = len - first;
      ActionSyscall_perform(pos, data, si, state);
      if (state->result != -1) {
        state->result += first;
      } else {
        state->result  = first;
      }
    }
    state->buf = bufbackup;
    state->len = lenbackup;
    return true;
  } else {
    return false;
  }
}

static void ActionSplit_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "split %d ", data[0].i);
}

void ActionSplit_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Split;
  definition->name     = "split";
  definition->argument = ActionSplit_argument;
  definition->perform  = ActionSplit_perform;
  definition->write    = ActionSplit_write;
  definition->close    = NULL;
}
