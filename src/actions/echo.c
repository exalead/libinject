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
