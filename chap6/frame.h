#pragma once
#ifndef _FRAME_H
#define _FRAME_H

#include "temp.h"
#include "tree.h"
#include "util.h"

extern const int F_WORD_SIZE;

typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;

typedef struct F_accessList_ {
  F_access head;
  struct F_accessList_ *tail;
} * F_accessList;

F_frame F_newFrame(Temp_label name, U_boolList formals);

Temp_label F_name(F_frame f);

F_accessList F_formals(F_frame f);

/* allocate a local variable in frame f */
F_access F_allocLocal(F_frame f, bool escape);

/* return the first arg as static link which was added in 'Tr_newLevel' */
F_access staticLinkFormal(F_frame f);

//

Temp_temp F_FP(void);

/* transfer 'F_access' to 'T_exp' */
T_exp F_Exp(F_access access, T_exp framePtr);

T_exp F_ExternalCall(string name, T_expList args);

#endif
