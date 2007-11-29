/******************************************************************************
 *
 *                               The NG Project
 *
 *                      Matched condition implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Matched
 * @@DOC@@  <b>matched</b> the socket has matched a previous rule
 */

static bool ActionMatched_match(ActionData* data, SocketInfo* si,
                                SocketInfoDirection direction, bool matched) {
  return matched;
}

void ActionMatched_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Matched;
  definition->name     = "matched";
  definition->argument = NULL;
  definition->match    = ActionMatched_match;
  definition->write    = NULL;
  definition->close    = NULL;
}
