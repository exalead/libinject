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

/* @@TYPE@@ Between
 * @@DOC@@  <b>between [time1] [time2]</b> The condition is true between [time1]
 * @@DOC@@  and [time2], these times being number of milliseconds since the
 * @@DOC@@  creation of the rule.
 */

static bool ActionBetween_argument(const char** from, void* dest,
                                   const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  const char* source = *from;

  if (Parse_int(&source, &data[0].i, NULL, status)
      && Parse_space(&source, NULL, NULL, status)
      && Parse_int(&source, &data[1].i, NULL, status)) {
    data[2].ul = getMSecTime();
    *from = source;
    return true;
  }
  return false;
}

static void ActionBetween_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "between %d %d ", data[0].i, data[1].i);
}

static bool ActionBetween_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  uint64_t t = (getMSecTime() - data[1].ul);
  return t >= (uint64_t)data[0].i && t < (uint64_t)data[1].i;
}

void ActionBetween_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Between;
  definition->name     = "between";
  definition->argument = ActionBetween_argument;
  definition->match    = ActionBetween_match;
  definition->write    = ActionBetween_write;
  definition->close    = NULL;
}
