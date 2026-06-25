// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_LOG10_SCALAR_H_
#define MATHLIB_LOG10_SCALAR_H_

#include "../common/MATHLIB_scalarTables.h"
#include "../common/MATHLIB_types.h"
#include "../common/MATHLIB_utility.h"
#include "c6x_migration.h"

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_log10_scalar_ci, ".text:optci");
#endif

template <typename T> static inline T MATHLIB_log10_scalar_ci(T a);

template <> inline float MATHLIB_log10_scalar_ci(float a)
{
   double       ln2  = 0.693147180559945f;
   double       base = 0.4342944819033f;
   float        c1   = -0.2302894f;
   float        c2   = 0.1908169f;
   float        c3   = -0.2505905f;
   float        c4   = 0.3333164f;
   float        c5   = -0.5000002f;
   float        MAXe = 3.402823466E+38f;
   float        pol, r1, r2, r3, r4, res;
   double       dr, frcpax, rcp, T;
   unsigned int T_index;
   int          N;

   /* r = x * frcpa(x) -1 */
   rcp    = _rcpdp((double) a);
   frcpax = _itod(_clr(_hi(rcp), 0u, 16u), 0u);
   dr     = (frcpax * (double) a) - 1.0;

   /* Polynomial p(r) that approximates ln(1+r) - r */
   r1 = (float) dr;
   r2 = r1 * r1;
   r3 = r1 * r2;
   r4 = r2 * r2;

   pol = (c5 * r2) + ((c4 * r3) + ((((c2 * r1) + c3) + (c1 * r2)) * r4));
   pol *= (float) base;

   /* Reconstruction: result = T + r + p(r) */
   N       = (int) _extu(_hi(frcpax), 1u, 21u) - 1023;
   T_index = _extu(_hi(frcpax), 12u, 29u);
   T       = (MATHLIB_logTable[T_index] - (ln2 * (double) N)) * base;
   res     = (float) ((dr * base) + T) + pol;

   if (a <= 0.0f) {
      res = _itof(0xFF800000u);
   }
   if (a > MAXe) {
      res = 308.2547f;
   }

   return (res);
}

inline double divdpMod_log10dpi(double a, double b) { return cmn_DIVDP(a, b); }

template <> inline double MATHLIB_log10_scalar_ci(double a)
{
   double       Half   = 0.5;
   double       MAXe   = 1.7976931348623157e+308;
   double       srHalf = 0.70710678118654752440; /* sqrt(0.5) */
   double       MINe   = 2.2250738585072014e-308;
   double       a0     = -0.64124943423745581147e+2;
   double       a1     = 0.16383943563021534222e+2;
   double       a2     = -0.78956112887491257267e+0;
   double       b0     = -0.76949932108494879777e+3;
   double       b1     = 0.31203222091924532844e+3;
   double       b2     = -0.35667977739034646171e+2; /* Note b3 = 1.0 */
   double       c1     = 0.693359375;                /*  355/512      */
   double       c2     = -2.121944400546905827679e-4;
   double       c10e   = 0.43429448190325182765; /* log (base 10) of e */
   double       W, X, Y, Z;
   double       zn, zd;
   double       Rz, Sa, Bd, Cn, Da;
   int          N, exp_;
   unsigned int upper;

   /* get unbiased exponent */
   Y    = a;
   exp_ = (int) _extu(_hi(Y), 1u, 21u);
   N    = exp_ - 1022;

   /* force DP exp = 1022 if not zero */
   upper = _clr(_hi(Y), 20u, 31u);
   upper = 0x3fe00000u | upper;
   Z     = _itod(upper, _lo(Y));

   if (exp_ == 0) {
      Z = 0.0;
   }

   if (Z > srHalf) {
      zn = (Z - Half) - Half;
      zd = (Z * Half) + Half;
   }
   else {
      zn = Z - Half;
      zd = (zn * Half) + Half;
      N  = N - 1;
   }

   X  = divdpMod_log10dpi(zn, zd);
   W  = X * X;
   Bd = ((((W + b2) * W) + b1) * W) + b0;
   Cn = (((W * a2) + a1) * W) + a0;
   Rz = W * divdpMod_log10dpi(Cn, Bd);
   Sa = X + (X * Rz);
   Cn = (double) N;
   Da = ((Cn * c2) + Sa) + (Cn * c1);
   Da = c10e * Da;

   if (Y < MINe) {
      Da = -MAXe;
   }
   if (Y > MAXe) {
      Da = 308.254715974092;
   }

   return (Da);
}

extern "C" {
static inline float MATHLIB_log10_scalar_sp(float a)
{
   float result = MATHLIB_log10_scalar_ci<float>(a);
   return result;
}

static inline double MATHLIB_log10_scalar_dp(double a, double b)
{
   double result = MATHLIB_log10_scalar_ci<double>(a);
   return result;
}
}

#endif // MATHLIB_LOG2_SCALAR_H_
