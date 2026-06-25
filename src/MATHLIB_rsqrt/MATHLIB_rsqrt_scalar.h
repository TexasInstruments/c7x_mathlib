// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_RSQRT_SCALAR_H_
#define MATHLIB_RSQRT_SCALAR_H_ 1

#include <c6x_migration.h>
#include <float.h>

static inline float MATHLIB_rsqrt_scalar(float a);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_rsqrt_scalar, ".text:optci");
#endif

static inline float MATHLIB_rsqrt_scalar(float a)
{
   float Half  = 0.5f;
   float OneP5 = 1.5f;
   float Small = 1.17549435e-38f;
   float X0, X1, X2, X3;

   X0 = _rsqrsp(a);
   X1 = a * X0;
   X3 = OneP5 - (X1 * X0 * Half);
   X1 = X0 * X3;
   X2 = X1 * (OneP5 - (a * X1 * X1 * Half));

   if (a < Small) {
      X2 = _itof(0x7F800000u);
   }

   return (X2);
}

#endif /* MATHLIB_RSQRT_SCALAR_H_ */
