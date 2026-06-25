// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"

template <typename vecType> static inline vecType MATHLIB_exp2_div(vecType a, vecType b)
{
   vecType TWO = 2.0f;
   vecType X;
   X = __recip(b);
   X = X * (TWO - (b * X));
   X = X * (TWO - (b * X));
   X = a * X;
   return X;
}

// this method performs exp2 compution of input vector
template <typename T> MATHLIB_STATUS MATHLIB_exp2(size_t length, T *pSrc, T *pDst)
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

      // open SE0, SE1, and SA0 for reading and writing operands
      MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);

      /***********************************************************************/
      /* Create and assign values for constants employed on exp2 computation */
      /***********************************************************************/

      vec zero, half, epsilon, minInput, maxVal, maxInput, A0, A1, B0, B1, CC1, CC2, ln2E, lnE2;

      zero     = (vec) 0.0f;
      half     = (vec) 0.5f;
      epsilon  = (vec) 9.313225746e-10f; // [2^-29] / 2
      minInput = (vec) -126.0f;
      maxVal   = (vec) 3.40282347e+38f;
      maxInput = (vec) 128.0f;
      A0       = (vec) 2.499999995e-1f;
      A1       = (vec) 4.1602886268e-3f;
      B0       = (vec) 0.5f;
      B1       = (vec) 4.9987178778e-2f;
      CC1      = (vec) 0.693359375f;       // 355/512
      CC2      = (vec) -2.12194440055e-4f; // lne(2) - 355/512
      ln2E     = (vec) 1.442695040889f;    // ln(base 2) of e
      lnE2     = (vec) 0.69314718056f;     // log (base e) of 2

      // compute loop to perform vector exp2
      for (size_t i = 0; i < numBlocks; i++) {
         vec inVec = c7x::strm_eng<0, vec>::get_adv();

         /***********************************************************************/
         /* Create and assign values for variables employed on exp2 computation */
         /***********************************************************************/

         vec           Y, X, W, R, S, B, C, D, absY;
         c7x::int_vec  N;
         c7x::uint_vec uN, uNShift;

         Y = inVec * lnE2;

         absY = __abs(Y);

         __vpred cmp_Y = __cmp_lt_pred(absY, epsilon);
         Y             = __select(cmp_Y, zero, Y);

         // Convert base e --> base 2 argument
         C = Y * ln2E;

         // Get unbiased exponent as int
         N = __float_to_int(C);
         S = __int_to_float(N);

         // Range reduction
         X = (Y - (S * CC1)) - (S * CC2);
         W = X * X;

         /**********************************************************************/
         /* Calculate numerator and denominator                                */
         /**********************************************************************/
         B = (B1 * W) + B0;
         D = ((A1 * W) + A0) * X;

         R       = half + MATHLIB_exp2_div(D, (B - D));
         uN      = c7x::convert<c7x::uint_vec>(N);
         uN      = uN + 128;
         uNShift = uN << 23u;
         S       = c7x::reinterpret<c7x::float_vec>(uNShift);

         // Scale final answer by power of 2
         C = R * S;

         /**********************************************************************/
         /* Bounds checking                                                    */
         /**********************************************************************/

         __vpred cmp_min = __cmp_lt_pred(inVec, minInput);
         C               = __select(cmp_min, zero, C);

         __vpred cmp_max = __cmp_lt_pred(maxInput, inVec);
         C               = __select(cmp_max, maxVal, C);

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, C);
      }

      MATHLIB_SE0SA0Close();
   }

   return status;
}

// single precision
template MATHLIB_STATUS MATHLIB_exp2<float>(size_t length, float *pSrc, float *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_exp2_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_exp2(length, pSrc, pDst);
   return status;
}

} // extern "C"
