// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_DIV_H_
#define MATHLIB_DIV_H_

#include "../common/MATHLIB_types.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_DIV MATHLIB_div
 * \brief Kernel for computing elementwise divide of dividend and divisor vectors.
 *        Supports float datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise divide of two input vectors. Function can
 *        be overloaded with float or double pointers, and the appropriate
 *        precision is employed to compute elementwise divide of two vectors.
 *
 * @tparam       T      : implementation datatype
 * @param  [in]  length : length of input vector
 * @param  [in]  pSrc0   : pointer to buffer holding input vector 0, the dividend
 * @param  [in]  pSrc1   : pointer to buffer holding input vector 1, the divisor
 * @param  [out] pDst   : pointer to buffer holding result vector
 *
 *
 * @return  Status of success.
 *
 * @remarks None
 */

template <typename T> MATHLIB_STATUS MATHLIB_div(size_t length, T* pSrc0, T* pSrc1, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_div.
 *        Function accepts float pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc0   : pointer to buffer holding input vector 0, the dividend
 * @param  [in]  pSrc1   : pointer to buffer holding input vector1, the divisor
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_div_sp(size_t length, float* pSrc0, float* pSrc1, float* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_DIV_H_
