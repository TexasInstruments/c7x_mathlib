// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_ATAN2_H_
#define MATHLIB_ATAN2_H_

#include "../common/MATHLIB_types.h"
#include "MATHLIB_atan2_scalar.h"
/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_ATAN2 MATHLIB_atan2
 * \brief Kernel for performing arctan2 computation on the input vector elementwise.
 *        Supports float and double datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise arc-tangent2 of the input vector. Function can
 *        be overloaded with float and double pointers, and the appropriate
 *        precision is employed to compute elementwise arc-tangent2 of the input vector.
 *
 * @tparam       T      : implementation datatype
 * @param  [in]  length : length of input vector
 * @param  [in]  pSrc0  : pointer to buffer holding input vector
 * @param  [in]  pSrc1  : pointer to buffer holding input vector
 * @param  [out] pDst   : pointer to buffer holding result vector
 *
 *
 * @return  Status of success
 *
 * @remarks None
 */

template <typename T> MATHLIB_STATUS MATHLIB_atan2(size_t length, T* pSrc0, T* pSrc1, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_atan2.
 *        Function accepts float pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc0  : pointer to buffer holding input vector
 * @param  [in]  pSrc1  : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_atan2_sp(size_t length, float* pSrc0, float* pSrc1, float* pDst);

/**
 * @brief This function is the C interface for @ref MATHLIB_atan2.
 *        Function accepts double pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc0  : pointer to buffer holding input vector
 * @param  [in]  pSrc1  : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_atan2_dp(size_t length, double* pSrc0, double* pSrc1, double* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_ATAN2_H_
