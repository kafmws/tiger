#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"
#include "symbol.h"
#include "util.h"

int main(int argc, char *argv[]) {
  if(argc < 2) {
    printf("usage: a.out filename\n");
    exit(0);
  }
  for (int i = 1; i < argc; i++) {
    A_exp program = parse(argv[i]);
    argv[i][strlen(argv[i]) - 4] = 0;
    FILE *fp = fopen(argv[i], "w");
    if (program) {
      pr_exp(fp, program, 2);
      fclose(fp);
    }
    else return -1;
  }
  return 0;
}
