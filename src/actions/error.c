/******************************************************************************
 *
 *                               The NG Project
 *
 *                        Error command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <errno.h>

#include "../actionsdk.h"

/* @@TYPE@@ Error
 * @@DOC@@  <b>error [errname]</b> Make the process return the given error.
 * @@DOC@@  Warning, this has the same level than syscall and mark the call as
 * @@DOC@@  being aborted. The available errors are EAGAIN, EBADF, EFAULT,
 * @@DOC@@  EINVAL, EIO and should be used properly in their context.
 */

static Parse_enumData errors[] = { { "EAGAIN", EAGAIN }, { "EBADF", EBADF },
  { "EFAULT", EFAULT }, { "EINVAL", EINVAL }, { "EIO", EIO }, { "EBADF", EBADF },
  { "EPERM", EPERM }, { "EACCES", EACCES }, { "EADDRINUSE", EADDRINUSE },
  { "EAFNOSUPPORT", EAFNOSUPPORT }, { "EALREADY", EALREADY }, { "EPERM",  EPERM },
  { "EBADF", EBADF }, { "ECONNREFUSED", ECONNREFUSED }, { "EFAULT" , EFAULT },
  { "EINPROGRESS", EINPROGRESS }, { "EISCONN", EISCONN }, { "ENETUNREACH", ENETUNREACH },
  { "ENOTSOCK", ENOTSOCK }, { "ETIMEDOUT", ETIMEDOUT }, { NULL, 0 } };

static bool ActionError_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_enum(from, &data[0].i, errors);
}

static void ActionError_write(char** buffer, ActionData* data) {
  Parse_enumData* src = errors;
  while (src->constant) {
    if (data[0].i == src->value) {
      *buffer += sprintf(*buffer, "error %s ", src->constant);
      return;
    }
  }
  *buffer += sprintf(*buffer, "error ???? ");
}

static bool ActionError_perform(int pos, ActionData* data, SocketInfo* si,
                                ActionCallData* state) {
  if (state->done) {
    return Action_error("Setting error while syscall already performed");
  }
  if (IS_BLOCKING(si, state) && data[0].i == EAGAIN) {
    return Action_error("EAGAIN is not valid for non blocking accesses");
  }
  state->err     = data[0].i;
  state->result  = -1;
  state->aborted = true;
  return true;
}

void ActionError_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Error;
  definition->name     = "error";
  definition->argument = ActionError_argument;
  definition->perform  = ActionError_perform;
  definition->write    = ActionError_write;
  definition->close    = NULL;
}
