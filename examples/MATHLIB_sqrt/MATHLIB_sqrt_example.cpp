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
   float inSp[] = {0., 0.5, 1., 1.5, 2., 2.5, 3, 3.5, 4., 4.5, 5., 5.5, 6., 6.5};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_sqrt
   MATHLIB_sqrt(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("sqrt(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_sqrt_sp(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("sqrt(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   return 0;
}
