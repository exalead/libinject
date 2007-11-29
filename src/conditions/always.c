/******************************************************************************
 *
 *                               The NG Project
 *
 *                       Always condition implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Always
 * @@DOC@@  <b>always</b> always true
 */

static bool ActionAlways_match(ActionData* data, SocketInfo* si,
                               SocketInfoDirection direction, bool matched) {
  return true;
}

void ActionAlways_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Always;
  definition->name     = "always";
  definition->argument = NULL;
  definition->match    = ActionAlways_match;
  definition->write    = NULL;
  definition->close    = NULL;
}
