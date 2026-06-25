// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_cosh_scalar.h"
#include "MATHLIB_ilut.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_permute.h"
#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"

#if defined(__C7504__)
template <typename T> static inline void MATHLIB_cosh_scalar(size_t length, T *restrict pSrc, T *restrict pDst)
{
   // Call scalar code
   for (uint32_t i = 0; i < length; i++) {
      pDst[i] = MATHLIB_cosh_scalar_ci(pSrc[i]);
   }
}

#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
template <typename T>
static inline void MATHLIB_cosh_exp(__SE_TEMPLATE_v1 *restrict se0Params,
                                    __SA_TEMPLATE_v1 *restrict sa0Params,
                                    T *restrict pSrc,
                                    T *restrict pDst,
                                    size_t numBlocks)
{
   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   // open SE0, SE1, and SA0 for reading and writing operands
   MATHLIB_SE0SA0Open(se0Params, sa0Params, pSrc);

   /**********************************************************************/
   /* Create and assign values for constants employed on exp computation */
   /**********************************************************************/

   vec bound;
   bound = (vec) 16.0;

   vec             log2_base_x16, half, negativeHalf, LnMax, Max, C0, C1, C2, ln2;
   c7x::uint_vec   mask;
   c7x::double_vec p;

   log2_base_x16 = (vec) 23.0831206542234f; // 1.442695041 * 16.0
   half          = (vec) 0.5f;
   negativeHalf  = (vec) -0.5f;
   LnMax         = (vec) 88.72283905f;
   Max           = (vec) 3.402823466E+38f;
   mask          = (c7x::uint_vec) 0x3u;
   p             = (c7x::double_vec) 0.0433216987816623; // 1/log2_base_x16
   ln2           = (vec) 0.693147180559945f;             // log(2)

   // coefficients to approximate the decimal part of the result
   C0 = (vec) 0.166668549286041f;
   C1 = (vec) 0.500016170012920f;
   C2 = (vec) 0.999999998618401f;

   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec = c7x::strm_eng<0, vec>::get_adv();

      /*****************************************************************/
      /* Create variables employed on exp computation                  */
      /*****************************************************************/
      vec inVec_abs;

      vec             pol, r, r2, r3, outVec, Nf, absNf, rVals_odd, rVals_even;
      c7x::uint_vec   J, K, uN, dTAdjusted_32_63, dT_32_63, dT_0_31, upperBitsK, lowerBitsK, upperBitsJ, lowerBitsJ;
      c7x::int_vec    N, minusN;
      c7x::double_vec KVals_8_15, KVals_0_7, JVals_8_15, JVals_0_7, dTVals_8_15, dTVals_0_7, pol_0_7, pol_8_15,
          outVec_0_7, outVec_8_15, inVecVals_odd, inVecVals_even, NVals_odd, NVals_even;

      inVec_abs = __abs(inVec);

      //    select absolute value of input or standard input for exp computation
      __vpred cmp_lt_exp = __cmp_lt_pred(bound, inVec_abs);
      inVec              = __select(cmp_lt_exp, inVec_abs, inVec);

      // Get N such that |N - inVec*16/ln(2)| is minimized
      Nf     = inVec * log2_base_x16;
      absNf  = Nf + half;
      N      = c7x::convert<c7x::int_vec>(absNf);
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

      // substract 16 in order to get (e^x)/2 as a result
      N = N - 16;

      /**********************************************************************/
      /* Get index of LUT and 2^M values                                    */
      /**********************************************************************/

      // Create vectors of LUT indices
      uN = c7x::convert<c7x::uint_vec>(N);
      K  = ((uN << 28u) >> 30) + MATHLIB_KTABLE_OFFSET;
      J  = (uN & mask) + MATHLIB_JTABLE_OFFSET;

      // Read values from LUT and convert and store as doubles in split vectors
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

      pol_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(MATHLIB_vperm_data_dp_interweave_0_63,
                                                                     c7x::as_uchar_vec(__high_float_to_double(pol)),
                                                                     c7x::as_uchar_vec(__low_float_to_double(pol))));
      pol_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(MATHLIB_vperm_data_dp_interweave_0_63,
                                                                       c7x::as_uchar_vec(__high_float_to_double(pol)),
                                                                       c7x::as_uchar_vec(__low_float_to_double(pol))));

      outVec_0_7  = dTVals_0_7 * (1.0f + pol_0_7);
      outVec_8_15 = dTVals_8_15 * (1.0f + pol_8_15);

      outVec = c7x::reinterpret<vec>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                             c7x::as_uchar_vec(__double_to_float(outVec_8_15)),
                                                             c7x::as_uchar_vec(__double_to_float(outVec_0_7))));

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      // > LnMax returns MAX (inf)
      // Ln2 adjusts the new boundary for exp(x)/2
      __vpred cmp_max = __cmp_lt_pred(LnMax, (inVec - ln2));
      outVec          = __select(cmp_max, Max, outVec);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec);
   }

   MATHLIB_SE0SA0Close();
}

template <typename T>
static inline void MATHLIB_cosh_pol(__SE_TEMPLATE_v1 *restrict se0Params,
                                    __SA_TEMPLATE_v1 *restrict sa0Params,
                                    T *restrict pSrc,
                                    T *restrict pDst,
                                    size_t numBlocks)
{
   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   // open SE0, SE1, and SA0 for reading and writing operands
   MATHLIB_SE0SE1SA0Open(se0Params, sa0Params, pSrc, pDst);

   /*************************************************************************************************/
   /* Create and assign values for constants employed on cosh polynomial and reciprocal computation */
   /*************************************************************************************************/
   vec pol_bound, bound, C1, C2, C3, C4, two, zero, fltMax, half, max, inf;
   pol_bound = (vec) 1.0f;
   bound     = (vec) 16.0f;
   C1        = (vec) 2.48015873015873e-5f;
   C2        = (vec) 0.00138888888888889f;
   C3        = (vec) 0.0416666666666667f;
   C4        = (vec) 0.5000000f;
   two       = (vec) 2.0f;
   zero      = (vec) 0.0f;
   fltMax    = (vec) 3.40282347e+38f;
   half      = (vec) 0.5f;
   max       = (vec) 89.41598629f;
   inf       = (vec) 0x7F800000;

   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec  = c7x::strm_eng<0, vec>::get_adv();
      vec expOut = c7x::strm_eng<1, vec>::get_adv();

      /**********************************************************************************************/
      /* Create variables employed on cosh polynomial and reciprocal computation                    */
      /**********************************************************************************************/
      vec inVec_abs, x2, x4, x6, x8, pol, x1, recip, expOut2, expRecip, outVec;

      /**************************************************************/
      /* Reciprocal computation done for inputs  1 < |input| <= 16  */
      /**************************************************************/

      expOut2 = expOut * two; // e^x
      x1      = __recip(expOut2);
      x1      = x1 * (two - (expOut2 * x1));
      recip   = x1 * (two - (expOut2 * x1)); // e^-x

      /************************************************/
      /* Boundary checking for reciprocal computation */
      /************************************************/

      __vpred cmp_eq_zero = __cmp_eq_pred(expOut2, zero);
      recip               = __select(cmp_eq_zero, zero, recip);

      __vpred cmp_gt_flt = __cmp_lt_pred(fltMax, __abs(expOut2));
      recip              = __select(cmp_gt_flt, zero, recip);

      expRecip = (expOut2 + recip) * half; // (e^x + e^-x)/2

      /**************************************************************/
      /* Polynomial computation done for inputs  -1 < input <= 1    */
      /**************************************************************/

      // calculate the power of input vector
      x2 = inVec * inVec;
      x4 = x2 * x2;
      x6 = x2 * x4;
      x8 = x4 * x4;

      pol = ((C4 * x2) + (C3 * x4)) + ((C1 * x8) + (C2 * x6));
      pol = pol + 1.0f;

      /**************************************************************/
      /* Determining cosh computation for each input                */
      /**************************************************************/

      /**************************************************************************/
      /* The type of calculation for cosh(x) depends on the value of x:         */
      /*                                                                        */
      /* for x_abs <= 1,          res = pol_est_coshsp_i (input x)              */
      /* for x_abs > 16,          res = expsp_coshsp_i (input x_abs),           */
      /*                                e^-|x| is negligible                    */
      /* for 1 < x_abs <= 16,     res = (e^x + e^-x)/2,                         */
      /*                                e^x = 2 * expsp_coshsp_i (input x)      */
      /* where x_abs is the absolute value of the input, sign has a value of 1  */
      /* or -1 depending on the sign of the input, and res is the value         */
      /* for cosh(x).                                                           */
      /* ************************************************************************/

      inVec_abs = __abs(inVec);

      //    compute polynomial
      __vpred cmp_le_pol = __cmp_le_pred(inVec_abs, pol_bound); // |x| <= 1

      //    exp computation with |x|
      __vpred cmp_lt_exp = __cmp_lt_pred(bound, inVec_abs); // |x| > 16

      __vpred cmp_else_exp = __negate(__or(cmp_lt_exp, cmp_le_pol)); // 1 < |x| <= 16

      outVec = __select(cmp_else_exp, expRecip, expOut);
      outVec = __select(cmp_le_pol, pol, outVec);

      //      res = _itof(0x7F800000u); // large x, res = INF

      __vpred cmp_lt_max = __cmp_lt_pred(max, inVec_abs);
      outVec             = __select(cmp_lt_max, inf, outVec);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec);
   }
}

// this method performs cosh computation of input vector
template <typename T> static inline void MATHLIB_cosh_vector(size_t length, T *restrict pSrc, T *restrict pDst)
{
   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<T>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc, pDst);

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   /***********************************************************************/
   /* Exp computation for cosh                                            */
   /***********************************************************************/

   MATHLIB_cosh_exp(&se0Params, &sa0Params, pSrc, pDst, numBlocks);

   /***********************************************************************/
   /* Polynomial and reciprocal computation for cosh                      */
   /***********************************************************************/
   MATHLIB_cosh_pol(&se0Params, &sa0Params, pSrc, pDst, numBlocks);
}
#else
#error "Unknown target!"
#endif

// this method performs exponential computation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_cosh(size_t length, T *restrict pSrc, T *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_SUCCESS; // return function status

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   if (status == MATHLIB_SUCCESS) {
#if defined(__C7504__)
      MATHLIB_cosh_scalar(length, pSrc, pDst);
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
      MATHLIB_cosh_vector(length, pSrc, pDst);
#else
#error "Unknown target!"
#endif
   }
   return status;
}

// single precision
template MATHLIB_STATUS MATHLIB_cosh<float>(size_t length, float *pSrc, float *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_cosh_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_cosh(length, pSrc, pDst);
   return status;
}

} // extern "C"
