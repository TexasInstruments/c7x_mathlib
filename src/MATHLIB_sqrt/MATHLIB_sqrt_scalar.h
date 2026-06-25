// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_SQRT_SCALAR_H_
#define MATHLIB_SQRT_SCALAR_H_ 1

#include <c6x_migration.h>
#include <float.h>

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_sqrt_scalar, ".text:optci");
#endif

/* =========================================================================== */
/* The sqrtsp function returns the square root of a real floating-point value. */
/* Newton-Rhapson algorithm is applied for better precision.                   */
/* =========================================================================== */

template <typename T> static inline T MATHLIB_sqrt_scalar_ci(T a);

template <> inline float MATHLIB_sqrt_scalar_ci<float>(float a)
{
   const float Half  = 0.5f;
   const float OneP5 = 1.5f;
   float       x, y;
   int         i;

   x = _rsqrsp(a); /* compute square root reciprocal */

#pragma UNROLL(1) /* PRAGMA: do not unroll this loop */
   for (i = 0; i < 2; i++) {
      x = x * (OneP5 - (a * x * x * Half));
   }
   y = a * x;

   if (a <= 0.0f) {
      y = 0.0f;
   }
   if (a > FLT_MAX) {
      y = FLT_MAX;
   }

   return (y);
}

template <> inline double MATHLIB_sqrt_scalar_ci<double>(double a)
{
   double half  = 0.5;
   double OneP5 = 1.5;
   double x, y;
   int    i;

   x = _rsqrdp(a);

#pragma UNROLL(1) /* PRAGMA: do not unroll this loop */
   for (i = 0; i < 3; i++) {
      x = x * (OneP5 - (a * x * x * half));
   }
   y = a * x;

   if (a <= 0.0) {
      y = 0.0;
   }
   if (a > DBL_MAX) {
      y = DBL_MAX;
   }

   return (y);
}

extern "C" {
static inline float MATHLIB_sqrt_scalar_sp(float a)
{
   float result = MATHLIB_sqrt_scalar_ci<float>(a);
   return result;
}

static inline double MATHLIB_sqrt_scalar_dp(double a)
{
   double result = MATHLIB_sqrt_scalar_ci<double>(a);
   return result;
}
}

#endif /* MATHLIB_SQRT_SCALAR_H_ */
