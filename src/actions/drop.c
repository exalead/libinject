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

/* @@TYPE@@ Drop
 * @@DOC@@  <b>drop</b> Drop the given packet: cancel the syscall and mark the
 * @@DOC@@  state as correct.
 */

static bool ActionDrop_perform(int pos, ActionData* data, SocketInfo* si,
                               ActionCallData* state) {
  if (state->done) {
    return Action_error("Aborting syscall while call already done");
  }
  state->aborted = true;
  state->result  = READ_BUFFER_LENGTH(state);
  return true;
}

void ActionDrop_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Drop;
  definition->name     = "drop";
  definition->argument = NULL;
  definition->perform  = ActionDrop_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
