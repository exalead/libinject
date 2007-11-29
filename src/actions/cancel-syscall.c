/******************************************************************************
 *
 *                               The NG Project
 *
 *                   Cancel-Syscall command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ CancelSyscall
 * @@DOC@@  <b>cancel-syscall</b> force the syscall to be skipped
 */

static bool ActionCancelSyscall_perform(int pos, ActionData* data, SocketInfo* si,
                                        ActionCallData* state) {
  if (state->done) {
    return Action_error("Aborting syscall while call already done");
  }
  state->aborted = true;
  state->result  = -1;
  return true;
}

void ActionCancelSyscall_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_CancelSyscall;
  definition->name     = "cancel-syscall";
  definition->argument = NULL;
  definition->perform  = ActionCancelSyscall_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
