/******************************************************************************/
/*                          libinject                                         */
/*                                                                            */
/*  Redistribution and use in source and binary forms, with or without        */
/*  modification, are permitted provided that the following conditions        */
/*  are met:                                                                  */
/*                                                                            */
/*  1. Redistributions of source code must retain the above copyright         */
/*     notice, this list of conditions and the following disclaimer.          */
/*  2. Redistributions in binary form must reproduce the above copyright      */
/*     notice, this list of conditions and the following disclaimer in the    */
/*     documentation and/or other materials provided with the distribution.   */
/*  3. The names of its contributors may not be used to endorse or promote    */
/*     products derived from this software without specific prior written     */
/*     permission.                                                            */
/*                                                                            */
/*  THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS   */
/*  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED         */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/*  DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY         */
/*  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS   */
/*  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)     */
/*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       */
/*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  */
/*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/*  POSSIBILITY OF SUCH DAMAGE.                                               */
/*                                                                            */
/*   Copyright (c) 2007-2010 Exalead S.A.                                     */
/******************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ MarkDone
 * @@DOC@@  <b>mark-done [type] [line]</b> Remove the given line of the rule set.
 * @@DOC@@  The type can be "call" or "socket".
 */

static bool ActionMarkDone_argument(const char** from, void* dest,
                                    const void* constraint, ParserStatus* status) {
  static Parse_enumData d[] = { { "call", 1 }, { "socket", 2 } };
  union ActionData* data = (union ActionData*)dest;
  const char* source = *from;
  if (Parse_enum(&source, &data[0].i, d, status) && Parse_space(&source, NULL, NULL, status)
      && Parse_int(&source, &data[1].i, NULL, status)) {
    *from = source;
    return true;
  }
  return false;
}

static void ActionMarkDone_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "mark-done %s %d ", data[0].i == 1 ? "call" : "socket",
                     data[1].i);
}

static bool ActionMarkDone_perform(int pos, ActionData* data, SocketInfo* si,
                                 ActionCallData* state) {
  const int line = data[1].i;
  if (data[0].i == 1) {
    (void)LigHT_put(state->calledLines, line, (void*)"OK", true, false);
  } else {
    struct ActionSocketData* socketState;
    socketState = ActionSocketData_get(si, state->queue);
    (void)LigHT_put(socketState->calledLines, line, (void*)"OK", true, true);
  }
  return true;
}

void ActionMarkDone_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_MarkDone;
  definition->name     = "mark-done";
  definition->argument = ActionMarkDone_argument;
  definition->perform  = ActionMarkDone_perform;
  definition->write    = ActionMarkDone_write;
  definition->close    = NULL;
}
