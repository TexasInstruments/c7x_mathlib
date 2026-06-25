// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_RECIP_TEST_PARAMS_H
#define MATHLIB_RECIP_TEST_PARAMS_H

/******************************************************************************/
/*                                                                            */
/* Includes                                                                   */
/*                                                                            */
/******************************************************************************/

#include "../common/MATHLIB_test.h"
#include <cstdint>
#include <stdint.h>

/**
 * @struct MATHLIB_recip_testParams_t
 * @brief  Data structure to hold parameters of given test case
 */
typedef struct {
   void    *staticIn;  // Input vector
   void    *staticOut; // Output vector
   size_t   length;    // Length of vector
   uint32_t dType;     // Data type (MATHLIB_FLOAT32)
   uint32_t testID;    // Test case ID
} MATHLIB_recip_testParams_t;

/**
 * @brief Utility function to obtain the total number of test cases to test
 * @param **params  [in]  : pointer to test case params
 * @param *numTests [out] : number of test cases
 *
 * @remarks None
 */

void MATHLIB_recip_getTestParams(MATHLIB_recip_testParams_t **params, int32_t *numTests);

#endif
