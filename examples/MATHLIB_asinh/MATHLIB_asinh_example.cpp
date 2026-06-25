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
   float inSp[] = {-2.14015943, 3.41413255,  -9.42016374, -0.82754576, -9.37322153, 6.83287024,  1.02197297,
                   -5.59255044, -2.70021723, -7.32790219, -9.57112329, -2.09970598, -5.60389267, -9.62063713};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_asinh
   MATHLIB_asinh(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("asinh(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_asinh_sp(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("asinh(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   return 0;
}
