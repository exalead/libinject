/******************************************************************************
 *
 *                               The NG Project
 *
 *                            Small unit test lib
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include "testlib.h"

/** Structure to store data and result of tests.
 */
struct TestData {
  bool     success; /**< Expected behaviour of the test. */
  bool     run;     /**< Result after having run the test. */
  TestFeed data;    /**< Source data. */
  TestFeed result;  /**< Expected result. */
};

/** Structure to store data associated with a test.
 */
struct Test {
  const char* name;         /**< Test name. */
  Test*       test;         /**< Test callback. */
  size_t      datalen;      /**< Size of the TestData buffer. */
  size_t      contentlen;   /**< Number of TestData element. */
  struct TestData*   data;  /**< Tests data. */
};

/** A test set.
 */
struct TestSet {
  const char* name;     /**< The name of the test set. */
  struct Test* tests;  /**< List of the tests. */
  size_t      length;   /**< Number of tests in the list. */
};

TestSet* TestSet_init(const char* name) {
  TestSet* set;
  set = (TestSet*)malloc(sizeof(TestSet));
  set->name  = name;
  set->tests = (struct Test*)calloc(1024, sizeof(struct Test));
  set->length = 0;
  return set;
}

testid TestSet_registerTest(TestSet* set, const char* name, Test* cb) {
  struct Test* test;
  test = &(set->tests[set->length]);
  test->name = name;
  test->test = cb;
  test->data = (struct TestData*)calloc(1024, sizeof(struct TestData));
  test->datalen = 1024;
  test->contentlen = 0;
  return set->length++;
}

bool TestSet_registerTestData(TestSet* set, testid tid, bool success,
                              TestFeed data, TestFeed result) {
  struct Test* test;
  struct TestData* tdata;
  test = &(set->tests[tid]);

  if (test->contentlen >= test->datalen) {
    test->data = (struct TestData*)realloc(test->data, test->datalen * 2 * sizeof(struct TestData));
    test->datalen *= 2;
  }
  tdata = &(test->data[test->contentlen]);
  tdata->data = data;
  tdata->result = result;
  tdata->success = success;
  ++test->contentlen;
  return true;
}

void TestSet_destroy(TestSet* set) {
  size_t i;

  for (i = 0 ; i < set->length ; ++i) {
    free(set->tests[i].data);
  }
  free(set->tests);
  free(set);
}

bool TestSet_run(TestSet* set) {
  size_t i, j;
  bool ok = true;

  printf("Running %s\n", set->name);
  for (i = 0 ; i < set->length ; ++i) {
    struct Test* test;
    int success = 0;
    int error   = 0;
    test = &(set->tests[i]);
    for (j = 0 ; j < test->contentlen ; ++j) {
      struct TestData* data;
      data = &test->data[j];
      data->run = (test->test(data->data, data->result) == data->success);
      if (data->run) {
        ++success;
      } else {
        ++error;
        ok = false;
      }
    }
    printf("%s %s ", error ? "ERROR" : "OK   ", test->name);
    for (j = 0 ; j < test->contentlen ; ++j) {
      printf("%s", test->data[j].run ? "." : "E");
    }
    printf(" (%d success, %d errors)\n", success, error);
  }
  TestSet_destroy(set);
  return ok;
}
