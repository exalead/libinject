/******************************************************************************
 *
 *                               The NG Project
 *
 *                       Syscall command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <errno.h>

#include "../actionsdk.h"

/* @@TYPE@@   Syscall
 * @@EXPORT@@ perform
 * @@DOC@@    <b>syscall</b> do the system call
 */

bool ActionSyscall_perform(int pos, ActionData* data, SocketInfo* si,
                           ActionCallData* state) {
  if (state->aborted) {
    return Action_error("Performing syscall while call aborted");
  }
  state->result = state->callback(si->fd, READ_BUFFER(state), READ_BUFFER_LENGTH(state),
                                          state->flags, state->data);
  state->err = errno;
  if (state->result >= 0) {
    if (state->buf) {
      state->len = state->result;
    }
    state->origLen = state->result;
  } else {
    if (state->buf) {
      state->len = 0;
    }
    state->origLen = 0;
  }
  state->aborted = false;
  state->done    = true;
  return true; /* The processing don't fail if the call fail... it's just
                  the expected behaviour, so, do not interfer! */
}

void ActionSyscall_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Syscall;
  definition->name     = "syscall";
  definition->argument = NULL;
  definition->perform  = ActionSyscall_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
