// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_RECIP_H_
#define MATHLIB_RECIP_H_

#include "../common/MATHLIB_types.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_RECIP MATHLIB_recip
 * \brief Kernel for computing elementwise reciprocal (1/x) of an input vector.
 *        Supports float datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise reciprocal of an input vector. Function can
 *        be overloaded with float pointers, and the appropriate
 *        precision is employed to compute elementwise reciprocal.
 *
 * @tparam       T      : implementation datatype
 * @param  [in]  length : length of input vector
 * @param  [in]  pSrc   : pointer to buffer holding input vector
 * @param  [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

template <typename T> MATHLIB_STATUS MATHLIB_recip(size_t length, T* pSrc, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_recip.
 *        Function accepts float pointers.
 *
 * @param [in]  length : length of input vector
 * @param [in]  pSrc   : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_recip_sp(size_t length, float* pSrc, float* pDst);

/**
 * @brief This function is the C interface for @ref MATHLIB_recip.
 *        Function accepts double pointers.
 *
 * @param [in]  length : length of input vector
 * @param [in]  pSrc   : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success.
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_recip_dp(size_t length, double* pSrc, double* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_RECIP_H_
