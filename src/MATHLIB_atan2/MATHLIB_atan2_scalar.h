// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ATAN2SP_I_H_
#define ATAN2SP_I_H_ 1

#include "../common/MATHLIB_utility.h"
#include <c6x_migration.h>
#include <stdbool.h>

typedef bool t_bool;
#define MATHLIB_TRUE ((t_bool) true)
#define MATHLIB_FALSE ((t_bool) false)

static double ti_math_vTable[4] = {0.00000000000000000000, 0.52359877559829887308, 1.57079632679489661923,
                                   1.04719755119659774615};

static inline float divspMod_atan2spi(float a, float b);
static inline float atan2f_sr1i_atan2spi(float g1, float pih, t_bool s, t_bool bn, t_bool an);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(divspMod_atan2spi, ".text:optci");
#endif

/* Pull in inline for divsp */
static inline float divspMod_atan2spi(float a, float b) { return cmn_DIVSP(b, a); }

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(atan2f_sr1i_atan2spi, ".text:optci");
#endif

static inline float atan2f_sr1i_atan2spi(float g1, float pih, t_bool s, t_bool bn, t_bool an)
{
   float coef;
   float g2;
   float g4;
   float g6;
   float g8;
   float g10;
   float g12;
   float pol;
   float tmp1;
   float tmp2;
   float c1 = 0.00230158202f;
   float c2 = -0.01394551000f;
   float c3 = 0.03937087815f;
   float c4 = -0.07235669163f;
   float c5 = 0.10521499322f;
   float c6 = -0.14175076797f;
   float c7 = 0.19989300877f;
   float c8 = -0.33332930041f;

   /* get coef based on the flags */
   coef = pih;
   if (s == MATHLIB_FALSE) {
      coef = 3.1415927f;
   }

   /* a & b have not been swapped, and b is negative*/
   if ((s == MATHLIB_FALSE) && (bn == MATHLIB_FALSE)) {
      coef = 0.0f;
   }

   /* MISRA requires explicit checks, != MATHLIB_FALSE is faster than == MATHLIB_TRUE*/
   if (an != MATHLIB_FALSE) {
      coef = -coef;
   }

   /* calculate polynomial */
   g2  = g1 * g1;
   g4  = g2 * g2;
   g6  = g2 * g4;
   g8  = g4 * g4;
   g10 = g6 * g4;
   g12 = g8 * g4;

   tmp1 = (c5 * g8) + (c6 * g6) + (c7 * g4) + (c8 * g2);
   tmp2 = (((c1 * g4) + (c2 * g2) + c3) * g12) + (c4 * g10);

   pol = tmp1 + tmp2;
   pol = (pol * g1) + g1;

   /* MISRA requires explicit checks, != MATHLIB_FALSE is faster than == MATHLIB_TRUE*/
   return ((s != MATHLIB_FALSE) ? (coef - pol) : (coef + pol));
}

/* Pull in inline for divdp */
static inline double divdpMod_atan2dpi(double a, double b) { return cmn_DIVDP(a, b); }

static inline double atandpMod_atan2dpi(double a)
{
   double p0    = -1.3688768894191926929e+1;
   double p1    = -2.0505855195861651981e+1;
   double p2    = -8.4946240351320683534e+0;
   double p3    = -8.3758299368150059274e-1;
   double q0    = 4.1066306682575781263e+1;
   double q1    = 8.6157349597130242515e+1;
   double q2    = 5.9578436142597344465e+1;
   double q3    = 1.5024001160028576121e+1;
   double sqrt3 = 1.7320508075688772935e+0;
   double iims3 = 2.6794919243112270647e-1;
   double F, G, H, R, RN, RD;
   int    N, Sign;

   Sign = 0;
   F    = a;
   N    = 0;

   if (F < 0.0) {
      F    = -F;
      Sign = 1;
   }

   if (F > 1.0) {
      F = divdpMod_atan2dpi(1.0, F);
      N = 2;
   }

   if (F > iims3) {
      N = N + 1;
      F = divdpMod_atan2dpi((F * sqrt3) - 1.0, F + sqrt3);
   }

   H = F;
   H = _fabs(H);

   G  = H * H;
   RN = ((((((p3 * G) + p2) * G) + p1) * G) + p0) * G;
   RD = ((((((G + q3) * G) + q2) * G) + q1) * G) + q0;
   R  = divdpMod_atan2dpi(RN, RD);

   F = F + (F * R);

   if (N > 1) {
      F = -F;
   }

   H = F + ti_math_vTable[N];

   if (Sign == 1) {
      H = -H;
   }

   return (H);
}

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(atan2sp_i, ".text:optci");
#endif

template <typename T> static inline T MATHLIB_atan2_scalar_ci(T a, T b);

template <> inline float MATHLIB_atan2_scalar_ci<float>(float a, float b)
{
   float  g, x, y;
   float  res;
   float  temp;
   float  pih = 1.570796327f;
   float  pi  = 3.141592741f;
   t_bool an;
   t_bool bn;
   t_bool s = MATHLIB_FALSE;

   x  = a;
   y  = b;
   an = (a < 0.0f) ? MATHLIB_TRUE : MATHLIB_FALSE; /* flag for a negative */
   bn = (b < 0.0f) ? MATHLIB_TRUE : MATHLIB_FALSE; /* flag for b negative */

   /* swap a and b before calling division sub routine if a > b */
   if (_fabsf(a) > _fabsf(b)) {
      temp = b;
      b    = a;
      a    = temp;
      s    = MATHLIB_TRUE; /* swap flag */
   }

   g = divspMod_atan2spi(b, a);

   /* do polynomial estimation */
   res = atan2f_sr1i_atan2spi(g, pih, s, bn, an);

   /* switch the returns so that the answer is equivalent */
   if (x == 0.0f) {
      res = y >= 0.0f ? 0.0f : pi;
   }

#if 0
   /* As the values of a and b are getting swapped the value of g is never going 
      to be out of range [-1, 1]. Hence following condition will never hit*/
   if (g > Max) {
      res = pih;
   }
   if (g < -Max) {
      res = -pih;
   }
#else
   /* Mathematically when y=0, value of res should be + / - pi depending on the sign of x
      This condition can be used insed of checking g value. */
   if (y == 0.0f && x > 0) {
      res = pih;
   }
   if (y == 0.0f && x < 0) {
      res = -pih;
   }
#endif

   return (res);
}

template <> inline double MATHLIB_atan2_scalar_ci<double>(double a, double b)
{
   double HalfPI     = 1.57079632679489661923;
   double MATHLIB_PI = 3.14159265358979323846;
   double Maxv       = 1.7976931348623157e+308;
   double X, Y, Z, W, res;

   Y = a;
   X = b;

   Z = divdpMod_atan2dpi(Y, X);

   W = atandpMod_atan2dpi(Z);

   res = W;

   if (X < 0.0) {
      res = MATHLIB_PI + W;
      if (W > 0.0) {
         res = W - MATHLIB_PI;
      }
   }

   if (X == 0.0f) {
      res = Y > 0.0f ? HalfPI : -HalfPI;
   }

   if (Y == 0.0f) {
      res = X >= 0.0f ? 0.0 : MATHLIB_PI;
   }

   if (Z > Maxv) {
      res = HalfPI;
   }

   if (Z < -Maxv) {
      res = -HalfPI;
   }

   return (res);
}

extern "C" {
static inline float MATHLIB_atan2_scalar_sp(float a, float b)
{
   float result = MATHLIB_atan2_scalar_ci<float>(a, b);
   return result;
}

static inline double MATHLIB_atan2_scalar_dp(double a, double b)
{
   double result = MATHLIB_atan2_scalar_ci<double>(a, b);
   return result;
}
}

#endif /* ATAN2SP_I_H_ */
