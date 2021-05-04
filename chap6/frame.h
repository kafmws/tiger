#pragma once
#ifndef _FRAME_H
#define _FRAME_H

#include "temp.h"
#include "util.h"

typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;

typedef struct F_accessList_ {
  F_access head;
  struct F_accessList_ *tail;
} * F_accessList;

F_frame F_newFrame(Temp_label name, U_boolList formals);

Temp_label F_name(F_frame f);

F_accessList F_formals(F_frame f);

F_access F_allocLocal(F_frame f, bool escape);

#endif
