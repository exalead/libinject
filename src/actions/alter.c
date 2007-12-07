/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Alter command implementation
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include "../actionsdk.h"

/* @@TYPE@@ Alter
 * @@DOC@@  <b>alter [prop]</b> Alter the given proportion of bits. The proportion
 * @@DOC@@  being given on 1/1,000,000
 */

static bool ActionAlter_argument(const char** from, void* dest, const void* constraint) {
  union ActionData* data = (union ActionData*)dest;
  return Parse_int(from, &data[0].i, NULL);
}

static void ActionAlter_write(char** buffer, ActionData* data) {
  *buffer += sprintf(*buffer, "alter %d ", data[0].i);
}

static bool ActionAlter_perform(int pos, ActionData* data, SocketInfo* si,
                                ActionCallData* state) {
  int length;
  int off = 0;
  int i;
  if (!(Data & state->direction) || (state->direction == Writing && state->done)) {
    return Action_error("You can't alter the data before reading or after writing");
  }
  if (state->direction == Reading && !state->done) {
    ActionSyscall_perform(pos, NULL, si, state);
  }
  if (!ActionCallData_prepareBuffer(state)) {
    return Action_error("Can't allocate the edition buffer");
  }
  length = state->len;
  while (length > 0) {
    int batch = length > 1024 ? 1024 : length;
    uint32_t val = rand() % (data[0].i * 2);
    val = (val * batch * 8 / 1000000);
    for (i = 0 ; i < (int)val ; ++i) {
      int sub = rand() % (8 * batch);
      int oct = sub / 8;
      int bit = 1 << (sub - (8 * oct));
      uint8_t* buf = ((uint8_t*)state->buf) + off + oct;
      *buf = (*buf & (~bit)) | ((*buf & bit) ^ bit);
    }
    off += batch;
    length -= batch;
  }
  return true;
}

void ActionAlter_register(ActionTaskDefinition* definition) {
  definition->type     = ATT_Alter;
  definition->name     = "alter";
  definition->argument = ActionAlter_argument;
  definition->perform  = ActionAlter_perform;
  definition->write    = ActionAlter_write;
  definition->close    = NULL;
}
