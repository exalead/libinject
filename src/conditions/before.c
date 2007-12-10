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

/* @@TYPE@@ Before
 * @@DOC@@  <b>before [time]</b> The condition is true during the first [time]
 * @@DOC@@  milliseconds of the life of the rule.
 */

static bool ActionBefore_argument(const char** from, void* dest,
                                  const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  data[1].ul = getMSecTime();
  return Parse_int(from, &data[0].i, NULL, status);
}

static void ActionBefore_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "before %d ", data[0].i);
}

static bool ActionBefore_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  return (getMSecTime() - data[1].ul) < (uint64_t)data[0].i;
}

void ActionBefore_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Before;
  definition->name     = "before";
  definition->argument = ActionBefore_argument;
  definition->match    = ActionBefore_match;
  definition->write    = ActionBefore_write;
  definition->close    = NULL;
}
