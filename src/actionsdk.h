/******************************************************************************
 *
 *                               The NG Project
 *
 *                           Action Development Kit
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#ifndef _ACTIONSDK_H_
#define _ACTIONSDK_H_

#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdio.h> /* Not explicitely needed, most writers need this. */

#include "parser.h"
#include "socketinfo.h"
#include "ligHT.h"
#include "actions.h"

/** @defgroup ActionDK Action Development Kit
 *
 * This contains some useful stuff to build and register new actions. @{
 */

/** Runtime data associated with a socket.
 */
typedef struct ActionSocketData ActionSocketData;

/** Callbacks associated with a given condition type.
 */
typedef struct ActionConditionDefinition ActionConditionDefinition;

/** List of available condition types.
 */
typedef enum ActionConditionType ActionConditionType;

/** Callbacks associated with a given task type.
 */
typedef struct ActionTaskDefinition ActionTaskDefinition;

/** List of available task types.
 */
typedef enum ActionTaskType ActionTaskType;

/** Data storage structure.
 */
typedef union ActionData ActionData;

/** Callback to parse the argument of an action.
 *
 * @param pos   The number of the current action.
 * @param data  Data associated with the action.
 * @param si    Socket informations.
 * @param state Call state.
 * @return true if the call succeeded.
 */
typedef bool (ActionPerformer)(int pos, ActionData* data,
                               SocketInfo* si, ActionCallData* state);

/** Callback to check if the rule should match.
 *
 * @param data      Data associated with the given condition.
 * @param si        Socket informations.
 * @param direction Data transit direction.
 * @param matched   true if a rule has already been matched.
 * @return true if the condition match given parameters.
 */
typedef bool (ActionMatcher)(ActionData* data, SocketInfo* si,
                             SocketInfoDirection direction, bool matched);

/** Callback to print the action to the given buffer.
 *
 * @param buffer  Destination buffer. The pointer must point after the last
 *                written character after the call.
 * @param data    Data associated with the given element.
 */
typedef void (ActionWriter)(char** buffer, ActionData* data);

/** Register an action.
 *
 * @param definition The registration structure to fill.
 */
typedef void (ActionTaskRegisterer)(ActionTaskDefinition* definition);

/** Register a condition.
 *
 * @param definition The registration structure to fill.
 */
typedef void (ActionConditionRegisterer)(ActionConditionDefinition* definition);

/** Close the data.
 *
 * @param data Data associated with the element.
 */
typedef void (ActionCloser)(ActionData* data);

#include "actionlist.h"
#include "conditionlist.h"


/** Action task data wrapper.
 */
union ActionData {
  int       i;  /**< <b>i</b>nteger version. */
  uint64_t  ul; /**< <b>u</b>nsigned <b>l</b>long version. */
  void*     p;  /**< <b>p</b>ointer version. */
  char*     str;/**< <b>str</b>ing pointer version. */
};

/** Condition with arguments.
 */
struct ActionCondition {
  enum ActionConditionType type;  /**< Condition type. */
  union ActionData data[16];      /**< Condition params. */
};

/** Define the callbacks needed to parse and process a condition.
 */
struct ActionConditionDefinition {
  enum ActionConditionType type;  /**< The identifier of the type. */
  const char*      name;          /**< The string of the condition. */

  ParserElement*   argument;  /**< Argument parser. */
  ActionMatcher*   match;     /**< Match the situation. */
  ActionWriter*    write;     /**< Write the condition to the given buffer. */
  ActionCloser*    close;     /**< Close the condition and remove all associated date. */
};

/** Action task.
 */
struct ActionTask {
  enum ActionTaskType  type; /**< Type of the task to execute. */
  union ActionData data[16]; /**< Data of the task. */
};

/** Define the callbacks needed to parse and process an action.
 */
struct ActionTaskDefinition {
  enum ActionTaskType type;   /**< The identifier of the type. */
  const char*         name;   /**< The string of the task. */

  ParserElement*   argument;  /**< Argument parser. */
  ActionPerformer* perform;   /**< Perform the corresponding action. */
  ActionWriter*    write;     /**< Write the action to the given buffer. */
  ActionCloser*    close;     /**< Close the action and remove all associated date. */
};

/** CallData
 */
struct ActionCallData {
  ActionQueue* queue;      /**< The processed queue. */

  /* Sys call data */
  Action_syscall* callback;/**< The callback. */
  void* origBuf;           /**< Original user buffer. */
  size_t origLen;          /**< User buffer length. */
  void* buf;               /**< Buffer for the callback. */
  size_t len;              /**< Length of the buffer. */
  int flags;               /**< Call flags. */
  void* data;              /**< Data for the syscall. */
  SocketInfoDirection direction; /**< Data direction (Reading|Writing) */

  /* Processing status */
  bool aborted;            /**< If true, indicates that the syscall has been aborted. */
  bool done;               /**< If true, indicates that hte syscall has been done. */
  LigHT* calledLines;      /**< LigHT which mark all line executed in the current call. */

  /* Result */
  bool keepError;          /**< Don't remember what this stand for o_O */
  int  err;                /**< Expected errno after processing. */
  ssize_t result;          /**< Result of the syscall (or any further manipulation :p). */
};

/** Data to be stored as context associated with a socket.
 */
struct ActionSocketData {
  LigHT* calledLines; /**< List line called. */
  bool   hanging;     /**< If true indicates that the socket is currently hanging asynchronously.
                           This only affect read operations. */
  uint64_t msec;      /**< Start time of the timer. */
  int      pos;       /**< Line of the instruction requesting the hang. */
};

/** Check if a call is blocking.
 *
 * @param si Informations about the socket.
 * @param state Informations about the call.
 * @return true if the call is blocking.
 */
#define IS_BLOCKING(si, state) ((si)->blocking && !((state)->flags & O_NONBLOCK))

/** Return the buffer to use for reading.
 *
 * @param state Informations about the call.
 * @return the current valid buffer.
 */
#define READ_BUFFER(state) ((state)->buf ? (state)->buf : (state)->origBuf)

/** Return the length of the buffer to use for reading.
 *
 * @param state Informations about the call.
 * @return the current valid buffer length.
 */
#define READ_BUFFER_LENGTH(state) ((state)->buf ? (state)->len : (state)->origLen)

/** Build the list of arguments for printf to show an ipv4 address.
 *
 * @param addr the address
 */
#define SHOW_ADDR(addr) ((addr) >> 24) & 0xff, ((addr) >> 16) & 0xff, \
                        ((addr) >> 8)  & 0xff, (addr) & 0xff

/** Print an error message.
 *
 * @param message The error message.
 * @return false
 */
bool Action_error(const char* message);

/** Fetch socket infos from.
 */
ActionSocketData* ActionSocketData_get(SocketInfo*si, ActionQueue* queue);

/** Prepare the call data for data edition.
 */
bool ActionCallData_prepareBuffer(ActionCallData* data);

/** Get current time with millisecond precision.
 *
 * @return Current time in millisecond since EPOCH
 */
static inline uint64_t getMSecTime(void) {
  uint64_t start;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start  = tv.tv_sec * 1000;
  start += (tv.tv_usec / 1000);
  return start;
}

/** @} */

#endif
