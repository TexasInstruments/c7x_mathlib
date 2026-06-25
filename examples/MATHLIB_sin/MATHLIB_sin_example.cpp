// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include "mathlib.h"
#include <stdint.h>

/******************************************************************************/
/*                                                                            */
/* main                                                                       */
/*                                                                            */
/******************************************************************************/

int main(void)
{

   // Setup input and output buffers for single precision datatypes
   float inSp[] = {0.,         0.78539816, 1.57079633, 2.35619449, 3.14159265, 3.92699082, 4.71238898,
                   5.49778714, 0.,         0.78539816, 1.57079633, 2.35619449, 3.14159265, 3.92699082};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_cos
   MATHLIB_sin(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("sin(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_sin_sp(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("sin(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   return 0;
}
