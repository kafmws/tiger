#ifndef _KAFM_C_TEST_H
#define _KAFM_C_TEST_H

#include <assert.h>
#include <math.h>
#include <string.h>

#ifndef _KAFM_C_TEST_
#define _KAFM_C_TEST_ 1
#endif

static int _MAIN_RET_ = 0;
static int _TEST_COUNT_ = 0;
static int _TEST_PASS_ = 0;

#ifdef _MSC_VER

#define SET_COLOR(stream, color) ;
#define RESET_COLOR(stream) ;

#else

#define TEST_RED "\033[40;31m"

#define SET_COLOR(stream, color) ((void)fprintf((stream), (color)))
#define RESET_COLOR(stream) ((void)fprintf(stream, "\033[0m"))

#endif

#define VAR_ERROR(expect, actual, format)                              \
  do {                                                                 \
    SET_COLOR(stderr, TEST_RED);                                       \
    ((void)fprintf(stderr, "%s:%d:\n\texpect：", __FILE__, __LINE__)); \
    RESET_COLOR(stderr);                                               \
    ((void)fprintf(stderr, "\"" format "\"\n", (expect)));             \
    SET_COLOR(stderr, TEST_RED);                                       \
    ((void)fprintf(stderr, "\tactual: "));                             \
    RESET_COLOR(stderr);                                               \
    ((void)fprintf(stderr, "\"" format "\"\n", (actual)));             \
  } while (0);

#define ARRAY_ERROR(expect, actual, format, num)                    \
  do {                                                              \
    SET_COLOR(stderr, TEST_RED);                                    \
    fprintf(stderr, "%s:%d: \n\texpect: \n\t", __FILE__, __LINE__); \
    RESET_COLOR(stderr);                                            \
    fprintf(stderr, "[");                                           \
    int i;                                                          \
    for (i = 0; i < (long)num - 1; i++) {                           \
      fprintf(stderr, format ", ", *((expect) + i));                \
    }                                                               \
    fprintf(stderr, format, *((expect) + (long)num - 1));           \
    fprintf(stderr, "]");                                           \
    SET_COLOR(stderr, TEST_RED);                                    \
    fprintf(stderr, "\n\tactual: ");                                \
    RESET_COLOR(stderr);                                            \
    fprintf(stderr, "\n\t[");                                       \
    for (i = 0; i < (long)num - 1; i++) {                           \
      fprintf(stderr, format ", ", *((actual) + i));                \
    }                                                               \
    fprintf(stderr, format, *((actual) + (long)num - 1));           \
    fprintf(stderr, "]\n");                                         \
  } while (0)

#if _KAFM_C_TEST_

#define EXPECT_EQ_BASE(equality, error_macro) \
  do {                                        \
    _TEST_COUNT_++;                           \
    if (!(_KAFM_C_TEST_) || (equality))       \
      _TEST_PASS_++;                          \
    else {                                    \
      error_macro;                            \
      _MAIN_RET_ = 1;                         \
    }                                         \
  } while (0)

#define MAIN_RET                                                 \
  (printf("%d/%d (%3.2f%%) passed\n", _TEST_PASS_, _TEST_COUNT_, \
          _TEST_PASS_ * 100.0 / _TEST_COUNT_),                   \
   _MAIN_RET_)

#else

#define EXPECT_EQ_BASE(equality, error_macro) ;
#define MAIN_RET 0

#endif  // if _KAFM_C_TEST_

#define EXPECT_EQ_INT(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), VAR_ERROR((expect), (actual), "%d"));

#define EXPECT_EQ_DOUBLE(expect, actual)                          \
  EXPECT_EQ_BASE(fabs((double)(expect) - (double)(actual)) < 1e6, \
                 VAR_ERROR(((double)expect), ((double)actual), "%f"));

#define EXPECT_EQ_STRING(expect, actual)          \
  EXPECT_EQ_BASE(strcmp((expect), (actual)) == 0, \
                 VAR_ERROR((expect), (actual), "%s"));

#define EXPECT_EQ_ARRAY(expect, actual, bytes, elem_format)   \
  EXPECT_EQ_BASE(memcmp((expect), (actual), (bytes)) == 0,    \
                 ARRAY_ERROR((expect), (actual), elem_format, \
                             ((bytes) / sizeof(*expect))));

#define EXPECT_NOT_EQ_INT(expect, actual) \
  EXPECT_EQ_BASE((expect) != (actual), VAR_ERROR((expect), (actual), "%d"));

#define EXPECT_NOT_EQ_DOUBLE(expect, actual)                  \
  EXPECT_EQ_BASE(fabs((double)expect, (double)actual) >= 1e6, \
                 VAR_ERROR(((double)expect), ((double)actual), "%f"));

#define EXPECT_NOT_EQ_STRING(expect, actual)      \
  EXPECT_EQ_BASE(strcmp((expect), (actual)) != 0, \
                 VAR_ERROR((expect), (actual), "%s"));

#define EXPECT_NOT_EQ_ARRAY(expect, actual, bytes, elem_format) \
  EXPECT_EQ_BASE(memcmp((expect), (actual), (bytes)) != 0,      \
                 ARRAY_ERROR((expect), (actual), elem_format, (bytes)));

#define EXPECT_TRUE(exp) \
  EXPECT_EQ_BASE((exp) != 0, VAR_ERROR(#exp " is true", "but false", "%s"))
#define EXPECT_FALSE(exp) \
  EXPECT_EQ_BASE((exp) == 0, VAR_ERROR(#exp " is false", "but true", "%s"))

#endif  // ifndef _KAFM_C_TEST_H

/* example.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define _KAFM_C_TEST_ 0
#include "test.h"

int main() {
  int arr[] = {1, 2, 3, 4, 5, 6};
  int arr2[] = {1, 2, 3, 4, 5, 5};
  EXPECT_EQ_ARRAY(arr, arr2, sizeof(arr), "%d");

  int *arrp[] = {arr, arr + 1, arr + 2};
  int *arrp2[] = {arr, arr + 1, arr + 3};
  EXPECT_EQ_ARRAY(arrp, arrp2, sizeof(arr), "%p");

  char *s1 = "hello world";
  char *s2 = "hello world ";
  EXPECT_EQ_STRING(s1, s2);

  char *strs[] = {"1", "2", "3", "4"};
  char *strs2[] = {"1", "2", "3", ""};
  EXPECT_EQ_ARRAY(strs, strs2, sizeof(strs), "\"%s\"");

  EXPECT_EQ_DOUBLE(1e6, 1000000);

  EXPECT_TRUE(1 == 0);

  return MAIN_RET;
}
*/