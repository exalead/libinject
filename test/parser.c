/******************************************************************************
 *
 *                               The NG Project
 *
 *                                Parser tests
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "../testlib/testlib.h"
#include "../src/parser.h"


static bool testInt(TestFeed data, TestFeed result) {
  int i;
  char* str;
  int expected;
  Parser* parser;

  str = (char*)data.p;
  expected = result.i;

  parser = Parser_init();
  Parser_addInt(parser, &i);
  return Parser_run(parser, str, PB_suite, true, NULL) && i == expected;
}

static bool testWord(TestFeed data, TestFeed result) {
  char* i = NULL;
  char* str;
  char* expected;
  bool ok;
  Parser* parser;

  str = (char*)data.p;
  expected = (char*)result.p;

  parser = Parser_init();
  Parser_add(parser, Parse_word, &i, NULL, NULL);
  ok = Parser_run(parser, str, PB_suite, true, NULL);
  if (i) {
    ok = (ok && strcmp(expected, i) == 0);
    free(i);
  }
  return ok;
}

static bool testConstant(TestFeed data, TestFeed result) {
  char* str;
  char* expected;
  Parser* parser;

  str = (char*)data.p;
  expected = (char*)result.p;

  parser = Parser_init();
  Parser_add(parser, Parse_word, NULL, expected, NULL);
  return Parser_run(parser, str, PB_suite, true, NULL);
}

static bool testSpace(TestFeed data, TestFeed result) {
  char* str;
  Parser* parser;

  str = (char*)data.p;

  parser = Parser_init();
  Parser_addSpace(parser);
  return Parser_run(parser, str, PB_suite, true, NULL);
}

static bool testEOB(TestFeed data, TestFeed result) {
  char* str;
  Parser* parser;

  str = (char*)data.p;

  parser = Parser_init();
  Parser_checkEOB(parser);
  return Parser_run(parser, str, PB_suite, true, NULL);
}

static bool testSuite(TestFeed data, TestFeed result) {
  bool ok;
  int  i;
  char* str = NULL;
  Parser* parser;

  parser = Parser_init();
  Parser_addSpacedConstant(parser, "on");
  Parser_addSpacedInt(parser, &i);
  Parser_addSpacedConstant(parser, "do");
  Parser_add(parser, Parse_word, &str, NULL, NULL);
  Parser_checkEOB(parser);
  ok = Parser_run(parser, "on 42 do BOUBOUBOUM", PB_suite, true, NULL);
  if (str) {
    ok = (ok && strcmp(str, "BOUBOUBOUM") == 0);
    free(str);
  }
  return ok && i == 42;
}

static bool testFirst(TestFeed data, TestFeed result) {
  char* str;
  Parser* parser;

  str = (char*)data.p;

  parser = Parser_init();
  Parser_addConstant(parser, "coucou");
  Parser_addConstant(parser, "cou");
  Parser_addConstant(parser, "c");
  Parser_addConstant(parser, "coucouc");
  return Parser_run(parser, str, PB_first, true, NULL);
}

static bool testOptional(TestFeed data, TestFeed result) {
  bool ok;
  char* str;
  char* i = NULL;
  char* expected;
  Parser* parser;

  str = (char*)data.p;
  expected = (char*)result.p;

  parser = Parser_init();
  Parser_addSpacedConstant(parser, "host");
  Parser_add(parser, Parse_word, &i, NULL, NULL);
  ok = Parser_run(parser, str, PB_optional, true, NULL);
  if (expected == NULL) {
    ok = (expected == i);
  } else if (i == NULL) {
    ok = false;
  } else {
    ok = (strcmp(i, expected) == 0);
  }
  if (i) {
    free(i);
  }
  return ok;
}

static bool testFalse(TestFeed data, TestFeed result) {
  int i;
  Parser* parser;

  parser = Parser_init();
  Parser_addInt(parser, &i);
  return ! Parser_run(parser, "1234", PB_false, true, NULL);
}

static bool testEnum(TestFeed data, TestFeed result) {
  Parser* parser;
  Parse_enumData* vals;
  int i;
  int res;

  vals = (Parse_enumData*)data.p;
  res = result.i;

  parser = Parser_init();
  Parser_add(parser, Parse_enum, &i, vals, NULL);
  return Parser_run(parser, "forty-two", PB_suite, true, NULL) && i == res;
}

static bool testNot(TestFeed data, TestFeed result) {
  Parser* parser;
  char* str;

  str = (char*)data.p;

  parser = Parser_init();
  Parser_add(parser, Parse_not, NULL, (void*)"]", NULL);
  Parser_addConstant(parser, "]");
  return Parser_run(parser, str, PB_suite, true, NULL);
}

static bool testLine(TestFeed data, TestFeed result) {
  Parser* parser;
  int lines;
  char* str;

  str  = (char*)data.p;
  lines = result.i;

  parser = Parser_init();
  while (lines > 0) {
    if (lines > 1) {
      Parser_addSpaced(parser, Parse_line, NULL, NULL, NULL);
    } else {
      Parser_add(parser, Parse_line, NULL, NULL, NULL);
    }
    --lines;
  }
  return Parser_run(parser, str, PB_suite, true, NULL);
}

int main(void) {
  TestSet* set;
  testid   tid;
  Parse_enumData d1[] = { { NULL, 0 } },
                 d2[] = { { "twenty", 20 }, { NULL, 0 } },
                 d3[] = { { "forty", 40 }, { NULL, 0 }, { "forty-two", 42 } },
                 d4[] = { { "forty", 40 }, { "forty-two", 42 }, { NULL, 0 } },
                 d5[] = { { "forty-two", 42 }, { NULL, 0 } },
                 d6[] = { { "forty-two", 43 }, { NULL, 0 } };

  set = TestSet_init("parser");

  /* Build int test */
  tid = TestSet_registerTest(set, "int", testInt);
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1234"), INT_FEED(1234));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("-1234"), INT_FEED(-1234));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1234abc"), INT_FEED(1234));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" 1234"), INT_FEED(1234));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" "), INT_FEED(0));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(""), INT_FEED(0));

  /* Build word test */
  tid = TestSet_registerTest(set, "word", testWord);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("coucou"), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("coucou "), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" coucou"), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" "), POINTER_FEED(" "));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" "), POINTER_FEED(""));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(""), POINTER_FEED("coucou"));

  /* Build constant test */
  tid = TestSet_registerTest(set, "constant", testConstant);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("coucou"), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("coucou  "), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("coucouc"), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("couc"), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" coucou"), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" "), POINTER_FEED("coucou"));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(""), POINTER_FEED("coucou"));

  /* Space test */
  tid = TestSet_registerTest(set, "space", testSpace);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("    "), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED(" truc"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("-    "), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(""), POINTER_FEED(NULL));

  /* End of buffer */
  tid = TestSet_registerTest(set, "eob", testEOB);
  TestSet_registerTestData(set, tid, true, POINTER_FEED(""), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(" "), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("a "), POINTER_FEED(NULL));

  /* Suite of element test */
  TestSet_registerNoArgTest(set, "suite", testSuite, true);

  /* Selection test */
  tid = TestSet_registerTest(set, "first", testFirst);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("coucou"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("cou"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("c"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("coucouc"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("couco"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(""), POINTER_FEED(NULL));

  /* Optional test */
  tid = TestSet_registerTest(set, "optional", testOptional);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("host"), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("host "), POINTER_FEED(NULL));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("host machin"), POINTER_FEED("machin"));

  /* Inverser test */
  TestSet_registerNoArgTest(set, "false", testFalse, true);

  /* Test enum */
  tid = TestSet_registerTest(set, "enum", testEnum);
  TestSet_registerTestData(set, tid, false, POINTER_FEED(d1), INT_FEED(42));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(d2), INT_FEED(42));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(d3), INT_FEED(42));
  TestSet_registerTestData(set, tid, true, POINTER_FEED(d4), INT_FEED(42));
  TestSet_registerTestData(set, tid, true, POINTER_FEED(d5), INT_FEED(42));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(d6), INT_FEED(42));

  /* Test not */
  tid = TestSet_registerTest(set, "not", testNot);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin]"), INT_FEED(0));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("machin"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin]        truc"), INT_FEED(0));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("]"), INT_FEED(0));

  /* Parse a lines */
  tid = TestSet_registerTest(set, "line", testLine);
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin"), INT_FEED(1));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin\ntruc"), INT_FEED(2));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin\ntruc\n"), INT_FEED(2));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin\n\ntruc"), INT_FEED(2));
  TestSet_registerTestData(set, tid, true, POINTER_FEED("machin\n     \n"), INT_FEED(1));
  TestSet_registerTestData(set, tid, false, POINTER_FEED(""), INT_FEED(1));

  /* Process all this... and return. */
  return TestSet_run(set) ? 0 : 1;
}
