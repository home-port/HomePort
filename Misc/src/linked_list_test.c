// linked_list_test.c

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

#include "linked_list.h"
#include "unit_test.h"

TEST_START("linked_list.h")

TEST(create_destroy)
   struct ll *list;
   ll_create(list);
   ll_destroy(list);
TSET()

TEST(insert)
   struct ll *list;
   int i = 3;
   ll_create(list);
   ll_insert(list, ll_tail(list), &i);
   ll_destroy(list);
TSET()

TEST(navigate_forward)
   struct ll *list;
   struct ll_iter *it;
   int a = 1;
   int b = 2;
   int c = 3;
   int d = 4;
   int e = 5;
   int *i;

   ll_create(list);
   ll_insert(list, ll_tail(list), &a);
   ll_insert(list, ll_tail(list), &b);
   ll_insert(list, ll_tail(list), &c);
   ll_insert(list, ll_tail(list), &d);
   ll_insert(list, ll_tail(list), &e);

   it = ll_head(list);
   i = ll_data(it);
   ASSERT_EQUAL(*i, a);
   it = ll_next(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, b);
   it = ll_next(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, c);
   it = ll_next(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, d);
   it = ll_next(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, e);
   ASSERT_NULL(ll_next(it));

   ll_destroy(list);
TSET()

TEST(navigate_backward)
   struct ll *list;
   struct ll_iter *it;
   int a = 1;
   int b = 2;
   int c = 3;
   int d = 4;
   int e = 5;
   int *i;

   ll_create(list);
   ll_insert(list, ll_tail(list), &a);
   ll_insert(list, ll_tail(list), &b);
   ll_insert(list, ll_tail(list), &c);
   ll_insert(list, ll_tail(list), &d);
   ll_insert(list, ll_tail(list), &e);

   it = ll_tail(list);
   i = ll_data(it);
   ASSERT_EQUAL(*i, e);
   it = ll_prev(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, d);
   it = ll_prev(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, c);
   it = ll_prev(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, b);
   it = ll_prev(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, a);
   ASSERT_NULL(ll_prev(it));

   ll_destroy(list);
TSET()

TEST(navigate_forward)
   struct ll *list;
   int a = 1;
   int b = 2;
   int c = 3;
   struct ll_iter *it;
   int *i;
   
   ll_create(list);
   ll_insert(list, ll_tail(list), &a);
   ll_insert(list, ll_tail(list), &b);
   ll_insert(list, ll_tail(list), &c);

   it = ll_head(list);
   it = ll_next(it);
   ll_remove(it);

   it = ll_head(list);
   ASSERT_NULL(ll_prev(it));
   i = ll_data(it);
   ASSERT_EQUAL(*i, a);
   it = ll_next(it);
   ASSERT_NULL(ll_next(it));
   i = ll_data(it);
   ASSERT_EQUAL(*i, c);
   it = ll_prev(it);
   i = ll_data(it);
   ASSERT_EQUAL(*i, a);

   ll_destroy(list);
TSET()

TEST(search)
   struct ll *list;
   struct ll_iter *it;
   int a = 1;
   int b = 2;
   int c = 3;
   int d = 4;
   int e = 5;
   int *i;

   ll_create(list);
   ll_insert(list, ll_tail(list), &a);
   ll_insert(list, ll_tail(list), &b);
   ll_insert(list, ll_tail(list), &c);
   ll_insert(list, ll_tail(list), &d);
   ll_insert(list, ll_tail(list), &e);

   ll_find(it, list, &d);
   i = ll_data(it);
   ASSERT_EQUAL(*i, d);

   ll_destroy(list);
TSET()

TEST_END()
