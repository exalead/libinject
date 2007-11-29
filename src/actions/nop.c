/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Skip command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Nop
 * @@DOC@@ <b>nop</b> do nothing
 */

static bool ActionNop_perform(int pos, ActionData* data, SocketInfo* si,
                              ActionCallData* state) {
  return true;
}

void ActionNop_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Nop;
  definition->name     = "nop";
  definition->argument = NULL;
  definition->perform  = ActionNop_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
