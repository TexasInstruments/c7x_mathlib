// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_LOG10_H_
#define MATHLIB_LOG10_H_

#include "../common/MATHLIB_types.h"
#include "MATHLIB_log10_scalar.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_LOG10 MATHLIB_log10
 * \brief Kernel for elementwise log base 10 of an input vector.
 *        Supports float and double datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise log base 10 of an input vector. Function can
 *        be overloaded with float and double pointers, and the appropriate
 *        precision is employed to compute elementwise log base 10 of the input vector.
 *
 * @tparam       T      : implementation datatype
 * @param  [in]  length : length of input vector
 * @param  [in]  pSrc   : pointer to buffer holding input vector
 * @param  [out] pDst   : pointer to buffer holding result vector
 *
 *
 * @return  Status of success.
 *
 * @remarks None
 */

template <typename T> MATHLIB_STATUS MATHLIB_log10(size_t length, T* pSrc, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_log10.
 *        Function accepts float pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc   : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_log10_sp(size_t length, float* pSrc, float* pDst);

/**
 * @brief This function is the C interface for @ref MATHLIB_log10.
 *        Function accepts double pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc   : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_log10_dp(size_t length, double* pSrc, double* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_LOG10_H_
