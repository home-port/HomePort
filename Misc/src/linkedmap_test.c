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

	char *testA = "Isis";
	char *testB = "Bastet";

	int r1 = lm_insert(map, "programming", testA);
	int r2 = lm_insert(map, "python", testB);
	int r3 = lm_insert(map, "programming", testB); // Already inserted

	ASSERT_EQUAL(r1,0);
	ASSERT_EQUAL(r2,0);
	ASSERT_EQUAL(r3,1);

	lm_destroy(map);
TSET()

TEST(find)
	struct lm *map;
	map = lm_create();

	char *testA = "Thoth";
	char *testB = "Bastet";
	char *testC = "Seth";

	lm_insert(map, "programming", testA);
	lm_insert(map, "python", testB);
	lm_insert(map, "C", testC);

	char *foundA = lm_find(map, "programming");
	char *foundC = lm_find(map, "C");
	char *foundB = lm_find(map, "python");

	ASSERT_STR_EQUAL(foundA, testA);
	ASSERT_STR_EQUAL(foundB, testB);
	ASSERT_STR_EQUAL(foundC, testC);

	ASSERT_NULL(lm_find(map, "I AM NOT IN THE MAP"));

	lm_destroy(map);
TSET()

TEST(remove)
	struct lm *map;
	map = lm_create();

	char *testA = "Thoth";
	char *testB = "Bastet";
	char *testC = "Seth";

	lm_insert(map, "programming", testA);
	lm_insert(map, "python", testB);
	lm_insert(map, "C", testC);

	ASSERT_NOT_NULL(lm_find(map,"python"));

	lm_remove(map, "python");

	ASSERT_NULL(lm_find(map,"python"));

	lm_destroy(map);
TSET()

TEST_END()