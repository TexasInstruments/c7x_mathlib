// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_ilut.h"
#include "MATHLIB_log2_scalar.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_permute.h"
#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"

#if defined(__C7504__)
template <typename T> static inline void MATHLIB_log2_scalar(size_t length, T *restrict pSrc, T *restrict pDst)
{
   // Call scalar code
   for (uint32_t i = 0; i < length; i++) {
      pDst[i] = MATHLIB_log2_scalar_ci(pSrc[i]);
   }
}
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
// this method performs log2 computation of input vector
template <typename T> static inline void MATHLIB_log2_vector(size_t length, T *restrict pSrc, T *restrict pDst)
{
   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<float>::type vec;

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
   /* Create and assign values for constants employed on log2 computation */
   /***********************************************************************/

   vec             C1, C2, C3, C4, C5, eMax, outVecMin, outVecMax;
   c7x::double_vec ln2, base;
   c7x::uint_vec   zero;
   zero = (c7x::uint_vec) 0;

   ln2       = (c7x::double_vec) 0.693147180559945;
   base      = (c7x::double_vec) 1.4426950408890;
   C1        = (vec) -0.2302894f;
   C2        = (vec) 0.1908169f;
   C3        = (vec) -0.2505905f;
   C4        = (vec) 0.3333164f;
   C5        = (vec) -0.5000002f;
   eMax      = (vec) 3.402823466e+38f;
   outVecMin = (vec) 0xFF800000u;
   outVecMax = (vec) 1024.0f;

   // compute loop to perform vector log2
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec = c7x::strm_eng<0, vec>::get_adv();

      /**********************************************************************/
      /* Create variables employed on log2 computation                      */
      /**********************************************************************/
      vec             pol, r1, r2, r3, r4;
      c7x::double_vec inVecVals_odd, inVecVals_even, inVecVals_oddReciprocal, inVecVals_evenReciprocal,
          inVecReciprocalApprox_8_15, inVecReciprocalApprox_0_7, inVecVals_8_15, inVecVals_0_7, rVals_0_7, rVals_8_15,
          TVals_8_15, TVals_0_7, NVals_odd, NVals_even, NVals_0_7, NVals_8_15, outVec_8_15, outVec_0_7;
      c7x::uint_vec inVecReciprocal_32_63, inVecReciprocalClr_32_63, inVecReciprocalApprox_32_63, indexT;
      c7x::int_vec  N;
      vec           outVec;

      /**********************************************************************/
      /* Calculate Taylor series approximation for log2                     */
      /**********************************************************************/

      // Split vectors to compute r with double precision
      inVecVals_odd            = __high_float_to_double(inVec);
      inVecVals_even           = __low_float_to_double(inVec);
      inVecVals_oddReciprocal  = __recip(inVecVals_odd);
      inVecVals_evenReciprocal = __recip(inVecVals_even);

      // Create floating point reciprocal approximation
      // Upper 32 bits of all inVec reciprocal values
      inVecReciprocal_32_63 = c7x::reinterpret<c7x::uint_vec>(
          __permute_odd_odd_int(MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(inVecVals_oddReciprocal),
                                c7x::as_uchar_vec(inVecVals_evenReciprocal)));

      // Clear bits 0-16 inclusive
      inVecReciprocalClr_32_63 = inVecReciprocal_32_63 & 0xFFFE0000u;

      // Concatenate cleared bit reciprocal with zero bits
      inVecReciprocalApprox_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(inVecReciprocalClr_32_63), c7x::as_uchar_vec(zero)));
      inVecReciprocalApprox_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(inVecReciprocalClr_32_63), c7x::as_uchar_vec(zero)));

      // Split inVec into two vectors with double precision
      inVecVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(inVecVals_odd), c7x::as_uchar_vec(inVecVals_even)));
      inVecVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(inVecVals_odd), c7x::as_uchar_vec(inVecVals_even)));

      // Calculate r in double precision
      rVals_0_7  = (inVecReciprocalApprox_0_7 * inVecVals_0_7) - 1.0;
      rVals_8_15 = (inVecReciprocalApprox_8_15 * inVecVals_8_15) - 1.0;

      // Convert r to float, compute r to the power of 2, 3, 4
      r1 = c7x::reinterpret<vec>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                         c7x::as_uchar_vec(__double_to_float(rVals_8_15)),
                                                         c7x::as_uchar_vec(__double_to_float(rVals_0_7))));
      r2 = r1 * r1;
      r3 = r1 * r2;
      r4 = r2 * r2;

      // Compute Taylor series polynomial
      pol = (C5 * r2) + ((C4 * r3) + ((((C2 * r1) + C3) + (C1 * r2)) * r4));
      pol = pol * __double_to_float(base);

      /**********************************************************************/
      /* Calculate N                                                        */
      /**********************************************************************/

      // Upper 32 bits of all inVec reciprocal approximation values
      inVecReciprocalApprox_32_63 = c7x::reinterpret<c7x::uint_vec>(
          __permute_odd_odd_int(MATHLIB_vperm_data_0_63, c7x::as_uchar_vec(inVecReciprocalApprox_8_15),
                                c7x::as_uchar_vec(inVecReciprocalApprox_0_7)));

      N = c7x::convert<c7x::int_vec>(((inVecReciprocalApprox_32_63 << 1) >> 21) - 1023);

      // Covert N to double precision for later calculation with LUT values
      NVals_odd  = __high_int_to_double(N);
      NVals_even = __low_int_to_double(N);
      NVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(NVals_odd), c7x::as_uchar_vec(NVals_even)));
      NVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(NVals_odd), c7x::as_uchar_vec(NVals_even)));

      /**********************************************************************/
      /* Determine LUT values                                               */
      /**********************************************************************/

      // Calculate LUT index
      indexT                        = (((inVecReciprocalApprox_32_63 << 12) >> 29) + MATHLIB_LOGTABLE_OFFSET);

#if defined(__C7100__) || defined(__C7120__)
      c7x::uint_vec upperBitsIndexT = MATHLIB_LUTReadUpperBits(indexT);
      c7x::uint_vec lowerBitsIndexT = MATHLIB_LUTReadLowerBits(indexT);
#elif defined(__C7524__)
      c7x::uint_vec upperBitsIndexT = MATHLIB_ILUTReadUpperBits(indexT);
      c7x::uint_vec lowerBitsIndexT = MATHLIB_ILUTReadLowerBits(indexT);
#else
#error "Missing *LUT implementation"
#endif

      // Read from LUT and reconstruct double values split into two vectors
      TVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsIndexT), c7x::as_uchar_vec(lowerBitsIndexT)));
      TVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsIndexT), c7x::as_uchar_vec(lowerBitsIndexT)));

      // Calculate an adjusted T
      TVals_8_15 = (TVals_8_15 - (ln2 * NVals_8_15)) * base;
      TVals_0_7  = (TVals_0_7 - (ln2 * NVals_0_7)) * base;

      /**********************************************************************/
      /* Calculate output with adjusted LUT and Taylor series values        */
      /**********************************************************************/

      // Add LUT values and Taylor series values
      // TODO: Multiple by base
      outVec_0_7  = (rVals_0_7 * base) + TVals_0_7;
      outVec_8_15 = (rVals_8_15 * base) + TVals_8_15;

      // Combine output vector into one floating point result
      outVec = c7x::reinterpret<vec>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                             c7x::as_uchar_vec(__double_to_float(outVec_8_15)),
                                                             c7x::as_uchar_vec(__double_to_float(outVec_0_7))));
      outVec = outVec + pol;

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      __vpred cmp_min = __cmp_le_pred(inVec, c7x::convert<vec>(zero));
      outVec          = __select(cmp_min, outVecMin, outVec);

      __vpred cmp_max = __cmp_lt_pred(eMax, inVec);
      outVec          = __select(cmp_max, outVecMax, outVec);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec);
   }

   MATHLIB_SE0SA0Close();
}
#else
#error "Unknown target!"
#endif

// this method performs exponential computation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_log2(size_t length, T *restrict pSrc, T *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_SUCCESS; // return function status

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {
#if defined(__C7504__)
      MATHLIB_log2_scalar(length, pSrc, pDst);
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
      MATHLIB_log2_vector(length, pSrc, pDst);
#else
#error "Unknown Target"
#endif
   }
   return status;
}

// single precision
template MATHLIB_STATUS MATHLIB_log2<float>(size_t length, float *restrict pSrc, float *restrict pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_log2_sp(size_t length, float *restrict pSrc, float *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_log2(length, pSrc, pDst);
   return status;
}

} // extern "C"
