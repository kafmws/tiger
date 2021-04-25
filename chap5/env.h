#pragma once
#ifndef _ENV_H
#define _ENV_H

#include "symbol.h"
#include "types.h"

typedef struct E_enventry_ *E_enventry;
struct E_enventry_ {
  enum { E_varEntry, E_funEntry } kind;
  union {
    struct {
      Ty_ty ty;
    } var;
    struct {
      Ty_tyList formals;
      Ty_ty result;
    } fun;
  } u;
};

E_enventry E_enventry_Var();
E_enventry E_enventry_Func();
E_enventry E_enventry_Array();
E_enventry E_enventry_Record();

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);

S_table E_base_tenv(void);  // type table          symbol -> type-def
S_table E_base_venv(void);  // identifier table    symbol -> type-info

#endif
