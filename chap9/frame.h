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

/* transfer 'F_access' to 'T_exp' */
T_exp F_Exp(F_access access, T_exp framePtr);

T_exp F_ExternalCall(string name, T_expList args);

// maintain the frame
T_stm F_procEntryExit1(F_frame frame, T_stm stm);

T_stm F_procEntryExit3(F_frame frame, T_stm stm);

//-------------------------- Fragment
typedef struct F_frag_ *F_frag;

struct F_frag_ {
  enum { F_stringFrag, F_procFrag } kind;
  union {
    struct {
      Temp_label label;
      string str;
    } stringg;
    struct {
      T_stm body;
      F_frame frame;
    } proc;
  } u;
};

F_frag F_StringFrag(Temp_label label, string str);

F_frag F_ProcFrag(T_stm body, F_frame frame);

typedef struct F_fragList_{
  F_frag head;
  struct F_fragList_ *tail;
} * F_fragList;

F_fragList F_FragList(F_frag head, F_fragList tail);

// abstract general register

// Frame pointer register
Temp_temp F_FP(void);

// Return value register
Temp_temp F_RV(void);

#endif
