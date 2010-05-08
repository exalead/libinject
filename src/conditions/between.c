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

/* @@TYPE@@ Between
 * @@DOC@@  <b>between [time1] [time2]</b> The condition is true between [time1]
 * @@DOC@@  and [time2], these times being number of milliseconds since the
 * @@DOC@@  creation of the rule.
 */

static bool ActionBetween_argument(const char** from, void* dest,
                                   const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  const char* source = *from;

  if (Parse_int(&source, &data[0].i, NULL, status)
      && Parse_space(&source, NULL, NULL, status)
      && Parse_int(&source, &data[1].i, NULL, status)) {
    data[2].ul = getMSecTime();
    *from = source;
    return true;
  }
  return false;
}

static void ActionBetween_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "between %d %d ", data[0].i, data[1].i);
}

static bool ActionBetween_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  uint64_t t = (getMSecTime() - data[1].ul);
  return t >= (uint64_t)data[0].i && t < (uint64_t)data[1].i;
}

void ActionBetween_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Between;
  definition->name     = "between";
  definition->argument = ActionBetween_argument;
  definition->match    = ActionBetween_match;
  definition->write    = ActionBetween_write;
  definition->close    = NULL;
}
