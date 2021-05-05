/* frame implementation for x86 architecture */

#include <stdio.h>

#include "frame.h"
#include "tree.h"

const int F_WORD_SIZE = 4;

struct F_access_ {
  enum {
    In_Frame,
    In_Reg,
  } kind;
  union {
    int offset;    /* in frame */
    Temp_temp reg; /* in register */
  } u;
};

static F_access InFrame(int offset) {
  F_access a = checked_malloc(sizeof(*a));
  a->kind = In_Frame;
  a->u.offset = offset;
  return a;
}

static F_access InReg(Temp_temp reg) {
  F_access a = checked_malloc(sizeof(*a));
  a->kind = In_Reg;
  a->u.reg = reg;
  return a;
}

static F_accessList F_AccessList(F_access head, F_accessList tail) {
  F_accessList list = checked_malloc(sizeof(*list));
  list->head = head;
  list->tail = tail;
  return list;
}

struct F_frame_ {
  Temp_label name;      /* lable */
  F_accessList formals; /* args. but the first is static link which was added in
                        'Translate' module, while 'Frame' knows nothing. */
  unsigned int argsCnt;
  F_accessList locals;   /* temporary variable cyclic list, tail pointer */
  unsigned int localCnt; /* counter of temporary variable in frame */

  /* void *instruction; */
  /* something maintaining a frame, like "[%esp] <- %ebp; %ebp <- %esp
                                          %esp -= frameSize" */
};

F_frame F_newFrame(Temp_label name, U_boolList formals) {
  F_frame f = checked_malloc(sizeof(*f));
  f->name = name;
  f->formals = NULL;
  f->argsCnt = 0;
  f->locals = NULL;
  f->localCnt = 0;

  // assume all args are escape and put them in the frame.
  if (formals) {
    // one for return address, one for old ebp(?)
    int offset = F_WORD_SIZE << 2;
    f->formals = checked_malloc(sizeof(*f->formals));
    F_accessList accessList = f->formals;

    while (formals) {
      accessList->head = InFrame(offset);
      f->argsCnt++;
      accessList->tail =
          formals->tail ? checked_malloc(sizeof(*accessList)) : NULL;
      offset += F_WORD_SIZE;
      formals = formals->tail;
    }
  }

  return f;
}

Temp_label F_name(F_frame f) { return f->name; }

F_accessList F_formals(F_frame f) { return f->formals; }

/* allocate a local variable in frame f */
F_access F_allocLocal(F_frame f, bool escape) {
  F_access newLocal;
  if (escape)
    // stack extend to lower addr
    newLocal = InFrame((++f->localCnt) * -F_WORD_SIZE);
  else
    newLocal = InReg(Temp_newtemp());
  if (f->locals == NULL) {
    f->locals = F_AccessList(newLocal, NULL);
    f->locals->tail = f->locals;  // cyclic list
  } else {                        // add to tail, and update the tail pointer
    f->locals->tail = F_AccessList(newLocal, f->locals->tail);
  }
  return newLocal;
}

F_access staticLinkFormal(F_frame f) { return f->formals->head; }

//

Temp_temp F_FP(void) {}

/* transfer 'F_access' to 'T_exp', access a variable */
T_exp F_Exp(F_access access, T_exp framePtr) {
  if (access->kind == In_Frame) {
    // MEM(Binop(plus, ebp, offset))
    return T_Mem(T_Binop(T_plus, framePtr, T_Const(access->u.offset)));
  } else if (access->kind == In_Reg) {
    return T_Temp(access->u.reg);
  }
  assert(0);
}
