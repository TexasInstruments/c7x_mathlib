// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_EXP2_H_
#define MATHLIB_EXP2_H_

#include "../common/MATHLIB_types.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_EXP2 MATHLIB_exp2
 * \brief Kernel for elementwise computation of 2 raised to a given power.
 *        Supports float datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise 2^x for all x in the input vector. Function can
 *        be overloaded with float pointers, and the appropriate
 *        precision is employed to compute elementwise exponentialization of the input vector.
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

template <typename T> MATHLIB_STATUS MATHLIB_exp2(size_t length, T* pSrc, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_exp2.
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
MATHLIB_exp2_sp(size_t length, float* pSrc, float* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_exp2_H_
