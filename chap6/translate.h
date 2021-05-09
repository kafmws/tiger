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

typedef struct Tr_expList_ *Tr_expList;

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

#endif
