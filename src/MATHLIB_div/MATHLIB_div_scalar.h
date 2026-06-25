// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_DIV_SCALAR_H_
#define MATHLIB_DIV_SCALAR_H_ 1

#include <c6x_migration.h>

static inline float MATHLIB_div_scalar(float a, float b);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_div_scalar, ".text:optci");
#endif

static inline float MATHLIB_div_scalar(float a, float b)
{
   float TWO  = 2.0f;
   float Maxe = 3.402823466E+38f;
   float X;

   X = _rcpsp(b);
   X = X * (TWO - (b * X));
   X = X * (TWO - (b * X));
   X = a * X;

   if (a == 0.0f) {
      X = 0.0f;
   }

   if ((_fabsf(b) > Maxe) && (_fabs(a) <= Maxe)) {
      X = 0.0f;
   }

   return (X);
}

#endif
