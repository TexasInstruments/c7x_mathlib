// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_SIN_SCALAR_H_
#define MATHLIB_SIN_SCALAR_H_ 1

#include <c6x_migration.h>

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_sin_scalar, ".text:optci");
#endif

template <typename T> static inline T MATHLIB_sin_scalar_ci(T a);

template <> inline float MATHLIB_sin_scalar_ci<float>(float a)
{
   float InvPI = 0.318309886183791f;
   float One   = 1.0f;
   float MAX   = 1048576.0f;
   float Zero  = 0.0f;
   float s1    = -1.666665668e-1f;
   float s2    = 8.333025139e-3f;
   float s3    = -1.980741872e-4f;
   float s4    = 2.601903036e-6f;
   float C1    = 3.140625f;
   float C2    = 9.67653589793e-4f;
   float Sign, X, Y, Z, F, G, R;
   int   N;

   Sign = One;
   Y    = a;

   if (_fabsf(Y) > MAX) {
      Y = Zero;
   }

   X = Y * InvPI; /* X = Y * (1/PI)  */
   N = _spint(X); /* N = integer part of X  */
   Z = (float) N;

   if ((N % 2) != 0) {
      Sign = -Sign; /* Quadrant 3 or 4 */
   }

   F = (Y - (Z * C1)) - (Z * C2);
   G = F * F;
   R = ((((((s4 * G) + s3) * G) + s2) * G) + s1) * G;

   return ((F + (F * R)) * Sign);
}

template <> inline double MATHLIB_sin_scalar_ci<double>(double a)
{
   double InvPI = 0.31830988618379067154;
   double C1    = 3.1416015625;
   double C2    = -8.908910206761537356617e-6;
   double r8    = 2.7204790957888846175e-15;
   double r7    = -7.6429178068910467734e-13;
   double r6    = 1.6058936490371589114e-10;
   double r5    = -2.5052106798274584544e-8;
   double r4    = 2.7557319210152756119e-6;
   double r3    = -1.9841269841201840457e-4;
   double r2    = 8.3333333333331650314e-3;
   double r1    = -1.6666666666666665052e-1;
   double MAX   = 1.073741824e+09;
   double Zero  = 0.0;
   double Sign  = 1.0;
   double X, Z, F1, F2, G, R;
   int    N;

   F1 = a;

   if (_fabs(F1) > MAX) {
      F1 = Zero;
   }

   X = F1 * InvPI; /* X = Y/PI  */
   N = _dpint(X);
   Z = (double) N;

   if ((N % 2) != 0) {
      Sign = -Sign; /* neg. quadrants  */
   }

   F1 = (F1 - (Z * C1)) - (Z * C2);
   F2 = F1 * F1;
   G  = F2 * F2;

   R = ((((((G * r8) + r6) * G) + r4) * G) + r2) * G;
   X = ((((((G * r7) + r5) * G) + r3) * G) + r1) * F2;
   R = R + X;
   G = (F1 + (F1 * R)) * Sign;

   return (G);
}

extern "C" {
static inline float MATHLIB_sin_scalar_sp(float a)
{
   float result = MATHLIB_sin_scalar_ci<float>(a);
   return result;
}

static inline double MATHLIB_sin_scalar_dp(double a)
{
   double result = MATHLIB_sin_scalar_ci<double>(a);
   return result;
}
}

#endif
