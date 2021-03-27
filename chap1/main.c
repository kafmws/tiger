#include <stdio.h>

#include "interpret.h"
#include "prog1.h"
#include "slp.h"
#include "util.h"

int max(int a, int b) { return a > b ? a : b; }

int maxargs(A_stm stm);

static int maxargs_exp(A_exp exp) {
  switch (exp->kind) {
    case A_eseqExp:
      return max(maxargs(exp->u.eseq.stm), maxargs_exp(exp->u.eseq.exp));
    case A_opExp:
      return max(maxargs_exp(exp->u.op.left), maxargs_exp(exp->u.op.right));
    case A_idExp:
    case A_numExp:
      return 0;
    default:
      assert(0);
      break;
  }
}

static int maxargs_expList(A_expList expList) {
  switch (expList->kind) {
    case A_pairExpList:
      return max(maxargs_exp(expList->u.pair.head),
                 maxargs_expList(expList->u.pair.tail));
    case A_lastExpList:
      return maxargs_exp(expList->u.last);
    default:
      assert(0);
      break;
  };
}

static int cnt_expList(A_expList expList) {
  switch (expList->kind) {
    case A_lastExpList:
      return 1;
    case A_pairExpList:
      return 1 + cnt_expList(expList->u.pair.tail);
    default:
      assert(0);
      break;
  }
}

int maxargs(A_stm stm) {
  switch (stm->kind) {
    case A_compoundStm:
      return max(maxargs(stm->u.compound.stm1), maxargs(stm->u.compound.stm2));
    case A_assignStm:
      return maxargs_exp(stm->u.assign.exp);
    case A_printStm:
      return max(cnt_expList(stm->u.print.exps),
                 maxargs_expList(stm->u.print.exps));
    default:
      assert(0);
      break;
  }
}

int main() {
  A_stm prog_2 = prog();
  A_stm prog_4 = prog4();
  printf("%d\n", maxargs(prog_2));
  printf("%d\n", maxargs(prog_4));
  interpret(prog_2);
  printf("\n");
  interpret(prog_4);
}