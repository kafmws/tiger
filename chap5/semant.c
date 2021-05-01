#include "semant.h"

#include <stdio.h>
#include <stdlib.h>

#include "absyn.h"
#include "env.h"
#include "errormsg.h"
#include "table.h"

int generic_list_cnt(void* head, void* next) {
  if (!head) return 0;
  return generic_list_cnt(*(void**)next,
                          (char*)*(void**)next + ((char*)next - (char*)head)) +
         1;
}

static Ty_ty actualTy(Ty_ty ty) {
  if (!ty) return NULL;
  while (ty->kind == Ty_name) ty = ty->u.name.ty;
  return ty;
}

static bool ty_eq(Ty_ty tt, Ty_ty yy) {
  assert(tt || yy);
  if (!tt || !yy) return FALSE;
  return actualTy(tt)->kind == Ty_record && actualTy(yy)->kind == Ty_nil ||
         actualTy(yy)->kind == Ty_record && actualTy(tt)->kind == Ty_nil ||
         tt == yy;
}

#define TYPE_CHECK(pos, actual, expect)                                    \
  do {                                                                     \
    /*assert((expect));*/                                                  \
    if (!ty_eq(actualTy((actual)), actualTy((expect)))) {                  \
      S_symbol required = binding2sym(E_base_tenv(), (expect));            \
      S_symbol found = binding2sym(E_base_tenv(), (actual));               \
      EM_error((pos), "'%s' required but '%s' is found", S_name(required), \
               S_name(found));                                             \
      (actual) = (expect); /* assuming it's right for going on */          \
      /*exit(-1); */                                                       \
    }                                                                      \
  } while (0)

#define NAME_CHECK(pos, actual, expect)                                    \
  do {                                                                     \
    /*assert((expect));*/                                                  \
    if (((actual) != (expect))) {                                          \
      S_symbol required = binding2sym(E_base_venv(), (expect));            \
      S_symbol found = binding2sym(E_base_venv(), (actual));               \
      EM_error((pos), "'%s' required but '%s' is found", S_name(required), \
               S_name(found));                                             \
      (actual) = (expect); /* assuming it's right for going on */          \
      /*exit(-1); */                                                       \
    }                                                                      \
  } while (0)

// only actual is nil is permitted
#define TYPE_CHECK_WITH_NIL_RECORD(pos, actual, expect) \
  do {                                                  \
    if (actualTy((expect))->kind != Ty_record ||        \
        actualTy((actual))->kind != Ty_nil)             \
      TYPE_CHECK(pos, actual, expect);                  \
  } while (0)

#define IS_ARRAY_TYPE(pos, actual)                                  \
  do {                                                              \
    if ((actual) == NULL || actualTy((actual))->kind != Ty_array) { \
      TYPE_CHECK(pos, actual, Ty_ArrayType());                      \
    }                                                               \
  } while (0)

#define IS_RECORD_TYPE(pos, actual)                                  \
  do {                                                               \
    if ((actual) == NULL || actualTy((actual))->kind != Ty_record) { \
      TYPE_CHECK(pos, actual, Ty_RecordType());                      \
    }                                                                \
  } while (0)

#define IS_TYPE(pos, actual)                  \
  do {                                        \
    if ((actual) == NULL) {                   \
      TYPE_CHECK(pos, actual, Ty_TypeType()); \
    }                                         \
  } while (0)

#define IS_FUNCTION_NAME(pos, actual)                       \
  do {                                                      \
    if ((actual) == NULL || (actual)->kind != E_funEntry) { \
      NAME_CHECK(pos, actual, E_enventry_Func());           \
    }                                                       \
  } while (0)

#define TYPE_CHECK_NAME(pos, var_ty, expect_entry)                       \
  do {                                                                   \
    S_symbol required_name = binding2sym(E_base_venv(), (expect_entry)); \
    S_symbol found_ty = binding2sym(E_base_tenv(), (var_ty));            \
    EM_error((pos), "%s required but variable of type '%s' is found",    \
             S_name(required_name), S_name(found_ty));                   \
  } while (0)

/* better error msg */
static void* obj_binding;  // multiple useage

static void get_typename(S_symbol sym, void* binding) {
  if (binding == obj_binding) obj_binding = sym;
}

static S_symbol binding2sym(S_table t, void* binding) {
  obj_binding = binding;
  S_dump(t, get_typename);
  if (obj_binding == binding) {
    // assert(0);  // not found
  }
  return (S_symbol)obj_binding;
}

/* for debug */
static void show_name(S_symbol sym, void* binding) {
  if (!binding) {
    printf("%20s:%s\n", S_name(sym), "NULL");
    return;
  }
  E_enventry entry = (E_enventry)binding;
  switch (entry->kind) {
    case E_varEntry: {
      printf("%20s:%s\n", S_name(sym),
             S_name(binding2sym(E_base_tenv(), entry->u.var.ty)));
    } break;
    case E_funEntry: {
      printf("%20s", S_name(sym));
      printf(":%-12s ",
             S_name(binding2sym(E_base_tenv(), entry->u.fun.result)));
      Ty_tyList tyList = entry->u.fun.formals;
      printf("(");
      if (tyList) {
        while (tyList) {
          printf("%s", S_name(binding2sym(E_base_tenv(), tyList->head)));
          tyList = tyList->tail;
          if (tyList) printf(", ");
        }
      }
      printf(")\n");
    } break;
    default:
      assert(0);  // unknown entry
      break;
  }
}

static void show_type(S_symbol sym, void* binding) {
  if (!binding) {
    printf("%20s:%s\n", S_name(sym), "NULL");
    return;
  }
  Ty_ty ty = (Ty_ty)binding;
  printf("%20s:", S_name(sym));
  switch (ty->kind) {
    case Ty_record: {
      printf("{");
      Ty_fieldList fieldsDec = ty->u.record;
      while (fieldsDec) {
        printf("%s:%s", S_name(fieldsDec->head->name),
               S_name(binding2sym(E_base_tenv(),
                                  fieldsDec->head->ty)));  // actualTy(
        fieldsDec = fieldsDec->tail;
        if (fieldsDec) printf(", ");
      }
      printf("}");
    } break;
    case Ty_nil: {
      printf("nil");
    } break;
    case Ty_int: {
      printf("int");
    } break;
    case Ty_string: {
      printf("string");
    } break;
    case Ty_array: {
      printf("array of %s", S_name(binding2sym(E_base_tenv(), ty->u.array)));
    } break;
    case Ty_name: {
      printf("type alias of %s", S_name(ty->u.name.sym));
    } break;
    case Ty_void:  // void & typetype
      printf("void");
      break;
    // case Ty_void + 1: {
    //   printf("typetype");
    // } break;
    default:
      assert(0);  // unknown type
      break;
  }
  printf("\n");
}

void show_names() {
  printf("\t====\tname table\t====\n\n");
  S_dump(E_base_venv(), show_name);
  printf("\n");
}

void show_types() {
  printf("\t====\ttype table\t====\n\n");
  S_dump(E_base_tenv(), show_type);
  printf("\n");
}

static void convertToRealTy(binder b) {
  // use 'obj_binding' save the Ty_name type which wait to convert

  // substitute Ty_name with its real ty
  Ty_ty objTy = (Ty_ty)obj_binding;
  if (b->value == obj_binding) {  // check itself
    b->value = objTy->u.name.ty;
  }
  // check the same layer type's constituent (include itself)
  Ty_ty ty = (Ty_ty)b->value;
  if (ty->kind == Ty_name) ty = ty->u.name.ty;
  // recursive defines only
  switch (ty->kind) {
    case Ty_record: {
      Ty_fieldList fieldList = ty->u.record;
      while (fieldList) {
        if (fieldList->head->ty == objTy)
          fieldList->head->ty = objTy->u.name.ty;
        fieldList = fieldList->tail;
      }
    } break;
    case Ty_array: {
      if (ty->u.array == objTy) ty->u.array = objTy->u.name.ty;
    } break;
    case Ty_name: {
      if (ty->u.name.ty == objTy) ty->u.name.ty = objTy->u.name.ty;
    } break;
    case Ty_nil:
    case Ty_int:
    case Ty_void:
    case Ty_string:
      // nothing should do
      break;
    default:
      assert(0);  // unknown type
      break;
  }
}

/* for breakExp to know where to break */
static A_exp forOrWhile;

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
      IS_FUNCTION_NAME(exp->pos, funcEntry);
      if ((funcEntry) == NULL || (funcEntry)->kind != E_funEntry)
        return expTy(NULL, NULL);

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
          if (actualTy(left.ty) != Ty_Int() &&
              actualTy(left.ty) != Ty_String()) {
            EM_error(exp->u.op.left->pos, "int/string required but %s is found",
                     S_name(binding2sym(tenv, left.ty)));
          }
          if (actualTy(right.ty) != Ty_Int() &&
              actualTy(right.ty) != Ty_String()) {
            EM_error(exp->u.op.right->pos,
                     "int/string required but %s is found",
                     S_name(binding2sym(tenv, right.ty)));
          }
        } break;
        case A_eqOp:
        case A_neqOp: {
          // logic op for int/string/record(Nil)/array
          if (actualTy(left.ty)->kind == Ty_record &&
                  actualTy(right.ty) == Ty_Nil() ||
              actualTy(right.ty)->kind == Ty_record &&
                  actualTy(left.ty) == Ty_Nil()) {
          } else {
            if (actualTy(left.ty) != Ty_Int() &&
                actualTy(left.ty) != Ty_String() &&
                actualTy(left.ty)->kind != Ty_record &&
                actualTy(left.ty)->kind != Ty_array) {
              EM_error(exp->u.op.left->pos,
                       "binary operator int/string/record/array required but "
                       "%s is found",
                       S_name(binding2sym(tenv, left.ty)));
            }
            if (actualTy(right.ty) != Ty_Int() &&
                actualTy(right.ty) != Ty_String() &&
                actualTy(right.ty)->kind != Ty_record &&
                actualTy(right.ty)->kind != Ty_array) {
              EM_error(exp->u.op.right->pos,
                       "binary operator int/string/record/array required but "
                       "%s is found",
                       S_name(binding2sym(tenv, right.ty)));
            }
            TYPE_CHECK(exp->u.op.right->pos, right.ty, left.ty);
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
      IS_RECORD_TYPE(exp->pos, recordType);
      Ty_fieldList fieldsDec = actualTy(recordType)->u.record;
      A_efieldList efields = exp->u.record.fields;
      while (efields && fieldsDec) {
        struct expty curFieldExpTy = transExp(venv, tenv, efields->head->exp);
        TYPE_CHECK(efields->head->exp->pos, curFieldExpTy.ty,
                   fieldsDec->head->ty);
        if (efields->head->name != fieldsDec->head->name) {
          EM_error((efields->head->exp->pos),
                   "field '%s' required but field '%s' is found",
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
      // loop variable cannot be assigned
      if (forOrWhile && forOrWhile->kind == A_forExp) {
        // can not judge from name. like for ... do let decs in exp end
        // someone in decs has the same name with loop variable

        // exp->u.assign.var ==
      }

      struct expty varE = transVar(venv, tenv, exp->u.assign.var);
      struct expty valE = transExp(venv, tenv, exp->u.assign.exp);
      TYPE_CHECK_WITH_NIL_RECORD(exp->pos, valE.ty, varE.ty);
      return expTy(NULL, varE.ty);
    } break;
    case A_ifExp: {
      struct expty test, then, elsee;
      test = transExp(venv, tenv, exp->u.iff.test);
      TYPE_CHECK(exp->u.iff.test->pos, test.ty, Ty_Int());
      then = transExp(venv, tenv, exp->u.iff.then);
      elsee = transExp(venv, tenv, exp->u.iff.elsee);
      if (exp->u.iff.elsee) {  // if then else
        if (ty_eq(actualTy(then.ty), actualTy(elsee.ty)) == FALSE) {
          string thenType = S_name(binding2sym(tenv, then.ty));
          string elseType = S_name(binding2sym(tenv, elsee.ty));
          EM_error(exp->u.iff.then->pos,
                   "then clause get type '%s' while elsee clause get type '%s'",
                   thenType, elseType);
        }

        /* impelement ifStm */
        // if(test is true)
        return expTy(NULL, then.ty);
      } else {  // if then
        return expTy(NULL, Ty_Void());
      }
    } break;
    case A_whileExp: {
      A_exp old_forOrWhile = forOrWhile;
      forOrWhile = exp;
      struct expty test = transExp(venv, tenv, exp->u.whilee.test);
      TYPE_CHECK(exp->u.whilee.test->pos, test.ty, Ty_Int());
      test.exp = test.ty;  // for active the loop
      while (test.exp) {   // test true
        struct expty body = transExp(venv, tenv, exp->u.whilee.body);
        test = transExp(venv, tenv, exp->u.whilee.test);
        if (TRUE) {  // test flase
          return expTy(NULL, Ty_Void());
        }
      }
      forOrWhile = old_forOrWhile;
    } break;
    case A_forExp: {
      A_exp old_forOrWhile = forOrWhile;
      forOrWhile = exp;
      struct expty lo = transExp(venv, tenv, exp->u.forr.lo);
      struct expty hi = transExp(venv, tenv, exp->u.forr.hi);
      TYPE_CHECK(exp->u.forr.lo->pos, lo.ty, Ty_Int());
      TYPE_CHECK(exp->u.forr.hi->pos, hi.ty, Ty_Int());
      if (lo.exp > hi.exp) exp->u.forr.escape = FALSE;  // jump loop

      // loop variable declaration
      S_beginScope(venv);
      S_enter(venv, exp->u.forr.var, E_VarEntry(lo.ty));

      // can not assign to exp->u.forr.var
      int i = 0;
      for (i = 0; i < 1; i++) {  // lo to hi
        struct expty body = transExp(venv, tenv, exp->u.forr.body);
      }
      S_endScope(venv);
      forOrWhile = old_forOrWhile;
      return expTy(NULL, Ty_Void());
    } break;
    case A_breakExp: {
      if (forOrWhile == NULL) {
        EM_error(exp->pos, "break clause is not in any loop");
      }
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

      show_types();
      show_names();

      struct expty body = transExp(venv, tenv, exp->u.let.body);
      S_endScope(tenv);
      S_endScope(venv);
      return expTy(NULL, body.ty);
    } break;
    case A_arrayExp: {
      Ty_ty arrayTy = (Ty_ty)S_look(tenv, exp->u.array.typ);
      IS_ARRAY_TYPE(exp->pos, arrayTy);
      struct expty size = transExp(venv, tenv, exp->u.array.size);
      TYPE_CHECK(exp->u.array.size->pos, size.ty, Ty_Int());
      if (size.exp) {  // check array size
        EM_error(exp->u.array.size->pos, "illegal array size");
      }
      struct expty init = transExp(venv, tenv, exp->u.array.init);
      TYPE_CHECK_WITH_NIL_RECORD(exp->u.array.init->pos, init.ty,
                                 actualTy(arrayTy)->u.array);
      return expTy(NULL, arrayTy);
    } break;
    default:
      assert(0); /* unknown exp */
      break;
  }
}

struct expty transVar(S_table venv, S_table tenv, A_var var) {
  switch (var->kind) {
    case A_simpleVar: {
      E_enventry entry = (E_enventry)S_look(venv, var->u.simple);
      if (entry == NULL || entry->kind != E_varEntry) {
        NAME_CHECK(var->pos, entry, E_enventry_Var());
        return expTy(NULL, NULL);
      }
      return expTy(NULL, entry->u.var.ty);
    } break;
    case A_fieldVar: {
      // for a var(A_var), can not check type from name
      struct expty v = transVar(venv, tenv, var->u.field.var);
      if (actualTy(v.ty)->kind != Ty_record) {
        TYPE_CHECK_NAME(var->pos, v.ty, E_enventry_Record());
        // exit(-1);
      } else {  // var is record
        Ty_fieldList fieldList = actualTy(v.ty)->u.record;
        while (fieldList) {
          if (fieldList->head->name == var->u.field.sym)
            return expTy(NULL, fieldList->head->ty);
          fieldList = fieldList->tail;
        }
        EM_error(var->pos, "type '%s' has no field '%s'",
                 S_name(binding2sym(tenv, v.ty)), S_name(var->u.field.sym));
      }
      return expTy(NULL, v.ty);
    } break;
    case A_subscriptVar: {
      struct expty v = transVar(venv, tenv, var->u.subscript.var);
      if (actualTy(v.ty)->kind != Ty_array) {
        TYPE_CHECK_NAME(var->pos, v.ty, E_enventry_Array());
        // exit(-1);
      }
      struct expty e = transExp(venv, tenv, var->u.subscript.exp);
      TYPE_CHECK(var->u.subscript.exp->pos, e.ty, Ty_Int());
      if (e.exp) {  // check index range
        EM_error(var->u.subscript.exp->pos, "illegal index of array");
      }
      return expTy(NULL, actualTy(v.ty)->u.array);
    } break;
    default:
      assert(0);  // unknown var
      break;
  }
}

void transDec(S_table venv, S_table tenv, A_dec dec) {
  switch (dec->kind) {
    case A_functionDec: {
      A_fundecList fundecList;

      // deal with mutual recursive functions
      fundecList = dec->u.function;
      while (fundecList) {
        A_fundec func = fundecList->head;

        // from parameters get types and check param names
        A_fieldList paramsDecList = func->params;
        Ty_tyList paramsTyList = NULL;
        if (paramsDecList) {  // params not null
          Ty_tyList tyList = paramsTyList = Ty_TyList(NULL, NULL);
          while (paramsDecList) {
            // check param name is distinct
            {
              A_fieldList nextList = paramsDecList->tail;
              while (nextList) {
                if (paramsDecList->head->name == nextList->head->name) {
                  EM_error(nextList->head->pos,
                           "param name '%s' has already uesd",
                           S_name(nextList->head->name));
                }
                nextList = nextList->tail;
              }
            }

            // build param type list
            tyList->head = (Ty_ty)S_look(tenv, paramsDecList->head->typ);
            if (tyList->head == NULL) {
              EM_error(paramsDecList->head->pos, "unknown type '%s'",
                       S_name(paramsDecList->head->typ));
            }
            if (paramsDecList->tail) {
              tyList->tail = Ty_TyList(NULL, NULL);
              tyList = tyList->tail;
            }
            paramsDecList = paramsDecList->tail;
          }
        }

        // register the function
        Ty_ty resultTy = Ty_Void();
        if (func->result != NULL) {  // function
          resultTy = (Ty_ty)S_look(tenv, func->result);
          if (resultTy == NULL) {
            EM_error(func->pos, "unknown result type '%s' of function '%s'",
                     S_name(func->result), S_name(func->name));
          }
        }
        S_enter(venv, func->name, E_FunEntry(paramsTyList, resultTy));
        fundecList = fundecList->tail;
      }

      // deal the parameters & func body
      fundecList = dec->u.function;
      while (fundecList) {
        A_fundec func = fundecList->head;

        // deal the declaration of the parameters
        E_enventry funcEntry = (E_enventry)S_look(venv, func->name);
        assert(funcEntry && funcEntry->kind == E_funEntry);
        A_fieldList paramsDecList = func->params;
        Ty_tyList paramsTyList = funcEntry->u.fun.formals;

        S_beginScope(venv);
        while (paramsDecList) {
          S_enter(venv, paramsDecList->head->name,
                  E_VarEntry(paramsTyList->head));
          paramsTyList = paramsTyList->tail;
          paramsDecList = paramsDecList->tail;
        }

        // deal the function body
        Ty_ty resultTy = funcEntry->u.fun.result;
        struct expty e = transExp(venv, tenv, func->body);
        if (actualTy(resultTy) != Ty_Void()) {
          // check body type and return type
          if (!(actualTy(e.ty) == Ty_Nil() &&
                actualTy(resultTy)->kind == Ty_record)) {
            TYPE_CHECK(func->body->pos, e.ty, resultTy);
          }
        }
        S_endScope(venv);
        fundecList = fundecList->tail;
      }
    } break;
    case A_varDec: {
      struct expty e = transExp(venv, tenv, dec->u.var.init);
      Ty_ty decTy = e.ty;
      if (dec->u.var.typ) {
        decTy = (Ty_ty)S_look(tenv, dec->u.var.typ);
        IS_TYPE(dec->pos, decTy);
        TYPE_CHECK(dec->u.var.init->pos, e.ty, decTy);
      } else if (e.ty == Ty_Nil()) {  // nil is not a real type
        EM_error(dec->u.var.init->pos,
                 "use 'nil' to initialize no-declar-type variable");
      }
      S_enter(venv, dec->u.var.var, E_VarEntry(decTy));
    } break;
    case A_typeDec: {
      A_nametyList tyDecList;

      /* tasks orderly:
       *        make sure all typenames are different.
       *        parse all declarations' header orderly, make a NULL 'Ty_name'.
       *        parse all declarations' body orderly, fill the 'ty' field
       *              in its 'Ty_name' generated last setp and use 'Ty_name'
       *              for recursive types' reference.
       *        [substitue the unnecessary 'Ty_name' by its real type]
       *        check the recursive declaration has no loop
       */

      // make sure typenames are different
      {
        bool sameNameError = FALSE;
        tyDecList = dec->u.type;
        while (tyDecList) {
          A_nametyList pList = tyDecList->tail;
          while (pList) {
            if (tyDecList->head->name == pList->head->name) {
              EM_error(pList->head->ty->pos,
                       "same name in mutually recursive types");
              sameNameError = TRUE;
              break;
            }
            pList = pList->tail;
          }
          tyDecList = tyDecList->tail;
        }
        // if (sameNameError) exit(-1);
      }

      // deal with type declarations' header
      tyDecList = dec->u.type;
      while (tyDecList) {
        S_enter(tenv, tyDecList->head->name,
                Ty_Name(tyDecList->head->name, NULL));
        tyDecList = tyDecList->tail;
      }

      // deal with the real define
      tyDecList = dec->u.type;
      while (tyDecList) {
        // check type define in transTy

        // S_enter(tenv, tyDecList->head->name,
        //         transTy(tenv, tyDecList->head->ty));

        // not enter a new binding, fill up the previous
        Ty_ty trueTy = transTy(tenv, tyDecList->head->ty);
        Ty_ty ty = (Ty_ty)S_look(tenv, tyDecList->head->name);
        assert(ty && ty->kind == Ty_name && ty->u.name.ty == NULL);
        ty->u.name.ty = trueTy;
        tyDecList = tyDecList->tail;
      }

      /* make full use of 'actualTy()', which makes the type system works well.
       * however, I want to convert the unnecessary 'Ty_name' to its real type.
       * only that can I feel the beauty of the code.
       */
      tyDecList = dec->u.type;
      while (tyDecList) {
        // convert all this type to its real ty in current layer
        Ty_ty ty = S_look(tenv, tyDecList->head->name);
        IS_TYPE(tyDecList->head->ty->pos, ty);
        obj_binding = ty;
        S_dump_enhance(tenv, convertToRealTy);
        tyDecList = tyDecList->tail;
      }

      // check each type is defined completely (no loop)
      {
        tyDecList = dec->u.type;
        // if a type is a alias (Ty_name) then the alias list must have at least
        // a real type.
        bool loopError = FALSE;
        int decCnt = generic_list_cnt(tyDecList, &tyDecList->tail);
        while (tyDecList) {
          int aliasCnt = 0;
          Ty_ty ty = S_look(tenv, tyDecList->head->name);
          if (ty->kind == Ty_array && ty == ty->u.array) {
            // type A is array of A
            EM_error(tyDecList->head->ty->pos, "type '%s' is array of '%s'",
                     S_name(tyDecList->head->name),
                     S_name(tyDecList->head->name));
            loopError = TRUE;
          }
          while (ty->kind == Ty_name) {  // is a alias
            aliasCnt++;
            if (aliasCnt > decCnt) {
              EM_error(tyDecList->head->ty->pos,
                       "type '%s' is in a declaration loop",
                       S_name(tyDecList->head->name));
              loopError = TRUE;
              break;
            }
            ty = ty->u.name.ty;
          }
          tyDecList = tyDecList->tail;
        }
        // if(loopError) exit(-1);
      }
    } break;
    default:
      assert(0);  // unknown declaration
      break;
  }
}

Ty_ty transTy(S_table tenv, A_ty ty) {
  assert(ty);
  switch (ty->kind) {
    case A_nameTy: {
      Ty_ty originTy = (Ty_ty)S_look(tenv, ty->u.name);
      IS_TYPE(ty->pos, originTy);
      return Ty_Name(ty->u.name, originTy);  // originName originType
    } break;
    case A_recordTy: {
      A_fieldList AfieldList = ty->u.record;
      Ty_fieldList tyFieldList = NULL;

      // convert A_fieldList to Ty_fieldList
      if (AfieldList) {
        Ty_fieldList tyList = tyFieldList = Ty_FieldList(NULL, NULL);
        while (AfieldList) {
          // check field names are different
          {
            bool sameNameError = FALSE;
            A_fieldList restList = AfieldList->tail;
            while (restList) {
              if (AfieldList->head->name == restList->head->name) {
                EM_error(restList->head->pos,
                         "same field name '%s' in the same type",
                         S_name(AfieldList->head->name));
                sameNameError = TRUE;
              }
              restList = restList->tail;
            }
            // if(sameNameError) exit(-1);
          }

          Ty_ty fieldTy = (Ty_ty)S_look(tenv, AfieldList->head->typ);
          IS_TYPE(AfieldList->head->pos, fieldTy);
          tyList->head = Ty_Field(AfieldList->head->name, fieldTy);
          if (AfieldList->tail) {
            tyList->tail = Ty_FieldList(NULL, NULL);
            tyList = tyList->tail;
          }
          AfieldList = AfieldList->tail;
        }
      }
      return Ty_Record(tyFieldList);
    } break;
    case A_arrayTy: {
      Ty_ty elemTy = (Ty_ty)S_look(tenv, ty->u.array);
      IS_TYPE(ty->pos, elemTy);
      return Ty_Array(elemTy);
    } break;
    default:
      assert(0);  // unknown A_ty
      break;
  }
}

struct expty expTy(Tr_exp exp, Ty_ty ty) {
  return (struct expty){exp, ty};
}

void SEM_transProg(A_exp exp) {
  struct expty e = transExp(E_base_venv(), E_base_tenv(), exp);
}
