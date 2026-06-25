// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_DEBUGPRINT_H_
#define MATHLIB_DEBUGPRINT_H_ 1

#include "c7x_scalable.h"
#include "stdio.h"

/**
 * \ingroup MATHLIB_MMA_COMMON
 */

/**@{*/

/**
 * @brief This method prints the contents of a vector datatype variable
 *
 * @tparam T        : vector datatype
 * @param  vec [in] : input vector to be printed
 *
 * @remarks None
 */

template <typename vecType> static inline void MATHLIB_printVec(vecType vec)
{
   int32_t i            = 0;
   int32_t elementCount = c7x::element_count_of<vecType>::value;

   printf("\n\n");
   for (i = 0; i < elementCount; i++) {
      printf("%f, ", vec.s[i]);
   }
   printf("\n\n");
}

/** @}*/

#endif // MATHLIB_DEBUGPRINT_H_
