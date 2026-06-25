// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value
#define ELEMENT_TYPE(x) typename c7x::element_type_of<x>::type

#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"
#include <cstddef>

// this method performs division computation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_div(size_t length, T *pSrc0, T *pSrc1, T *pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // check for null pointers and non-zero length, pSrc0 check
   status = MATHLIB_checkParams(length, pSrc0, pSrc1, pDst);

   if (status == MATHLIB_SUCCESS) {

      MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc0, pDst);

      // calculate compute loop's iteration counter
      numBlocks    = length / c7x::element_count_of<vec>::value;
      remNumBlocks = length % c7x::element_count_of<vec>::value;
      if (remNumBlocks) {
         numBlocks++;
      }

      // open SE0, SE1, and SA0 for reading and writing operands
      MATHLIB_SE0SE1SA0Open(&se0Params, &sa0Params, pSrc0, pSrc1);

      /**********************************************************************/
      /* Create and assign values for constants employed on div computation */
      /**********************************************************************/

      vec Two;

      Two = (vec) 2.0;

      // compute loop to perform vector div
      for (size_t i = 0; i < numBlocks; i++) {
         vec a = c7x::strm_eng<0, vec>::get_adv();
         vec b = c7x::strm_eng<1, vec>::get_adv();

         /**********************************************************************/
         /* Create variables employed on div computation                       */
         /**********************************************************************/

         vec d0, d1, p0, p1;

         /**********************************************************************/
         /* Div computation                                                    */
         /**********************************************************************/

         // Calculate reciprocal
         p0 = __recip(b);
         d0 = p0 * b;

         p1 = Two - d0;
         d1 = p0 * p1;

         vec outVec = a * d1;

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, outVec);
      }

      MATHLIB_SE0SE1SA0Close();
   }

   return status;
}

// single precision
template MATHLIB_STATUS MATHLIB_div<float>(size_t length, float *pSrc0, float *pSrc1, float *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_div_sp(size_t length, float *pSrc0, float *pSrc1, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_div(length, pSrc0, pSrc1, pDst);
   return status;
}

} // extern "C"
