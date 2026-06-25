// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_POW_H_
#define MATHLIB_POW_H_

#include "../common/MATHLIB_types.h"
#include "MATHLIB_pow_scalar.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_POW MATHLIB_pow
 * \brief Kernel for elementwise power of a base and an exponent vector.
 *        Supports float and double datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise power of input vectors. Function can
 *        be overloaded with float and double pointers, and the appropriate
 *        precision is employed to compute elementwise power of the input vectors.
 *
 * @tparam       T      : implementation datatype
 * @param  [in]  length : length of input vector
 * @param  [in]  pSrc0   : pointer to buffer holding input vector 0, the base
 * @param  [in]  pSrc1   : pointer to buffer holding input vector 1, the exponent
 * @param  [out] pDst   : pointer to buffer holding result vector
 *
 *
 * @return  Status of success.
 *
 * @remarks None
 */

template <typename T> MATHLIB_STATUS MATHLIB_pow(size_t length, T* pSrc0, T* pSrc1, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_pow.
 *        Function accepts float pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc0   : pointer to buffer holding input vector 0, the base
 * @param [in]  pSrc1   : pointer to buffer holding input vector 1, the exponent
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_pow_sp(size_t length, float* pSrc0, float* pSrc1, float* pDst);

/**
 * @brief This function is the C interface for @ref MATHLIB_pow.
 *        Function accepts double pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc0   : pointer to buffer holding input vector 0, the base
 * @param [in]  pSrc1   : pointer to buffer holding input vector 1, the exponent
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_pow_dp(size_t length, double* pSrc0, double* pSrc1, double* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_POW_H_
