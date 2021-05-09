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

Tr_accessList Tr_formals(Tr_level level) {}

struct Tr_level_ {
  F_frame frame;
  Tr_level parent;
};

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

struct Tr_expList_ {
  Tr_exp head;
  struct Tr_expList_ *tail;
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
      Temp_label t = Temp_newlabel(NULL);  // mov val, 1;
      Temp_label f = Temp_newlabel(NULL);  // cjmp e->u.cx.stm;
      doPatch(e->u.cx.trues, t);           // f:
      doPatch(e->u.cx.falses, f);          //  mov val, 0;
      Temp_temp val = Temp_newtemp();      // t:
      return T_Eseq(                       //  val
          T_Move(T_Temp(val), T_Const(1)),
          T_Eseq(e->u.cx.stm,
                 T_Eseq(T_Label(f), T_Eseq(T_Move(T_Temp(val), T_Const(0)),
                                           T_Eseq(T_Label(t), T_Temp(val))))));
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
  return Tr_Ex(F_Exp(access, traceStaticLink(lev, access->level)));
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
  // F_string(label, s);
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
  // Temp_label t = Temp_newlabel(NULL);
  Temp_label f = Temp_newlabel(NULL);
  // doPatch(test.falses, t); // not necessary
  doPatch(test.falses, f);
  // assume jmp clause only jmp false                    // jmpF test f
  T_stm then = toNx(thenExp);                            // then
  T_stm stm = T_Seq(test.stm, T_Seq(then, T_Label(f)));  // f:
  return Tr_Nx(stm);
}

Tr_exp Tr_IfThenElseExp(Tr_exp testExp, Tr_exp thenExp, Tr_exp elseExp) {
  struct Cx test = toCx(testExp);
  // Temp_label t = Temp_newlabel(NULL);
  Temp_label f = Temp_newlabel(NULL);
  Temp_label end = Temp_newlabel(NULL);
  // doPatch(test.falses, t);
  doPatch(test.falses, f);

  if (thenExp->kind == Tr_nx) {  // no value ifThenElse
    assert(elseExp->kind == Tr_nx);
    T_stm then = unNx(thenExp);
    T_stm elsee = unNx(elseExp);
    // assume jmp clause only jmp false
    T_stm stm = T_Seq(  // jmpF test.stm f
        test.stm,       // then
        T_Seq(          // jmp end
            then,       // f:
            T_Seq(T_Jump(T_Name(end), Temp_LabelList(end, NULL)),    // elsee
                  T_Seq(T_Label(f), T_Seq(elsee, T_Label(end))))));  // end:
    return Tr_Nx(stm);
  } else if (thenExp->kind == Tr_cx || elseExp->kind == Tr_cx) {
    struct Cx thenCx = toCx(thenExp);

  } else if (thenExp->kind == Tr_ex && elseExp->kind == Tr_ex) {
    T_exp then = unEx(thenExp);
    T_exp elsee = unEx(elseExp);
    Temp_temp r = Temp_newtemp();

    T_exp exp = T_Eseq(
        T_Eseq(
            test.stm,                        // jmpF test.stm f
            T_Eseq(T_Move(T_Temp(r), then),  // mov r then
                   T_Eseq(T_Jump(T_Name(end),
                                 Temp_LabelList(end, NULL)),      // jmp end
                          T_Eseq(T_Label(f),                      // f:
                                 T_Seq(T_Move(T_Temp(r), elsee),  // mov r elsee
                                       T_Label(end)))))),         // end:
        T_Temp(r));
    return Tr_Ex(exp);
  } else
    assert(0);
}

Tr_exp Tr_CallExp(Tr_level lev, Temp_label name, Tr_expList args) {
  T_expList argsList = NULL;
  if (args) {  // transfer Tr_expList to T_expList
    argsList = T_ExpList(NULL, NULL);
    T_expList tail = argsList;
    while (args) {
      tail->head = args->head;
      tail->tail = NULL;
      if (args->tail) {
        tail = T_ExpList(NULL, NULL);
      }
      args = args->tail;
    }
  }
  T_exp callExp = T_Call(toEx(T_Label(label)), argsList);
  return Tr_Ex(callExp);
}
