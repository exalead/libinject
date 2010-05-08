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

/* @@TYPE@@ Remove
 * @@DOC@@  <b>remove [line]</b> Remove the given line of the rule set. You
 * @@DOC@@  can't remove system lines
 */

static bool ActionRemove_argument(const char** from, void* dest,
                                  const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL, status);
}

static void ActionRemove_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "remove %d ", data[0].i);
}

static bool ActionRemove_perform(int pos, ActionData* data, SocketInfo* si,
                                 ActionCallData* state) {
  if (data[0].i < 10) {
    return Action_error("You have not the right to remove system lines");
  }
  ActionQueue_remove(state->queue, data[0].i, true);
  return true;
}

void ActionRemove_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Remove;
  definition->name     = "remove";
  definition->argument = ActionRemove_argument;
  definition->perform  = ActionRemove_perform;
  definition->write    = ActionRemove_write;
  definition->close    = NULL;
}
