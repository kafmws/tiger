/*
 * tree.h - Definitions for intermediate representation (IR) trees.
 *
 */

#pragma once
#ifndef _TREE_H
#define _TREE_H

#include "temp.h"

typedef struct T_stm_ *T_stm;
typedef struct T_exp_ *T_exp;
typedef struct T_expList_ *T_expList;
struct T_expList_ {
  T_exp head;
  T_expList tail;
};
typedef struct T_stmList_ *T_stmList;
struct T_stmList_ {
  T_stm head;
  T_stmList tail;
};

typedef enum {
  T_plus,
  T_minus,
  T_mul,
  T_div,
  T_and,
  T_or,
  T_lshift,
  T_rshift,
  T_arshift,
  T_xor
} T_binOp;

typedef enum {
  T_eq,
  T_ne,
  T_lt,
  T_gt,
  T_le,
  T_ge,
  T_ult,
  T_ule,
  T_ugt,
  T_uge
} T_relOp;

struct T_stm_ {
  enum { T_SEQ, T_LABEL, T_JUMP, T_CJUMP, T_MOVE, T_EXP } kind;
  union {
    struct {
      T_stm left, right;
    } SEQ;
    Temp_label LABEL;
    struct {
      T_exp exp;
      Temp_labelList jumps;
    } JUMP;
    struct {
      T_relOp op;
      T_exp left, right;
      Temp_label true, false;
    } CJUMP;
    struct {
      T_exp dst, src;
    } MOVE;
    T_exp EXP;
  } u;
};

struct T_exp_ {
  enum { T_BINOP, T_MEM, T_TEMP, T_ESEQ, T_NAME, T_CONST, T_CALL } kind;
  union {
    struct {
      T_binOp op;
      T_exp left, right;
    } BINOP;
    T_exp MEM;
    Temp_temp TEMP;
    struct {
      T_stm stm;
      T_exp exp;
    } ESEQ;
    Temp_label NAME;
    int CONST;
    struct {
      T_exp fun;
      T_expList args;
    } CALL;
  } u;
};

T_expList T_ExpList(T_exp head, T_expList tail);
T_stmList T_StmList(T_stm head, T_stmList tail);

/* a statement followed by another. */
T_stm T_Seq(T_stm left, T_stm right);

/* define 'l' as current machine code address, like a label in assembly. */
/* simply says: define a label use 'T_Lable', jmp to a label use 'T_Name' */
T_stm T_Label(Temp_label l);

/* transfer control flow to address 'e', labels are all addresses that 'e' may
 * equal to. Its typical application is 'switch' statement. */
T_stm T_Jump(T_exp e, Temp_labelList labels);

/* condition transfer. evaluate left and right, and judge opExp,
 * if true jump to label_true else jump to label_false.
 */
T_stm T_Cjump(T_relOp op, T_exp left, T_exp right, Temp_label true,
              Temp_label false);

/* two formals :
 *  MOVE(Temp_temp t(e1), T_exp e) : evaluate e and store in t
 *  MOVE(MEM(e1), T_exp e2) : evaluate e1 get address a and store e2 to a
 */
T_stm T_Move(T_exp e1, T_exp e2);

/* evaluate exp but produce no value */
T_stm T_Exp(T_exp e);

//                  produce value                 //

/* e1 op e2, exp, evaluate e1 then evaluate e2 */
T_exp T_Binop(T_binOp op, T_exp e1, T_exp e2);

/* the 'wordSize' bytes begin from arg 'e', exp
   represents 'write' only in the 'MOVE''s left part,
   represents 'read' in other situation */
T_exp T_Mem(T_exp e);

/* temporary variable, exp */
T_exp T_Temp(Temp_temp t);

/* evaluate stm first, then  */
T_exp T_Eseq(T_stm s, T_exp e);

/* generate a symbol constant equivalent to 'label' in assembly, exp */
/* as an argument, like 'jmp', 'call''s target or represents a string */
/* simply says: define a label use 'T_Lable', jmp to a label use 'T_Name' */
T_exp T_Name(Temp_label label);

/* integer, exp */
T_exp T_Const(int i);

/* evaluate f before evaluate l, exp */
T_exp T_Call(T_exp f, T_expList l);

T_relOp T_notRel(T_relOp op);  /* a op b    ==     not(a notRel(op) b)  */
T_relOp T_commute(T_relOp op); /* a op b    ==    b commute(op) a       */

#endif
