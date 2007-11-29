/******************************************************************************
 *
 *                               The NG Project
 *
 *                    Read and parse a configuration file
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

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
