/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Emit command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

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
  { "SIGTTOU", SIGTTOU },
  { "SIGBUS",  SIGBUS },  { "SIGPOLL", SIGPOLL }, { "SIGPROF", SIGPROF },
  { "SIGSYS",  SIGSYS },  { "SIGTRAP", SIGTRAP }, { "SIGURG",  SIGURG },
  { "SIGVTALRM", SIGVTALRM }, { "SIGXCPU", SIGXCPU }, { "SIGXFSZ", SIGXFSZ },
  { "SIOT",    SIGIOT },  { "SIGSTKFLT", SIGSTKFLT }, { "SIGIO",   SIGIO },
  { "SIGCLD",  SIGCLD },  { "SIGPWR",  SIGPWR },  { "SIGWINCH", SIGWINCH },
  { "SIGUNUSED", SIGUNUSED } };

static bool ActionEmit_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_enum(from, &data[0].i, signals);
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
