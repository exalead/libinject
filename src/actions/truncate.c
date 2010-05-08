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

/* @@TYPE@@   Truncate
 * @@EXPORT@@ perform
 * @@DOC@@    <b>truncate [size]</b> send/receive at most the given size
 */

static bool ActionTruncate_argument(const char** from, void* dest,
                                    const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL, status);
}

bool ActionTruncate_perform(int pos, ActionData* data, SocketInfo* si,
                            ActionCallData* state) {
  if (state->done || state->aborted) {
    return Action_error("Trying to truncate buffer while syscall already marked as done");
  }
  if (!(Data & state->direction)) {
    return false;
  }
  if (state->origLen > (size_t)data[0].i) {
    state->origLen = data[0].i;
  }
  if (state->len > (size_t)data[0].i) {
    state->len = data[0].i;
  }
  return true;
}

static void ActionTruncate_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "trucate %d ", data[0].i);
}

void ActionTruncate_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Truncate;
  definition->name     = "truncate";
  definition->argument = ActionTruncate_argument;
  definition->perform  = ActionTruncate_perform;
  definition->write    = ActionTruncate_write;
  definition->close    = NULL;
}
