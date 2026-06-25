// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_RECIP_SCALAR_H_
#define MATHLIB_RECIP_SCALAR_H_

#include <c7x.h>
#include <float.h>
#include <math.h>

static inline float  MATHLIB_recip_scalar_sp(float a);
static inline double MATHLIB_recip_scalar_dp(double a);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_recip_scalar_sp, ".text:optci");
#pragma CODE_SECTION(MATHLIB_recip_scalar_dp, ".text:optci");
#endif

/**
 * @brief Computes the reciprocal (1/a) of a single-precision floating-point value.
 *
 * Uses C7x __recip intrinsic for initial estimate followed by two Newton-Raphson
 * iterations to achieve full single-precision accuracy.
 *
 * @param  a  Input value
 * @return    Reciprocal of input (1/a)
 *
 * @remarks
 * - For very large inputs (|a| > FLT_MAX), returns 0.0f
 * - For a = 0, behavior is undefined (returns inf or very large value)
 */
static inline float MATHLIB_recip_scalar_sp(float a)
{
   float TWO  = 2.0f;
   float Maxe = 3.402823466E+38f; /* Close to FLT_MAX */
   float X;

   /* Get initial reciprocal estimate (~8-bit accuracy) */
   X = __recip(a);

   /* Newton-Raphson iteration 1: X = X * (2 - a*X), ~16-bit accuracy */
   X = X * (TWO - (a * X));

   /* Newton-Raphson iteration 2: X = X * (2 - a*X), ~24+ bit accuracy */
   X = X * (TWO - (a * X));

   /* Handle overflow case: very large input produces ~0 output */
   if (fabs(a) > Maxe) {
      X = 0.0f;
   }

   return (X);
}

/**
 * @brief Computes the reciprocal (1/a) of a double-precision floating-point value.
 *
 * Uses C7x __recip intrinsic for initial estimate followed by three Newton-Raphson
 * iterations to achieve full double-precision accuracy.
 *
 * @param  a  Input value
 * @return    Reciprocal of input (1/a)
 *
 * @remarks
 * - For very large inputs (|a| > DBL_MAX), returns 0.0
 * - For a = 0, behavior is undefined (returns inf or very large value)
 */
static inline double MATHLIB_recip_scalar_dp(double a)
{
   double TWO  = 2.0;
   double Maxe = 1.7976931348623157E+308; /* Close to DBL_MAX */
   double X;

   /* Get initial reciprocal estimate (~8-bit accuracy) */
   X = __recip(a);

   /* Newton-Raphson iteration 1: X = X * (2 - a*X), ~16-bit accuracy */
   X = X * (TWO - (a * X));

   /* Newton-Raphson iteration 2: X = X * (2 - a*X), ~32-bit accuracy */
   X = X * (TWO - (a * X));

   /* Newton-Raphson iteration 3: X = X * (2 - a*X), ~53+ bit accuracy (full double) */
   X = X * (TWO - (a * X));

   /* Handle overflow case: very large input produces ~0 output */
   if (fabs(a) > Maxe) {
      X = 0.0;
   }

   return (X);
}

#endif /* MATHLIB_RECIP_SCALAR_H_ */
