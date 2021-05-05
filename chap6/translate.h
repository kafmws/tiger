#pragma once
#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "temp.h"
#include "util.h"

typedef struct Tr_access_ *Tr_access;

typedef struct Tr_level_ *Tr_level;

typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_accessList_ {
  Tr_access head;
  struct Tr_accessList_ *tail;
} * Tr_accessList;

Tr_level Tr_outermost(void);

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);

Tr_accessList Tr_formals(Tr_level level);

/* allocate a local variable in layer 'level' */
Tr_access Tr_allocLocal(Tr_level level, bool escape);

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);

#endif
