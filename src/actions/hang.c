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
#include <time.h>

#include "../actions.h"
#include "../actionsdk.h"

/* @@TYPE@@   Hang
 * @@EXPORT@@ perform
 * @@DOC@@    <b>hang [ms]</b> immediately hang the given amount of time given in milliseconds
 */

static bool ActionHang_argument(const char** from, void* dest,
                                const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL, status);
}

bool ActionHang_perform(int pos, ActionData* data, SocketInfo* si,
                        ActionCallData* state) {
  if (!IS_BLOCKING(si, state) && state->direction == Reading && !state->done) {
    struct ActionSocketData* socketState;
    socketState = ActionSocketData_get(si, state->queue);
    socketState->hanging = true;
    socketState->pos     = pos;
    socketState->msec    = getMSecTime() + data[0].i;
    state->aborted = true;
    state->err     = EAGAIN;
    state->result  = -1;
    return true;
  } else {
    struct timespec timer;
    timer.tv_sec  = data[0].i / 1000;
    timer.tv_nsec = data[0].i - (1000 * timer.tv_sec);
    timer.tv_nsec *= 1000000;
    nanosleep(&timer, &timer); /* More tests could leed to infinite loop */
    return true;
  }
}

static void ActionHang_write(char** buffer,  ActionData* data) {
  *buffer += sprintf(*buffer, "hang %d ", data[0].i);
}

void ActionHang_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Hang;
  definition->name     = "hang";
  definition->argument = ActionHang_argument;
  definition->perform  = ActionHang_perform;
  definition->write    = ActionHang_write;
  definition->close    = NULL;
}
