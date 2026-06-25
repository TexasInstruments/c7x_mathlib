// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"
#include <cstddef>

// this method performs reciprocal computation of input vector (float version)
template <typename T> MATHLIB_STATUS MATHLIB_recip(size_t length, T *pSrc, T *pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {

      MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc, pDst);

      // calculate compute loop's iteration counter
      numBlocks    = length / c7x::element_count_of<vec>::value;
      remNumBlocks = length % c7x::element_count_of<vec>::value;
      if (remNumBlocks) {
         numBlocks++;
      }

      // open SE0 and SA0 for reading and writing operands
      MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);

      /**********************************************************************/
      /* Create and assign values for constants employed on recip computation */
      /**********************************************************************/

      vec Two;
      vec p0, d0, p1;
      Two = (vec) 2.0;

      // compute loop to perform vector reciprocal
      // 23 + trip_cnt * 3
      for (size_t i = 0; i < numBlocks; i++) {
         vec a = c7x::strm_eng<0, vec>::get_adv();

         /**********************************************************************/
         /* Reciprocal computation using Newton-Raphson (2 iterations)         */
         /**********************************************************************/

         // Calculate reciprocal: initial estimate
         p0 = __recip(a);

         // Newton-Raphson iteration 1
         d0         = p0 * a;
         p1         = Two - d0;
         vec outVec = p0 * p1;

         // Newton-Raphson iteration 2
         d0     = outVec * a;
         p1     = Two - d0;
         outVec = outVec * p1;

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, outVec);
      }

      MATHLIB_SE0SA0Close();
   }

   return status;
}

// this method performs reciprocal computation of input vector (double version)
template <> MATHLIB_STATUS MATHLIB_recip<double>(size_t length, double *pSrc, double *pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type for double
   typedef typename c7x::make_full_vector<double>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {

      MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc, pDst);

      // calculate compute loop's iteration counter
      numBlocks    = length / c7x::element_count_of<vec>::value;
      remNumBlocks = length % c7x::element_count_of<vec>::value;
      if (remNumBlocks) {
         numBlocks++;
      }

      // open SE0 and SA0 for reading and writing operands
      MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);

      /**********************************************************************/
      /* Create and assign values for constants employed on recip computation */
      /**********************************************************************/

      vec Two;
      vec p0, d0, p1;
      Two = (vec) 2.0;

      // compute loop to perform vector reciprocal
      // 37 + trip_cnt * 6
      for (size_t i = 0; i < numBlocks; i++) {
         vec a = c7x::strm_eng<0, vec>::get_adv();

         /**********************************************************************/
         /* Reciprocal computation using Newton-Raphson (3 iterations)         */
         /**********************************************************************/

         // Calculate reciprocal: initial estimate
         p0 = __recip(a);

         // Newton-Raphson iteration 1
         d0         = p0 * a;
         p1         = Two - d0;
         vec outVec = p0 * p1;

         // Newton-Raphson iteration 2
         d0     = outVec * a;
         p1     = Two - d0;
         outVec = outVec * p1;

         // Newton-Raphson iteration 3
         d0     = outVec * a;
         p1     = Two - d0;
         outVec = outVec * p1;

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, outVec);
      }

      MATHLIB_SE0SA0Close();
   }

   return status;
}

// single precision (double is already specialized above)
template MATHLIB_STATUS MATHLIB_recip<float>(size_t length, float *pSrc, float *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_recip_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_recip(length, pSrc, pDst);
   return status;
}

// double-precision wrapper
MATHLIB_STATUS MATHLIB_recip_dp(size_t length, double *pSrc, double *pDst)
{
   MATHLIB_STATUS status = MATHLIB_recip(length, pSrc, pDst);
   return status;
}

} // extern "C"
