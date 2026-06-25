// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_EXP2_SCALAR_H_
#define MATHLIB_EXP2_SCALAR_H_

#include "../common/MATHLIB_types.h"
#include "c6x_migration.h"

static inline float divspMod_exp2spi(float a, float b);
static inline float exp2sp_i(float a);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(divspMod_exp2spi, ".text:optci");
#endif

/* Pull in inline for divsp */
static inline float divspMod_exp2spi(float a, float b)
{
   float TWO = 2.0f;
   float X;
   X = _rcpsp(b);
   X = X * (TWO - (b * X));
   X = X * (TWO - (b * X));
   X = a * X;
   return X;
}

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_exp2_scalar_ci, ".text:optci");
#endif

static inline float MATHLIB_exp2_scalar_ci(float a)
{
   float Zeroe  = 0.0f;
   float Halfe  = 0.5f;
   float epse   = 9.313225746e-10f; /* [2^-29] / 2  */
   float LnMine = -126.0f;
   float MAXe   = 3.40282347e+38f;
   float LnMaxe = 128.0f;
   float a0e    = 2.499999995e-1f;
   float a1e    = 4.1602886268e-3f;
   float b0e    = 0.5f;
   float b1e    = 4.9987178778e-2f;
   float CC1E   = 0.693359375f;       /* 355/512 */
   float CC2E   = -2.12194440055e-4f; /* lne(2) - 355/512 */
   float Ln2E   = 1.442695040889f;    /* ln(base 2) of e */
   float k2e    = 0.69314718056f;     /* log (base e) of  2 */

   float Ye, Xe, We, Re, Se, Be, Ce, De;
   int   Ne;

   Ye = k2e * a;

   /* < epsilon returns unity */
   if (_fabsf(Ye) < epse) {
      Ye = 0.0f;
   }

   Ce = Ye * Ln2E;  /* base e --> base 2 argument */
   Ne = _spint(Ce); /* get unbiased exponent as int */
   Se = (float) Ne; /* float(int N) */

   Xe = (Ye - (Se * CC1E)) - (Se * CC2E); /* range reduction  */
   We = Xe * Xe;
   Be = (b1e * We) + b0e;        /* denominator  */
   De = ((a1e * We) + a0e) * Xe; /* numerator  */

   Re = Halfe + divspMod_exp2spi(De, Be - De);
   Se = _itof(_extu((unsigned int) Ne + 128u, 23u, 0u));
   Ce = Re * Se; /* scale by power of 2  */

   /* < LnMin returns 0 */
   if (a < LnMine) {
      Ce = Zeroe;
   }

   /* > LnMax returns MAX */
   if (a > LnMaxe) {
      Ce = MAXe;
   }

   return Ce;
}

#endif