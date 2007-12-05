/******************************************************************************
 *
 *                               The NG Project
 *
 *                        Replay command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "../actionsdk.h"

/* @@TYPE@@   Replay
 * @@DOC@@    <b>replay [file]</b> The file must be a dump trace (produced using
 * @@DOC@@    the dump command). It's read to feed read/write actions.
 * @@DOC@@    WARNING: you should not use the same instance of this action for
 * @@DOC@@    both reading and writing.
 */

static bool ActionReplay_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  if (Parse_word(from, &data[1].str, NULL)) {
    data[0].p = NULL;
    pthread_mutex_init(&data[2].mtx, NULL);
    return true;
  }
  return true;
}

static bool ActionReplay_perform(int pos, ActionData* data, SocketInfo* si,
                               ActionCallData* state) {
  FILE* file = NULL;
  int8_t direction, proto, success;
  int16_t localport, remoteport;
  int32_t addr, length;
  uint64_t timer;

  if (state->done) {
    return Action_error("Can't replay a dump when syscall already performed");
  }

#define ERROR(message)                                                         \
  pthread_mutex_unlock(&data[2].mtx);                                          \
  return Action_error(message);

#define READ(var)                                                              \
  if (fread(&(var), sizeof(var), 1, file) != 1) {                              \
    ERROR("Invalid dump file: Read error");                                    \
  }

  pthread_mutex_lock(&data[2].mtx);
  file = (FILE*)data[0].p;
  if (file == NULL) {
    data[0].p = file = fopen(data[1].str, "r");
    if (data[0].p == NULL) {
      ERROR("Can't open dump file... abort");
    }
  }

  READ(timer);
  READ(direction);
  READ(proto);
  READ(localport);
  READ(addr);
  READ(remoteport);
  READ(success);
  if (success != 0 && success != 1 && success != -1) {
    ERROR("Invalid dump file: Success value must be in [-1:1]");
  }
  if (success == -1 || (Data & direction)) {
    READ(length);
    if (success != -1) {
      ActionCallData_prepareBuffer(state);
      if (state->len < (uint32_t)length) {
        ERROR("Invalid dump file: Data larger than buffer");
      }
      if (fread(state->buf, 1, length, file) != (size_t)length) {
        ERROR("Invalid dump file: Not enough data");
      }
    }
  }
  if ((SocketInfoDirection)direction != state->direction) {
    ERROR("Dump does not match current direction");
  }
  state->result = success == -1 ? -1
                : (Data & direction) ? length
                : 0;
  state->err    = success == -1 ? length : 0;
  if ((Data & direction) && success != -1) {
    state->origLen = length;
    if (state->buf) {
      state->len = length;
    }
  }
  if (state->direction == Writing) {
    ActionSyscall_perform(pos, NULL, si, state);
  }
  state->aborted = true;

  pthread_mutex_unlock(&data[2].mtx);
  return true;
}

static void ActionReplay_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "replay %s ", data[1].str);
}

static void ActionReplay_close(ActionData* data) {
  if (data[0].p) {
    fclose(data[0].p);
  }
  free(data[1].str);
  pthread_mutex_destroy(&data[2].mtx);
}

void ActionReplay_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Replay;
  definition->name     = "replay";
  definition->argument = ActionReplay_argument;
  definition->perform  = ActionReplay_perform;
  definition->write    = ActionReplay_write;
  definition->close    = ActionReplay_close;
}
