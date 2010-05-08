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

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "conffile.h"

#if defined(__linux)
extern const char* __progname;
static inline const char* getprogname(void) {
  return __progname;
}
#elif defined(__sun)
static inline const char* getprogname(void) {
  static const char* progname = NULL;
  if (progname == NULL) {
    const char* pos;
    progname = getexecname();
    pos = strrchr(progname, '/');
    if (pos != NULL) {
      progname = pos + 1;
    }
  }
  return progname;
}
#endif

/** Section of the configuration file.
 */
enum Section {
  S_Unknown, /**< Unknown section. */
  S_Rules,   /**< Rules section. */
  S_Runtime  /**< Runtime update section. */
};

/** Parse comments.
 */
static bool Config_parse_comment(const char** from, void* dest,
                                 const void* constraint, ParserStatus* status) {
  static Parser* parser = NULL;

  if (parser == NULL) {
    Parser* subparser;
    parser = Parser_init();
    Parser_addSpace(parser);
    subparser = Parser_newSubParser(parser, PB_optional);
    Parser_addConstant(subparser, ';');
    Parser_add(subparser, Parse_line, NULL, NULL, NULL);
  }
  return Parse_optional(from, dest, parser, status);
}

Config* Config_init(const char* file) {
  Config* config;
  FILE*  f;
  char*  line;
  enum Section section = S_Rules;
  Parser* parseSection = NULL;
  Parser* parseRuntime = NULL;
  ParserStatus* status = NULL;
  bool ok = true;

  if (!file) {
    return NULL;
  }
  f = fopen(file, "r");
  if (f == NULL) {
    return NULL;
  }
  line = (char*)malloc(1024);
  if (line == NULL) {
    return NULL;
  }
  line[1023] = '\0';

  config = (Config*)malloc(sizeof(Config));
  config->runtime.type = RT_None;
  config->runtime.file = NULL;
  config->command      = NULL;
  config->queue = ActionQueue_init(2000);
  status = ParserStatus_init();

  while (!feof(f)) {
    size_t len;
    (void)CLEAR_PARSE_ERROR;
    if (fgets(line, 1023, f) == NULL && !feof(f)) {
      fprintf(stderr, "Read error\n");
      ok = false;
      break;
    }
    len = strlen(line);
    while (len > 0 && isspace(line[len - 1])) {
      line[--len] = '\0';
    }

    /* Skip full line comments */
    if (len == 0 || *line == ';') {
      continue;
    }

    /* Changing current section. */
    if (*line == '[') {
      static Parse_enumData d[] = { { "Rules", S_Rules }, { "Runtime", S_Runtime }, { NULL, S_Unknown } };
      if (parseSection == NULL) {
        parseSection = Parser_init();
        Parser_addChar(parseSection, "[");
        Parser_add(parseSection, Parse_enum, &section, d, NULL);
        Parser_addChar(parseSection, "]");
        Parser_add(parseSection, Config_parse_comment, NULL, NULL, NULL);
        Parser_checkEOB(parseSection);
      }
      if (!Parser_run(parseSection, line, PB_suite, false, status)) {
        fprintf(stderr, "Invalid section: \"%s\"\n", line);
        ok = false;
        break;
      }
    } else if (section == S_Rules) {
      if (!ActionQueue_put(config->queue, line, status, true, false)) {
        char* err;
        err = ParserStatus_error(status, line);
        fprintf(stderr, "%s", err);
        free(err);
        ok = false;
        break;
      }
    } else if (section == S_Runtime) {
      static Parse_enumData d[] = { { "tcp", RT_TCP }, { "udp", RT_UDP }, { "pipe", RT_Pipe }, { "command", 2000 }, { NULL, RT_None } };
      static int type;
      static int port;
      static char* readfile = NULL;
      if (parseRuntime == NULL) {
        Parser* subparser;
        parseRuntime = Parser_init();
        Parser_add(parseRuntime, Parse_enum, &type, d, NULL);
        Parser_addChar(parseRuntime, ":");
        subparser = Parser_newSubParser(parseRuntime, PB_first);
        Parser_addInt(subparser, &port);
        Parser_add(subparser, Parse_word, &readfile, NULL, NULL);
        Parser_add(parseRuntime, Config_parse_comment, NULL, NULL, NULL);
      }
      if (!Parser_run(parseRuntime, line, PB_suite, false, status)) {
        fprintf(stderr, "Invalid runtime: \"%s\"\n", line);
        ok = false;
        break;
      } else if (type == 2000) {
        config->command = readfile;
      } else {
        config->runtime.type = type;
        config->runtime.port = port;
        config->runtime.file = readfile;
      }
      readfile = NULL;
    }
  }

  /* Is the config valid for these program ? */
  if (config->command && strcmp(config->command, getprogname())) {
    ok = false;
  }

  if (parseSection) {
    Parser_destroy(parseSection);
  }
  if (parseRuntime) {
    Parser_destroy(parseRuntime);
  }
  if (!ok) {
    Config_destroy(config);
    config = NULL;
  }
  free(line);
  fclose(f);
  return config;
}

void Config_destroy(Config* config) {
  if (config->queue) {
    ActionQueue_destroy(config->queue);
    config->queue = NULL;
  }
  if (config->runtime.file) {
    free(config->runtime.file);
    config->runtime.file = NULL;
  }
  if (config->command) {
    free(config->command);
    config->command = NULL;
  }
  free(config);
}
