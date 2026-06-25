// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_LOG10_TEST_PARAMS_H
#define MATHLIB_LOG10_TEST_PARAMS_H

/******************************************************************************/
/*                                                                            */
/* Includes                                                                   */
/*                                                                            */
/******************************************************************************/

#include "../common/MATHLIB_test.h"
#include <cstdint>
#include <stdint.h>

/**
 * @struct MATHLIB_log10_testParams_t
 * @brief  Data structure to hold a parameters of given test case
 */
typedef struct {
   void    *staticIn;
   void    *staticOut;
   size_t   length;
   uint32_t dType;
   uint32_t testID;
} MATHLIB_log10_testParams_t;

/**
 * @brief Utility function to obtain the total number of test cases to test
 * @param **params  [in]  : pointer to test case params
 * @param *numTests [out] : number of test cases
 *
 * @remarks None
 */

void MATHLIB_log10_getTestParams(MATHLIB_log10_testParams_t **params, int32_t *numTests);

#endif
