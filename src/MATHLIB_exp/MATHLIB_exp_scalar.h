// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_EXP_SCALAR_H_
#define MATHLIB_EXP_SCALAR_H_

#include "../common/MATHLIB_scalarTables.h"
#include "../common/MATHLIB_types.h"
#include "c6x_migration.h"

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_exp_scalar_ci, ".text:optci");
#endif

inline float MATHLIB_exp_scalar_ci(float x)
{
   const float  log2_base_x16 = 23.083120654f; /*1.442695041 * 16*/
   const float  half          = 0.5f;
   const float  LnMin         = -87.33654475f;
   const float  LnMax         = 88.72283905f;
   const float  Max           = 3.402823466E+38f;
   const double p             = 0.0433216987816623; /* 1/log2_base_x16 */

   /* coefficients to approximate the decimal part of the result */
   const float c0 = 0.166668549286041f;
   const float c1 = 0.500016170012920f;
   const float c2 = 0.999999998618401f;

   float        pol, r, r2, r3, res;
   unsigned int Ttemp, J, K;
   float        Nf;
   int          N;
   double       dT;

   /* Get N such that |N - x*16/ln(2)| is minimized */
   Nf = (x * log2_base_x16) + half;
   N  = (int) Nf; /* Cast from intermediate variable to appease MISRA */

   if ((x * log2_base_x16) < -half) {
      N--;
   }

   /* Argument reduction, r, and polynomial approximation pol(r) */
   r  = (float) ((double) x - (p * (double) N));
   r2 = r * r;
   r3 = r * r2;

   pol = (r * c2) + ((r3 * c0) + (r2 * c1));

   /* Get index for ktable and jtable */
   K  = _extu((unsigned int) N, 28u, 30u);
   J  = (unsigned int) N & 0x3u;
   dT = MATHLIB_kTable[K] * MATHLIB_jTable[J];

   /* Scale exponent to adjust for 2^M */
   Ttemp = _hi(dT) + (((unsigned int) N >> 4) << 20);
   dT    = _itod(Ttemp, _lo(dT));
   res   = (float) dT * (1.0f + pol);

   /* < LnMin returns 0 */
   if (x < LnMin) {
      res = 0.0f;
   }

   /* > LnMax returns MAX */
   if (x > LnMax) {
      res = Max;
   }

   return (res);
}

#endif // MATHLIB_EXP_SCALAR_H_
