#include "env.h"
#include <stdio.h>

E_enventry E_VarEntry(Ty_ty ty) {
  E_enventry e = (E_enventry) checked_malloc(sizeof(*e));
  e->kind = E_varEntry;
  e->u.var.ty = ty;
  return e;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result) {
  E_enventry e = (E_enventry)checked_malloc(sizeof(*e));
  e->kind = E_funEntry;
  e->u.fun.formals = formals;
  e->u.fun.result = result;
  return e;
}

static S_table tyEnv;
S_table E_base_tenv(void) {
  if (!tyEnv) {
    tyEnv = S_empty();
    S_enter(tyEnv, S_Symbol("int"),     Ty_Int());
    /* one more blank for the real name 'void'
     * add to the behind for print
     */
    S_enter(tyEnv, S_Symbol("nil "),    Ty_Nil());
    S_enter(tyEnv, S_Symbol("void "),   Ty_Void());
    S_enter(tyEnv, S_Symbol("string"),  Ty_String());

    /*for better error msg*/
    S_enter(tyEnv, S_Symbol("type name"),     Ty_TypeType());
    S_enter(tyEnv, S_Symbol("array type"),    Ty_ArrayType());
    S_enter(tyEnv, S_Symbol("record type"),   Ty_RecordType());
    S_enter(tyEnv, S_Symbol("unknown type"),  NULL);
  }
  return tyEnv;
}

static S_table idEnv;
S_table E_base_venv(void) {
  if (!idEnv) {
    idEnv = S_empty();
    S_enter(idEnv, S_Symbol("print"),      E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Void()));
    S_enter(idEnv, S_Symbol("flush"),      E_FunEntry(NULL, Ty_Void()));
    S_enter(idEnv, S_Symbol("getchar"),    E_FunEntry(NULL, Ty_String()));
    S_enter(idEnv, S_Symbol("ord"),        E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int()));
    S_enter(idEnv, S_Symbol("chr"),        E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_String()));
    S_enter(idEnv, S_Symbol("size"),       E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int()));
    S_enter(idEnv, S_Symbol("substring"),  E_FunEntry(Ty_TyList(Ty_String(), Ty_TyList(Ty_Int(), Ty_TyList(Ty_Int(), NULL))), Ty_String()));
    S_enter(idEnv, S_Symbol("concat"),     E_FunEntry(Ty_TyList(Ty_String(), Ty_TyList(Ty_String(), NULL)), Ty_String()));
    S_enter(idEnv, S_Symbol("not"),        E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Int()));
    S_enter(idEnv, S_Symbol("exit"),       E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Void()));

    /*for better error msg*/
    S_enter(idEnv, S_Symbol("array name"),          E_enventry_Array());
    S_enter(idEnv, S_Symbol("record name"),         E_enventry_Record());
    S_enter(idEnv, S_Symbol("variable name"),       E_enventry_Var());
    S_enter(idEnv, S_Symbol("function name"),       E_enventry_Func());
    S_enter(idEnv, S_Symbol("unknown identifier"),  NULL);
  }
  return idEnv;
}

static E_enventry entryfunc;
E_enventry E_enventry_Func() {
  if(!entryfunc){
    entryfunc = E_FunEntry(NULL, NULL);
  }
  return entryfunc;
}

static E_enventry entryarray;
E_enventry E_enventry_Array() {
  if (!entryarray) {
    entryarray = E_VarEntry(NULL);
  }
  return entryarray;
}

static E_enventry entryrecord;
E_enventry E_enventry_Record() {
  if (!entryrecord) {
    entryrecord = E_VarEntry(NULL);
  }
  return entryrecord;
}

static E_enventry entryvar;
E_enventry E_enventry_Var() {
  if (!entryvar) {
    entryvar = E_VarEntry(NULL);
  }
  return entryvar;
}
