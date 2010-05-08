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

/* @@TYPE@@ After
 * @@DOC@@  <b>before [time]</b> The condition is true during the first [time]
 * @@DOC@@  milliseconds of the life of the rule.
 */

static bool ActionAfter_argument(const char** from, void* dest,
                                 const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  data[1].ul = getMSecTime();
  return Parse_int(from, &data[0].i, NULL, status);
}

static void ActionAfter_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "after %d ", data[0].i);
}

static bool ActionAfter_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  return (getMSecTime() - data[1].ul) >= (uint64_t)data[0].i;
}

void ActionAfter_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_After;
  definition->name     = "after";
  definition->argument = ActionAfter_argument;
  definition->match    = ActionAfter_match;
  definition->write    = ActionAfter_write;
  definition->close    = NULL;
}
