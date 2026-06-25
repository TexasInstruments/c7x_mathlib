// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include "MATHLIB_lut.h"
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
   float inSp1[] = {1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5};
   float inSp2[] = {0.0, 1.0, 2.0, 3.0, 0.0, 1.0, 2.0, 3.0, 4.0, 0.0, 1.0, 2.0, 3.0, 0.0};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

#if !defined(__C7504__)
   MATHLIB_LUTInit();
#endif

   // call single-precision version of MATHLIB_cos
   MATHLIB_pow(length, inSp1, inSp2, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("pow(%10g, %10g) = %10g\n", inSp1[c], inSp2[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_pow_sp(length, inSp1, inSp2, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("pow(%10g, %10g) = %10g\n", inSp1[c], inSp2[c], outSp[c]);
   }

   return 0;
}
