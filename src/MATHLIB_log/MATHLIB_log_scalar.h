// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_LOG_SCALAR_H_
#define MATHLIB_LOG_SCALAR_H_

#include "../common/MATHLIB_scalarTables.h"
#include "../common/MATHLIB_types.h"
#include "c6x_migration.h"

static inline float MATHLIB_log_scalar_ci(float a)
{
   double       ln2  = 0.693147180559945f;
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

   /* Reconstruction: result = T + r + p(r) */
   N       = (int) _extu(_hi(frcpax), 1u, 21u) - 1023;
   T_index = _extu(_hi(frcpax), 12u, 29u);
   T       = MATHLIB_logTable[T_index] - (ln2 * (double) N);
   res     = (float) (dr + T) + pol;

   if (a <= 0.0f) {
      res = _itof(0xFF800000u);
   }
   if (a > MAXe) {
      res = 709.7827f;
   }

   return (res);
}

#endif // MATHLIB_LOG_SCALAR_H_
