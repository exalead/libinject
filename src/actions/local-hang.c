/******************************************************************************
 *
 *                               The NG Project
 *
 *                     Local-hang command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ LocalHang
 * @@DOC@@  <b>local-hang [ms]</b> hang in order to simulate a local hang. This
 * @@DOC@@  is similar to hang if writing, but it will ensure the socket has
 * @@DOC@@  been read in case of reading.
 */

static bool ActionLocalHang_argument(const char** from, void* dest,
                                     const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL, status);
}

static void ActionLocalHang_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "local-hang %d ", data[0].i);
}

static bool ActionLocalHang_perform(int pos, ActionData* data,
                                    SocketInfo* si, ActionCallData* state) {
  if (state->direction == Reading && !state->done && !state->aborted) {
    if (!ActionSyscall_perform(pos, NULL, si, state)) {
      return false;
    }
  }
  return ActionHang_perform(pos, data, si, state);
}

void ActionLocalHang_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_LocalHang;
  definition->name     = "local-hang";
  definition->argument = ActionLocalHang_argument;
  definition->perform  = ActionLocalHang_perform;
  definition->write    = ActionLocalHang_write;
  definition->close    = NULL;
}
