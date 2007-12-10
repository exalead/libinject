/******************************************************************************
 *
 *                               The NG Project
 *
 *                       Truncate command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@   Truncate
 * @@EXPORT@@ perform
 * @@DOC@@    <b>truncate [size]</b> send/receive at most the given size
 */

static bool ActionTruncate_argument(const char** from, void* dest,
                                    const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL, status);
}

bool ActionTruncate_perform(int pos, ActionData* data, SocketInfo* si,
                            ActionCallData* state) {
  if (state->done || state->aborted) {
    return Action_error("Trying to truncate buffer while syscall already marked as done");
  }
  if (!(Data & state->direction)) {
    return false;
  }
  if (state->origLen > (size_t)data[0].i) {
    state->origLen = data[0].i;
  }
  if (state->len > (size_t)data[0].i) {
    state->len = data[0].i;
  }
  return true;
}

static void ActionTruncate_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "trucate %d ", data[0].i);
}

void ActionTruncate_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Truncate;
  definition->name     = "truncate";
  definition->argument = ActionTruncate_argument;
  definition->perform  = ActionTruncate_perform;
  definition->write    = ActionTruncate_write;
  definition->close    = NULL;
}
