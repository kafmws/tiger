#pragma once
#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "temp.h"
#include "util.h"

typedef struct Tr_level_ *Tr_level;

Tr_level Tr_outermost(void);

/* allocate a local variable in layer 'level' */
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);

typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_expList_ {
  Tr_exp head;
  struct Tr_expList_ *tail;
} * Tr_expList;

Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail);

typedef struct Tr_access_ *Tr_access;

Tr_access Tr_allocLocal(Tr_level level, bool escape);

typedef struct Tr_accessList_ {
  Tr_access head;
  struct Tr_accessList_ *tail;
} * Tr_accessList;

Tr_accessList Tr_formals(Tr_level level);

/*          translate functions          */

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);

Tr_exp Tr_FieldVar(Tr_exp var, int fieldCnt);

Tr_exp Tr_SubscriptVar(Tr_exp var, Tr_exp exp);

Tr_exp Tr_NilExp();

Tr_exp Tr_IntExp(int i);

Tr_exp Tr_StringExp(string s);

Tr_exp Tr_OpPlusExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpMinusExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpTimesExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpDivideExp(Tr_exp left, Tr_exp right);

Tr_exp Tr_OpGtExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpGeExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpLtExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpLeExp(Tr_exp left, Tr_exp right);

Tr_exp Tr_OpEqExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_OpNeqExp(Tr_exp left, Tr_exp right);

Tr_exp Tr_IfThenExp(Tr_exp testExp, Tr_exp thenExp);
Tr_exp Tr_IfThenElseExp(Tr_exp testExp, Tr_exp thenExp, Tr_exp elseExp);

Tr_exp Tr_RecordExp(Tr_expList fields);

Tr_exp Tr_SeqExp(Tr_expList reverseSeqList);

Tr_exp Tr_AssignExp(Tr_exp varE, Tr_exp valE);

Tr_exp Tr_ArrayExp(Tr_exp sizeExp, Tr_exp initExp);

Tr_exp Tr_WhileExp(Tr_exp testExp, Tr_exp bodyExp, Temp_label done);

Tr_exp Tr_ForExp(Tr_access loopVar, Tr_exp loExp, Tr_exp hiExp, Tr_exp bodyExp,
                 Temp_label done);

Tr_exp Tr_BreakExp(Temp_label done);

Tr_exp Tr_CallExp(Tr_level curLev, Tr_level funcLev, Temp_label name,
                  Tr_expList args);

Tr_exp Tr_InitExp(Tr_access access, Tr_exp initExp);

Tr_exp Tr_Nop();

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);

F_fragList Tr_getResult();

#endif
