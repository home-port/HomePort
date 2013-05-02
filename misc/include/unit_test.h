// unit_test.h

/*  Copyright 2013 Aalborg University. All rights reserved.
*   
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*  
*  2. Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
*  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*  
*  The views and conclusions contained in the software and
*  documentation are those of the authors and should not be interpreted
*  as representing official policies, either expressed.
*/

#include <stdio.h>
#include <string.h>

#define TEST_START(X) \
   int main(int argc, char **argv) { \
      int _stat = 0; \
      printf("unit: Starting test %s ...\n", #X);

#define TEST_END() \
      return _stat; \
   }

#define TEST(X) \
   do { \
      printf("unit: Running %s...\n", #X); \
      int _errors = 0; \

#define TSET() \
      if (_errors == 0) { \
         printf("unit: Test succeeded\n"); \
      } else { \
         printf("unit: %i errors found!\n", _errors); \
      } \
      _stat += _errors; \
   } while (0);

#define ASSERT(X) \
   if (X) { \
      printf("   Assertion FAILED in %s at %s:%d\n", \
            __func__, __FILE__, __LINE__); \
      _errors++; \
   }

#define ASSERT_NOT_NULL(X) \
   if (X == NULL) { \
      ASSERT(1) \
   }

#define ASSERT_NULL(X) \
   if (X != NULL) { \
      ASSERT(1) \
   }

#define PRINT_CHAR(X) \
   switch (X) { \
      case '\r': \
         printf("\\r"); \
         break; \
      case '\n': \
         printf("\\n"); \
         break; \
      default: \
         printf("%c", X); \
         break; \
   }

#define ASSERT_STR_EQUAL(X,Y) \
   if (strcmp(X,Y)) { \
      char *c; \
      ASSERT(1) \
      printf("   Got '"); \
      for (c = X; c != '\0'; c++) \
         PRINT_CHAR(*c); \
      printf("' - expected '"); \
      for (c = X; c != '\0'; c++) \
         PRINT_CHAR(*c); \
      printf("'\n"); \
   }

#define ASSERT_EQUAL(X,Y) \
   if (X != Y) { \
      ASSERT(1) \
   }

