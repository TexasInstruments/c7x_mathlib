// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_ASINH_H_
#define MATHLIB_ASINH_H_

#include "../common/MATHLIB_types.h"

/**
 * \ingroup MATHLIB_KERNELS
 */

/**
 * \defgroup MATHLIB_ASINH MATHLIB_asinh
 * \brief Kernel for computing elementwise hyperbolic arc-sine of an input vector.
 *        Supports float datatype.
 */

/**@{*/

/**
 * @brief Performs the elementwise hyperbolic arc-sine of an input vector. Function can
 *        be overloaded with float pointers, and the appropriate
 *        precision is employed to compute elementwise hyperbolic arc-sine of a vector.
 *
 * @tparam       T      : implementation datatype
 * @param  [in]  length : length of input vector
 * @param  [in]  pSrc  : pointer to buffer holding input vector
 * @param  [out] pDst   : pointer to buffer holding result vector
 *
 *
 * @return  Status of success
 *
 * @remarks None
 */

template <typename T> MATHLIB_STATUS MATHLIB_asinh(size_t length, T* pSrc, T* pDst);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function is the C interface for @ref MATHLIB_asinh.
 *        Function accepts float pointers.
 *
 * @tparam      T      : implementation datatype
 * @param [in]  length : length of input vector
 * @param [in]  pSrc  : pointer to buffer holding input vector
 * @param [out] pDst   : pointer to buffer holding result vector
 *
 * @return  Status of success
 *
 * @remarks None
 */

MATHLIB_STATUS
MATHLIB_asinh_sp(size_t length, float* pSrc, float* pDst);

/**@}*/

#ifdef __cplusplus
}

#endif
#endif // MATHLIB_ASINH_H_
