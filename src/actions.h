/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Action queue and processor
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include <sys/types.h>
#include <stdbool.h>

#include "socketinfo.h"


/** @defgroup Action Action rule
 *
 * An action is set of match directives that tell when to apply the action, and
 * a set of directives that describe the action.
 *
 * The action also contains some informations about the queue it belongs to and
 * its position in that queue. @{
 */

/** Global initialisation of the actions.
 */
void ActionSet_init(void);

/** An action.
 */
typedef struct Action Action;

/** Build a new action from an action description.
 *
 * An @p instruction has te following structure :<br>
 * <code><i>line</i> on <i>proto</i> [[from <i>source</i>|connect|close]
 * to <i>dest</i>|with <i>host</i>|talk-with <i>host</i>] [when <i>cond</i>]
 * [<i>domode</i> <i>action</i>] <i>next</i></code>
 *
 * Arguments :
 * - <b>line</b> Line is the id of the instruction. The set of instruction
 *     is processed in increasing id order. Lines 0 to 9 are system rules and must
 *     not be defined by user even if there is no test to check user's behaviour.
 * - <b>proto</b> Protocole of the socket. This can be <code>tcp, udp or ip</code>.
 *     ip matches both tcp and udp
 * - <b>host</b>, <b>dest</b> or <b>source</b> Address to match. An adress as the
 *     following structure:
 *      <code>host (port port)</code>
 *      - <b>host</b> should be an ip address, a DNS name or one of the following
 *          special term: <code>me, any, dns</code>.
 *      - <b>port</b> if port is not specified, all the trafic from/to the given
 *          host match the rule. If host is dns, the port is ignored
 *          and is always 53. The port must be a valid number or a service
 *          name (listed in /etc/services)
 * - <b>cond</b> A matching condition. This parameters is optional. @ref ActionConditionType
 * - <b>domode</b> The domode explains what to do with the action when it has
 *      been processed. There are currently 4 modes:
 *      - <b>do</b> do nothing more
 *      - <b>do-once</b> remove the action from the queue once it has been
 *          processed.
 *      - <b>do-once-per-call</b> process this action only once during an
 *          action processing call (so it will be once per read/write)
 *      - <b>do-once-per-socket</b> process this action only once during the
 *          lifetime of a socket.
 * - <b>action</b> The action to execute (a list of valid version will be
 *     published later). @ref ActionTaskType
 * - <b>next</b> The next action to execute. This can be one of the following:
 *     - <b>continue</b> continue processing of actions
 *     - <b>goto <i>line</i></b> continue execution starting on the given line
 *     - <b>next</b> process the socket against the next rule even if it does not
 *         match the filter.
 *     - <b>exec <i>line</i></b> process the socket against the rule at line
 *         <i>line</i> even if it does not match the filter.
 *     - <b>stop</b> stop processing
 *
 * @param instruction An instruction detailing the rule.
 * @return A new Action, or NULL if an error occured.
 */
Action* Action_init(const char* instruction);

/** Destroy an action.
 *
 * @param action The action.
 */
void Action_destroy(Action* action);

/** Test if the given socket matches the action.
 *
 * @param action The action.
 * @param si     The socket to test.
 * @param direction Data direction (Reading|Writing).
 * @param matched True if the socket already matched rule.
 * @return true if the socket matches the rule.
 */
bool Action_match(Action* action, SocketInfo* si, SocketInfoDirection direction,
                  bool matched);

/** Call back for apply user requested action.
 */
typedef ssize_t (Action_syscall)(int fd, void* buf, size_t len, int flags, void* data);

/** Call state structure.
 */
typedef struct ActionCallData ActionCallData;

/** Apply the action on the given socket.
 *
 * This function process the current action using the given arguments (cool
 * description, isn't it ?). The function can modify the content of the buffer
 * and the len. It also can realloc the buffer if it need a larger one (The buffer
 * isn't the one given by the user but a copy for processing purpose).
 *
 * @param action The action.
 * @param si     The socket.
 * @param state  Call state.
 * @return the size of data after the action.
 */
bool Action_process(Action* action, SocketInfo* si, ActionCallData* state);

/** Write the action to the given file.
 *
 * @param action The action.
 * @param fd The destination.
 */
void Action_show(Action* action, int fd);

/** @} */


/** @defgroup ActionQueue Action Queue
 *
 * This is just a set of ordered actions that allow you to find actions. The
 * action queue offers a set of function that help accessing the @ref Action
 * rules either by enumerating them or by finding the rules matching a given
 * condition. @{
 */

/** An action queue.
 */
typedef struct ActionQueue ActionQueue;

/** Create a new Action Queue of the given size.
 *
 * @param capacity The size of the queue.
 * @return NULL on error, or a new well initialized ActionQueue structure.
 */
ActionQueue* ActionQueue_init(size_t capacity);

/** Free an ActionQueue and all the actions it contains.
 *
 * @param queue The queue.
 */
void ActionQueue_destroy(ActionQueue* queue);

/** Add a new element in a queue.
 *
 * @param queue   The queue.
 * @param instruction The action to add. It must be well initialized
 *                and contains an offset.
 * @param replace If false, the the function will fail if the
 *                offset of the action is already filled by another
 *                action. If true, the slot requested by the action
 *                if first freed.
 * @param mt      If true, the function lock the accesses to the queue.
 * @return false if the function failed, true if it succeed.
 */
bool ActionQueue_put(ActionQueue* queue, const char* instruction, bool replace, bool mt);

/** Get the posth element of the queue.
 *
 * @param queue The queue
 * @param pos   The requested offset.
 * @param mt    If true, the function lock the accesses to the queue.
 * @return The action at the given offset, of NULL.
 */
Action* ActionQueue_get(ActionQueue* queue, off_t pos, bool mt);

/** Get the first element from the given offset.
 *
 * @param queue The queue.
 * @param pos   Position from which the search much start
 * @param mt    If true, the function lock the accesses to the queue.
 * @return The first action found, or NULL.
 */
Action* ActionQueue_getFrom(ActionQueue* queue, off_t pos, bool mt);

/** Get the first element of the queue.
 *
 * @param queue The queue.
 * @param mt    If true, the function lock the accesses to the queue.
 * @return The first action found, or NULL.
 */
#define ActionQueue_getFirst(queue, mt) ActionQueue_getFrom(queue, 0, mt)

/** Get the first element that matches the given socket from the given offset.
 *
 * @param queue The queue.
 * @param si    The socket to match against.
 * @param direction Data direction (Reading|Writing)
 * @param matched True if the socket already matched a rule.
 * @param pos   Position from which the search much start
 * @param mt    If true, the function lock the accesses to the queue.
 * @return The first action found, or NULL.
 */
Action* ActionQueue_getMatch(ActionQueue* queue, SocketInfo* si, SocketInfoDirection direction,
                             bool matched, off_t pos, bool mt);

/** Get the first element that matches the given socket.
 *
 * @param queue The queue.
 * @param si    The socket to match against.
 * @param direction Data direction (Reading|Writing)
 * @param mt    If true, the function lock the accesses to the queue.
 * @return The first action found, or NULL.
 */
#define ActionQueue_getFirstMatch(queue, si, direction, mt) \
  ActionQueue_getMatch(queue, si, direction, false, 0, mt)

/** Process a socket against an action queue.
 *
 * @param queue The queue
 * @param si    The socket.
 * @param direction Data direction (Reading|Writing)
 * @param callback Callback to execute the system call.
 * @param buf   Data buffer.
 * @param len   Buffer length.
 * @param flags system call flags.
 * @param data  User data to be passed to the system call callback.
 * @return Size read/written
 */
ssize_t ActionQueue_process(ActionQueue* queue, SocketInfo* si,
                            SocketInfoDirection direction, Action_syscall callback,
                            void* buf, size_t len, int flags, void* data);

/** Remove an action.
 *
 * @param queue The queue
 * @param pos   The offset of the action to be removed.
 * @param mt    If true, the function lock the accesses to the queue.
 */
void ActionQueue_remove(ActionQueue* queue, off_t pos, bool mt);

/** List all actions in a queue.
 *
 * @param queue The queue...
 * @param fd The file descriptor on which the data should be written
 */
void ActionQueue_list(ActionQueue* queue, int fd);

/** @} */


#endif
