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
