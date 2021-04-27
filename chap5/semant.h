#pragma once
#ifndef _SEMANT_H
#define _SEMANT_H

#include "absyn.h"
#include "types.h"
#include "symbol.h"

typedef void * Tr_exp;

struct expty {
    Tr_exp exp;
    Ty_ty ty;
};

/* struct expty in the stack frame */
struct expty expTy(Tr_exp exp, Ty_ty ty);

struct expty transVar(S_table venv, S_table tenv, A_var var);
struct expty transExp(S_table venv, S_table tenv, A_exp exp);
void         transDec(S_table venv, S_table tenv, A_dec dec);
       Ty_ty transTy (              S_table tenv,  A_ty ty );

void  SEM_transProg(A_exp exp);

void show_names();
void show_types();

#endif
