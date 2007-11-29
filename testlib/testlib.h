/******************************************************************************
 *
 *                               The NG Project
 *
 *                            Small unit test lib
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#ifndef _TESTLIB_H_
#define _TESTLIB_H_

#include <stdbool.h>

/** @defgroup testlib Unit test library
 *
 * This small library introduces some useful function to perform unit tests and
 * display a summary of the results. @{
 */

/** A test set.
 */
typedef struct TestSet TestSet;

/** The id of a test.
 */
typedef int testid;

/** Enum for allowing multiple type of data to be used in tests.
 */
typedef union {
  unsigned int ui; /**< <b>u</b>nsigned <b>i</b>nt alternative */
  int          i;  /**< <b>i</b>nt alternative */
  float        f;  /**< <b>f</b>loat alternative */
  double       d;  /**< <b>d</b>ouble float alternative */
  void*        p;  /**< <b>p</b>ointer alternative */
} TestFeed;

/** Make int feed.
 */
#define INT_FEED(i) ((TestFeed)(i))

/** Make pointer feed
 */
#define POINTER_FEED(p) ((TestFeed)((void*)(p)))

/** Prototype for testing functions.
 *
 * As you can see, the test take 2 unions as its entry. This is to allow the
 * largest set of possible values.
 */
typedef bool (Test)(TestFeed data, TestFeed result);

/** Initialize a test set.
 *
 * @param name Text to display to identify the test.
 * @return A new TestSet.
 */
TestSet* TestSet_init(const char* name);

/** Register a new test callback.
 *
 * @param set The test set.
 * @param name The name of the test.
 * @param cb The test callback.
 * @return The id of the new test.
 */
testid TestSet_registerTest(TestSet* set, const char* name, Test* cb);

/** Register a new data to be tested by a test.
 *
 * @param set The test set.
 * @param tid The id of the test to edit.
 * @param success If true the test is expected to succeed, else it shoud fail
 * @param data The source data for the test.
 * @param result The result expected.
 * @return true if the data has been properly added to the test set.
 */
bool TestSet_registerTestData(TestSet* set, testid tid, bool success,
                              TestFeed data, TestFeed result);

/** Add a test running without parameters.
 *
 * @param set The test set.
 * @param name The name of the test.
 * @param cb The call back.
 * @param succ The expected behaviour of the test
 */
#define TestSet_registerNoArgTest(set, name, cb, succ)                         \
  do {                                                                         \
    testid __tid;                                                              \
    __tid = TestSet_registerTest(set, name, cb);                               \
    (void)TestSet_registerTestData(set, __tid, succ, INT_FEED(0), INT_FEED(0));\
  } while(0)


/** Destroy a test set.
 *
 * @param set The test set.
 */
void TestSet_destroy(TestSet* set);

/** Run the tests on all register test with corresponding data.
 *
 * This will destroy the test set after running the tests.
 *
 * @param set The test set.
 * @return true if all test passed successfully
 *
 */
bool TestSet_run(TestSet* set);

/** @} */

#endif
