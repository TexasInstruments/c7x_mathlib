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
   float inSp[] = {0.,         0.48332195, 0.96664389, 1.44996584, 1.93328779, 2.41660973, 2.89993168,
                   3.38325363, 3.86657557, 4.34989752, 4.83321947, 5.31654141, 5.79986336, 6.28318531};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_tan
   MATHLIB_tan(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("tan(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_tan_sp(length, inSp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("tan(%10g) = %10g\n", inSp[c], outSp[c]);
   }

   return 0;
}
