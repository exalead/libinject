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

#include <stdlib.h>
#include <string.h>
#include "../testlib/testlib.h"
#include "../src/conffile.h"
#include "../src/actions.h"

static bool testParser(TestFeed data, TestFeed result) {
  Action* action;
  char*   instr;

  instr = (char*)data.p;
  action = Action_init(instr, NULL);
  if (action) {
    Action_destroy(action);
    return true;
  }
  return false;
}

static bool testFile(TestFeed data, TestFeed result) {
  Config* config;
  char* file;

  file = (char*)data.p;
  if ((config = Config_init(file)) == NULL) {
    return false;
  }
  Config_destroy(config);
  return true;
}

int main(void) {
  TestSet* set;
  testid   tid;

  set = TestSet_init("config");

  /* Build parser tests */
  tid = TestSet_registerTest(set, "rules", testParser);
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on udp from me to any when always do nop continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on udp connect to any do nop continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on tcp from dns to any port 53 when unmatched do nop goto 20"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on tcp from any port 234 to any when prob 50 do-once nop next"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on ip from any port ftp to 192.168.0.1 when cycle true 10000 10000 do-once-per-call nop stop  "), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on ip from 224.0.0.1 to 192.168.0.1 port 2222 when prob 75 do-once-per-socket nop exec 1000"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on ip with any continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on ip talk-with any continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on ip close to any continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("1 on ip with 224.0.0.1 port 2222 do truncate 10 continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("1 on ip with 224.0.0.1 port 2222 to any do truncate 10 continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, false,  POINTER_FEED("i on udp from me to any when always do nop continue"), INT_FEED(0));
  TestSet_registerTestData(set, tid, false,  POINTER_FEED("1000 on udp from me to any when always do nop continue 10"), INT_FEED(0));

  /* Build int test */
  tid = TestSet_registerTest(set, "file", testFile);
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("testrules.rules"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("testrulessection.rules"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("testruntime.rules"), INT_FEED(0));
  TestSet_registerTestData(set, tid, true,  POINTER_FEED("testall.rules"), INT_FEED(0));
  TestSet_registerTestData(set, tid, false, POINTER_FEED("idontexists"), INT_FEED(0));

  /* Process all this... and return. */
  return TestSet_run(set) ? 0 : 1;
}
