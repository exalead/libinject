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

/* @@TYPE@@ Cycle
 * @@DOC@@  <b>cycle [first] [firsttime] [sndtime]</b> This condition is
 * @@DOC@@  alternatively true or false depending on the time elapsed since
 * @@DOC@@  the creation of the rule. [firsttime] is the during which the
 * @@DOC@@  condition will have the value of [first], [sndtime] is the duration
 * @@DOC@@  of the contrary. eg <code>cycle true 10000 5000</code> will be
 * @@DOC@@  true during 10 seconds, then false during 5 seconds, and then
 * @@DOC@@  true for 10 seconds again...
 */

static bool ActionCycle_argument(const char** from, void* dest,
                                 const void* constraint, ParserStatus* status) {
  ActionData* data = (ActionData*)dest;
  bool ret;
  const char* source = *from;
  Parser* parser;

  parser = Parser_init();
  Parser_addSpacedBool(parser, &data[0].i);
  Parser_addSpacedInt(parser, &data[1].i);
  Parser_addInt(parser, &data[2].i);
  ret = Parse_suite(&source, NULL, parser, status);
  Parser_destroy(parser);
  if (!ret) {
    return false;
  }
  data[3].ul = getMSecTime();
  *from = source;
  return true;
}

static void ActionCycle_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "cycle %s %d %d ", data[0].i ? "true" : "false",
                                                 data[1].i, data[2].i);
}

static bool ActionCycle_match(ActionData* data, SocketInfo* si,
                              SocketInfoDirection direction, bool matched) {
  uint32_t t;
  t = (uint32_t)(uint64_t)(getMSecTime() - data[3].ul);
  t %= data[1].i + data[2].i;
  return (t < (uint32_t)data[1].i) ? !!data[0].i : !data[0].i;
}

void ActionCycle_register(ActionConditionDefinition* definition) {
  definition->type     = ACT_Cycle;
  definition->name     = "cycle";
  definition->argument = ActionCycle_argument;
  definition->match    = ActionCycle_match;
  definition->write    = ActionCycle_write;
  definition->close    = NULL;
}
