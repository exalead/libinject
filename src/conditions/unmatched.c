/******************************************************************************
 *
 *                               The NG Project
 *
 *                     Unmatched condition implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Unmatched
 * @@DOC@@  <b>unmatched</b> the socket has not matched a rule yet
 */

static bool ActionUnmatched_match(ActionData* data, SocketInfo* si,
                                SocketInfoDirection direction, bool matched) {
  return !matched;
}

void ActionUnmatched_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Unmatched;
  definition->name     = "unmatched";
  definition->argument = NULL;
  definition->match    = ActionUnmatched_match;
  definition->write    = NULL;
  definition->close    = NULL;
}
