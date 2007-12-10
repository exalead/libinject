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

/* @@TYPE@@   Prob
 * @@EXPORT@@ match
 * @@DOC@@    <b>prob [percent]</b> probability of the condition to be truc
 * @@DOC@@    <code>prob 100</code> is the same as always and <code>prod 0</code>
 * @@DOC@@    is 'never'
 */

static bool ActionProb_argument(const char** from, void* dest,
                                const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  const char* source = *from;
  if (Parse_int(&source, &data[0].i, NULL, status)
      && data[0].i >= 0 && data[0].i <= 100) {
    *from = source;
    return true;
  }
  return SET_PARSE_ERROR(*from, "Probability must be between 0 and 100");
}

static void ActionProb_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "prob %d ", data[0].i);
}

bool ActionProb_match(ActionData* data, SocketInfo* si,
                      SocketInfoDirection direction, bool matched) {
  unsigned int val = rand();
  return (val % 100) <= (unsigned int)data[0].i;
}

void ActionProb_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Prob;
  definition->name     = "prob";
  definition->argument = ActionProb_argument;
  definition->match    = ActionProb_match;
  definition->write    = ActionProb_write;
  definition->close    = NULL;
}
