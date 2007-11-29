/******************************************************************************
 *
 *                               The NG Project
 *
 *                      Mark-done command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ MarkDone
 * @@DOC@@  <b>mark-done [type] [line]</b> Remove the given line of the rule set.
 * @@DOC@@  The type can be "call" or "socket".
 */

static bool ActionMarkDone_argument(const char** from, void* dest, const void* constraint) {
  static Parse_enumData d[] = { { "call", 1 }, { "socket", 2 } };
  union ActionData* data = (union ActionData*)dest;
  const char* source = *from;
  if (Parse_enum(&source, &data[0].i, d) && Parse_space(&source, NULL, NULL)
      && Parse_int(&source, &data[1].i, NULL)) {
    *from = source;
    return true;
  }
  return false;
}

static void ActionMarkDone_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "mark-done %s %d ", data[0].i == 1 ? "call" : "socket",
                     data[1].i);
}

static bool ActionMarkDone_perform(int pos, ActionData* data, SocketInfo* si,
                                 ActionCallData* state) {
  const int line = data[1].i;
  if (data[0].i == 1) {
    (void)LigHT_put(state->calledLines, line, (void*)"OK", true, false);
  } else {
    struct ActionSocketData* socketState;
    socketState = ActionSocketData_get(si, state->queue);
    (void)LigHT_put(socketState->calledLines, line, (void*)"OK", true, true);
  }
  return true;
}

void ActionMarkDone_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_MarkDone;
  definition->name     = "mark-done";
  definition->argument = ActionMarkDone_argument;
  definition->perform  = ActionMarkDone_perform;
  definition->write    = ActionMarkDone_write;
  definition->close    = NULL;
}
