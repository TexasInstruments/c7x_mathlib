// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_PROFILE_H_
#define MATHLIB_PROFILE_H_ 1

/******************************************************************************/
/*                                                                            */
/* Includes                                                                   */
/*                                                                            */
/******************************************************************************/

#include "MATHLIB_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/******************************************************************************/
/*                                                                            */
/* Profiling macros and variables                                             */
/*                                                                            */
/******************************************************************************/

extern int test_index; // index of current active test
extern int act_kernel; // active kernel type for test and profile

/******************************************************************************/
/*                                                                            */
/* Defines                                                                    */
/*                                                                            */
/******************************************************************************/

// profiling defines
#define MATHLIB_PROFILE_KERNEL_OPT_COLD 0 // kernel type optimized (C/C++-intrinsics, assembly)
#define MATHLIB_PROFILE_KERNEL_OPT_WARM 1 // kernel type optimized (C-intrinsics, assembly) with warm instruction cache
#define MATHLIB_PROFILE_KERNEL_CNT 6      // various cycle counts
#define MATHLIB_PROFILE_TEST_TYPES 2      // cold and warm runs
#define MATHLIB_PROFILE_MAX_TESTS 256     // maximum number of test cases
#define MATHLIB_PROFILE_KERNEL_CN 3       // kernel type natural C, reserved for future
#define MATHLIB_PROFILE_KERNEL_INIT 4     // kernel init for C7x, reserved for future
#define MATHLIB_PROFILE_KERNEL_CNINIT 5   // kernel init for natural C, reserved for future

// profiling macros
#ifdef _HOST_BUILD
#define PROFILE_INIT
#define PROFILE_READ 0
#else
#if defined(__C7100__) || defined(__C7120__) || defined(__C7502__) || defined(__C7504__) || defined(__C7508__) ||      \
    defined(__C7524__)
#define PROFILE_INIT
#define PROFILE_READ (__get_GTSC(1), __STSC)
#else
#define PROFILE_INIT TSCL = 0, TSCH = 0
#define PROFILE_READ _itoll(TSCH, TSCL)
#endif
#endif

// profiling globals
extern uint64_t beg_count;                          // begin cycle count for profiling
extern uint64_t end_count;                          // end cycle count for profiling
extern uint64_t overhead;                           // cycle profiling overhead
extern uint64_t cycles[MATHLIB_PROFILE_KERNEL_CNT]; // cycle counts for C-natural

/******************************************************************************/
/*                                                                            */
/* Function prototypes                                                        */
/*                                                                            */
/******************************************************************************/

void     MATHLIB_profile_init(char *kernel_name);
int64_t  MATHLIB_profile_get_cycles(int kernel);
void     MATHLIB_profile_add_test(uint32_t testNum,
                                  int      Ax0,
                                  uint64_t arch_cycles,
                                  uint64_t est_cycles,
                                  int      errorFlag,
                                  char    *desc);
void     MATHLIB_profile_clear_run_stats();
void     MATHLIB_profile_skip_test(char *desc);
void     MATHLIB_profile_cycle_report(int reportType, char *formulaInfo, char *rangeInfo);
void     MATHLIB_profile_initStack(uint32_t SP);
void     MATHLIB_profile_setStackDepth(void);
uint32_t MATHLIB_profile_getSP(void);
void     MATHLIB_profile_stack_memory(void);
void     MATHLIB_profile_clear_cycle_counts(void);
void     MATHLIB_profile_clear_stack_sizes(void);
void     MATHLIB_profile_clear_cycle_count_single(int kernel);

/******************************************************************************/
/*                                                                            */
/* MATHLIB_profile_start                                                       */
/*                                                                            */
/******************************************************************************/

static inline void MATHLIB_profile_start(int kernel)
{
   /* Set the active kernel for subsequent calls */
   act_kernel = kernel;

   /* Register starting timestamp */
   beg_count = PROFILE_READ;
}

/* Profiling stop (and calculate) routine */
static inline void MATHLIB_profile_stop(void)
{
   /* Register ending timestamp */
   end_count = PROFILE_READ;

   /* Calculate and store cycles */
   cycles[act_kernel] += end_count - beg_count - overhead;

#if (!defined(RTL_TEST) && !defined(QT_TEST) && !defined(EVM_TEST))
   /* Analyze the stack and register stack depth used */
   MATHLIB_profile_setStackDepth();
#endif
}

#endif
