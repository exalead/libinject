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

#ifndef _CONFIGFILE_H_
#define _CONFIGFILE_H_

#include "actions.h"

/** @defgroup ConfigFile Configuration file
 *
 * Tool for parsing a configuration file. @{
 */

/** Store the result of the parsing of a configuration file.
 */
typedef struct Config Config;

/** Type of source of runtime update of the ruleset.
 */
enum RuntimeType {
  RT_None = 0,    /**< No update source. */
  RT_TCP  = 1,    /**< Update source is a TCP socket. */
  RT_UDP  = 2,    /**< Update source is an UDP socket. */
  RT_Pipe = 3,    /**< Update source is pipe. */
};

/** Store the information about the runtime update of the ruleset.
 */
struct Runtime {
  enum  RuntimeType type; /**< Type of the update source. */
  int   port;             /**< Source port for network feed. */
  char* file;             /**< Source file for pipe feed. */
};

/** Explicit storage structure for configuration file data.
 */
struct Config {
  ActionQueue*   queue;   /**< Message rule set. */
  struct Runtime runtime; /**< Runtime update source. */
  char*          command; /**< Path to the command to which the rules should be applied. */
};

/** Parse a configuration file...
 *
 * @param file The file to parse.
 * @return NULL if a parse error occured, or a well filled Config structure.
 */
Config* Config_init(const char* file);

/** Free a configuration.
 *
 * @param config The configuration.
 */
void Config_destroy(Config* config);

/** @} */

#endif
