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
    fp = fopen("out", "w");
    if (program) {
      /* check grammar */
      pr_exp(fp, program, indent);
      fprintf(fp, "\n");
      // pr_exp(stdout, program, indent);
      // puts("");
      fclose(fp);

      /* check sement */
      SEM_transProg(absyn_root);

      if (EM_anyErrors) {
        fprintf(stderr, "semant error in program\n");
      }

      // show_types();
      // show_names();

    } else {
      fprintf(stderr, "grammar error in program\n");
      return -1;
    }
  }
  return 0;
}
