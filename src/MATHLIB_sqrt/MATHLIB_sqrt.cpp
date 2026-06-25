// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value
#define ELEMENT_TYPE(x) typename c7x::element_type_of<x>::type

#include "MATHLIB_sqrt_scalar.h"
#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"
#include <cstddef>
#include <limits>

template <typename T> static inline void MATHLIB_sqrt_vector(size_t length, T *pSrc, T *pDst);

// this method performs sqrt computation of input vector
template <> inline void MATHLIB_sqrt_vector<float>(size_t length, float *pSrc, float *pDst)
{

   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<c7x::float_vec>::type vec;

   /* define type of elements vec vector holds as elemType */
   typedef ELEMENT_TYPE(vec) elemType;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

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
   /* Create and assign values for constants employed on sqrt computation */
   /***********************************************************************/

   vec half, OneP5, zero, maxValue;

   half     = (vec) 0.5;
   OneP5    = (vec) 1.5;
   zero     = (vec) 0.0;
   maxValue = (vec) std::numeric_limits<elemType>::max();

   // compute loop to perform vector sqrt
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec = c7x::strm_eng<0, vec>::get_adv();

      /**********************************************************************/
      /* Create variables employed on sqrt computation                      */
      /**********************************************************************/

      vec y, p0, p1, d0;

      /**********************************************************************/
      /* Sqrt computation                                                   */
      /**********************************************************************/

      // Reciprocal square root calculation
      p0 = __recip_sqrt(inVec);
      d0 = p0 * inVec;
      p1 = OneP5 - d0 * p0 * half;
      y  = inVec * p0 * p1;

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      // If input is <= 0, output defaults to 0
      __vpred cmp_lezero = __cmp_le_pred(inVec, zero);
      y                  = __select(cmp_lezero, zero, y);

      // If input is greater than maximum allowed by data type, then output is max of datatype
      __vpred cmp_gtmax = __cmp_le_pred(maxValue, inVec);
      vec     outVec    = __select(cmp_gtmax, maxValue, y);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec);
   }

   MATHLIB_SE0SA0Close();
}

// this method performs sqrt computation of input vector
template <> inline void MATHLIB_sqrt_vector<double>(size_t length, double *pSrc, double *pDst)
{
   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<c7x::double_vec>::type vec;

   /* define type of elements vec vector holds as elemType */
   typedef ELEMENT_TYPE(vec) elemType;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc, pDst);

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   // open SE0, and SA0 for reading and writing operands
   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);

   /***********************************************************************/
   /* Create and assign values for constants employed on sqrt computation */
   /***********************************************************************/

   vec half, OneP5, zero, maxValue;

   half     = (vec) 0.5;
   OneP5    = (vec) 1.5;
   zero     = (vec) 0.0;
   maxValue = (vec) std::numeric_limits<elemType>::max();

   // compute loop to perform vector sqrt
   for (size_t i = 0; i < numBlocks; i++) {

      vec invec = c7x::strm_eng<0, vec>::get_adv();

      vec x = __recip_sqrt(invec);

      x = x * (OneP5 - (invec * x * x * half));
      x = x * (OneP5 - (invec * x * x * half));
      x = x * (OneP5 - (invec * x * x * half));

      vec y = invec * x;

      __vpred cond1 = __cmp_le_pred(invec, zero);
      y             = __select(cond1, zero, y);

      __vpred cond2 = __cmp_lt_pred(maxValue, invec);
      y             = __select(cond2, maxValue, y);

      __vpred temp = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(temp, addr, y);
   }
   MATHLIB_SE0SA0Close();
}

template <typename T> MATHLIB_STATUS MATHLIB_sqrt(size_t length, T *pSrc, T *pDst)
{

   MATHLIB_STATUS status = MATHLIB_SUCCESS; // return function status

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {

      if (length < 2) {
         pDst[0] = MATHLIB_sqrt_scalar_ci<T>(pSrc[0]);
      }
      else {
         MATHLIB_sqrt_vector<T>(length, pSrc, pDst);
      }
   }
   return status;
}

template MATHLIB_STATUS MATHLIB_sqrt<float>(size_t length, float *pSrc, float *pDst);
template MATHLIB_STATUS MATHLIB_sqrt<double>(size_t length, double *pSrc, double *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_sqrt_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_sqrt<float>(length, pSrc, pDst);
   return status;
}

// double-precision wrapper
MATHLIB_STATUS MATHLIB_sqrt_dp(size_t length, double *pSrc, double *pDst)
{
   MATHLIB_STATUS status = MATHLIB_sqrt<double>(length, pSrc, pDst);
   return status;
}

} // extern "C"
