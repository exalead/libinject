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

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>
#include <stdio.h>

/** @defgroup parser Line parser
 *
 * Light line parser. @{
 */

/** Status of the parser.
 *
 * Store the error status of a parsing. This is useful to print parse error
 */
typedef struct ParserStatus ParserStatus;

/** An element of the parser.
 *
 * An element is designed to extract a part of a @p from string and set the
 * result in a @p dest structure. The given structure depends on the element.
 *
 * A @p ParserElement MUST have the following behaviour :
 * - if the parser failed, it returns false and let @p from unchanged.
 * - if the parser succeed, if returns true and make @p from pointing to the first
 *   unparsed character of the string.
 * - the parser must not considered as parsed the spaces that follow the matched
 *   string. So, if I want to match "on" on the input string "on foo", I MUST reply
 *   true and set @p from to " foo".
 *
 * @param from The string to parse.
 * @param dest The structure to fill with the result of the parsing.
 * @param constraint An extra argument giving constraints to the parser.
 * @return success or not success ? that is the question.
 */
typedef bool (ParserElement)(const char** from, void* dest,
                             const void* constraint, ParserStatus* status);

/** A callback to destroy a constraint data.
 *
 * @param data The data to destroy.
 */
typedef void (ParserDestroyData)(void* data);

/** A Parser structure.
 *
 * Just a way to remember user requests.
 */
typedef struct Parser Parser;

/** Define the possible behaviours of a subparser.
 */
typedef enum ParserBehaviour {
  PB_suite,    /**< Suite of actions. */
  PB_first,    /**< First matching action. */
  PB_optional, /**< Optional match. */
  PB_false     /**< Should not match. */
} ParserBehaviour;

/** Build a new parser.
 *
 * @return A new parser or NULL if an error occurred.
 */
Parser* Parser_init(void);

/** Destroy a parser.
 *
 * @param parser The parser...
 */
void Parser_destroy(Parser* parser);

/** Add a sub parser.
 *
 * This is just a wrapper around Parse_add.
 * @param parser The parser
 * @param behaviour The behaviour of the sub parser (suite|first|false|optional)
 */
Parser* Parser_newSubParser(Parser* parser, ParserBehaviour behaviour);

/** Add a new element to the parser.
 *
 * @param parser The parser to which the element is added.
 * @param element The parser element callback.
 * @param dest Where to store the result of the parser.
 * @param constraint Data to pass as constraint to the callback.
 * @param remover Callback to remove the constraint (or NULL if none).
 * @return success
 */
bool Parser_add(Parser* parser, ParserElement* element, void* dest,
                void* constraint, ParserDestroyData* remover);

/** Add a constant to the parser.
 *
 * This is just a wrapper around Parser_add.
 * @param parser The parser
 * @param constant The constant to match
 * @return success
 */
#define Parser_addConstant(parser, constant) \
  Parser_add(parser, Parse_word, NULL, (void*)(constant), NULL)

/** Add a space to the parser.
 *
 * This is just a wrapper around Parse_add.
 * @param parser The parser
 * @return success.
 */
#define Parser_addSpace(parser) \
  Parser_add(parser, Parse_space, NULL, NULL, NULL)

/** Add a end-of-buffer check to the parser.
 *
 * This is jsut a wrapper around Parse_add.
 * @param parser The parser.
 * @return success
 */
#define Parser_checkEOB(parser) \
  Parser_add(parser, Parse_eob, NULL, NULL, NULL)

/** Add a int to the parser.
 *
 * This is just a wrapper around Parse_add.
 * @param parser The parser
 * @param dest The destination.
 * @return success.
 */
#define Parser_addInt(parser, dest) \
  Parser_add(parser, Parse_int, dest, NULL, NULL)

/** Add a boolean to the parser.
 *
 * This is just a wrapper around Parse_add
 * @param parser The parser
 * @param dest The destination.
 * @return success.
 */
#define Parser_addBool(parser, dest) \
  Parser_add(parser, Parse_bool, dest, NULL, NULL);

/** Add a character to the parser.
 *
 * This is just a wrapper around Parse_add.
 * @param parser The parser
 * @param car    A pointer to the list of character.
 * @return success.
 */
#define Parser_addChar(parser, car) \
  Parser_add(parser, Parse_char, NULL, (void*)car, NULL);

/** Add a parser followed by a space.
 *
 * @param parser The parser to which the element is added.
 * @param element The parser element callback.
 * @param dest Where to store the result of the parser.
 * @param constraint Data to pass as constraint to the callback.
 * @param remover Callback to remove the constraint (or NULL if none).
 * @return success
 */
bool Parser_addSpaced(Parser* parser, ParserElement* element, void* dest,
                      void* constraint, ParserDestroyData* remover);

/** Add a constant to the parser.
 *
 * This is just a wrapper around Parser_add.
 * @param parser The parser
 * @param constant The constant to match
 * @return success
 */
#define Parser_addSpacedConstant(parser, constant) \
  Parser_addSpaced(parser, Parse_word, NULL, (void*)(constant), NULL)

/** Add a int to the parser.
 *
 * This is just a wrapper around Parse_add.
 * @param parser The parser
 * @param dest The destination.
 * @return success.
 */
#define Parser_addSpacedInt(parser, dest) \
  Parser_addSpaced(parser, Parse_int, dest, NULL, NULL)

/** Add a boolean to the parser.
 *
 * This is just a wrapper around Parse_add
 * @param parser The parser
 * @param dest The destination.
 * @return success.
 */
#define Parser_addSpacedBool(parser, dest) \
  Parser_addSpaced(parser, Parse_bool, dest, NULL, NULL);

/** Run a parser with a specific behaviour.
 *
 * @param parser The parser.
 * @param string The string to parse.
 * @param destroy If true, destroy the parser after execution.
 * @param behaviour The behaviour of the root parser (its suite by default
 *                  when running Parser_run).
 * @param status The parser status or null.
 * @return True if the string has been correctly matched.
 */
bool Parser_run(Parser* parser, const char* string, ParserBehaviour behaviour,
                bool destroy, ParserStatus* status);

/** Status of the parser.
 */
struct ParserStatus {
  char  str[1024]; /**< String describing the error. */
  const char* pos; /**< Position of the error. */
};

/** Build a new ParserStatus.
 *
 * @return A new parser status or null.
 */
ParserStatus* ParserStatus_init(void);

/** Set the error if no error is currently set.
 *
 * @param where  The position in the string where the parse error occured.
 * @param error  The error string.
 * @return Always return false.
 */
#define SET_PARSE_ERROR(where, error)                                          \
  ParserStatus_set(status, where, error, false, __FUNCTION__, __FILE__, __LINE__)

/** Set the error.
 *
 * @param where  The position in the string where the parse error occured.
 * @param error  The error string.
 * @return Always return false.
 */
#define FORCE_PARSE_ERROR(where, error)                                        \
  ParserStatus_set(status, where, error, true, __FUNCTION__, __FILE__, __LINE__)

static inline bool ParserStatus_set(ParserStatus* status, const char* where,
                                    const char* error, bool force,
                                    const char* function, const char* file, int line) {
  if (status && (force || status->pos == NULL)) {
    status->pos = where;
    snprintf(status->str, 1023, "Parser error in %s at %s:%d: \"%s\"",
             function, file, line, error);
  }
  return false;
}

/** Clear status error state.
 *
 * @return Always return true.
 */
#define CLEAR_PARSE_ERROR                                                      \
  ParserStatus_clear(status)

static inline bool ParserStatus_clear(ParserStatus* status) {
  if (status) {
    status->pos = NULL;
  }
  return true;
}

/** Check if the status contains an error.
 *
 * @param status The status to check.
 * @return True if the status indicates that an error has occurred.
 */
bool ParserStatus_check(ParserStatus* status);

/** Print the error associated to the given instruction.
 *
 * @param status The status.
 * @param instruction The instruction the status has been run against.
 * @return A string (to remove afterward) or NULL if no error was reported.
 */
char* ParserStatus_error(ParserStatus* status, const char* instruction);

/** Delete the status.
 *
 * @param status the status.
 */
void ParserStatus_destroy(ParserStatus* status);

/** Parse rules one after the other.
 *
 * Fail at the first error. The constraint must be a Parser.
 */
ParserElement Parse_suite;

/** Parse rules until one match.
 *
 * Fail if no rule matched. The constraint must be a Parser.
 */
ParserElement Parse_first;

/** Parse an int.
 */
ParserElement Parse_int;

/** Parse a word and compare it to the constraint if one was given.
 *
 * If no constraint is given, the first word is returned, in the other case
 * the word is returned if (and only if) it matches the constraint.
 *
 * The destination must be a char** or NULL,
 * The constraint must be a char* or NULL
 *
 * If neither the destination nor the constraint is given, this element
 * skip the next word.
 */
ParserElement Parse_word;

/** Parse spaces.
 *
 * Special parse that skip spaces. It takes neither data nor constraint. This
 * parser requires to match at least one space.
 */
ParserElement Parse_space;

/** Parse end of buffer.
 *
 * Ensure the buffer is finished. This parser do not move the current pointer.
 */
ParserElement Parse_eob;

/** Parse all while remain until the end of the line.
 */
ParserElement Parse_line;

/** Parse all that is not one of the given characters.
 */
ParserElement Parse_not;

/** Parse an optional part.
 *
 * Return true even if the content was false. The constraint must be a parser.
 */
ParserElement Parse_optional;

/** Parse a part that should fail.
 *
 * Return true if the subparser return false, and vice-versa.
 */
ParserElement Parse_false;

/** Data structure for enum parser.
 */
typedef struct {
  const char* constant; /**< String constant representing the enum value. */
  int value;            /**< Enum value associated. */
} Parse_enumData;

/** Parse an enum.
 *
 * Return true if one of the constant match and set the dest to the corresponding
 * integer value. The constraint must be a set Parse_enumData structure.
 */
ParserElement Parse_enum;

/** Parse a character.
 *
 * Compare a character with the current one and continue...
 */
ParserElement Parse_char;

/** Parse a boolean.
 *
 * A boolean can be either true or false
 */
ParserElement Parse_bool;

/** @} */

#endif
