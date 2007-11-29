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
#include <unistd.h>

#include "../actionsdk.h"

/* @@TYPE@@   Reset
 * @@DOC@@    <b>reset</b> reset the connection.
 * @@DOC@@    WARNING: reset action must always issue to a 'stop' next rule
 * @@DOC@@    identifier.
 */

static bool ActionReset_perform(int pos, ActionData* data, SocketInfo* si,
                                ActionCallData* state) {
  if (state->done) {
    return Action_error("Can't reset a connection when action has already been performed");
  }
  close(si->fd);
  state->done = true;
  state->result = -1;
  state->err    = EBADF;
  return true;
}

void ActionReset_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Reset;
  definition->name     = "reset";
  definition->argument = NULL;
  definition->perform  = ActionReset_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
