#include "interpret.h"

#include <stdio.h>
#include <string.h>

#include "util.h"

typedef struct table* Table;

struct table {
  string id;
  int value;
  Table tail;
};

struct IntAndTable {
  int i;
  Table table;
};

static Table interStm(A_stm s, Table t);

static struct IntAndTable interpExp(A_exp e, Table t);

static Table interpExpList(A_expList expList, Table t);

static int lookup(Table t, string key);

static Table update(Table t, string id, int value);

static Table newTable(string id, int value, Table tail);

static Table newTable(string id, int value, Table tail) {
  Table t = checked_malloc(sizeof(*t));
  t->id = id;
  t->value = value;
  t->tail = tail;
  return t;
}

static Table interStm(A_stm s, Table t) {
  switch (s->kind) {
    case A_compoundStm:
      return interStm(s->u.compound.stm2, interStm(s->u.compound.stm1, t));
    case A_assignStm: {
      struct IntAndTable iat = interpExp(s->u.assign.exp, t);
      return update(iat.table, s->u.assign.id, iat.i);
    }
    case A_printStm:
      t = interpExpList(s->u.print.exps, t);
      (void)printf("\n");
      return t;
    default:
      assert(0);
      break;
  }
}

static Table interpExpList(A_expList expList, Table t) {
  switch (expList->kind) {
    case A_pairExpList: {
      struct IntAndTable iat = interpExp(expList->u.pair.head, t);
      (void)printf("%d ", iat.i);
      return interpExpList(expList->u.pair.tail, iat.table);
    }
    case A_lastExpList: {
      struct IntAndTable iat = interpExp(expList->u.last, t);
      (void)printf("%d ", iat.i);
      return iat.table;
    }
    default:
      assert(0);
      break;
  }
}

static struct IntAndTable interpExp(A_exp e, Table t) {
  switch (e->kind) {
    case A_idExp:
      return (struct IntAndTable){lookup(t, e->u.id), t};
    case A_numExp:
      return (struct IntAndTable){e->u.num, t};
    case A_opExp: {
      struct IntAndTable leftIAT = interpExp(e->u.op.left, t);
      struct IntAndTable rightIAT = interpExp(e->u.op.right, leftIAT.table);
      switch (e->u.op.oper) {
        case A_plus:
          return (struct IntAndTable){leftIAT.i + rightIAT.i, rightIAT.table};
        case A_minus:
          return (struct IntAndTable){leftIAT.i - rightIAT.i, rightIAT.table};
        case A_times:
          return (struct IntAndTable){leftIAT.i * rightIAT.i, rightIAT.table};
        case A_div:
          return (struct IntAndTable){leftIAT.i / rightIAT.i, rightIAT.table};
        default:
          assert(0);
          break;
      }
    }
    case A_eseqExp:
      return interpExp(e->u.eseq.exp, interStm(e->u.eseq.stm, t));
    default:
      assert(0);
      break;
  }
}

static int lookup(Table t, string key) {
  while (t != NULL && strcmp(t->id, key) != 0) t = t->tail;
  assert(t);
  return t->value;
}

static Table update(Table t, string id, int value) {
  return newTable(id, value, t);
}

void interpret(A_stm stm) { interStm(stm, NULL); }