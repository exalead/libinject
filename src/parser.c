/******************************************************************************
 *
 *                               The NG Project
 *
 *                                 Line parser
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "parser.h"

/** An internal parser piece.
 */
struct ParserPiece {
  ParserElement* element;  /**< The parser element. */
  void* dest;              /**< The destination of the data. */
  void* constraint;        /**< The constraint data. */
  ParserDestroyData* free; /**< The callback to free the constraint afterward. */
};

/** The parser structure.
 */
struct Parser {
  size_t capacity;            /**< The capacity of the parser. */
  size_t length;              /**< The number of parser pieces. */
  struct ParserPiece* pieces; /**< The element of the parser. */
};

/** Status of the parser.
 */
struct ParserStatus {
  char  str[1024]; /**< String describing the error. */
  const char* pos; /**< Position of the error. */
};

Parser* Parser_init(void) {
  Parser* parser;
  parser = (Parser*)malloc(sizeof(Parser));
  if (!parser) {
    return NULL;
  }
  parser->capacity = 128;
  parser->length   = 0;
  parser->pieces   = (struct ParserPiece*)malloc(parser->capacity * sizeof(struct ParserPiece));
  return parser;
}

void Parser_destroy(Parser* parser) {
  size_t i;
  for (i = 0 ; i < parser->length ; ++i) {
    struct ParserPiece* piece;
    piece = parser->pieces + i;
    if (piece->free) {
      piece->free(piece->constraint);
    }
  }
  free(parser->pieces);
  free(parser);
}

inline bool Parser_add(Parser* parser, ParserElement* element, void* dest,
                       void* constraint, ParserDestroyData* remover) {
  struct ParserPiece* piece;
  if (parser->length == parser->capacity) {
    struct ParserPiece* pieces;
    pieces = (struct ParserPiece*)realloc(parser->pieces,
                                          sizeof(struct ParserPiece) * (parser->capacity) * 2);
    if (!pieces) {
      return false;
    }
    parser->capacity *= 2;
    parser->pieces = pieces;
  }
  piece = parser->pieces + parser->length;
  piece->element    = element;
  piece->dest       = dest;
  piece->constraint = constraint;
  piece->free       = remover;
  ++parser->length;
  return true;
}

bool Parser_addSpaced(Parser* parser, ParserElement* element, void* dest,
                      void* constraint, ParserDestroyData* remover) {
  parser = Parser_newSubParser(parser, PB_suite);
  return Parser_add(parser, element, dest, constraint, remover)
      && Parser_addSpace(parser);
}

Parser* Parser_newSubParser(Parser* parser, ParserBehaviour behaviour) {
  Parser* subparser;
  ParserElement* element;
  switch (behaviour) {
    case PB_suite:    element = Parse_suite; break;
    case PB_first:    element = Parse_first; break;
    case PB_optional: element = Parse_optional; break;
    case PB_false:    element = Parse_false; break;
    default: return NULL;
  }
  subparser = Parser_init();
  Parser_add(parser, element, NULL, subparser, (ParserDestroyData*)Parser_destroy);
  return subparser;
}

ParserStatus* ParserStatus_init() {
  ParserStatus* status;
  status = (ParserStatus*)malloc(sizeof(ParserStatus));
  status->str[1023] = '\0';
  status->pos = NULL;
  return status;
}

void ParserStatus_destroy(ParserStatus* status) {
  free(status);
}

bool ParserStatus_check(ParserStatus* status) {
  if (!status) {
    return false;
  }
  return status->pos != NULL;
}

bool ParserStatus_set(ParserStatus* status, const char* where, const char* error,
                      bool force, const char* function, const char* file, int line) {
  if (status && (force || status->pos == NULL)) {
    status->pos = where;
    snprintf(status->str, 1023, "Parser error in %s at %s:%d: \"%s\"",
             function, file, line, error);
  }
  return false;
}

bool ParserStatus_clear(ParserStatus* status) {
  if (status) {
    status->pos = NULL;
  }
  return true;
}

char* ParserStatus_error(ParserStatus* status, const char* instruction) {
  char* string = NULL;
  int pos = 0;
  if (status && status->pos != NULL) {
    string = (char*)malloc(4096);
    string[4095] = '\0';
    pos = snprintf(string, 4095, "%s\n%s\n", status->str, instruction);
    if (pos > 0) {
      int offset = status->pos - instruction;
      int count  = 0;
      for (count = 0 ; count < offset && pos < 4095 ; ++count) {
        string[pos] = ' ';
        ++pos;
        string[pos] = '\0';
      }
      if (pos < 4094) {
        string[pos] = '^';
        ++pos;
        string[pos] = '\n';
        ++pos;
        string[pos] = '\0';
      }
    }
  }
  return string;
}

bool Parser_run(Parser* parser, const char* string, ParserBehaviour behaviour,
                bool destroy, ParserStatus* status) {
  bool res;
  switch (behaviour) {
    case PB_suite:    res = Parse_suite(&string, NULL, parser, status); break;
    case PB_first:    res = Parse_first(&string, NULL, parser, status); break;
    case PB_optional: res = Parse_optional(&string, NULL, parser, status); break;
    case PB_false:    res = Parse_false(&string, NULL, parser, status); break;
    default: res = false;
  }
  if (destroy) {
    Parser_destroy(parser);
  }
  return res;
}

bool Parse_suite(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  Parser* parser = (Parser*)constraint;
  const char* string;
  size_t i;

  string = *from;
  for (i = 0 ; i < parser->length ; ++i) {
    struct ParserPiece* piece;
    piece = parser->pieces + i;
    if (!piece->element(&string, piece->dest, piece->constraint, status)) {
      return SET_PARSE_ERROR(*from, "Parse error");
    }
  }
  *from = string;
  return CLEAR_PARSE_ERROR;
}

bool Parse_first(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  Parser* parser = (Parser*)constraint;
  size_t i;

  for (i = 0 ; i < parser->length ; ++i) {
    struct ParserPiece* piece;
    piece = parser->pieces + i;
    if (piece->element(from, piece->dest, piece->constraint, status)) {
      return CLEAR_PARSE_ERROR;
    }
  }
  return FORCE_PARSE_ERROR(*from, "No alternative matched");
}

bool Parse_int(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  char* endofint;
  int*  intDest = (int*)dest;

  if (**from != '-' && !isdigit(**from)) {
    return SET_PARSE_ERROR(*from, "Not an integer value");
  }

  *intDest = strtol(*from, &endofint, 10);
  if (*from != endofint) {
    *from = endofint;
    return CLEAR_PARSE_ERROR;
  }
  return SET_PARSE_ERROR(*from, "Not an integer value");
}

static inline bool Parse_until(const char** from, void* dest,
                               const void* constraint, const char* chars,
                               ParserStatus* status) {
  const char* pos = NULL;
  char** dstring = (char**)dest;
  const char* cstring = (const char*)constraint;
  size_t len = 0;
  size_t matchLen = 0;

  if (dstring) {
    *dstring = NULL;
  }
  if (cstring) {
    len = strlen(cstring);
    if (len > strlen(*from)) {
      return SET_PARSE_ERROR(*from, "Constant not found");
    }
  }
  do {
    const char* tmp = (*from) + len;
    while (*tmp != *chars && *tmp != '\0') {
      ++tmp;
    }
    if (pos == NULL || tmp < pos) {
      pos = tmp;
    }
    ++chars;
  } while (*chars != '\0');
  matchLen = pos - (*from);

  if (matchLen == 0) {
    return SET_PARSE_ERROR(*from, "Reading an empty word");
  }
  if (cstring) {
    size_t cLen;
    cLen = strlen(cstring);
    if (cLen != matchLen || strncmp(*from, constraint, cLen) != 0) {
      return SET_PARSE_ERROR(*from, "Word do not match expected constant");
    }
  }
  if (dstring) {
    *dstring = (char*)malloc(matchLen + 1);
    memcpy(*dstring, *from, matchLen);
    (*dstring)[matchLen] = '\0';
  }
  *from = pos;
  return CLEAR_PARSE_ERROR;
}

bool Parse_word(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  return Parse_until(from, dest, constraint, " \t\r\n<>[][()\"\'", status);
}

bool Parse_space(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  if (!isspace(**from)) {
    return SET_PARSE_ERROR(*from, "Space expected");
  }
  while (isspace(**from)) {
    ++*from;
  }
  return CLEAR_PARSE_ERROR;
}

bool Parse_eob(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  return **from == '\0' ? CLEAR_PARSE_ERROR
       : SET_PARSE_ERROR(*from, "Not at end of buffer");
}

bool Parse_line(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  return Parse_until(from, dest, constraint, "\r\n", status);
}

bool Parse_not(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  return Parse_until(from, dest, NULL, (const char*)constraint, status);
}

bool Parse_optional(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  (void) Parse_suite(from, NULL, constraint, status);
  return CLEAR_PARSE_ERROR;
}

bool Parse_false(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  return ! Parse_suite(from, NULL, constraint, status);
}

inline bool Parse_enum(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  const Parse_enumData* data = (const Parse_enumData*)constraint;
  int* value = (int*)dest;

  while (data->constant != NULL) {
    if (Parse_until(from, NULL, data->constant, " \t\r\n<>[][()\"\'.:;!?", status)) {
      *value = data->value;
      return true; /* error cleared by Parse_until */
    }
    ++data;
  }
  *value = data->value;
  return FORCE_PARSE_ERROR(*from, "No enumerated value found");
}

bool Parse_char(const char** from, void* dest, const void* constraint, ParserStatus* status) {
  const char* test = (const char*)constraint;
  while (*test != '\0') {
    if (**from == *test) {
      ++(*from);
      return CLEAR_PARSE_ERROR;
    }
    ++test;
  }
  return SET_PARSE_ERROR(*from, "Character not found");
}

bool Parse_bool(const char** from, void* test, const void* constraint, ParserStatus* status) {
  static Parse_enumData bools[] = { { "true", 1 }, { "false", 0 }, { NULL, 0 } };
  return Parse_enum(from, test, &bools, status);
}
