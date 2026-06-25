// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_ACOSH_SCALAR_H_
#define MATHLIB_ACOSH_SCALAR_H_

#include "../common/MATHLIB_scalarTables.h"
#include "../common/MATHLIB_types.h"
#include "c6x_migration.h"
#include <float.h>

static inline float logsp_acoshsp_i(float x);
static inline float sqrtsp_acoshsp_i(float a, float x);
static inline float MATHLIB_acosh_scalar_ci(float x);

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(logsp_acoshsp_i, ".text:optci");
#endif

/* ======================================================================== */
/* This function returns the logarithm value of a real floating-point       */
/* argument x. The return value is the base e logarithmic value of x.       */
/* ======================================================================== */

static inline float logsp_acoshsp_i(float x)
{
   /* coefficients for the polynomial p(r) */
   const float c1 = -0.2302894f;
   const float c2 = 0.1908169f;
   const float c3 = -0.2505905f;
   const float c4 = 0.3333164f;
   const float c5 = -0.5000002f;

   const double ln2 = 0.693147180559945;
   const float  max = 88.7228390519551f;
   float        pol, r1, r2, r3, r4, res;
   double       dr, frcpax, rcp, T;
   unsigned int T_index;
   int          N;

   /* r = x * frcpa(x) -1 */
   rcp    = _rcpdp((double) x);
   frcpax = _itod(_clr(_hi(rcp), 0u, 16u), 0u);
   dr     = (frcpax * (double) x) - 1.0;

   /* calculate powers of r */
   r1 = (float) dr;
   r2 = r1 * r1;
   r3 = r1 * r2;
   r4 = r2 * r2;

   /* Polynomial p(r) that approximates ln(1+r) - r */
   pol = (c5 * r2) + ((c4 * r3) + ((((c2 * r1) + c3) + (c1 * r2)) * r4));

   /* Reconstruction: result = T + r + p(r) */
   N       = (int) _extu(_hi(frcpax), 1u, 21u) - 1023;
   T_index = _extu(_hi(frcpax), 12u, 29u);
   T       = MATHLIB_logTable[T_index] - (ln2 * (double) N);
   res     = (float) (dr + T) + pol;

   if (x > FLT_MAX) {
      res = max;
   }

   return (res);
} /* logsp_acoshsp_i */

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(sqrtsp_acoshsp_i, ".text:optci");
#endif

/* ======================================================================== */
/* This function returns the square root of the argument a. This function   */
/* has been modified to return the argument x when a = x*x. The argument a  */
/* is equal to x * x + 1, if a = x* x then +1 is irrelevant or x * x        */
/* overflows and the real sqrt of a is lost.                                */
/* ======================================================================== */

static inline float sqrtsp_acoshsp_i(float a, float x)
{
   const float half  = 0.5f;
   const float OneP5 = 1.5f;
   float       x0, x1, x2, x_half, res;

   x_half = a * half;
   x0     = _rsqrsp(a);

   x1  = OneP5 - (x_half * x0 * x0);
   x1  = x0 * x1;
   x2  = x1 * (OneP5 - (x1 * x1 * x_half));
   res = a * x2;

   if (a == (x * x)) {
      res = x;
   }

   if (a == 0.0f) {
      res = 0.0f;
   }

   return res;
} /* End of sqrtsp_acoshsp_i */

#ifndef __cplusplus /* FOR PROTECTION PURPOSE - C++ NOT SUPPORTED. */
#pragma CODE_SECTION(MATHLIB_acosh_scalar_ci, ".text:optci");
#endif

/* ======================================================================== */
/* This function returns the value of acosh(x), which is estimated as       */
/* follows:                                                                 */
/*   res = ln( x + sqrt (x^2 + 1))                                          */
/* where x is the input, sqrt stands for the square root function, and      */
/* res is the calculated value for acosh(x). This equation is modified as   */
/* follows to avoid overflow for a large x;                                 */
/*     ln([x + sqrt(x^2 + 1)]/2] + ln(2)                                    */
/* ======================================================================== */

static inline float MATHLIB_acosh_scalar_ci(float x)
{
   const float ln2  = 0.69314718056f; /* ln(2) */
   const float half = 0.5f;

   float res, sqrt_, temp, x2;

   x2 = x * x; /* x^2 */

   sqrt_ = sqrtsp_acoshsp_i(x2 - 1.0f, x); /* sqrt(x^2 + 1) */
   temp  = (sqrt_ * half) + (x * half);    /* (x+sqrt(x^2 + 1))/2 */
   res   = logsp_acoshsp_i(temp) + ln2;    /* ln((x + sqrt(x^2 + 1))/2) +ln(2) */

   if (x < 1.0f) {
      res = _itof(0x7FFFFFFFu); /* NaN */
   }

   return res;
}

#endif /* ACOSHSP_H_ */
