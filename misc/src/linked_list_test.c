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
   ll_insert(list, &i);
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
   ll_insert(list, &a);
   ll_insert(list, &b);
   ll_insert(list, &c);
   ll_insert(list, &d);
   ll_insert(list, &e);

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
   ll_insert(list, &a);
   ll_insert(list, &b);
   ll_insert(list, &c);
   ll_insert(list, &d);
   ll_insert(list, &e);

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
   ll_insert(list, &a);
   ll_insert(list, &b);
   ll_insert(list, &c);

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
   ll_insert(list, &a);
   ll_insert(list, &b);
   ll_insert(list, &c);
   ll_insert(list, &d);
   ll_insert(list, &e);

   ll_find(it, list, &d);
   i = ll_data(it);
   ASSERT_EQUAL(*i, d);

   ll_destroy(list);
TSET()

TEST_END()
