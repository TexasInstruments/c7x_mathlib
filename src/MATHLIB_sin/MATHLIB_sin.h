// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_SIN_H_
#define MATHLIB_SIN_H_

#include "../common/MATHLIB_types.h"
#include "MATHLIB_sin_scalar.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_SIN MATHLIB_sin
 * \brief Kernel for computing elementwise sine of an input vector.
 *        Supports float and double datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise sine of an input vector. Function can
 *        be overloaded with float and double pointers, and the appropriate
 *        precision is employed to compute elementwise sine of the input vector.
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

template <typename T> MATHLIB_STATUS MATHLIB_sin(size_t length, T* pSrc, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_sin.
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
MATHLIB_sin_sp(size_t length, float* pSrc, float* pDst);

/**
 * @brief This function is the C interface for @ref MATHLIB_sin.
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
MATHLIB_sin_dp(size_t length, double* pSrc, double* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_SIN_H_
