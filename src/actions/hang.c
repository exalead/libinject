/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Hang command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <errno.h>
#include <time.h>

#include "../actions.h"
#include "../actionsdk.h"

/* @@TYPE@@   Hang
 * @@EXPORT@@ perform
 * @@DOC@@    <b>hang [ms]</b> immediately hang the given amount of time given in milliseconds
 */

static bool ActionHang_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL);
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
