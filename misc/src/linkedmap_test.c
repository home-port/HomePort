#include "linkedmap.h"
#include "unit_test.h"
#include <stdio.h>

TEST_START("linkedmap.c")

TEST(createAndDestroy)
	struct lm *map;
	map = lm_create();
	ASSERT_NOT_NULL(map);
	lm_destroy(map);
TSET()

TEST(insert)
	struct lm *map;
	map = lm_create();

	int testA = 10;
	int testB = -10;

	lm_insert(map, "programming", &testA);
	lm_insert(map, "python", &testB);

	lm_destroy(map);
TSET()

TEST(find)
	struct lm *map;
	map = lm_create();

	int testA = 10;
	int testB = -10;
	int testC = 5;

	lm_insert(map, "programming", &testA);
	lm_insert(map, "python", &testB);
	lm_insert(map, "C", &testC);

	int *foundA = lm_find(map, "programming");
	int *foundC = lm_find(map, "C");
	int *foundB = lm_find(map, "python");

	ASSERT_EQUAL(*foundA, testA);
	ASSERT_EQUAL(*foundB, testB);
	ASSERT_EQUAL(*foundC, testC);

	ASSERT_NULL(lm_find(map, "I AM NOT IN THE MAP"));

	lm_destroy(map);
TSET()

TEST(remove)
	struct lm *map;
	map = lm_create();

	int testA = 10;
	int testB = -10;
	int testC = 5;

	lm_insert(map, "programming", &testA);
	lm_insert(map, "python", &testB);
	lm_insert(map, "C", &testC);

	ASSERT_NOT_NULL(lm_find(map,"python"));

	lm_remove(map, "python");

	ASSERT_NULL(lm_find(map,"python"));

TSET()



TEST_END()