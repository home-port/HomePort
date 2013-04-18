#include <stdio.h>
#include "unit_test.h"
#include "hashtable.h"

static struct nlist* table[10];

TEST_START("hashtable.c")

TEST(lookup)

install("a", "apple", table);
install("b", "banana", table);

char *fruit = lookup("a", table)->defn;
ASSERT_STR_EQUAL(fruit, "apple")
ASSERT_STR_EQUAL(lookup("b", table)->defn, "banana")
ASSERT_NULL(lookup("g",table))

TSET()

TEST_END()
