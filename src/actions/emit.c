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
#include <signal.h>

#include "../actionsdk.h"

/* @@TYPE@@ Emit
 * @@DOC@@   <b>emit [signame]</b> Emit the given signal and set the corresponding error.
 */

static Parse_enumData signals[] = {
  { "SIGHUP",  SIGHUP },  { "SIGINT",  SIGINT },  { "SIGQUIT", SIGQUIT },
  { "SIGILL",  SIGILL },  { "SIGABRT", SIGABRT }, { "SIGFPE",  SIGFPE },
  { "SIGKILL", SIGKILL }, { "SIGSEGV", SIGSEGV }, { "SIGPIPE", SIGPIPE },
  { "SIGALRM", SIGALRM }, { "SIGTERM", SIGTERM }, { "SIGUSR1", SIGUSR1 },
  { "SIGUSR2", SIGUSR2 }, { "SIGCHLD", SIGCHLD }, { "SIGCONT", SIGCONT },
  { "SIGSTOP", SIGSTOP }, { "SIGTSTP", SIGTSTP }, { "SIGTTIN", SIGTTIN },
  { "SIGTTOU", SIGTTOU }
#ifdef SIGBUS
  , { "SIGBUS", SIGBUS }
#endif
#ifdef SIGPOLL
    , { "SIGPOLL", SIGPOLL }
#endif
#ifdef SIGPROF
    , { "SIGPROF", SIGPROF }
#endif
#ifdef SIGSYS
    , { "SIGSYS", SIGSYS }
#endif
#ifdef SIGTRAP
    , { "SIGTRAP", SIGTRAP }
#endif
#ifdef SIGURG
    , { "SIGURG", SIGURG }
#endif
#ifdef SIGVTALRM
    , { "SIGVTALRM", SIGVTALRM }
#endif
#ifdef SIGXCPU
    , { "SIGXCPU", SIGXCPU }
#endif
#ifdef SIGXFSZ
    , { "SIGXFSZ", SIGXFSZ }
#endif
#ifdef SIGIOT
    , { "SIGIOT", SIGIOT }
#endif
#ifdef SIGSTKFLT
    , { "SIGSTKFLT", SIGSTKFLT }
#endif
#ifdef SIGIO
    , { "SIGIO", SIGIO }
#endif
#ifdef SIGCLD
    , { "SIGCLD", SIGCLD }
#endif
#ifdef SIGPWR
    , { "SIGPWR", SIGPWR }
#endif
#ifdef SIGWINCH
    , { "SIGWINCH", SIGWINCH }
#endif
#ifdef SIGUNUSED
    , { "SIGUNUSED", SIGUNUSED }
#endif
};

static bool ActionEmit_argument(const char** from, void* dest,
                                const void* constraint, ParserStatus* status) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_enum(from, &data[0].i, signals, status);
}

static void ActionEmit_write(char** buffer, ActionData* data) {
  Parse_enumData* src = signals;
  while (src->constant) {
    if (data[0].i == src->value) {
      *buffer += sprintf(*buffer, "emit %s ", src->constant);
      return;
    }
  }
  *buffer += sprintf(*buffer, "emit ???? ");
}

static bool ActionEmit_perform(int pos, ActionData* data, SocketInfo* si,
                               ActionCallData* state) {
  if (state->done) {
    return Action_error("Emitting a signal while syscall alread performed");
  }
  state->result  = -1;
  state->aborted = true;
  state->err     = data[0].i == SIGPIPE ? EPIPE : EINTR;
  return true;
}

void ActionEmit_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Emit;
  definition->name     = "emit";
  definition->argument = ActionEmit_argument;
  definition->perform  = ActionEmit_perform;
  definition->write    = ActionEmit_write;
  definition->close    = NULL;
}
