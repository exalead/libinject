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

/* @@TYPE@@   Prob
 * @@EXPORT@@ match
 * @@DOC@@    <b>prob [percent]</b> probability of the condition to be truc
 * @@DOC@@    <code>prob 100</code> is the same as always and <code>prod 0</code>
 * @@DOC@@    is 'never'
 */

static bool ActionProb_argument(const char** from, void* dest,
                                const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  const char* source = *from;
  if (Parse_int(&source, &data[0].i, NULL, status)
      && data[0].i >= 0 && data[0].i <= 100) {
    *from = source;
    return true;
  }
  return SET_PARSE_ERROR(*from, "Probability must be between 0 and 100");
}

static void ActionProb_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "prob %d ", data[0].i);
}

bool ActionProb_match(ActionData* data, SocketInfo* si,
                      SocketInfoDirection direction, bool matched) {
  unsigned int val = rand();
  return (val % 100) <= (unsigned int)data[0].i;
}

void ActionProb_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Prob;
  definition->name     = "prob";
  definition->argument = ActionProb_argument;
  definition->match    = ActionProb_match;
  definition->write    = ActionProb_write;
  definition->close    = NULL;
}
