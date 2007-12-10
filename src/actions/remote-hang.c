/******************************************************************************
 *
 *                               The NG Project
 *
 *                     Remote-hang command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ RemoteHang
 * @@DOC@@  <b>remote-hang [ms]</b> hang in order to simulate a hang in the remote system
 */

static bool ActionRemoteHang_argument(const char** from, void* dest,
                                      const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL, status);
}

static void ActionRemoteHang_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "remote-hang %d ", data[0].i);
}

void ActionRemoteHang_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_RemoteHang;
  definition->name     = "remote-hang";
  definition->argument = ActionRemoteHang_argument;
  definition->perform  = ActionHang_perform;
  definition->write    = ActionRemoteHang_write;
  definition->close    = NULL;
}
