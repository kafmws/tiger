#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

#define SIZE 32768 << 2

static void test_hash_table() {
  hash_table *table = new_hash_table(1, NULL, NULL, NULL, NULL);
  EXPECT_EQ_INT(8, table->table_size);

  char keybase[] = "key";
  char *keys[SIZE] = {0};
  int  *vals[SIZE] = {0};
  int i, j;
  for (i = 0; i < SIZE; i++) {
    char *key = (char *)malloc(sizeof(char) * 10);
    keys[i] = key;
    sprintf(key, "%s%d", keybase, i);
    int *val = (int *)malloc(sizeof(int));
    *val = i;
    vals[i] = val;
    void *p = hash_table_put(table, key, val);
    EXPECT_TRUE(p == NULL);
  }

  for (j = 0; j < SIZE; j++) {
    char key[15];
    strcpy(key, keys[j]);
    void *val = hash_table_remove(table, key);
    EXPECT_EQ_INT(*(int *)val, j);
    free(val);
    val = hash_table_remove(table, key);
    EXPECT_TRUE(val == NULL);
  }
  hash_table_free(table);
}

int main() {
  // _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  test_hash_table();
  return MAIN_RET;
}
