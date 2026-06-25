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
   float in0Sp[] = {1.,          0.97094182,  0.88545603,  0.74851075,  0.56806475,  0.35460489,  0.12053668,
                    -0.12053668, -0.35460489, -0.56806475, -0.74851075, -0.88545603, -0.97094182, -1.};

   float in1Sp[] = {0.97811795, 0.5650689,  0.15125672, 0.80331535, 0.76576596, 0.53121862, 0.42114974,
                    0.98006015, 0.36362693, 0.85114115, 0.06140721, 0.29113601, 0.70821542, 0.26322483};

   float outSp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   size_t length = 14;

   // call single-precision version of MATHLIB_atan
   MATHLIB_atan2(length, in0Sp, in1Sp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("atan2(%10g, %10g) = %10g\n", in0Sp[c], in1Sp[c], outSp[c]);
   }

   // single precision C interface
   MATHLIB_atan2_sp(length, in0Sp, in1Sp, outSp);
   // print results
   for (size_t c = 0; c < length; c++) {
      printf("atan2(%10g, %10g) = %10g\n", in0Sp[c], in1Sp[c], outSp[c]);
   }

   return 0;
}
