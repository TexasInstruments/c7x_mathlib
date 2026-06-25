// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value
#define ELEMENT_TYPE(x) typename c7x::element_type_of<x>::type

#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"
#include <cstddef>

// this method performs rsqrt computation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_rsqrt(size_t length, T *pSrc, T *pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {
      MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc, pDst);

      // calculate compute loop's iteration counter
      numBlocks    = length / c7x::element_count_of<vec>::value;
      remNumBlocks = length % c7x::element_count_of<vec>::value;
      if (remNumBlocks) {
         numBlocks++;
      }

      // open SE0, SE1, and SA0 for reading and writing operands
      MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);

      /***********************************************************************/
      /* Create and assign values for constants employed on rsqrt computation */
      /***********************************************************************/

      vec half, OneP5, small, inf;

      half  = (vec) 0.5;
      OneP5 = (vec) 1.5;
      small = (vec) 1.17549435e-38f;
      inf   = (vec) 0x7F800000u;

      // compute loop to perform vector rsqrt
      for (size_t i = 0; i < numBlocks; i++) {
         vec inVec = c7x::strm_eng<0, vec>::get_adv();

         /**********************************************************************/
         /* Create variables employed on rsqrt computation                      */
         /**********************************************************************/

         vec x0, x1, x2, x3, outVec;

         /**********************************************************************/
         /* rsqrt computation                                                   */
         /**********************************************************************/

         // Reciprocal square root calculation
         x0 = __recip_sqrt(inVec);
         x1 = x0 * inVec;
         x3 = OneP5 - (x1 * x0 * half);
         x1 = x0 * x3;
         x2 = x1 * (OneP5 - (inVec * x1 * x1 * half));

         /**********************************************************************/
         /* Bounds checking                                                    */
         /**********************************************************************/

         // If input is <= 0, output defaults to 0
         __vpred cmp_lezero = __cmp_lt_pred(inVec, small);
         outVec             = __select(cmp_lezero, inf, x2);

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, outVec);
      }

      MATHLIB_SE0SA0Close();
   }

   return status;
}

// single precision
template MATHLIB_STATUS MATHLIB_rsqrt<float>(size_t length, float *pSrc, float *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_rsqrt_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_rsqrt(length, pSrc, pDst);
   return status;
}

} // extern "C"
