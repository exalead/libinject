/******************************************************************************
 *
 *                               The NG Project
 *
 *                                 Line parser
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#define _GNU_SOURCE /* For strchrnul */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

bool Parser_run(Parser* parser, const char* string, ParserBehaviour behaviour,
                bool destroy) {
  bool res;
  switch (behaviour) {
    case PB_suite:    res = Parse_suite(&string, NULL, parser); break;
    case PB_first:    res = Parse_first(&string, NULL, parser); break;
    case PB_optional: res = Parse_optional(&string, NULL, parser); break;
    case PB_false:    res = Parse_false(&string, NULL, parser); break;
    default: res = false;
  }
  if (destroy) {
    Parser_destroy(parser);
  }
  return res;
}

bool Parse_suite(const char** from, void* dest, const void* constraint) {
  Parser* parser = (Parser*)constraint;
  const char* string;
  size_t i;

  string = *from;
  for (i = 0 ; i < parser->length ; ++i) {
    struct ParserPiece* piece;
    piece = parser->pieces + i;
    if (!piece->element(&string, piece->dest, piece->constraint)) {
      return false;
    }
  }
  *from = string;
  return true;
}

bool Parse_first(const char** from, void* dest, const void* constraint) {
  Parser* parser = (Parser*)constraint;
  size_t i;

  for (i = 0 ; i < parser->length ; ++i) {
    struct ParserPiece* piece;
    piece = parser->pieces + i;
    if (piece->element(from, piece->dest, piece->constraint)) {
      return true;
    }
  }
  return false;
}

bool Parse_int(const char** from, void* dest, const void* constraint) {
  char* endofint;
  int*  intDest = (int*)dest;

  if (**from != '-' && !isdigit(**from)) {
    return false;
  }

  *intDest = strtol(*from, &endofint, 10);
  if (*from != endofint) {
    *from = endofint;
    return true;
  }
  return false;
}

static inline bool Parse_until(const char** from, void* dest, const void* constraint, const char* chars) {
  char* pos = NULL;
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
      return false;
    }
  }
  do {
    char* tmp;
    tmp = strchrnul((*from) + len, *chars);
    if (pos == NULL || tmp < pos) {
      pos = tmp;
    }
    ++chars;
  } while (*chars != '\0');
  matchLen = pos - (*from);

  if (matchLen == 0) {
    return false;
  }
  if (cstring) {
    size_t cLen;
    cLen = strlen(cstring);
    if (cLen != matchLen || strncmp(*from, constraint, cLen) != 0) {
      return false;
    }
  }
  if (dstring) {
    *dstring = (char*)malloc(matchLen + 1);
    memcpy(*dstring, *from, matchLen);
    (*dstring)[matchLen] = '\0';
  }
  *from = pos;
  return true;
}

bool Parse_word(const char** from, void* dest, const void* constraint) {
  return Parse_until(from, dest, constraint, " \t\r\n<>[][()\"\'");
}

bool Parse_space(const char** from, void* dest, const void* constraint) {
  if (!isspace(**from)) {
    return false;
  }
  while (isspace(**from)) {
    ++*from;
  }
  return true;
}

bool Parse_eob(const char** from, void* dest, const void* constraint) {
  return **from == '\0';
}

bool Parse_line(const char** from, void* dest, const void* constraint) {
  return Parse_until(from, dest, constraint, "\r\n");
}

bool Parse_not(const char** from, void* dest, const void* constraint) {
  return Parse_until(from, dest, NULL, (const char*)constraint);
}

bool Parse_optional(const char** from, void* dest, const void* constraint) {
  (void) Parse_suite(from, NULL, constraint);
  return true;
}

bool Parse_false(const char** from, void* dest, const void* constraint) {
  return ! Parse_suite(from, NULL, constraint);
}

inline bool Parse_enum(const char** from, void* dest, const void* constraint) {
  const Parse_enumData* data = (const Parse_enumData*)constraint;
  int* value = (int*)dest;

  while (data->constant != NULL) {
    if (Parse_until(from, NULL, data->constant, " \t\r\n<>[][()\"\'.:;!?")) {
      *value = data->value;
      return true;
    }
    ++data;
  }
  *value = data->value;
  return false;
}

bool Parse_char(const char** from, void* dest, const void* constraint) {
  const char* test = (const char*)constraint;
  while (*test != '\0') {
    if (**from == *test) {
      ++(*from);
      return true;
    }
    ++test;
  }
  return false;
}

bool Parse_bool(const char** from, void* test, const void* constraint) {
  static Parse_enumData bools[] = { { "true", 1 }, { "false", 0 }, { NULL, 0 } };
  return Parse_enum(from, test, &bools);
}
