#include "translate.h"

#include "frame.h"
#include "tree.h"

struct Tr_access_ {
  Tr_level level;
  F_access access;
};

Tr_access Tr_Access(Tr_level level, F_access access) {
  Tr_access a = checked_malloc(sizeof(*a));
  a->access = access;
  a->level = level;
  return a;
}

static Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail) {
  Tr_accessList list = checked_malloc(sizeof(*list));
  list->head = head;
  list->tail = tail;
  return list;
}

struct Tr_level_ {
  F_frame frame;
  Tr_level parent;
};

// return 'Tr_access' of parameters in 'level'
Tr_accessList Tr_formals(Tr_level level) {
  F_accessList accessList = F_formals(level->frame);
  Tr_accessList list = NULL, listTail = NULL;
  if (accessList) list = listTail = Tr_AccessList(NULL, NULL);
  while (accessList) {
    list->head = Tr_Access(level, accessList->head);
    accessList = accessList->tail;
    if (accessList) {
      listTail->tail = Tr_AccessList(NULL, NULL);
      listTail = listTail->tail;
    }
  }
  return list;
}

Tr_level Tr_outermost() {
  static Tr_level outer;
  if (!outer) {
    outer = checked_malloc(sizeof(*outer));
    outer->parent = NULL;
    // infact the library functions implement in C functions
    outer->frame = NULL;
  }
  return outer;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals) {
  Tr_level l = checked_malloc(sizeof(*l));
  l->frame =
      F_newFrame(name, U_BoolList(TRUE, formals));  // add static link pos
  l->parent = parent;
  return l;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape) {
  return Tr_Access(level, F_allocLocal(level->frame, escape));
}

/* patchList start */

typedef struct patchList_ {
  Temp_label *head; /* pointer to patch */
  struct patchList_ *tail;
} * patchList;

static patchList PatchList(Temp_label *head, patchList tail) {
  patchList p = checked_malloc(sizeof(*p));
  p->head = head;
  p->tail = tail;
  return p;
}

void doPatch(patchList p, Temp_label label) {
  while (p) {
    *p->head = label;
    p = p->tail;
  }
}

patchList joinPatch(patchList first, patchList second) {
  if (!first) return second;
  while (first->tail) first = first->tail;
  first->tail = second;
  return first;  //?
}

/* patchList end */

/* condition STATEMENT, NOT value */
struct Cx {
  patchList trues;  /* true patch list */
  patchList falses; /* false patch list */
  T_stm stm;
};

struct Tr_exp_ {
  enum { Tr_ex, Tr_nx, Tr_cx } kind;
  union {
    T_exp ex;
    T_stm nx;
    struct Cx cx;
  } u;
};

Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail) {
  Tr_expList list = checked_malloc(sizeof(*list));
  list->head = head;
  list->tail = tail;
  return list;
}

static Tr_exp Tr_Ex(T_exp ex) {
  Tr_exp exp = checked_malloc(sizeof(*exp));
  exp->kind = Tr_ex;
  exp->u.ex = ex;
  return exp;
}

static Tr_exp Tr_Nx(T_stm nx) {
  Tr_exp exp = checked_malloc(sizeof(*exp));
  exp->kind = Tr_nx;
  exp->u.nx = nx;
  return exp;
}

static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm) {
  Tr_exp exp = checked_malloc(sizeof(*exp));
  exp->kind = Tr_cx;
  exp->u.cx = (struct Cx){trues, falses, stm};
  return exp;
}

static T_exp toEx(Tr_exp e) {
  switch (e->kind) {
    case Tr_ex:
      return e->u.ex;
    case Tr_nx:
      if (e->u.nx->kind == T_EXP) {
        return e->u.nx->u.EXP;
      }
      // return T_Eseq(e->u.nx, T_Const(0));
      assert(0);  // no value get
      break;
    case Tr_cx: {
      // transfer bool to integer
      Temp_label t = Temp_newlabel("true");   // mov r, 1;
      Temp_label f = Temp_newlabel("false");  // cjmp e->u.cx.stm;
      doPatch(e->u.cx.trues, t);              // f:
      doPatch(e->u.cx.falses, f);             //  mov r, 0;
      Temp_temp r = Temp_newtemp();           // t:
      return T_Eseq(                          //  r
          T_Move(T_Temp(r), T_Const(1)),
          T_Eseq(e->u.cx.stm,
                 T_Eseq(T_Label(f), T_Eseq(T_Move(T_Temp(r), T_Const(0)),
                                           T_Eseq(T_Label(t), T_Temp(r))))));
    }
    default:
      assert(0);  // unknow Tr_exp
      break;
  }
}

static T_stm toNx(Tr_exp e) {
  switch (e->kind) {
    case Tr_ex:
      return T_Exp(e->u.ex);
    case Tr_nx:
      return e->u.nx;
    case Tr_cx: {
      assert(e->u.cx.trues && e->u.cx.falses);
      return e->u.cx.stm;
    }
    default:
      assert(0);  // unknow Tr_exp
      break;
  }
}

static struct Cx toCx(Tr_exp e) {
  switch (e->kind) {
    case Tr_ex: {
      if (e->u.ex->kind == T_CONST) {  // special process for const0 & const1
        if (e->u.ex->u.CONST == 0) {
          ////
        } else if (e->u.ex->u.CONST == 1) {
          ////
        }
      }
      T_stm stm = T_Cjump(T_ne, toEx(e), T_Const(0), NULL, NULL);
      patchList t = PatchList(&stm->u.CJUMP.true, NULL);
      patchList f = PatchList(&stm->u.CJUMP.false, NULL);
      return (struct Cx){.trues = t, .falses = f, .stm = stm};
    }
    case Tr_nx:
      assert(0);  // impossible
    case Tr_cx:
      return e->u.cx;
    default:
      assert(0);  // unknow Tr_exp
      break;
  }
}

static F_fragList frags;
static F_fragList fragsTail;

void addFrag(F_frag f) {
  if (!fragsTail) {
    frags = fragsTail = F_FragList(f, NULL);
  } else {
    fragsTail->tail = F_FragList(f, NULL);
    fragsTail = fragsTail->tail;
  }
}

//          translate functions         //

/* trace static link from current level to declare level which the var def in.
and return frame pointer to access the variable */
static T_exp traceStaticLink(Tr_level current, Tr_level declare) {
  assert(current && declare);
  T_exp fp = T_Temp(F_FP());
  while (current != declare) {
    fp = F_Exp(staticLinkFormal(current->frame), fp);  // access static link
    current = current->parent;
  }
  return fp;
}

Tr_exp Tr_SimpleVar(Tr_access access, Tr_level lev) {
  return Tr_Ex(F_Exp(access->access, traceStaticLink(lev, access->level)));
}

Tr_exp Tr_FieldVar(Tr_exp var, int fieldCnt) {
  assert(var->kind == Tr_ex);
  T_exp offset = T_Binop(T_mul, T_Const(fieldCnt), T_Const(F_WORD_SIZE));
  return Tr_Ex(T_Mem(T_Binop(T_plus, toEx(var), offset)));
}

Tr_exp Tr_SubscriptVar(Tr_exp var, Tr_exp exp) {
  assert(var->kind == Tr_ex && exp->kind == Tr_ex);
  //  a[i]    ->    a + i * stepSize
  T_exp offset = T_Binop(T_mul, toEx(exp), T_Const(F_WORD_SIZE));
  // return address as left value, may it will be part of another left value
  return Tr_Ex(T_Mem(T_Binop(T_plus, toEx(var), offset)));
}

Tr_exp Tr_NilExp() {
  /* NULL */
  return Tr_Ex(T_Const(0));
}

Tr_exp Tr_IntExp(int i) { return Tr_Ex(T_Const(i)); }

Tr_exp Tr_StringExp(string s) {
  Temp_label label = Temp_newlabel(s);
  addFrag(F_StringFrag(label, s));
  return Tr_Ex(T_Name(label));
}

/* that I think translate.c should not
   include "absyn.h" except 'A_oper' */

// would never be used, because uminus exp has been replaced by '0 - '
// Tr_exp Tr_OpUminusExp(Tr_exp left){
//   return Tr_Ex(T_Binop(T_minus, T_Const(0), toEx(left)));
// }

Tr_exp Tr_OpPlusExp(Tr_exp left, Tr_exp right) {
  return Tr_Ex(T_Binop(T_plus, toEx(left), toEx(right)));
}

Tr_exp Tr_OpMinusExp(Tr_exp left, Tr_exp right) {
  return Tr_Ex(T_Binop(T_minus, toEx(left), toEx(right)));
}

Tr_exp Tr_OpTimesExp(Tr_exp left, Tr_exp right) {
  return Tr_Ex(T_Binop(T_mul, toEx(left), toEx(right)));
}

Tr_exp Tr_OpDivideExp(Tr_exp left, Tr_exp right) {
  return Tr_Ex(T_Binop(T_div, toEx(left), toEx(right)));
}

/* relationship operator corresponds condition STATEMENT,
   NOT ANY actual value, no bool/int value is produced
   while it is implyed by executing different branches */

Tr_exp Tr_OpGtExp(Tr_exp left, Tr_exp right) {
  T_stm stm;
  T_exp l = toEx(left), r = toEx(right);
  if (l->kind == T_CONST)
    stm = T_Cjump(T_gt, l, r, NULL, NULL);
  else if (l->kind == T_NAME) {
    T_exp res =
        F_ExternalCall("stringCompare", T_ExpList(l, T_ExpList(r, NULL)));
    stm = T_Cjump(T_gt, res, T_Const(0), NULL, NULL);
  } else
    assert(0);
  patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
  patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
  return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_OpGeExp(Tr_exp left, Tr_exp right) {
  T_stm stm;
  T_exp l = toEx(left), r = toEx(right);
  if (l->kind == T_CONST)
    stm = T_Cjump(T_ge, l, r, NULL, NULL);
  else if (l->kind == T_NAME) {
    T_exp res =
        F_ExternalCall("stringCompare", T_ExpList(l, T_ExpList(r, NULL)));
    stm = T_Cjump(T_ge, res, T_Const(0), NULL, NULL);
  } else
    assert(0);
  patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
  patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
  return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_OpLtExp(Tr_exp left, Tr_exp right) {
  T_stm stm;
  T_exp l = toEx(left), r = toEx(right);
  if (l->kind == T_CONST)
    stm = T_Cjump(T_lt, l, r, NULL, NULL);
  else if (l->kind == T_NAME) {
    T_exp res =
        F_ExternalCall("stringCompare", T_ExpList(l, T_ExpList(r, NULL)));
    stm = T_Cjump(T_lt, res, T_Const(0), NULL, NULL);
  } else
    assert(0);
  patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
  patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
  return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_OpLeExp(Tr_exp left, Tr_exp right) {
  T_stm stm;
  T_exp l = toEx(left), r = toEx(right);
  if (l->kind == T_CONST)
    stm = T_Cjump(T_le, l, r, NULL, NULL);
  else if (l->kind == T_NAME) {
    T_exp res =
        F_ExternalCall("stringCompare", T_ExpList(l, T_ExpList(r, NULL)));
    stm = T_Cjump(T_le, res, T_Const(0), NULL, NULL);
  } else
    assert(0);
  patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
  patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
  return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_OpEqExp(Tr_exp left, Tr_exp right) {
  T_stm stm;
  T_exp l = toEx(left), r = toEx(right);
  if (l->kind == T_CONST || l->kind == T_MEM)
    // T_MEM, compare reference (address) for record of array variable
    stm = T_Cjump(T_eq, l, r, NULL, NULL);
  else if (l->kind == T_NAME) {
    T_exp res = F_ExternalCall("stringEqual", T_ExpList(l, T_ExpList(r, NULL)));
    stm = T_Cjump(T_eq, res, T_Const(1), NULL, NULL);
  } else
    assert(0);
  patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
  patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
  return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_OpNeqExp(Tr_exp left, Tr_exp right) {
  T_stm stm;
  T_exp l = toEx(left), r = toEx(right);
  if (l->kind == T_CONST || l->kind == T_MEM)
    // T_MEM, compare reference (address) for record of array variable
    stm = T_Cjump(T_ne, l, r, NULL, NULL);
  else if (l->kind == T_NAME) {
    T_exp res = F_ExternalCall("stringEqual", T_ExpList(l, T_ExpList(r, NULL)));
    stm = T_Cjump(T_eq, res, T_Const(0), NULL, NULL);
  } else
    assert(0);
  patchList trues = PatchList(&stm->u.CJUMP.true, NULL);
  patchList falses = PatchList(&stm->u.CJUMP.false, NULL);
  return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_IfThenExp(Tr_exp testExp, Tr_exp thenExp) {
  struct Cx test = toCx(testExp);
  Temp_label t = Temp_newlabel("t");
  Temp_label f = Temp_newlabel("f");
  doPatch(test.falses, t);  // not necessary
  doPatch(test.falses, f);
  // assume jmp clause only jmp false

  T_stm then = toNx(thenExp);  // jmpF test f
  T_stm stm =                  // then
      T_Seq(test.stm, T_Seq(T_Label(t), T_Seq(then, T_Label(f))));  // f:
  return Tr_Nx(stm);
}

Tr_exp Tr_IfThenElseExp(Tr_exp testExp, Tr_exp thenExp, Tr_exp elseExp) {
  struct Cx test = toCx(testExp);
  Temp_label t = Temp_newlabel("t");
  Temp_label f = Temp_newlabel("f");
  Temp_label end = Temp_newlabel("end");
  doPatch(test.falses, t);  // not necessary
  doPatch(test.falses, f);

  if (thenExp->kind == Tr_nx) {  // no value ifThenElse
    assert(elseExp->kind == Tr_nx);
    T_stm then = toNx(thenExp);
    T_stm elsee = toNx(elseExp);
    // assume jmp clause only jmp false
    T_stm stm = T_Seq(   // jmpF test.stm f
        test.stm,        // then
        T_Seq(           // jmp end
            T_Label(t),  // f:
            T_Seq(       // elsee
                then,    // end:
                T_Seq(T_Jump(T_Name(end), Temp_LabelList(end, NULL)),
                      T_Seq(T_Label(f), T_Seq(elsee, T_Label(end)))))));
    return Tr_Nx(stm);
  }
  //  else if (thenExp->kind == Tr_cx || elseExp->kind == Tr_cx) {
  //   struct Cx thenCx = toCx(thenExp);
  // }
  // else if (thenExp->kind == Tr_ex && thenExp->kind == Tr_ex)
  else if (thenExp->kind == Tr_ex || thenExp->kind == Tr_cx) {
    T_exp then = toEx(thenExp);
    T_exp elsee = toEx(elseExp);
    Temp_temp r = Temp_newtemp();

    T_exp exp = T_Eseq(
        T_Seq(
            test.stm,                              // jmpF test.stm f
            T_Seq(T_Label(t),                      // mov r then
                  T_Seq(T_Move(T_Temp(r), then),   // jmp end
                        T_Seq(T_Jump(T_Name(end),  // f:
                                     Temp_LabelList(end, NULL)),  // mov r elsee
                              T_Seq(T_Label(f),                   // end:
                                    T_Seq(T_Move(T_Temp(r), elsee),
                                          T_Label(end))))))),
        T_Temp(r));
    return Tr_Ex(exp);
  } else
    assert(0);
}

// fields list has no type, becase of all type are the same length in Tiger.
Tr_exp Tr_RecordExp(Tr_expList fields) {
  int fieldCnt = 0;
  T_stm stm = NULL;
  Temp_temp r = Temp_newtemp();  // store the address of the record var
  if (fields) {
    stm = T_Move(
        T_Mem(T_Binop(T_plus, T_Temp(r), T_Const(fieldCnt * F_WORD_SIZE))),
        toEx(fields->head));
    fieldCnt++;
    fields = fields->tail;
    while (fields) {
      // MOVE(MEM(BINOP(addr, cnt*F_WORD_SIZE)), exp)
      stm = T_Seq(stm, T_Move(T_Mem(T_Binop(T_plus, T_Temp(r),
                                            T_Const(fieldCnt * F_WORD_SIZE))),
                              toEx(fields->head)));
      fieldCnt++;
      fields = fields->tail;
    }
  }
  T_exp recordAddr = F_ExternalCall(
      "allocRecord", T_ExpList(T_Const(fieldCnt * F_WORD_SIZE), NULL));
  /* the code above is a little interesting, we discribe how to fill the fields
     before we really allocate actual memory to the variable. that's the magic
     of the INDIRECTION of the address 'r'. we use it while it isn't real. */
  return Tr_Ex(T_Eseq(T_Seq(T_Move(T_Temp(r), recordAddr), stm), T_Temp(r)));
}

Tr_exp Tr_SeqExp(Tr_expList reverseSeqList) {
  T_exp exp = NULL;
  while (reverseSeqList) {
    exp = (exp ? T_Eseq(toNx(reverseSeqList->head), exp)
               : toEx(reverseSeqList->head));
    reverseSeqList = reverseSeqList->tail;
  }
  return reverseSeqList ? Tr_Ex(exp) : Tr_Nop();
}

Tr_exp Tr_AssignExp(Tr_exp varE, Tr_exp valE) {
  return Tr_Nx(T_Move(toEx(varE), toEx(valE)));
}

Tr_exp Tr_ArrayExp(Tr_exp sizeExp, Tr_exp initExp) {
  assert(sizeExp->kind == Tr_ex &&
         (initExp->kind == Tr_ex ||
          initExp->kind == Tr_nx));  // Tr_nx for record array
  T_exp array = F_ExternalCall(
      "initArray", T_ExpList(toEx(sizeExp), T_ExpList(toEx(initExp), NULL)));
  return Tr_Ex(array);
}

Tr_exp Tr_WhileExp(Tr_exp testExp, Tr_exp bodyExp, Temp_label done) {
  assert(done);
  Temp_label whilee = Temp_newlabel("while");
  Temp_label loop = Temp_newlabel("loop");
  /* 'done' label generate before 'Tr_WhileExp' is invoked,
     because the 'break' in the body needs it. */
  // Temp_label done = Temp_newlabel("done");
  struct Cx test = toCx(testExp);
  doPatch(test.trues, loop);
  doPatch(test.falses, done);
  T_stm stm = T_Seq(
      T_Label(whilee),                                // whilee:
      T_Seq(test.stm,                                 // jmpF test done
            T_Seq(T_Label(loop),                      // t:(not necessary)
                  T_Seq(toNx(bodyExp),                // body
                        T_Seq(T_Jump(T_Name(whilee),  // jmp whilee
                                     Temp_LabelList(whilee, NULL)),  // done:
                              T_Label(done))))));
  return Tr_Nx(stm);
}

Tr_exp Tr_ForExp(Tr_access loopVar, Tr_exp loExp, Tr_exp hiExp, Tr_exp bodyExp,
                 Temp_label done) {
  T_exp lo = toEx(loExp);
  T_exp hi = toEx(hiExp);
  // assume lo, hi is instant value.
  T_exp var = F_Exp(loopVar->access, T_Temp(F_FP()));
  Temp_label loop = Temp_newlabel("loop");
  Temp_label inc = Temp_newlabel("inc");

  T_stm stm = T_Seq(
      T_Move(var, lo),                                        // mov var, lo
      T_Seq(                                                  // jle var hi loop
          T_Cjump(T_le, var, hi, loop, done),                 // jmp done
          T_Seq(T_Label(loop),                                // loop:
                T_Seq(                                        //   body
                    toNx(bodyExp),                            // jlt var hi inc
                    T_Seq(T_Cjump(T_lt, var, hi, inc, done),  // jmp done
                          T_Seq(T_Label(inc),                 // inc var
                                T_Seq(T_Move(var,             // jmp loop
                                             T_Binop(T_plus, var, T_Const(1))),
                                      T_Seq(T_Jump(T_Name(loop),  // done:
                                                   Temp_LabelList(loop, NULL)),
                                            T_Label(done)))))))));
  return Tr_Nx(stm);
}

Tr_exp Tr_CallExp(Tr_level curLev, Tr_level funcLev, Temp_label name,
                  Tr_expList args) {
  T_expList argsList = NULL;
  if (args) {  // transfer Tr_expList to T_expList
    argsList = T_ExpList(NULL, NULL);
    T_expList tail = argsList;
    while (args) {
      tail->head = toEx(args->head);
      if (args->tail) {
        tail->tail = T_ExpList(NULL, NULL);
        tail = tail->tail;
      }
      args = args->tail;
    }
  }

  /* add static link to the outermost
   * 1. call the direct inner layer function:
   *             pass current framePtr as static link
   * 2. call itself or the same layer functions:
   *             pass itself static link
   * 3. call the outer layer function:
   *             trace outer layer frame from the static link in its frame
   */

  T_exp staticLink = NULL;
  if (funcLev->parent == curLev) {  // call direct inner layer function
    staticLink = T_Temp(F_FP());
  } else if (funcLev ==
             curLev) {  // self-recursive or invoke the same layer func
    staticLink = F_Exp(staticLinkFormal(curLev->frame), T_Temp(F_FP()));
  } else {  // call the outer layer function
    Tr_level lev = curLev;
    staticLink = T_Temp(F_FP());
    while (lev != funcLev /*->parent*/) {  // find the static outer layer
      staticLink = F_Exp(staticLinkFormal(lev->frame), staticLink);
      lev = lev->parent;
    }
  }
  if (staticLink) {
    argsList = T_ExpList(staticLink, argsList);
  }

  T_exp callExp = T_Call(T_Name(name), argsList);
  return Tr_Ex(callExp);
}

Tr_exp Tr_BreakExp(Temp_label done) {
  return Tr_Nx(T_Jump(T_Name(done), Temp_LabelList(done, NULL)));
}

Tr_exp Tr_InitExp(Tr_access access, Tr_exp initExp) {
  return Tr_Nx(T_Move(F_Exp(access->access, T_Temp(F_FP())), toEx(initExp)));
}

Tr_exp Tr_Nop() { return Tr_Ex(T_Const(0)); }

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals) {}

F_fragList Tr_getResult() { return frags; }
