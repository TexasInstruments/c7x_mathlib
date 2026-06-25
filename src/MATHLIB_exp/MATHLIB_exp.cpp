// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_exp_scalar.h"
#include "MATHLIB_ilut.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_permute.h"
#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"
#if defined(__C7504__)
template <typename T> static inline void MATHLIB_exp_scalar(size_t length, T *restrict pSrc, T *restrict pDst)
{
   // Call scalar code
   for (uint32_t i = 0; i < length; i++) {
      pDst[i] = MATHLIB_exp_scalar_ci(pSrc[i]);
   }
}
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
template <typename T> static inline void MATHLIB_exp_vector(size_t length, T *pSrc, T *pDst)
{
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   // Compile-time decision: float_vec => int_vec and double_vec=> long_vec
   typedef
       typename std::conditional<ELEMENT_COUNT(c7x::float_vec) == ELEMENT_COUNT(vec), c7x::int_vec, c7x::long_vec>::type
           vec_type;

   typedef typename std::conditional<ELEMENT_COUNT(c7x::float_vec) == ELEMENT_COUNT(vec), c7x::uint_vec,
                                     c7x::ulong_vec>::type uvec_type;

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

   /**********************************************************************/
   /* Create and assign values for constants employed on exp computation */
   /**********************************************************************/

   vec             log2_base_x16, half, negativeHalf, zero, LnMin, LnMax, Max, C0, C1, C2;
   uvec_type       mask;
   c7x::double_vec p;

   log2_base_x16 = (vec) 23.083120654f;
   half          = (vec) 0.5f;
   negativeHalf  = (vec) -0.5f;
   zero          = (vec) 0.0f;
   LnMin         = (vec) -87.33654475f;
   LnMax         = (vec) 88.72283905f;
   Max           = (vec) 3.402823466E+38f;
   C0            = (vec) 0.166668549286041f;
   C1            = (vec) 0.500016170012920f;
   C2            = (vec) 0.999999998618401f;
   mask          = (uvec_type) 0x3u;
   p             = (c7x::double_vec) 0.0433216987816623;

   // compute loop to perform vector exp
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec = c7x::strm_eng<0, vec>::get();

      /**********************************************************************/
      /* Create variables employed on exp computation                       */
      /**********************************************************************/

      vec             pol, r, r2, r3, outVec, Nf, absNf, dT, rVals_odd, rVals_even;
      uvec_type       J, K, uN, dTAdjusted_32_63, dT_32_63, dT_0_31;
      vec_type        N, minusN;
      c7x::double_vec inVecVals_odd, inVecVals_even, NVals_odd, NVals_even, KVals_8_15, KVals_0_7, JVals_8_15,
          JVals_0_7, dTVals_8_15, dTVals_0_7;
      c7x::uint_vec upperBitsK, lowerBitsK, upperBitsJ, lowerBitsJ;

      // Get N such that |N - inVec*16/ln(2)| is minimized
      Nf     = inVec * log2_base_x16;
      absNf  = Nf + half;
      N      = c7x::convert<vec_type>(absNf);
      minusN = N - 1;

      //    N--;
      __vpred cmp_N = __cmp_lt_pred(Nf, negativeHalf);
      N             = __select(cmp_N, minusN, N);

      /**********************************************************************/
      /* Calculate Taylor series approximation for exp                      */
      /**********************************************************************/

      // Split vectors to compute r with double precision
      inVecVals_odd  = __high_float_to_double(inVec);
      inVecVals_even = __low_float_to_double(inVec);
      NVals_odd      = __high_int_to_double(N);
      NVals_even     = __low_int_to_double(N);
      rVals_odd      = __double_to_float((inVecVals_odd - (p * NVals_odd)));
      rVals_even     = __double_to_float((inVecVals_even - (p * NVals_even)));

      // Rejoin vectors to create r with floating point precision
      r = c7x::reinterpret<vec>(__permute_even_even_int(MATHLIB_vperm_data_interweave_0_63,
                                                        c7x::as_uchar_vec(rVals_odd), c7x::as_uchar_vec(rVals_even)));

      // Taylor series approximation
      r2  = r * r;
      r3  = r2 * r;
      pol = (r * C2) + ((r3 * C0) + (r2 * C1));

      /**********************************************************************/
      /* Get index of LUT and 2^M values                                    */
      /**********************************************************************/

      // Create vectors of LUT indices
      uN = c7x::convert<uvec_type>(N);
      K  = ((uN << 28u) >> 30) + MATHLIB_KTABLE_OFFSET;
      J  = (uN & mask) + MATHLIB_JTABLE_OFFSET;

      // Read values from LUT or ILUT and convert and store as doubles in split vectors
#if defined(__C7100__) || defined(__C7120__)
      upperBitsK = MATHLIB_LUTReadUpperBits(K);
      lowerBitsK = MATHLIB_LUTReadLowerBits(K);
      upperBitsJ = MATHLIB_LUTReadUpperBits(J);
      lowerBitsJ = MATHLIB_LUTReadLowerBits(J);
#elif defined(__C7524__)
      upperBitsK = MATHLIB_ILUTReadUpperBits(K);
      lowerBitsK = MATHLIB_ILUTReadLowerBits(K);
      upperBitsJ = MATHLIB_ILUTReadUpperBits(J);
      lowerBitsJ = MATHLIB_ILUTReadLowerBits(J);
#else
#error "Missing *LUT implementation"
#endif

      KVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsK), c7x::as_uchar_vec(lowerBitsK)));
      KVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsK), c7x::as_uchar_vec(lowerBitsK)));
      JVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsJ), c7x::as_uchar_vec(lowerBitsJ)));
      JVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsJ), c7x::as_uchar_vec(lowerBitsJ)));

      // Multiply LUT values
      dTVals_8_15 = KVals_8_15 * JVals_8_15;
      dTVals_0_7  = KVals_0_7 * JVals_0_7;

      /**********************************************************************/
      /* Scale exponent to adjust for 2^M                                   */
      /**********************************************************************/

      // Upper 32 bits of all dT values and lower 32 bits of all dT values

      dT_32_63 = c7x::reinterpret<c7x::uint_vec>(__permute_odd_odd_int(
          MATHLIB_vperm_data_0_63, c7x::as_uchar_vec(dTVals_8_15), c7x::as_uchar_vec(dTVals_0_7)));
      dT_0_31  = c7x::reinterpret<c7x::uint_vec>(__permute_even_even_int(
           MATHLIB_vperm_data_0_63, c7x::as_uchar_vec(dTVals_8_15), c7x::as_uchar_vec(dTVals_0_7)));

      uN               = (uN >> 4) << 20;
      dTAdjusted_32_63 = dT_32_63 + uN;

      // Concatenate the adjusted upper 32 bits of dT values to the lower 32 bits, convert dT to float
      dTVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
          MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(dTAdjusted_32_63), c7x::as_uchar_vec(dT_0_31)));
      dTVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
           MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(dTAdjusted_32_63), c7x::as_uchar_vec(dT_0_31)));
      dT          = c7x::reinterpret<vec>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                                  c7x::as_uchar_vec(__double_to_float(dTVals_8_15)),
                                                                  c7x::as_uchar_vec(__double_to_float(dTVals_0_7))));

      outVec = dT * (1.0f + pol);

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/
      // Read inVec again for ii optimization
      inVec = c7x::strm_eng<0, vec>::get_adv();

      // < LnMin returns 0
      __vpred cmp_min = __cmp_lt_pred(inVec, LnMin);
      outVec          = __select(cmp_min, zero, outVec);

      // > LnMax returns MAX
      __vpred cmp_max = __cmp_lt_pred(LnMax, inVec);
      outVec          = __select(cmp_max, Max, outVec);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec);
   }

   MATHLIB_SE0SA0Close();
}
#else
#error "Unknown target"
#endif

// this method performs exponential computation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_exp(size_t length, T *restrict pSrc, T *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_SUCCESS; // return function status

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {
#if defined(__C7504__)
      MATHLIB_exp_scalar(length, pSrc, pDst);
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
      MATHLIB_exp_vector(length, pSrc, pDst);
#else
#error "Unknown Target!"
#endif
   }
   return status;
}

// single precision
template MATHLIB_STATUS MATHLIB_exp<float>(size_t length, float *pSrc, float *pDst);

// double precision
// template MATHLIB_STATUS MATHLIB_exp<double>(size_t length, double *pSrc0, double *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_exp_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_exp(length, pSrc, pDst);
   return status;
}

// double-precision wrapper
// MATHLIB_STATUS MATHLIB_exp_dp(size_t length, double *pSrc, double *pDst)
//    MATHLIB_STATUS status = MATHLIB_exp(length, pSrc, pDst);
} // extern "C"
