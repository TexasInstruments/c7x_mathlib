// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_SQRT_H_
#define MATHLIB_SQRT_H_

#include "../common/MATHLIB_types.h"
#include "MATHLIB_sqrt_scalar.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_SQRT MATHLIB_sqrt
 * \brief Kernel for computing elementwise square root of an input vector.
 *        Supports float and double datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise square root of an input vectors. Function can
 *        be overloaded with float and double pointers, and the appropriate
 *        precision is employed to compute elementwise square root of an input vector.
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

template <typename T> MATHLIB_STATUS MATHLIB_sqrt(size_t length, T* pSrc, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_sqrt.
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
MATHLIB_sqrt_sp(size_t length, float* pSrc, float* pDst);

/**
 * @brief This function is the C interface for @ref MATHLIB_sqrt.
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
MATHLIB_sqrt_dp(size_t length, double* pSrc, double* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_SQRT_H_
