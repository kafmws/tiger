/*
 * table.c - Functions to manipulate generic tables.
 * Copyright (c) 1997 Andrew W. Appel.
 */

#include "table.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#define TABSIZE 127

typedef struct binder_ *binder;
struct binder_ {
  void *key;
  void *value;
  binder next;
  void *prevtop;
};
struct TAB_table_ {
  binder table[TABSIZE];
  void *top;
  int cnt;
};

static binder Binder(void *key, void *value, binder next, void *prevtop) {
  binder b = checked_malloc(sizeof(*b));
  b->key = key;
  b->value = value;
  b->next = next;
  b->prevtop = prevtop;
  return b;
}

TAB_table TAB_empty(void) {
  TAB_table t = checked_malloc(sizeof(*t));
  int i;
  t->top = NULL;
  for (i = 0; i < TABSIZE; i++) t->table[i] = NULL;
  t->cnt = 0;
  return t;
}

/* The cast from pointer to integer in the expression
 *   ((unsigned)key) % TABSIZE
 * may lead to a warning message.  However, the code is safe,
 * and will still operate correctly.  This line is just hashing
 * a pointer value into an integer value, and no matter how the
 * conversion is done, as long as it is done consistently, a
 * reasonable and repeatable index into the table will result.
 */

void TAB_enter(TAB_table t, void *key, void *value) {
  int index;
  assert(t && key);
  index = ((unsigned)key) % TABSIZE;
  t->table[index] = Binder(key, value, t->table[index], t->top);
  t->top = key;
  t->cnt++;
}

void *TAB_look(TAB_table t, void *key) {
  int index;
  binder b;
  assert(t && key);
  index = ((unsigned)key) % TABSIZE;
  for (b = t->table[index]; b; b = b->next)
    if (b->key == key) return b->value;
  return NULL;
}

void *TAB_pop(TAB_table t) {
  void *k;
  binder b;
  int index;
  assert(t);
  k = t->top;
  assert(k);
  index = ((unsigned)k) % TABSIZE;
  b = t->table[index];
  assert(b);
  t->table[index] = b->next;
  t->top = b->prevtop;
  t->cnt--;
  return b->key;
}

void *TAB_update(TAB_table t, void *key, void *newval) {
  int index;
  binder b;
  assert(t && key);
  index = ((unsigned)key) % TABSIZE;
  for (b = t->table[index]; b; b = b->next)
    if (b->key == key) {
      void *old = b->value;
      b->value = newval;
      return old;
    }
  assert(0); // not found
}

//※不可重入  递归函数依赖于 t->top, 在 TAB_dump 中开启另一轮 TAB_dump 可能出错
void TAB_dump(TAB_table t, void (*show)(void *key, void *value)) {  // orderly
  void *k = t->top;
  int index = ((unsigned)k) % TABSIZE;
  binder b = t->table[index];
  if (b == NULL) return;
  t->table[index] = b->next;
  t->top = b->prevtop;
  show(b->key, b->value);
  TAB_dump(t, show);
  assert(t->top == b->prevtop && t->table[index] == b->next);
  t->top = k;
  t->table[index] = b;
}

int TAB_size(TAB_table t) {
  // int i, cnt = 0;
  // for(i = 0;i<TABSIZE;i++){
  //   binder b = t->table[i];
  //   if (b) {
  //     b = b->next;
  //     cnt++;
  //   }
  // }
  return t->cnt;
}

// can be re-entered but not entirely by order
void TAB_dump_safe(TAB_table t, void (*show)(void *key, void *value)) {
  int i;
  for (i = 0; i < TABSIZE; i++) {
    binder b = t->table[i];
    if (b) {
      show(b->key, b->value);
      b = b->next;
    }
  }
}

// by order
void TAB_dump_safe_orderly(TAB_table t, void (*show)(void *key, void *value)) {
  void **temp = checked_malloc(sizeof(*temp) * TAB_size(t));
  // to do : save the order to the temp from 'top'
  free(temp);
}
