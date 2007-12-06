/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Dump command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "../actionsdk.h"

/* @@TYPE@@   Dump
 * @@DOC@@    <b>dump [file]</b> the current event to the given file. The informations
 * @@DOC@@    are stored in the following order:
 * @@DOC@@      - current time (8 bytes, milliseconds)
 * @@DOC@@      - type (1 byte, Connecting, Reading, Writing, Closing...)
 * @@DOC@@      - proto (1 byte, TCP, UDP)
 * @@DOC@@      - localport (2 bytes local port - 0 on connection)
 * @@DOC@@      - address (4 bytes IPv4 address)
 * @@DOC@@      - port (2 bytes port)
 * @@DOC@@      - success (1 byte: -1 for error, 0 for syscall not performed, 1 for success)
 * @@DOC@@      - length (4 bytes, errno on error)
 * @@DOC@@      - data...
 * @@DOC@@
 * @@DOC@@    The default installation, install a script named injecthexdump that can
 * @@DOC@@    read the content of a dump file and print it on stdout with hexdump -C format
 * @@DOC@@    for data, and the log action format for actions.
 */

static bool ActionDump_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  if (Parse_word(from, &data[1].str, NULL)) {
    data[0].p = NULL;
    pthread_mutex_init(&data[2].mtx, NULL);
    return true;
  }
  return true;
}

static bool ActionDump_perform(int pos, ActionData* data, SocketInfo* si,
                               ActionCallData* state) {
  FILE* file = NULL;
  int8_t oneByte;
  int16_t twoBytes;
  int32_t fourBytes;
  uint64_t timer = getMSecTime();

  pthread_mutex_lock(&data[2].mtx);
  file = (FILE*)data[0].p;
  if (file == NULL) {
    char fname[FILENAME_MAX + 1];
    fname[FILENAME_MAX] = '\0';
    if (snprintf(fname, FILENAME_MAX, "%s.%d", data[1].str, (int)getpid()) > 0) {
      data[0].p = file = fopen(fname, "w");
    }
    if (data[0].p == NULL) {
      pthread_mutex_unlock(&data[2].mtx);
      return Action_error("Can't open dump file... abort");
    }
  }

  fwrite(&timer, 8, 1, file);
  oneByte = state->direction;
  fwrite(&oneByte, 1, 1, file);
  oneByte = si->proto;
  fwrite(&oneByte, 1, 1, file);
  twoBytes = si->local.port;
  fwrite(&twoBytes, 2, 1, file);
  fwrite(&si->remote.addr, 4, 1, file);
  twoBytes = si->remote.port;
  fwrite(&twoBytes, 2, 1, file);
  if (state->done || state->aborted) {
    oneByte = state->result == -1 ? -1 : 1;
    fwrite(&oneByte, 1, 1, file);
    if (state->result == -1) {
      fwrite(&errno, 4, 1, file);
    }
  } else {
    oneByte = 0;
    fwrite(&oneByte, 1, 1, file);
  }
  if ((Data & state->direction) && state->result != -1) {
    fourBytes = READ_BUFFER_LENGTH(state);
    fwrite(&fourBytes, 4, 1, file);
    fwrite(READ_BUFFER(state), 1, fourBytes, file);
  }
  fflush(file);
  pthread_mutex_unlock(&data[2].mtx);
  return true;
}

static void ActionDump_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "dump %s ", data[1].str);
}

static void ActionDump_close(ActionData* data) {
  if (data[0].p) {
    fclose(data[0].p);
  }
  free(data[1].str);
  pthread_mutex_destroy(&data[2].mtx);
}

void ActionDump_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Dump;
  definition->name     = "dump";
  definition->argument = ActionDump_argument;
  definition->perform  = ActionDump_perform;
  definition->write    = ActionDump_write;
  definition->close    = ActionDump_close;
}
