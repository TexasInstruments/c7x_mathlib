// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_POW_SCALAR_H_
#define MATHLIB_POW_SCALAR_H_

#include "../common/MATHLIB_scalarTables.h"
#include "../common/MATHLIB_types.h"
#include "../common/MATHLIB_utility.h"
#include "c6x_migration.h"

template <typename T> static inline T MATHLIB_log_pow_scalar_ci(T a);
template <typename T> static inline T MATHLIB_exp_pow_scalar_ci(T a);
template <typename T> static inline T MATHLIB_pow_scalar_ci(T a, T b);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_log_pow_scalar_ci, ".text:optci");
#endif

template <> inline float MATHLIB_log_pow_scalar_ci<float>(float a)
{
   double ln2  = 0.693147180559945;
   float  MAXe = 3.402823466E+38f;
   float  c1   = -0.2302894f;
   float  c2   = 0.1908169f;
   float  c3   = -0.2505905f;
   float  c4   = 0.3333164f;
   float  c5   = -0.5000002f;
   float  pol, r1, r2, r3, r4, res;
   double dr, frcpax, rcp, T;
   int    N, T_index;

   /* r = x * frcpa(x) -1 */
   rcp    = _rcpdp((double) a);
   frcpax = _itod(_clr(_hi(rcp), 0u, 16u), 0u);
   dr     = (frcpax * (double) a) - 1.0;

   /* Polynomial p(r) that approximates ln(1+r) - r */
   r1 = (float) dr;
   r2 = r1 * r1;
   r3 = r1 * r2;
   r4 = r2 * r2;

   pol = (c5 * r2) + (c4 * r3) + (((c2 * r1) + c3 + (c1 * r2)) * r4);

   /* Reconstruction: result = T + r + p(r) */
   N       = (int) _extu(_hi(frcpax), 1u, 21u) - 1023;
   T_index = (int) _extu(_hi(frcpax), 12u, 29u);
   T       = MATHLIB_logTable[T_index] - (ln2 * (double) N);
   res     = (float) (dr + T + (double) pol);

   if (a > MAXe) {
      res = 88.72283905313f;
   }

   return (res);
}

inline double divdpMod_powdpi(double a, double b) { return cmn_DIVDP(a, b); }

template <> inline double MATHLIB_log_pow_scalar_ci<double>(double a)
{
   double       Half   = 0.5;
   double       MAXe   = 1.7976931348623157e+308;
   double       srHalf = 0.70710678118654752440; /* sqrt(0.5) */
   double       a0     = -0.64124943423745581147e+2;
   double       a1     = 0.16383943563021534222e+2;
   double       a2     = -0.78956112887491257267e+0;
   double       b0     = -0.76949932108494879777e+3;
   double       b1     = 0.31203222091924532844e+3;
   double       b2     = -0.35667977739034646171e+2; /* Note b3 = 1.0 */
   double       c1     = 0.693359375;                /*  355/512      */
   double       c2     = -2.121944400546905827679e-4;
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

   X  = divdpMod_powdpi(zn, zd);
   W  = X * X;
   Bd = ((((W + b2) * W) + b1) * W) + b0;
   Cn = (((W * a2) + a1) * W) + a0;
   Rz = W * divdpMod_powdpi(Cn, Bd);
   Sa = X + (X * Rz);
   Cn = (double) N;
   Da = ((Cn * c2) + Sa) + (Cn * c1);

   if (Y > MAXe) {
      Da = 709.782712893384;
   }

   return (Da);
}

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_exp_pow_scalar_ci, ".text:optci");
#endif

template <> inline float MATHLIB_exp_pow_scalar_ci<float>(float a)
{
   float                 log2_base_x16 = 1.442695041f * 16.0f;
   float                 Halfe         = 0.5f;
   float                 LnMine        = -87.33654475f;
   float                 LnMaxe        = 88.72283905f;
   float                 Maxe          = 3.402823466E+38f;
   float                 c0            = 0.1667361910f;
   float                 c1            = 0.4999999651f;
   float                 c2            = 0.9999998881f;
   float                 P1            = 0.04331970214844f;
   float                 P2            = 1.99663646e-6f;
   float                 pol, r, r2, r3, res;
   unsigned int          Ttemp;
   unsigned /*!blw*/ int J, K;
   float                 Nf;
   int                   N;
   double                dT;

   /* Get N such that |N - x*16/ln(2)| is minimized */
   Nf = (a * log2_base_x16) + Halfe;
   N  = (int) Nf; /* Cast from intermediate variable to appease MISRA */

   if ((a * log2_base_x16) < -Halfe) {
      N--;
   }

   /* Argument reduction, r, and polynomial approximation pol(r) */
   r  = (a - (P1 * (float) N)) - (P2 * (float) N);
   r2 = r * r;
   r3 = r * r2;

   pol = (r * c2) + ((r3 * c0) + (r2 * c1));

   /* Get index for ktable and jtable */
   K  = _extu((unsigned int /*!blw*/) N, 28u, 30u);
   J  = (unsigned int /*!blw*/) N & 0x3u;
   dT = MATHLIB_kTable[K] * MATHLIB_jTable[J];

   /* Scale exponent to adjust for 2^M */
   Ttemp = _hi(dT) + (((unsigned int) /*!blw*/ N >> 4) << 20);
   dT    = _itod(Ttemp, _lo(dT));

   res = (float) (dT * (1.0 + (double) pol));

   /* Early exit for small a */
   if (_extu(_ftoi(a), 1u, 24u) < 114u) {
      res = 1.0f + a;
   }

   /* < LnMin returns 0 */
   if (a < LnMine) {
      res = 0.0f;
   }

   /* > LnMax returns MAX */
   if (a > LnMaxe) {
      res = Maxe;
   }

   return (res);
}

template <> inline double MATHLIB_exp_pow_scalar_ci<double>(double a)
{
   double Halfe  = 0.5;
   double Maxe   = 1.7976931348623157e+308;
   double LnMaxe = 709.78271289338;
   double LnMine = -708.3964185322641;
   double a0e    = 0.249999999999999993;
   double a1e    = 0.694360001511792852e-2;
   double a2e    = 0.165203300268279130e-4;
   double b0e    = 0.5;
   double b1e    = 0.555538666969001188e-1;
   double b2e    = 0.495862884905441294e-3;
   double c1e    = 0.693359375; /* 355/512 */
   double C2e    = -2.1219444005469058277e-4;
   double L2e    = 1.4426950408889634074; /* log (base 2) of e */
   double Ye, Xe, We, Re, Se, Be, Ce, De;
   int    Ne, upper;

   Ye    = a;
   Ce    = Ye * L2e;
   Ne    = _dpint(Ce);
   Se    = (double) Ne;                    /* double(int N) */
   Xe    = (Ye - (Se * c1e)) - (Se * C2e); /*  range reduction  */
   We    = Xe * Xe;
   Be    = (((b2e * We) + b1e) * We) + b0e;        /* denominator  */
   De    = ((((a2e * We) + a1e) * We) + a0e) * Xe; /* numerator  */
   Re    = Halfe + divdpMod_powdpi(De, Be - De);
   upper = 1024 + Ne;
   upper = (int) _extu((unsigned int) upper, 20u, 0u);
   Se    = _itod((unsigned int) upper, 0x00000000u);
   Ce    = Re * Se; /* scale by power of 2  */
   /* > LnMax returns MAX */
   if (a > LnMaxe) {
      Ce = Maxe;
   }

   /* > LnMin returns 0 */
   if (a < LnMine) {
      Ce = 0.0;
   }

   return (Ce);
}

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_pow_scalar_ci, ".text:optci");
#endif

template <> inline float MATHLIB_pow_scalar_ci<float>(float a, float b)
{
   float arg, W, X2;
   int   y, Sign = 1;

   /*!blw Negative base w/ odd-integer power should be negative */
   /*!blw this is not fully correct */
   y = _spint(b);
   if ((a < 0.0) && /*!blw*/ (b == (float) y)) {
      if ((y % 2 /*!blw*/) != 0) {
         Sign = -1;
      }
   }

   arg = _fabsf(a);
   W   = b * MATHLIB_log_pow_scalar_ci(arg);

   if (arg == 1.0f) {
      W = 0.0f;
   }

   X2 = (float /*!blw*/) Sign * MATHLIB_exp_pow_scalar_ci(W);

   if ((a < 0.0f) && /*!blw*/ (b != (float) y)) { /*!blw combine with other sign check*/
      X2 = _itof(0x7fffffffu);
   }

   if (a == 0.0f) {
      X2 = (b >= 0.0f) ? 0.0f : _itof(0x7F800000u);
   }

   if (b == 0.0f) {
      X2 = 1.0f;
   }

   return (X2);
}

template <> inline double MATHLIB_pow_scalar_ci<double>(double a, double b)
{
   double arg, W, X2;
   int    y;
   int    Sign = 1;

   y = _dpint(b);

   if ((a < 0.0) && (b == (double) y)) {
      if ((y % 2) != 0) {
         Sign = -1;
      }
   }
   arg = _fabs(a);

   W = b * MATHLIB_log_pow_scalar_ci(arg);
   if (arg == 1.0) {
      W = 0.0;
   }

   X2 = (double) Sign * MATHLIB_exp_pow_scalar_ci(W);

   if ((a < 0.0) && (b != (double) y)) {
      X2 = _lltod(0x7fffffffffffffff);
   }

   if ((a == 0.0) && (b < 0.0)) {
      X2 = _lltod(0x7ff0000000000000);
   }

   return (X2);
}

extern "C" {
static inline float MATHLIB_pow_scalar_sp(float a, float b)
{
   float result = MATHLIB_pow_scalar_ci<float>(a, b);
   return result;
}

static inline double MATHLIB_pow_scalar_dp(double a, double b)
{
   double result = MATHLIB_pow_scalar_ci<double>(a, b);
   return result;
}
}

#endif // MATHLIB_POW_SCALAR_H_
