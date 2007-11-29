/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Runtime update of the queue
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "runtime.h"
#include "parser.h"

typedef int  (Runtime_getter)(int fd);
typedef void (Runtime_worker)(int fd);
typedef bool (Runtime_checker)(int fd);

/** Datas stored in the Runtime input thread.
 */
struct RuntimeData {
  int fd;                        /**< File descriptor of the input. */
  Runtime_getter*   accept;      /**< Callback to accept a connection. */
  Runtime_getter*   writer;      /**< Callback to get the file where to write output. */
  Runtime_worker*   localClose;  /**< Callback to close a connection. */
  Runtime_worker*   globalClose; /**< Callback to close the listener. */
  Config* config;                /**< Configuration to be used. */

  pthread_t         thread;      /**< The thread running the interface. */
};

/* Useful stuff */

static void RuntimeAll_close(int fd) {
  static char buffer[1024];
  int flags;
  flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  while (read(fd, buffer, 1024) > 0);
  close(fd);
}

static int RuntimeAll_id(int fd) {
  return fd;
}

static void RuntimeAll_nop(int fd) {
  return;
}

static int RuntimeAll_writeStderr(int fd) {
  return fileno(stderr);
}

static void RuntimeAll_write(int fd, const char* string) {
  write(fd, string, strlen(string));
}


/* Pipe interaction callbacks */

static int RuntimePipe_create(const char* file) {
  struct stat attr;
  int fp = -1;

  if (stat(file, &attr) == -1 && mkfifo(file, 0666) == -1) {
    return -1;
  }
  if (stat(file, &attr) == -1 || !S_ISFIFO(attr.st_mode)) {
    unlink(file);
    return -1;
  }
  fp = open(file, O_RDONLY | O_NONBLOCK);

  if (fp != -1) { /* To be sure, that we can read/write the FIFO */
      int flags;
      flags = fcntl(fp, F_GETFL, 0);
      fcntl(fp, F_SETFL, flags & (~O_NONBLOCK));
      chmod(file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  } else {
    fprintf(stderr, "Error while opening the pipe\n");
  }
  return fp;
}


/* TCP Socket interaction callbacks */

static int RuntimeTCP_create(uint16_t port) {
  int sock;
  struct sockaddr_in addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
    fprintf(stderr, "Can't create a new socket\n");
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  addr.sin_addr.s_addr = 0;

  if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    fprintf(stderr, "Error while binding socket to port %d: %s\n", port, strerror(errno));
    close(sock);
    return -1;
  }
  if (listen(sock, 1) == -1) {
    fprintf(stderr, "Error while listening on socket: %s\n", strerror(errno));
    close(sock);
    return -1;
  }
  return sock;
}

static int RuntimeTCP_accept(int fd) {
  struct sockaddr addr;
  socklen_t addrlen;

  return accept(fd, &addr, &addrlen);
}


/* Thread stuff */

enum Runtime_Command {
  RC_None     = 0,
  RC_List     = 1,
  RC_Add      = 2,
  RC_Replace  = 3,
  RC_Remove   = 4,
  RC_Close    = 5,
  RC_Exit     = 6,
  RC_Help     = 7
};

static inline void Runtime_greeting(int fd, struct RuntimeData* d) {
  RuntimeAll_write(fd,
               "                          ,---------------------------------------------,\n"
               "                          | Welcome to inject ruleset edition interface |\n"
               ",-------------------------^---------------------------------------------^------------------------,\n"
               "| Command format is:                                                                             |\n"
               "|    command: arguments                                                                          |\n"
               "|    (eg. \"add: 20 on tcp from me to any when always do echo SentTCP continue\")                  |\n"
               "|                                                                                                |\n"
               "| Available commands are:                                                                        |\n"
               "|    add: rule       Add a new line to the ruleset. It does not replace rules that already exist |\n"
               "|    replace: rule   Add or replace a line to the ruleset.                                       |\n"
               "|    remove: line    Remove given line                                                           |\n"
               "|    list:           List all rules                                                              |\n"
               "|    quit:           Close the connection (no effect for pipes)                                  |\n"
               "|    exit:           Close the program injected program                                          |\n"
               "|    help:           Print this help                                                             |\n"
               "'------------------------------------------------------------------------------------------------'\n");
}

static void* Runtime_thread(void* d) {
  struct RuntimeData* data = (struct RuntimeData*)d;
  Parser* parser;
  Parser* subparser;
  Parse_enumData ed[] = { { "list", RC_List }, { "add", RC_Add },
    { "replace", RC_Replace }, { "remove", RC_Remove }, { "quit", RC_Close },
    { "exit", RC_Exit }, { "help", RC_Help }, { NULL, RC_None } };
  enum Runtime_Command command;
  int    fd;
  char   buffer[1024];
  bool   closeProg = false;

  buffer[1023] = '\0';

  /* Build the parser */
  parser = Parser_init();
  Parser_add(parser, Parse_enum, &command, ed, NULL);
  Parser_addChar(parser, ":");
  subparser = Parser_newSubParser(parser, PB_optional);
  Parser_addSpace(subparser);

  /* Do not support multi-connection */
  while (!closeProg && (fd = data->accept(data->fd)) != -1) {
    ssize_t r;
    bool closeCon = false;

    Runtime_greeting(fd, data);
    while (!closeCon && (r = read(fd, buffer, 1023)) >= 0) {
      char* pos  = buffer;
      buffer[r] = '\0';
      if (r <= 3) {
        if (r == 0) {
          usleep(200000);
        }
        continue;
      }
      while (!closeCon && pos) {
        const char* line = pos;
        pos = strchr(pos, '\n');
        if (pos != NULL) {
          *pos = '\0';
          ++pos;
        }
        if (strlen(line) <= 3) {
          continue;
        }
        if (!Parse_suite(&line, NULL, parser) || command == RC_None) {
          RuntimeAll_write(data->writer(fd), "Error: invalid command\n\n");
          continue;
        }
        switch (command) {
         case RC_List:
          ActionQueue_list(data->config->queue, data->writer(fd));
          RuntimeAll_write(data->writer(fd), "Done\n\n");
          break;
         case RC_Add:
          if (!ActionQueue_put(data->config->queue, line, false, true)) {
            RuntimeAll_write(data->writer(fd), "Error: invalid rule. It may be due to line number collision.\n\n");
          } else {
            RuntimeAll_write(data->writer(fd), "Done\n\n");
          }
          break;
         case RC_Replace:
          if (!ActionQueue_put(data->config->queue, line, true, true)) {
            RuntimeAll_write(data->writer(fd), "Error: invalid rule.\n\n");
          } else {
            RuntimeAll_write(data->writer(fd), "Done\n\n");
          }
          break;
         case RC_Remove:
         {
          int lnb;
          if (!Parse_int(&line, &lnb, NULL)) {
            RuntimeAll_write(data->writer(fd), "Error: 'remove' require a line number as argument.\n\n");
            break;
          }
          ActionQueue_remove(data->config->queue, lnb, true);
          RuntimeAll_write(data->writer(fd), "Done\n\n");
          break;
         }
         case RC_Exit:
          closeProg = true;
         case RC_Close:
          closeCon  = true;
          break;
         case RC_Help:
           Runtime_greeting(fd, data);
           break;
         default:
          RuntimeAll_write(data->writer(fd), "Warning: command not yet implemented\n\n");
          break;
        }
      }
    }
    if (fd != -1) {
      RuntimeAll_write(data->writer(fd), "Bye\n");
      data->localClose(fd);
    }
  }
  if (closeProg) {
    data->globalClose(data->fd);
    raise(SIGTERM);
  }
  Parser_destroy(parser);
  return NULL;
}

bool Runtime_start(Config* config) {
  struct RuntimeData* data;
  int err;

  data = (struct RuntimeData*)malloc(sizeof(struct RuntimeData));
  data->config = config;

  switch (config->runtime.type) {
   case RT_TCP:
    data->fd          = RuntimeTCP_create(config->runtime.port);
    data->accept      = RuntimeTCP_accept;
    data->localClose  = RuntimeAll_close;
    data->globalClose = RuntimeAll_close;
    data->writer      = RuntimeAll_id;
    {
      char buffer[1024];
      buffer[1023] = '\0';
      snprintf(buffer, 1023, "0 on tcp with me port %d do nop stop", config->runtime.port);
      ActionQueue_put(config->queue, buffer, true, false);
    }
    break;
   case RT_Pipe:
    data->fd          = RuntimePipe_create(config->runtime.file);
    data->accept      = RuntimeAll_id;
    data->localClose  = RuntimeAll_nop;
    data->globalClose = RuntimeAll_close;
    data->writer      = RuntimeAll_writeStderr; /* A pipe is uni-directional */
    break;
   default:
    data->fd = -1;
  }
  if (data->fd == -1) {
    free(data);
    return false;
  }
  if ((err = pthread_create(&data->thread, NULL, Runtime_thread, data))) {
    fprintf(stderr, "Can't launch Runtime thread: %s", strerror(err));
    return false;
  }
  return true;
}
