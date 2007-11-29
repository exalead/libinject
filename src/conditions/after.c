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

/* @@TYPE@@ After
 * @@DOC@@  <b>before [time]</b> The condition is true during the first [time]
 * @@DOC@@  milliseconds of the life of the rule.
 */

static bool ActionAfter_argument(const char** from, void* dest, const void* constraint) {
  ActionData* data = (ActionData*)dest;
  data[1].ul = getMSecTime();
  return Parse_int(from, &data[0].i, NULL);
}

static void ActionAfter_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "after %d ", data[0].i);
}

static bool ActionAfter_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  return (getMSecTime() - data[1].ul) >= (uint64_t)data[0].i;
}

void ActionAfter_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_After;
  definition->name     = "after";
  definition->argument = ActionAfter_argument;
  definition->match    = ActionAfter_match;
  definition->write    = ActionAfter_write;
  definition->close    = NULL;
}
