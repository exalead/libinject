/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Echo command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Echo
 * @@DOC@@  <b>echo [word]</b> display the given word
 */

static bool ActionEcho_argument(const char** from, void* dest,
                                const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  if (!Parse_word(from, &data[0].str, NULL, status)) {
    if (data[0].str) {
      free(data[0].str);
    }
    data[0].str = NULL;
    return false;
  }
  return true;
}

static bool ActionEcho_perform(int pos, ActionData* data, SocketInfo* si,
                               ActionCallData* state) {
  fprintf(stderr, "[line %d] [socket %d] %s\n", pos, si->fd, data[0].str);
  return true;
}

static void ActionEcho_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "echo %s ", data[0].str);
}

static void ActionEcho_close(ActionData* data) {
  free(data[0].str);
}

void ActionEcho_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Echo;
  definition->name     = "echo";
  definition->argument = ActionEcho_argument;
  definition->perform  = ActionEcho_perform;
  definition->write    = ActionEcho_write;
  definition->close    = ActionEcho_close;
}
