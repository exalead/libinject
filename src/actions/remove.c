/******************************************************************************
 *
 *                               The NG Project
 *
 *                        Remove command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Remove
 * @@DOC@@  <b>remove [line]</b> Remove the given line of the rule set. You
 * @@DOC@@  can't remove system lines
 */

static bool ActionRemove_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL);
}

static void ActionRemove_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "remove %d ", data[0].i);
}

static bool ActionRemove_perform(int pos, ActionData* data, SocketInfo* si,
                                 ActionCallData* state) {
  if (data[0].i < 10) {
    return Action_error("You have not the right to remove system lines");
  }
  ActionQueue_remove(state->queue, data[0].i, true);
  return true;
}

void ActionRemove_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Remove;
  definition->name     = "remove";
  definition->argument = ActionRemove_argument;
  definition->perform  = ActionRemove_perform;
  definition->write    = ActionRemove_write;
  definition->close    = NULL;
}
