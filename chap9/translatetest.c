#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"
#include "semant.h"
#include "symbol.h"
#include "util.h"

extern A_exp absyn_root;

void print_frag(F_frag f) {
  static int strCnt = 0;
  static int procCnt = 0;

  switch (f->kind) {
    case F_stringFrag:
      printf("string_%d:{label=%s, str=\"%s\"}\n", strCnt++,
             S_name(f->u.stringg.label), f->u.stringg.str);
      break;
    case F_procFrag:
      printf("proc_%d\n", procCnt++);
      break;
    default:
      assert(0);
      break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: a.out filename\n");
    exit(0);
  }

  int indent = 0;
  for (int i = 1; i < argc; i++) {
    A_exp program = parse(argv[i]);
    // argv[i][strlen(argv[i]) - 4] = 0;
    FILE *fp;
    if (program) {
      /* check grammar */
      // fp = fopen("out", "w");
      // pr_exp(fp, program, indent);
      // fprintf(fp, "\n");
      // pr_exp(stdout, program, indent);
      // puts("");
      // fclose(fp);

      /* check sement */
      F_fragList frags = SEM_transProg(absyn_root);

      if (EM_anyErrors) {
        fprintf(stderr, "semant error in program\n");
      }

      // show_types();
      // show_names();
      int fragsCnt = 0;
      while (frags) {
        fragsCnt++;
        // print_frag(frags->head);
        frags = frags->tail;
      }
      printf("%d\n", fragsCnt);

    } else {
      fprintf(stderr, "grammar error in program\n");
      return -1;
    }
  }
  return 0;
}
