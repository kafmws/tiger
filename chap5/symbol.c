#include "symbol.h"

#include <stdio.h>
#include <string.h>

#include "table.h"
#include "util.h"

struct S_symbol_ {
  string name;
  S_symbol next;
};

static S_symbol mksymbol(string name, S_symbol next) {
  S_symbol s = checked_malloc(sizeof(*s));
  s->name = name;
  s->next = next;
  return s;
}

#define SIZE 109 /* should be prime */

static S_symbol hashtable[SIZE];

static unsigned int hash(char *s0) {
  unsigned int h = 0;
  char *s;
  for (s = s0; *s; s++) h = h * 65599 + *s;
  return h;
}

static int streq(string a, string b) { return !strcmp(a, b); }

S_symbol S_Symbol(string name) {
  int index = hash(name) % SIZE;
  S_symbol syms = hashtable[index], sym;
  for (sym = syms; sym; sym = sym->next)
    if (streq(sym->name, name)) return sym;
  sym = mksymbol(name, syms);
  hashtable[index] = sym;
  return sym;
}

string S_name(S_symbol sym) {
  assert(sym);
  return sym->name;
}

S_table S_empty(void) { return TAB_empty(); }

void S_enter(S_table t, S_symbol sym, void *value) { TAB_enter(t, sym, value); }

void *S_look(S_table t, S_symbol sym) { return TAB_look(t, sym); }

void *S_update(S_table t, S_symbol sym, void *newval) { return TAB_update(t, sym, newval); }

static struct S_symbol_ marksym = {"<mark>", 0};

static void *markspace = NULL;
void S_beginScope(S_table t) {
  if (!markspace) markspace = checked_malloc(1);
  S_enter(t, &marksym, markspace);
}

void S_endScope(S_table t) {
  S_symbol s;
  do s = TAB_pop(t);
  while (s != &marksym);
}

void S_dump(S_table t, void (*show)(S_symbol sym, void *binding)) {
  // can not be re-entered
  // TAB_dump(t, (void (*)(void *, void *))show);

  // can be re-entered but not entirely by order
  TAB_dump_safe(t, (void (*)(void *, void *))show);
}

// can not be re-entered & orderly access only one level
void S_dump_enhance(S_table t, void (*f)(binder b)) {
  void *k = t->top;
  int index = ((unsigned)k) % TABSIZE;
  binder b = t->table[index];
  if (b == NULL || b->value == markspace) return;
  t->table[index] = b->next;
  t->top = b->prevtop;
  f(b);
  S_dump_enhance(t, f);
  assert(t->top == b->prevtop && t->table[index] == b->next);
  t->top = k;
  t->table[index] = b;
}
