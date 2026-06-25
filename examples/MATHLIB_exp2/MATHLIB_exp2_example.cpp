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

   // Setup input and output buffers for single-precision datatypes
   float inSp[] = {0., 0.5, 1., 1.5, 2., 2.5, 3, 3.5, 4., 4.5, 5., 5.5, 6., 6.5};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_exp2
   MATHLIB_exp2(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("exp2(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_exp2_sp(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("exp2(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   return 0;
}
