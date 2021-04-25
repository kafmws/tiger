#include "semant.h"

#include "absyn.h"
#include "env.h"
#include "errormsg.h"
#include "table.h"

#define BASE_CHECK(pos, actual, expect, env)                           \
  do {                                                                 \
    if ((actual) != (expect)) {                                        \
      S_symbol required = binding2sym((env), (expect));                \
      S_symbol found = binding2sym((env), (actual));                   \
      EM_error((pos), "%s required but %s is found", S_name(required), \
               S_name(found));                                         \
    }                                                                  \
  } while (0)

#define TYPE_CHECK(pos, actual, expect) \
  BASE_CHECK(pos, actual, expect, E_base_tenv());

#define NAME_CHECK(pos, actual, expect) \
  BASE_CHECK(pos, actual, expect, E_base_venv());

/* better error msg */
static void* obj_binding;

static void get_typename(S_symbol sym, void* binding) {
  if (binding == obj_binding) obj_binding = sym;
}

static S_symbol binding2sym(S_table t, void* binding) {
  obj_binding = binding;
  S_dump(t, get_typename);
  return (S_symbol)obj_binding;
}

struct expty transExp(S_table venv, S_table tenv, A_exp exp) {
  if (exp == NULL) {  // No Value expression, like exp:() | ifThenExp's elsee |
                      // procedure
    return expTy(NULL, Ty_Void());
  }
  switch (exp->kind) {
      /*A_varExp, A_nilExp, A_intExp, A_stringExp, A_callExp, A_opExp,
       * A_recordExp, A_seqExp, A_assignExp, A_ifExp, A_whileExp, A_forExp,
       * A_breakExp, A_letExp, A_arrayExp
       * */
    case A_varExp: {
      struct expty e = transVar(venv, tenv, exp->u.var);
      return expTy(NULL, e.ty);
    } break;
    case A_nilExp: {
      return expTy(NULL, Ty_Nil());
    } break;
    case A_intExp: {
      return expTy(NULL, Ty_Int());
    } break;
    case A_stringExp: {
      return expTy(NULL, Ty_String());
    } break;
    case A_callExp: {  // callExp : ID '(' paramList ')'
      // check function name and parameters
      E_enventry funcEntry = (E_enventry)S_look(venv, exp->u.call.func);
      if (funcEntry == NULL || funcEntry->kind != E_funEntry) {
        // EM_error(exp->pos, "%s is not a procedure or a function",
        //          S_name(exp->u.call.func));
        NAME_CHECK(exp->pos, funcEntry, E_enventry_Func());
        // exit(-1);
      }

      A_expList params = exp->u.call.args;
      Ty_tyList paramsTys = funcEntry->u.fun.formals;
      while (params && paramsTys) {
        struct expty e = transExp(venv, tenv, params->head);
        TYPE_CHECK(params->head->pos, e.ty, paramsTys->head);
        params = params->tail;
        paramsTys = paramsTys->tail;
      }
      if (params) {
        EM_error(params->head->pos,
                 "error: too many parameters in procedure/function '%s'",
                 S_name(exp->u.call.func));
      }
      if (paramsTys) {
        EM_error(exp->pos,
                 "error: too few parameters in procedure/function '%s'",
                 S_name(exp->u.call.func));
      }
      return expTy(NULL, funcEntry->u.fun.result);
    } break;
    case A_opExp: {  // opExp
      A_oper op = exp->u.op.oper;
      struct expty left = transExp(venv, tenv, exp->u.op.left);
      struct expty right = transExp(venv, tenv, exp->u.op.right);
      switch (op) {
        case A_plusOp:
        case A_minusOp:
        case A_timesOp:
        case A_divideOp: {  // arithmetic operation
          TYPE_CHECK(exp->u.op.left->pos, left.ty, Ty_Int());
          TYPE_CHECK(exp->u.op.right->pos, right.ty, Ty_Int());
        } break;
        case A_gtOp:
        case A_geOp:
        case A_ltOp:
        case A_leOp: {  // logic operation for int/string
          if (left.ty != Ty_Int() && left.ty != Ty_String()) {
            EM_error(exp->u.op.left->pos, "int/string required but %s is found",
                     S_name(binding2sym(tenv, left.ty)));
          }
          if (right.ty != Ty_Int() && left.ty != Ty_String()) {
            EM_error(exp->u.op.right->pos,
                     "int/string required but %s is found",
                     S_name(binding2sym(tenv, right.ty)));
          }
        } break;
        case A_eqOp:
        case A_neqOp: {
          // logic op for int/string/record/array
          if (left.ty != Ty_Int() && left.ty != Ty_String() &&
              left.ty->kind != Ty_record && left.ty->kind != Ty_array) {
            EM_error(exp->u.op.left->pos,
                     "int/string/record/array required but %s is found",
                     S_name(binding2sym(tenv, left.ty)));
          }
          if (right.ty != Ty_Int() && left.ty != Ty_String() &&
              right.ty->kind != Ty_record && right.ty->kind != Ty_array) {
            EM_error(exp->u.op.right->pos,
                     "int/string/record/array required but %s is found",
                     S_name(binding2sym(tenv, right.ty)));
          }
        } break;
        default:
          assert(0); /*unknown operator*/
          break;
      }
      return expTy(NULL, Ty_Int());
    } break;
    case A_recordExp: {  // recordExp : type-id '{' id=exp {,id=exp} '}'
      // field names and types should be consistent with declaration
      Ty_ty recordType = (Ty_ty)S_look(tenv, exp->u.record.typ);
      if (recordType == NULL || recordType->kind != Ty_array) {
        TYPE_CHECK(exp->pos, recordType, Ty_RecordType());
      }
      Ty_fieldList fieldsDec = recordType->u.record;
      A_efieldList efields = exp->u.record.fields;
      while (efields && fieldsDec) {
        struct expty curFieldExpTy = transExp(venv, tenv, efields->head->exp);
        TYPE_CHECK(efields->head->exp->pos, curFieldExpTy.ty,
                   fieldsDec->head->ty);
        if (efields->head->name != fieldsDec->head->name) {
          EM_error((efields->head->exp->pos),
                   "field '%s' required but '%s' is found",
                   S_name(fieldsDec->head->name), S_name(efields->head->name));
        }
        efields = efields->tail;
        fieldsDec = fieldsDec->tail;
      }
      if (efields) {
        EM_error(efields->head->exp->pos,
                 "error: too many fields given for type '%s'",
                 S_name(binding2sym(tenv, recordType)));
      }
      if (fieldsDec) {
        EM_error(exp->pos, "error: too few fields given for type '%s'",
                 S_name(binding2sym(tenv, recordType)));
      }
      return expTy(NULL, recordType);
    } break;
    case A_seqExp: {
      // seqExp must not NULL
      A_expList expList = exp->u.seq;
      while (expList) {
        struct expty e = transExp(venv, tenv, expList->head);
        expList = expList->tail;
        if (!expList) return expTy(NULL, e.ty);
      }
    } break;
    case A_assignExp: {
      struct expty varE = transVar(venv, tenv, exp->u.assign.var);
      struct expty valE = transExp(venv, tenv, exp->u.assign.exp);
      TYPE_CHECK(exp->pos, valE.ty, varE.ty);
      return expTy(NULL, varE.ty);
    } break;
    case A_ifExp: {
      struct expty test, then, elsee;
      test = transExp(venv, tenv, exp->u.iff.test);
      TYPE_CHECK(exp->u.iff.test->pos, test.ty, Ty_Int());
      if (test.exp)  // test true
        then = transExp(venv, tenv, exp->u.iff.then);
      else
        elsee = transExp(venv, tenv, exp->u.iff.elsee);
      if (exp->u.iff.elsee) {  // if then else
        if (then.ty != elsee.ty) {
          string thenType = S_name(binding2sym(then.ty));
          string elseType = S_name(binding2sym(elsee.ty));
          EM_error(exp->u.iff.then->pos,
                   "then clause get type '%s' while elsee clause get type '%s'",
                   thenType, elseType);
        }
        return expTy(NULL, then.ty);
      } else {  // if then
        return expTy(NULL, Ty_Void());
      }
    } break;
    case A_whileExp: {
      while (true) {
        struct expty test = transExp(venv, tenv, exp->u.whilee.test);
        TYPE_CHECK(exp->u.whilee.test->pos, test.ty, Ty_Int());
        if (test.exp) {  // test true
          struct expty body = transExp(venv, tenv, exp->u.whilee.body);
        } else
          return expTy(NULL, Ty_Void());
      }
    } break;
    case A_forExp: {
      struct expty lo = transExp(venv, tenv, exp->u.forr.lo);
      struct expty hi = transExp(venv, tenv, exp->u.forr.hi);
      TYPE_CHECK(exp->u.forr.lo->pos, lo.ty, Ty_Int());
      TYPE_CHECK(exp->u.forr.hi->pos, hi.ty, Ty_Int());
      if (lo.exp > hi.exp) exp->u.forr.escape = 1;  // jump loop

      // loop variable declaration
      S_beginScope(venv);
      S_enter(venv, exp->u.forr.var, lo.exp);

      // can not assign to exp->u.forr.var
      int i = 0;
      for (i = 0; i < 1; i++) {  // lo to hi
        struct expty body = transExp(venv, tenv, exp->u.forr.body);
      }
      S_endScope(venv);
      return expTy(NULL, Ty_Void());
    } break;
    case A_breakExp: {
      return expTy(NULL, Ty_Void());
    } break;
    case A_letExp: {
      A_decList decList = exp->u.let.decs;
      S_beginScope(tenv);
      S_beginScope(venv);
      while (decList) {
        transDec(venv, tenv, decList->head);
        decList = decList->tail;
      }
      struct expty body = transExp(venv, tenv, exp->u.let.body);
      S_endScope(tenv);
      S_endScope(venv);
      return expTy(NULL, body.ty);
    } break;
    case A_arrayExp: {
      Ty_ty arrayTy = (Ty_ty)S_look(tenv, exp->u.array.typ);
      if (arrayTy == NULL || arrayTy->kind != Ty_array) {
        TYPE_CHECK(exp->pos, arrayTy, Ty_ArrayType());
      }
      struct expty size = transExp(venv, tenv, exp->u.array.size);
      TYPE_CHECK(exp->u.array.size->pos, size.ty, Ty_Int());
      if (size.exp) {  // check array size
        EM_error(exp->u.array.size->pos, "illegal array size");
      }
      struct expty init = transExp(venv, tenv, exp->u.array.init);
      TYPE_CHECK(exp->u.array.init->pos, init.ty, arrayTy);
      return expTy(NULL, arrayTy);
    } break;
    default:
      assert(0); /*unknown exp*/
      break;
  }
}

struct expty transVar(S_table venv, S_table tenv, A_var var) {
  switch (var->kind) {
    case A_simpleVar: {
      E_enventry entry = (E_enventry)S_look(venv, var->u.simple);
      if (entry == NULL || entry->kind != E_varEntry) {
        NAME_CHECK(var->pos, entry, E_enventry_Var());
      }
      return expTy(NULL, entry->u.var.ty);
    } break;
    case A_fieldVar: {
      /*
      //E_enventry entry = (E_enventry)S_look(venv, var->u.field.var);
      // if (entry == NULL || entry->kind != E_varEntry) {  // if not a var
      //   NAME_CHECK(var->pos, entry, E_enventry_Record());
      // } else if (entry->u.var.ty->kind != Ty_record) {
      //   TYPE_CHECK(var->pos, entry->u.var.ty, Ty_RecordType());
      // } else {  // a record var, check field name
      //   Ty_fieldList recordTy = entry->u.var.ty->u.record;
      //   while (recordTy) {
      //     if (recordTy->head->name == var->u.field.sym)
      //       return expTy(NULL, recordTy->head->ty);
      //     recordTy = recordTy->tail;
      //   }
      //   EM_error();
      // }
      */

      // for a var(A_var), can not check from name
      struct expty v = transVar(venv, tenv, var->u.field.var);
      if (v.ty->kind != Ty_record) {
        TYPE_CHECK(var->pos, v.ty, Ty_RecordType());
        // exit(-1);
      }else{
        
      }
        EM_error(var->pos, "variable %s of type '%s' has no field '%s'",
                 S_name(var->u.field.var), S_name(), S_name());
      return expTy(NULL, entry->u.var.ty);
    } break;
    case A_subscriptVar:

      break;

    default:
      break;
  }
}

void transDec(S_table venv, S_table tenv, A_dec dec) {}

Ty_ty transTy(S_table tenv, A_ty ty) {}
