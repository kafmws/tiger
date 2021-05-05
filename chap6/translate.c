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
  l->frame = F_newFrame(name, U_BoolList(TRUE, formals));  // add static link
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

/* condition statement */
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
      Temp_label t = Temp_newlabel(NULL);  // mov val, 1;
      Temp_label f = Temp_newlabel(NULL);  // cjmp e->u.cx.stm;
      doPatch(e->u.cx.trues, t);           // f:
      doPatch(e->u.cx.falses, f);          //  mov val, 0;
      Temp_temp val = Temp_newtemp();      // t:
      return T_Eseq(                       //  val
          T_Move(T_Temp(val), T_Const(1)),
          T_Eseq(e->u.cx.stm,
                 T_Eseq(T_Label(f), T_Eseq(T_Move(T_Temp(val), T_Const(0)),
                                           T_Eseq(T_Label(t), T_temp(val))))));
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
      T_stm stm = T_Cjump(T_ne, e->u.ex, T_Const(0), NULL, NULL);
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
and return frame pointer to access the variable*/
static T_exp traceStaticLink(Tr_level current, Tr_level declare) {
  assert(current && declare);
  T_exp fp = T_Temp(F_FP());
  while (current != declare) {
    fp = F_Exp(staticLinkFormal(current->frame), fp); // access static link
    current = current->parent;
  }
  return fp;
}

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level) {
  return Tr_Ex(F_Exp(access, traceStaticLink(level, access->level)));
}
