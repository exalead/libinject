/******************************************************************************
 *
 *                               The NG Project
 *
 *                         Runtime update of the queue
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include "conffile.h"

/** @defgroup Runtime Runtime update of the queue
 *
 * This functions run a thread that wait for input from the user in order to
 * update the ruleset. Their are currently two possible kinds of input:
 *    - A network socket. In that cause, the socket is whitelisted in the queue
 *    in order to avoid alteration of the input.
 *    - A pipe.
 *
 * The pipe can be useful for scripting whereas the socket way brings more
 * interactivity allowing the user to get real time status. @{
 */

/** Start waiting for user commands.
 */
bool Runtime_start(Config* config);

/** @} */

#endif
