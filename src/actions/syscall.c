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

#include <errno.h>

#include "../actionsdk.h"

/* @@TYPE@@   Syscall
 * @@EXPORT@@ perform
 * @@DOC@@    <b>syscall</b> do the system call
 */

bool ActionSyscall_perform(int pos, ActionData* data, SocketInfo* si,
                           ActionCallData* state) {
  if (state->aborted) {
    return Action_error("Performing syscall while call aborted");
  }
  state->result = state->callback(si->fd, READ_BUFFER(state), READ_BUFFER_LENGTH(state),
                                          state->flags, state->data);
  state->err = errno;
  if (state->result >= 0) {
    if (state->buf) {
      state->len = state->result;
    }
    state->origLen = state->result;
  } else {
    if (state->buf) {
      state->len = 0;
    }
    state->origLen = 0;
  }
  state->aborted = false;
  state->done    = true;
  return true; /* The processing don't fail if the call fail... it's just
                  the expected behaviour, so, do not interfer! */
}

void ActionSyscall_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Syscall;
  definition->name     = "syscall";
  definition->argument = NULL;
  definition->perform  = ActionSyscall_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
