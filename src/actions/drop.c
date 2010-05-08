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

/* @@TYPE@@ Drop
 * @@DOC@@  <b>drop</b> Drop the given packet: cancel the syscall and mark the
 * @@DOC@@  state as correct.
 */

static bool ActionDrop_perform(int pos, ActionData* data, SocketInfo* si,
                               ActionCallData* state) {
  if (state->done) {
    return Action_error("Aborting syscall while call already done");
  }
  state->aborted = true;
  state->result  = READ_BUFFER_LENGTH(state);
  return true;
}

void ActionDrop_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Drop;
  definition->name     = "drop";
  definition->argument = NULL;
  definition->perform  = ActionDrop_perform;
  definition->write    = NULL;
  definition->close    = NULL;
}
