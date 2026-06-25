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
   float inSp[] = {1.,          0.97094182,  0.88545603,  0.74851075,  0.56806475,  0.35460489,  0.12053668,
                   -0.12053668, -0.35460489, -0.56806475, -0.74851075, -0.88545603, -0.97094182, -1.};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_atan
   MATHLIB_atan(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("atan(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_atan_sp(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("atan(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   return 0;
}
