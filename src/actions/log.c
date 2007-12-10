/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Log command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "../actionsdk.h"

/* @@TYPE@@   Log
 * @@EXPORT@@ perform
 * @@DOC@@    <b>log [file]</b> log packet information to the given file. If this
 * @@DOC@@    is -, stderr is assumed. The default installation install a script
 * @@DOC@@    named injectgraph that can build a set of graph of network activity
 * @@DOC@@    from the output of this action. This script can also be used in
 * @@DOC@@    conjunction of injecthexdump to graph the output of the dump action.
 */

static bool ActionLog_argument(const char** from, void* dest,
                               const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  if (Parse_word(from, &data[1].str, NULL, status)) {
    data[0].p = NULL;
    if (strcmp(data[1].str, "-") == 0) {
      free(data[1].str);
      data[1].str = NULL;
    }
    pthread_mutex_init(&data[2].mtx, NULL);
    return true;
  }
  return false;
}

bool ActionLog_perform(int pos, ActionData* data, SocketInfo* si,
                       ActionCallData* state) {
  FILE* file;

  pthread_mutex_lock(&data[2].mtx);
  file = (FILE*)data[0].p;
  if (data[1].str == NULL) {
    file = stderr;
  } else if (file == NULL) {
    char fname[FILENAME_MAX + 1];
    fname[FILENAME_MAX] = '\0';
    if (snprintf(fname, FILENAME_MAX, "%s.%d", data[1].str, (int)getpid()) > 0) {
      data[0].p = file = fopen(fname, "w");
    }
    if (data[0].p == NULL) {
      (void)Action_error("Can't open log file, fallback to stderr");
      free(data[1].str);
      data[1].str = NULL;
      file = stderr;
    }
  }

  fprintf(file, "[ts %llu] [line %d] ", (long long unsigned int)getMSecTime(), pos);
  if (state->direction == Reading) {
    fprintf(file, "read %s:%d:[%d.%d.%d.%d]:%d length=%zu\n",
                  si->proto & AP_TCP ? "tcp" : "udp", si->local.port,
                  SHOW_ADDR(si->remote.addr), si->remote.port, READ_BUFFER_LENGTH(state));
  } else if (state->direction == Writing) {
    fprintf(file, "write %s:%d:[%d.%d.%d.%d]:%d length=%zu\n",
                  si->proto & AP_TCP ? "tcp" : "udp", si->local.port,
                  SHOW_ADDR(si->remote.addr), si->remote.port, READ_BUFFER_LENGTH(state));
  } else {
    fprintf(file, "%s %s:%d:[%d.%d.%d.%d]:%d\n",
                  state->direction == Connecting ? "connect" : "close",
                  si->proto & AP_TCP ? "tcp" : "udp", si->local.port,
                  SHOW_ADDR(si->remote.addr), si->remote.port);
  }

  fflush(file);
  pthread_mutex_unlock(&data[2].mtx);
  return true;
}

static void ActionLog_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "log %s ", data[1].str ? data[1].str : "-");
}

static void ActionLog_close(ActionData* data) {
  if (data[0].p) {
    fclose(data[0].p);
  }
  free(data[1].str);
  pthread_mutex_destroy(&data[2].mtx);
}

void ActionLog_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Log;
  definition->name     = "log";
  definition->argument = ActionLog_argument;
  definition->perform  = ActionLog_perform;
  definition->write    = ActionLog_write;
  definition->close    = ActionLog_close;
}
