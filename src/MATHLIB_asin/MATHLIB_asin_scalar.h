// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_ASIN_SCALAR_H_
#define MATHLIB_ASIN_SCALAR_H_ 1

#include <c6x_migration.h>

static inline float sqrtsp_asinsp_i(float x);
static inline float pol_est_asinsp_i(float x);
static inline float MATHLIB_asin_scalar(float x);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_asin_scalar, ".text:optci");
#endif

/* =========================================================================== */
/* The sqrtsp function returns the square root of a real floating-point value. */
/* =========================================================================== */

static inline float sqrtsp_asinsp_i(float x)
{
   const float half  = 0.5f;
   const float OneP5 = 1.5f;
   float       y, y0, y1, y2, x_half;

   x_half = x * half;
   y0     = _rsqrsp(x); /* y0 = 1/ sqrt(x) */

   y1 = OneP5 - (y0 * y0 * x_half);
   y1 = y0 * y1;
   y2 = y1 * (OneP5 - (y1 * y1 * x_half));
   y  = x * y2;

   if (x <= 0.0f) {
      y = 0.0f;
   }

   return y;
} /* sqrtsp_asinsp_i */

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(pol_est_asinsp_i, ".text:optci");
#endif

/* ======================================================================== */
/* Polynomial calculation to estimate the arc_sine funtion.                 */
/* The polynomial used is as follows:                                       */
/*   pol = x + c2 x^3 + c4 x^5 + c6 x^7 + c8 x^9 + c10 x^11 + c12 x^13 +    */
/*          c14 x^15 + c16 x^17,                                            */
/* where x is the input, c2 through c16 are the corresponding coefficients  */
/* to the polynomial, and pol is the result of the polynomial. This         */
/* polynomial only covers inputs in the range [0, 1/sqrt(2)].               */
/* ======================================================================== */

static inline float pol_est_asinsp_i(float x)
{
   /* coefficients for the polynomial for asin(x) */
   const float c16 = 0.053002771381990f;
   const float c14 = -0.010980624698693f;
   const float c12 = 0.020659425186833f;
   const float c10 = 0.022862784546374f;
   const float c8  = 0.030636056280974f;
   const float c6  = 0.044450959710588f;
   const float c4  = 0.075034659380970f;
   const float c2  = 0.166664771293503f;

   float x2, x4, x6, x8, x10, x12;
   float pol, tmp1, tmp2;

   /* calculate the powers of x */
   x2  = x * x;
   x4  = x2 * x2;
   x6  = x2 * x4;
   x8  = x4 * x4;
   x10 = x6 * x4;
   x12 = x8 * x4;

   /* ====================================================================== */
   /* The polynomial calculation is done in two seperate parts.              */
   /*   tmp1 =  c2 x^2 + c4 x^4 + c6 x^6 + c8 x^8                            */
   /*   tmp2 =  c10 x^10 + c12 x^12 + c14 x^14 + c16 x^16                    */
   /* In order to reduce the number of multiplications x is factored out of  */
   /* the polynomial and multiplied by later.                                */
   /* ====================================================================== */

   tmp1 = ((c8 * x8) + (c6 * x6)) + ((c4 * x4) + (c2 * x2));
   tmp2 = ((((c16 * x4) + (c14 * x2)) + c12) * x12) + (c10 * x10);

   pol = tmp1 + tmp2;
   pol = (pol * x) + x;

   return pol;
} /* pol_est_asinsp_i */

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(asinsp_i, ".text:optci");
#endif

/* ====================================================================== */
/* The type of calculation for asin(x) depends on the value of x:         */
/*                                                                        */
/* for x_abs <= 1/sqrt(2), res = pol_est_asinsp_i (input x)               */
/* for x_abs > 1/sqrt(2),  res = pi/2 - pol_est_asinsp_i (input a)        */
/*                         a = sqrt(1 - x^2)                              */
/* where x_abs is the absolute value of the input, a is calculated as     */
/* shown above and it's used as an input for the polynomial, and res is   */
/* the value for asin(x).                                                 */
/* ====================================================================== */

static inline float MATHLIB_asin_scalar(float x)
{
   const float pi2   = 1.570796327f; /* pi/2 */
   const float rsqr2 = 0.7071067811f;
   float       s     = 1.0f;
   float       res, x_abs, a, temp;

   x_abs = _fabsf(x);

   if (x_abs > rsqr2) { /* |x| > 1/sqrt(2) */
      temp = 1.0f - (x_abs * x_abs);
      a    = sqrtsp_asinsp_i(temp); /* a= sqrt(1 - x^2) */
      temp = pol_est_asinsp_i(a);
      res  = pi2 - temp;
   }
   else { /* |x| <= 1/sqrt(2) */
      res = pol_est_asinsp_i(x_abs);
   }

   if (x < 0.0f) {
      s = -s; /* sign var */
   }

   if (x_abs > 1.0f) {
      res = _itof(0x7FFFFFFFu); /* NaN */
   }

   return (res * s); /* restore sign for quadrant 3 & 4*/
} /* asinsp_i */

#endif /* MATHLIB_ASIN_SCALAR_H_ */
