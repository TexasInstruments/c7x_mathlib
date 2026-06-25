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
   float in0Sp[] = {0., 0.5, 1., 1.5, 2., 2.5, 3, 3.5, 4., 4.5, 5., 5.5, 6., 6.5};

   float in1Sp[] = {1.5, 3.5, 5.5, 7.5, 9.5, 11.5, 13.5, 15.5, 17.5, 20., 25., 30., 35., 40.};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_div
   MATHLIB_div(length, in0Sp, in1Sp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("div(%10g, %10g) = %10g\n", in0Sp[c], in1Sp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_div_sp(length, in0Sp, in1Sp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("div(%10g, %10g) = %10g\n", in0Sp[c], in1Sp[c], outSp[c]);
   }

   return 0;
}
