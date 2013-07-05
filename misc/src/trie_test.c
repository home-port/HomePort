// trie_test.c

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

#include "trie.h"
#include "unit_test.h"

void dealloc(void *val)
{
   free(val);
}

TEST_START("trie.c")

TEST(create_destroy)
   struct trie *root = trie_create();
   trie_destroy(root, dealloc);
TSET()

TEST(simple_insert)
   char *key = "cat";
   struct trie *trie = trie_create();

   struct trie_iter *iter = trie_insert(trie, key, key);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), key);

   iter = trie_lookup(trie, key);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), key);

   trie_destroy(trie, NULL);
TSET()

TEST(simple_remove)
   char *key = "cat";
   struct trie *trie = trie_create();

   struct trie_iter *iter = trie_insert(trie, key, key);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), key);

   iter = trie_lookup(trie, key);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), key);
   
   char *value = trie_remove(trie, key);
   ASSERT_STR_EQUAL(value, key);
   
   iter = trie_lookup(trie, key);
   ASSERT_EQUAL(iter, NULL);

   trie_destroy(trie, NULL);
TSET()

TEST(insert_null)
   struct trie *root = trie_create();
   struct trie_iter *iter = trie_insert(root, NULL, NULL);
   ASSERT_EQUAL(iter, NULL);
   trie_destroy(root, dealloc);
TSET()

TEST(multi_insert_1)
   struct trie_iter *iter;
   char *value;
   char *key1 = "cat";
   char *key2 = "dog";
   char *key3 = "penguin";
   char *key4 = "mouse";
   char *key5 = "dogma";
   char *key6 = "dogwood";
   char *key7 = "pen";
   char *key8 = "donation";
   struct trie *trie = trie_create();

   iter = trie_insert(trie, key1, key1);
   ASSERT_STR_EQUAL(trie_key(iter), key1);
   ASSERT_STR_EQUAL(trie_value(iter), key1);

   iter = trie_insert(trie, key2, key2);
   ASSERT_STR_EQUAL(trie_key(iter), key2);
   ASSERT_STR_EQUAL(trie_value(iter), key2);

   iter = trie_insert(trie, key3, key3);
   ASSERT_STR_EQUAL(trie_key(iter), key3);
   ASSERT_STR_EQUAL(trie_value(iter), key3);

   iter = trie_insert(trie, key4, key4);
   ASSERT_STR_EQUAL(trie_key(iter), key4);
   ASSERT_STR_EQUAL(trie_value(iter), key4);

   iter = trie_insert(trie, key2, key2);
   ASSERT_EQUAL(iter, NULL);

   iter = trie_lookup(trie, key2);
   ASSERT_STR_EQUAL(trie_key(iter), key2);
   ASSERT_STR_EQUAL(trie_value(iter), key2);

   iter = trie_insert(trie, key5, key5);
   ASSERT_STR_EQUAL(trie_key(iter), &key5[3]);
   ASSERT_STR_EQUAL(trie_value(iter), key5);

   iter = trie_lookup(trie, key2);
   ASSERT_STR_EQUAL(trie_key(iter), key2);
   ASSERT_STR_EQUAL(trie_value(iter), key2);

   iter = trie_lookup(trie, key5);
   ASSERT_STR_EQUAL(trie_key(iter), &key5[3]);
   ASSERT_STR_EQUAL(trie_value(iter), key5);

   iter = trie_insert(trie, key6, key6);
   ASSERT_STR_EQUAL(trie_key(iter), &key6[3]);
   ASSERT_STR_EQUAL(trie_value(iter), key6); 
   
   iter = trie_insert(trie, key7, key7);
   ASSERT_STR_EQUAL(trie_key(iter), key7);
   ASSERT_STR_EQUAL(trie_value(iter), key7);
   
   iter = trie_lookup(trie, key7);
   ASSERT_STR_EQUAL(trie_key(iter), key7);
   ASSERT_STR_EQUAL(trie_value(iter), key7);

   iter = trie_lookup(trie, key3);
   ASSERT_STR_EQUAL(trie_key(iter), &key3[3]);
   ASSERT_STR_EQUAL(trie_value(iter), key3);

   iter = trie_insert(trie, key8, key8);
   ASSERT_STR_EQUAL(trie_key(iter), &key8[2]);
   ASSERT_STR_EQUAL(trie_value(iter), key8);

   iter = trie_lookup(trie, key8);
   ASSERT_STR_EQUAL(trie_key(iter), &key8[2]);
   ASSERT_STR_EQUAL(trie_value(iter), key8);

   iter = trie_lookup(trie, key5);
   ASSERT_STR_EQUAL(trie_key(iter), &key5[3]);
   ASSERT_STR_EQUAL(trie_value(iter), key5);

   value = trie_remove(trie, key7);
   ASSERT_STR_EQUAL(value, key7);
   iter = trie_lookup(trie, key7);
   ASSERT_EQUAL(iter, NULL);

   value = trie_remove(trie, key3);
   ASSERT_STR_EQUAL(value, key3);
   iter = trie_lookup(trie, key3);
   ASSERT_EQUAL(iter, NULL);

   trie_destroy(trie, NULL);
TSET()

TEST(multi_insert_2)
   struct trie_iter *iter;
   char *key;
   char *val = "Value";
   struct trie *trie = trie_create();

   key = "abenar";
   iter = trie_insert(trie, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "ab";
   iter = trie_insert(trie, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "abe";
   iter = trie_insert(trie, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), &key[2]);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   iter = trie_lookup(trie, "abenar");
   ASSERT_STR_EQUAL(trie_key(iter), "nar");
   ASSERT_STR_EQUAL(trie_value(iter), "Value");

   iter = trie_lookup(trie, "ab");
   ASSERT_STR_EQUAL(trie_key(iter), "ab");
   ASSERT_STR_EQUAL(trie_value(iter), "Value");

   iter = trie_lookup(trie, "abe");
   ASSERT_STR_EQUAL(trie_key(iter), "e");
   ASSERT_STR_EQUAL(trie_value(iter), "Value");

   trie_destroy(trie, NULL);
TSET()

TEST(multi_insert_3)
   struct trie_iter *iter;
   char *key;
   char *val = "Value";
   struct trie *root = trie_create();

   key = "abenar";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "ab";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "abekat";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), &key[3]);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "abe";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), &key[2]);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   trie_destroy(root, NULL);
TSET()

TEST(multi_insert_4)
   struct trie_iter *iter;
   char *key;
   char *val = "Value";
   struct trie *root = trie_create();

   key = "fisk";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "abenar";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "abe";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "fiskekutter";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), &key[4]);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   key = "ab";
   iter = trie_insert(root, key, val);
   ASSERT_STR_EQUAL(trie_key(iter), key);
   ASSERT_STR_EQUAL(trie_value(iter), val);

   trie_destroy(root, NULL);
TSET()

TEST_END()
