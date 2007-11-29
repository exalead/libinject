/******************************************************************************
 *
 *                               The NG Project
 *
 *                       Never condition implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Never
 * @@DOC@@  <b>never</b> can be useful
 */

static bool ActionNever_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  return false;
}

void ActionNever_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Never;
  definition->name     = "never";
  definition->argument = NULL;
  definition->match    = ActionNever_match;
  definition->write    = NULL;
  definition->close    = NULL;
}
