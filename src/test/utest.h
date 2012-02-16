


#ifndef UTEST_H_
#define UTEST_H_

#ifdef NDEBUG
#undef NDEBUG
#endif

#include  <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>

extern int test_results;

int utst_actual_test;
int utst_test_ok;
jmp_buf utst_jmp;


static void utst_sigabrt(int dummy)
{
  longjmp(utst_jmp, 1);
}


#define run_test(message, test)  do {                           \
    printf("Test %d %s ... ", utst_actual_test, message);       \
    signal(SIGABRT, utst_sigabrt);                              \
    ++utst_actual_test;                                         \
    if (!setjmp(utst_jmp)) {                                    \
      assert(test);                                             \
      puts("OK");                                               \
      ++utst_test_ok;                                           \
    } else {                                                    \
      puts("FAILED");                                           \
    }                                                           \
    fflush(stdin);                                              \
  }while (0)



#define run_end() do {                            \
  if (utst_actual_test == utst_test_ok)           \
    puts("ALL TEST PASSED");                      \
  else                                            \
    printf("FAILED %d TESTS\n",                   \
           utst_actual_test - utst_test_ok);      \
  test_results = utst_actual_test - utst_test_ok ;\
  utst_test_ok = 0;                               \
  utst_actual_test = 0;                           \
  } while (0)




#endif /* UTEST_H_ */

