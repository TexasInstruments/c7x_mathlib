// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_ilut.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_permute.h"
#include "MATHLIB_pow_scalar.h"
#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"

template <typename T>
static inline void MATHLIB_pow_vector(size_t length, T *restrict pSrc0, T *restrict pSrc1, T *restrict pDst);

template <typename T>
static inline MATHLIB_STATUS MATHLIB_pow_exp(__SE_TEMPLATE_v1 *restrict se0Params,
                                             __SA_TEMPLATE_v1 *restrict sa0Params,
                                             size_t length,
                                             T *restrict pSrc0,
                                             T *restrict pDst);

template <typename T>
static inline MATHLIB_STATUS MATHLIB_pow_cond(size_t length, T *restrict pSrc0, T *restrict pSrc1, T *restrict pDst);

#if defined(__C7504__)
template <>
inline void MATHLIB_pow_vector<float>(size_t length, float *restrict pSrc0, float *restrict pSrc1, float *restrict pDst)
{
   // Call scalar code
   for (uint32_t i = 0; i < length; i++) {
      pDst[i] = MATHLIB_pow_scalar_ci(pSrc0[i], pSrc1[i]);
   }
}
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
template <typename vecType> static inline vecType MATHLIB_pow_log(vecType inVec);

template <> inline c7x::float_vec MATHLIB_pow_log<c7x::float_vec>(c7x::float_vec inVec)
{
   /**********************************************************************/
   /* Create and assign values for constants employed on log computation */
   /**********************************************************************/
   typedef typename c7x::make_full_vector<float>::type vecType;

   vecType         C1, C2, C3, C4, C5, eMax, outVecMax;
   c7x::double_vec ln2_vec;
   c7x::uint_vec   zero;
   zero = (c7x::uint_vec) 0;

   ln2_vec   = (c7x::double_vec) 0.693147180559945;
   C1        = (vecType) -0.2302894f;
   C2        = (vecType) 0.1908169f;
   C3        = (vecType) -0.2505905f;
   C4        = (vecType) 0.3333164f;
   C5        = (vecType) -0.5000002f;
   eMax      = (vecType) 3.402823466e+38f;
   outVecMax = (vecType) 88.72283905313f;

   /**********************************************************************/
   /* Create variables employed on log computation                       */
   /**********************************************************************/

   vecType         Pol, R1, R2, R3, R4, outvec;
   c7x::double_vec inVecVals_odd, inVecVals_even, inVecVals_oddReciprocal, inVecVals_evenReciprocal,
       inVecReciprocalApprox_8_15, inVecReciprocalApprox_0_7, inVecVals_8_15, inVecVals_0_7, rVals_0_7, rVals_8_15,
       TVals_8_15, TVals_0_7, NVals_odd, NVals_even, NVals_0_7, NVals_8_15, Pol_0_7, Pol_8_15, outvec_8_15, outvec_0_7;
   c7x::uint_vec inVecReciprocal_32_63, inVecReciprocalClr_32_63, inVecReciprocalApprox_32_63, indexT, upperBitsIndexT,
       lowerBitsIndexT;
   c7x::int_vec Nvec;

   /**********************************************************************/
   /* Calculate Taylor series approximation for log                      */
   /**********************************************************************/

   // Split vectors to compute r with double precision
   inVecVals_odd            = __high_float_to_double(inVec);
   inVecVals_even           = __low_float_to_double(inVec);
   inVecVals_oddReciprocal  = __recip(inVecVals_odd);
   inVecVals_evenReciprocal = __recip(inVecVals_even);

   // Create floating point reciprocal approximation
   // Upper 32 bits of all inVec reciprocal values
   inVecReciprocal_32_63 = c7x::as_uint_vec(__permute_odd_odd_int(MATHLIB_vperm_data_interweave_0_63,
                                                                  c7x::as_uchar_vec(inVecVals_oddReciprocal),
                                                                  c7x::as_uchar_vec(inVecVals_evenReciprocal)));

   // Clear bits 0-16 inclusive
   inVecReciprocalClr_32_63 = inVecReciprocal_32_63 & (c7x::uint_vec) 0xFFFE0000U;

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
   R1 = c7x::reinterpret<vecType>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                          c7x::as_uchar_vec(__double_to_float(rVals_8_15)),
                                                          c7x::as_uchar_vec(__double_to_float(rVals_0_7))));
   R2 = R1 * R1;
   R3 = R1 * R2;
   R4 = R2 * R2;

   // Compute Taylor series polynomial
   Pol = (C5 * R2) + ((C4 * R3) + ((((C2 * R1) + C3) + (C1 * R2)) * R4));

   /**********************************************************************/
   /* Calculate Nvec                                                        */
   /**********************************************************************/

   // Upper 32 bits of all inVec reciprocal approximation values
   inVecReciprocalApprox_32_63 =
       c7x::as_uint_vec(__permute_odd_odd_int(MATHLIB_vperm_data_0_63, c7x::as_uchar_vec(inVecReciprocalApprox_8_15),
                                              c7x::as_uchar_vec(inVecReciprocalApprox_0_7)));

   c7x::uint_vec Nvec_temp = __shift_left(inVecReciprocalApprox_32_63, (c7x::uint_vec) 1U);
   Nvec_temp               = __shift_right(Nvec_temp, (c7x::uint_vec) 21U);
   Nvec_temp               = Nvec_temp - (c7x::uint_vec) 1023U;
   Nvec                    = c7x::convert_int_vec(Nvec_temp);

   // Covert Nvec to double precision for later calculation with LUT values
   NVals_odd  = __high_int_to_double(Nvec);
   NVals_even = __low_int_to_double(Nvec);
   NVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
        MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(NVals_odd), c7x::as_uchar_vec(NVals_even)));
   NVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
       MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(NVals_odd), c7x::as_uchar_vec(NVals_even)));

   /**********************************************************************/
   /* Determine LUT values                                               */
   /**********************************************************************/

   // Calculate LUT index
   indexT = __shift_left(inVecReciprocalApprox_32_63, (c7x::uint_vec) 12U);
   indexT = __shift_right(indexT, (c7x::uint_vec) 29U);
   indexT = indexT + (c7x::uint_vec) MATHLIB_LOGTABLE_OFFSET;

   // Read from LUT and reconstruct double values split into two vectors

#if defined(__C7100__) || defined(__C7120__)
   upperBitsIndexT = MATHLIB_LUTReadUpperBits(indexT);
   lowerBitsIndexT = MATHLIB_LUTReadLowerBits(indexT);
#elif defined(__C7524__)
   upperBitsIndexT = MATHLIB_ILUTReadUpperBits(indexT);
   lowerBitsIndexT = MATHLIB_ILUTReadLowerBits(indexT);
#else
#error "Missing *LUT implementation"
#endif

   TVals_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(
       MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsIndexT), c7x::as_uchar_vec(lowerBitsIndexT)));
   TVals_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(
        MATHLIB_vperm_data_interweave_0_63, c7x::as_uchar_vec(upperBitsIndexT), c7x::as_uchar_vec(lowerBitsIndexT)));

   // Calculate an adjusted T
   TVals_8_15 = TVals_8_15 - (ln2_vec * NVals_8_15);
   TVals_0_7  = TVals_0_7 - (ln2_vec * NVals_0_7);

   /**********************************************************************/
   /* Calculate output with adjusted LUT and Taylor series values        */
   /**********************************************************************/

   // Split polynomial result into two vectors with double precision
   Pol_0_7  = c7x::reinterpret<c7x::double_vec>(__permute_low_low(MATHLIB_vperm_data_dp_interweave_0_63,
                                                                  c7x::as_uchar_vec(__high_float_to_double(Pol)),
                                                                  c7x::as_uchar_vec(__low_float_to_double(Pol))));
   Pol_8_15 = c7x::reinterpret<c7x::double_vec>(__permute_high_high(MATHLIB_vperm_data_dp_interweave_0_63,
                                                                    c7x::as_uchar_vec(__high_float_to_double(Pol)),
                                                                    c7x::as_uchar_vec(__low_float_to_double(Pol))));

   // Add LUT values and Taylor series values
   outvec_0_7  = rVals_0_7 + TVals_0_7 + Pol_0_7;
   outvec_8_15 = rVals_8_15 + TVals_8_15 + Pol_8_15;

   // Combine output vector into one floating point result
   outvec = c7x::reinterpret<vecType>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                              c7x::as_uchar_vec(__double_to_float(outvec_8_15)),
                                                              c7x::as_uchar_vec(__double_to_float(outvec_0_7))));

   /**********************************************************************/
   /* Bounds checking                                                    */
   /**********************************************************************/

   __vpred cmp_max = __cmp_lt_pred(eMax, inVec);
   outvec          = __select(cmp_max, outVecMax, outvec);

   return outvec;
}

template <>
inline MATHLIB_STATUS
MATHLIB_pow_cond<float>(size_t length, float *restrict pSrc0, float *restrict pSrc1, float *restrict pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<float>::type vec;

   // Compile-time decision: float_vec => int_vec and double_vec=> long_vec
   typedef
       typename std::conditional<ELEMENT_COUNT(c7x::float_vec) == ELEMENT_COUNT(vec), c7x::int_vec, c7x::long_vec>::type
           vec_type;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SE_TEMPLATE_v1 se1Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   __SE_ELETYPE SE_ELETYPE;
   __SE_VECLEN  SE_VECLEN;
   __SA_VECLEN  SA_VECLEN;

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<float>::type vec;

   // assign SE and SA params based on vector type
   SE_VECLEN  = c7x::se_veclen<vec>::value;
   SA_VECLEN  = c7x::sa_veclen<vec>::value;
   SE_ELETYPE = c7x::se_eletype<vec>::value;

   se0Params.ICNT0   = length;
   se0Params.ELETYPE = SE_ELETYPE;
   se0Params.VECLEN  = SE_VECLEN;
   se0Params.DIMFMT  = __SE_DIMFMT_1D;

   se1Params.DIMFMT        = __SE_DIMFMT_3D;
   se1Params.ICNT0         = c7x::element_count_of<vec>::value;
   se1Params.ELETYPE       = SE_ELETYPE;
   se1Params.VECLEN        = SE_VECLEN;
   se1Params.ICNT1         = 2;
   se1Params.ICNT2         = numBlocks;
   se1Params.DIM2          = c7x::element_count_of<vec>::value;
   se1Params.DECDIM1       = __SE_DECDIM_DIM2;
   se1Params.DECDIM1_WIDTH = length;
   se1Params.DIM1          = ((float *) pSrc1) - ((float *) pSrc0);

   sa0Params.ICNT0  = length;
   sa0Params.DIM1   = length;
   sa0Params.VECLEN = SA_VECLEN;
   sa0Params.DIMFMT = __SA_DIMFMT_1D;

   // open SE0, SE1, and SA0 for reading and writing operands
   __SE0_OPEN(pDst, se0Params);
   __SE1_OPEN(pSrc0, se1Params);
   __SA0_OPEN(sa0Params);

   /**********************************************************************/
   /* Create and assign values for constants employed on pow computation */
   /**********************************************************************/
   vec      Zero, one, intMax, inf;
   vec_type zeroInt, signNegative;

   Zero         = (vec) 0.0f;
   one          = (vec) 1.0f;
   intMax       = (vec) 0x7fffffffu;
   inf          = (vec) 0x7F800000u;
   zeroInt      = (vec_type) 0;
   signNegative = (vec_type) -1;
   // compute loop to perform vector pow
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec0  = c7x::strm_eng<1, vec>::get_adv();
      vec outVec1 = c7x::strm_eng<0, vec>::get_adv();
      vec inVec1  = c7x::strm_eng<1, vec>::get_adv();

      /**********************************************************************/
      /* Create variables employed on pow computation                       */
      /**********************************************************************/
      vec_type inVec1_round, inVec1_roundEven, sign;

      sign = (vec_type) 1;

      /**********************************************************************/
      /* Calculate sign of final result                                     */
      /**********************************************************************/
      // Negative base w/ odd-integer power should be negative
      inVec1_round     = __float_to_int(inVec1);
      inVec1_roundEven = inVec1_round & 1;

      // Check if base is < 0, if the power is an integer, if the power is odd
      __vpred cmp_zero    = __cmp_lt_pred(inVec0, Zero);
      __vpred cmp_floateq = __cmp_eq_pred(inVec1, __int_to_float(inVec1_round));
      __vpred cmp_odd     = __negate(__cmp_eq_pred(inVec1_roundEven, zeroInt));
      __vpred cond1       = __and(cmp_zero, cmp_floateq);
      __vpred cond2       = __and(cond1, cmp_odd);

      // If all conditions are true, result is negative
      sign = __select(cond2, signNegative, sign);

      outVec1 = __int_to_float(sign) * outVec1;

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/
      // Error if base < 0 and power is not an integer
      __vpred cond3 = __and(cmp_zero, __negate(cmp_floateq));
      outVec1       = __select(cond3, intMax, outVec1);

      __vpred cmp_inVec0_zero    = __cmp_eq_pred(inVec0, Zero);
      __vpred cmp_inVec1_zero_lt = __cmp_lt_pred(inVec1, Zero);
      __vpred cond4              = __and(cmp_inVec0_zero, __negate(cmp_inVec1_zero_lt));
      __vpred cond5              = __and(cmp_inVec0_zero, cmp_inVec1_zero_lt);

      // 0 if base is 0 and power is greater than or equal to 0
      // inf if base is 0 and power is less than Zero
      outVec1 = __select(cond4, Zero, outVec1);
      outVec1 = __select(cond5, inf, outVec1);

      // 1 if power is 0
      __vpred cmp_inVec1_zero = __cmp_eq_pred(inVec1, Zero);
      outVec1                 = __select(cmp_inVec1_zero, one, outVec1);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec1);
   }
   MATHLIB_SE0SE1SA0Close();

   return status;
}

template <>
inline MATHLIB_STATUS MATHLIB_pow_exp<float>(__SE_TEMPLATE_v1 *restrict se0Params,
                                             __SA_TEMPLATE_v1 *restrict sa0Params,
                                             size_t length,
                                             float *restrict pSrc0,
                                             float *restrict pDst)
{
   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<float>::type vec;

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   // open SE0, SE1, and SA0 for reading and writing operands
   MATHLIB_SE0SA0Open(se0Params, sa0Params, pDst);

   /**********************************************************************/
   /* Create and assign values for constants employed on pow computation */
   /**********************************************************************/

   vec           log2_base_x16, half, negativeHalf, Zero, LnMin, LnMax, Max, C0, c1, c2, P1, P2;
   c7x::uint_vec mask, inVec_min;

   log2_base_x16 = (vec) 23.083120654f;
   half          = (vec) 0.5f;
   negativeHalf  = (vec) -0.5f;
   Zero          = (vec) 0.0f;
   LnMin         = (vec) -87.33654475f;
   LnMax         = (vec) 88.72283905f;
   Max           = (vec) 3.402823466E+38f;
   C0            = (vec) 0.166668549286041f;
   c1            = (vec) 0.500016170012920f;
   c2            = (vec) 0.999999998618401f;
   P1            = (vec) 0.04331970214844f;
   P2            = (vec) 1.99663646e-6f;
   mask          = (c7x::uint_vec) 0x3u;
   inVec_min     = (c7x::uint_vec) 114u;

   // compute loop to perform vector pow
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec = c7x::strm_eng<0, vec>::get_adv();

      /**********************************************************************/
      /* Create variables employed on exp computation                       */
      /**********************************************************************/

      vec           pol_vec, R, R2_vec, R3_vec, outVec, Nf, absNf;
      c7x::uint_vec J, K, uN, dTAdjusted_32_63, dT_32_63, dT_0_31, inVec_small, upperBitsK, lowerBitsK, upperBitsJ,
          lowerBitsJ;
      c7x::int_vec    N_vec, minusN;
      c7x::double_vec KVals_8_15, KVals_0_7, JVals_8_15, JVals_0_7, dTVals_8_15, dTVals_0_7, pol_0_7, pol_8_15,
          outVec_0_7, outVec_8_15;

      // Get N_vec such that |N_vec - inVec*16/ln(2)| is minimized
      Nf     = inVec * log2_base_x16;
      absNf  = Nf + half;
      N_vec  = c7x::convert<c7x::int_vec>(absNf);
      minusN = N_vec - 1;

      //    N_vec--;
      __vpred cmp_N = __cmp_lt_pred(Nf, negativeHalf);
      N_vec         = __select(cmp_N, minusN, N_vec);

      /**********************************************************************/
      /* Calculate Taylor series approximation for exp                      */
      /**********************************************************************/

      // Split vectors to compute r with double precision
      R = (inVec - (P1 * __int_to_float(N_vec))) - (P2 * __int_to_float(N_vec));

      // Taylor series approximation
      R2_vec  = R * R;
      R3_vec  = R2_vec * R;
      pol_vec = (R * c2) + ((R3_vec * C0) + (R2_vec * c1));

      /**********************************************************************/
      /* Get index of LUT and 2^M values                                    */
      /**********************************************************************/

      // Create vectors of LUT indices
      uN = c7x::convert<c7x::uint_vec>(N_vec);
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
                                                                     c7x::as_uchar_vec(__high_float_to_double(pol_vec)),
                                                                     c7x::as_uchar_vec(__low_float_to_double(pol_vec))));
      pol_8_15 = c7x::reinterpret<c7x::double_vec>(
          __permute_high_high(MATHLIB_vperm_data_dp_interweave_0_63, c7x::as_uchar_vec(__high_float_to_double(pol_vec)),
                              c7x::as_uchar_vec(__low_float_to_double(pol_vec))));

      outVec_0_7  = dTVals_0_7 * (1.0f + pol_0_7);
      outVec_8_15 = dTVals_8_15 * (1.0f + pol_8_15);

      outVec = c7x::reinterpret<vec>(__permute_even_even_int(MATHLIB_vperm_data_0_63,
                                                             c7x::as_uchar_vec(__double_to_float(outVec_8_15)),
                                                             c7x::as_uchar_vec(__double_to_float(outVec_0_7))));

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      inVec_small       = ((c7x::as_uint_vec(inVec) << 1u) >> 24u);
      __vpred cmp_inVec = __cmp_gt_pred(inVec_min, inVec_small);
      outVec            = __select(cmp_inVec, 1.0f + inVec, outVec);

      // < LnMin returns 0
      __vpred cmp_min = __cmp_lt_pred(inVec, LnMin);
      outVec          = __select(cmp_min, Zero, outVec);

      // > LnMax returns MAX
      __vpred cmpMax = __cmp_lt_pred(LnMax, inVec);
      outVec         = __select(cmpMax, Max, outVec);

      // Store exp computation result in
      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, outVec);
   }
   MATHLIB_SE0SA0Close();
   return status;
}

template <>
inline void MATHLIB_pow_vector<float>(size_t length, float *restrict pSrc0, float *restrict pSrc1, float *restrict pDst)
{
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<float>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }
   MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc0, pDst);

   // open SE0, SE1, and SA0 for reading and writing operands
   MATHLIB_SE0SE1SA0Open(&se0Params, &sa0Params, pSrc0, pSrc1);

   /**********************************************************************/
   /* Create and assign values for constants employed on pow computation */
   /**********************************************************************/

   vec zero_vec, one;

   zero_vec = (vec) 0.0f;
   one      = (vec) 1.0f;

   // compute loop to perform vector pow
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec0 = c7x::strm_eng<0, vec>::get_adv();
      vec inVec1 = c7x::strm_eng<1, vec>::get_adv();

      /**********************************************************************/
      /* Create variables employed on pow computation                       */
      /**********************************************************************/
      vec inVec_log, inVec_exp;

      /**********************************************************************/
      /* Log and exp computation                                            */
      /**********************************************************************/
      inVec_log       = __abs(inVec0);
      inVec_exp       = inVec1 * MATHLIB_pow_log(inVec_log);
      __vpred cmp_one = __cmp_eq_pred(inVec_log, one);
      inVec_exp       = __select(cmp_one, zero_vec, inVec_exp);

      // Store log computation result in
      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, inVec_exp);
   }
   MATHLIB_SE0SE1SA0Close();

   MATHLIB_pow_exp(&se0Params, &sa0Params, length, pSrc0, pDst);
   MATHLIB_pow_cond(length, pSrc0, pSrc1, pDst);
}

#endif

static inline c7x::double_vec cmn_DIVDP_opt(c7x::double_vec a, c7x::double_vec b)
{

   c7x::double_vec Two = (c7x::double_vec)(2.0f);
   c7x::double_vec X;

   X = __recip(b);
   X = X * (Two - (b * X));
   X = X * (Two - (b * X));
   X = X * (Two - (b * X));
   X = a * X;

   return X;
}

template <typename T>
static inline void MATHLIB_pow_log_opt(size_t length, T *restrict pSrc0, T *restrict pSrc1, T *restrict pDst);

template <>
inline void
MATHLIB_pow_log_opt<double>(size_t length, double *restrict pSrc0, double *restrict pSrc1, double *restrict pDst)
{
   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<double>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pDst, pDst);

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   // open SE0, SE1, and SA0 for reading and writing operands
   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc0);

   vec Half, MAXe, srHalf, Half_sq, MINe, a0, a1, a2, b0, b1, b2, c1_vec, c2_vec, W, X, Y, Z, zn, zd, Rz, Sa, Bd, Cn,
       Da;

   Half    = (vec) 0.5;
   Half_sq = (vec) 0.5 * 0.5;
   MAXe    = (vec) 1.7976931348623157e+308;
   srHalf  = (vec) 0.70710678118654752440; /* sqrt(0.5) */
   MINe    = (vec) 2.2250738585072014e-308;
   a0      = (vec) -0.64124943423745581147e+2;
   a1      = (vec) 0.16383943563021534222e+2;
   a2      = (vec) -0.78956112887491257267e+0;
   b0      = (vec) -0.76949932108494879777e+3;
   b1      = (vec) 0.31203222091924532844e+3;
   b2      = (vec) -0.35667977739034646171e+2; /* Note b3 = 1.0 */
   c1_vec  = (vec) 0.693359375;                /*  355/512      */
   c2_vec  = (vec) -2.121944400546905827679e-4;

   c7x::long_vec long_zero_vec   = (c7x::long_vec) 0;
   vec           double_zero_vec = (vec) 0.0;
   vec           outMAX          = (vec) (308.254715974092);

   for (size_t i = 0; i < numBlocks; i++) {

      vec a              = c7x::strm_eng<0, vec>::get_adv();
      Y                  = __abs(a);
      c7x::long_vec exp_ = c7x::as_long_vec((c7x::as_ulong_vec(Y) << 1) >> 53);

      c7x::ulong_vec upper = c7x::as_ulong_vec(Y) & (0x000FFFFF00000000u);
      upper                = 0x3FE0000000000000u | upper;

      Z = c7x::as_double_vec((0x00000000FFFFFFFFu & c7x::as_ulong_vec(Y)) | upper);

      __vpred cmp1 = __cmp_eq_pred(exp_, long_zero_vec);
      Z            = __select(cmp1, double_zero_vec, Z);

      vec     z_minus_half = Z - Half;
      vec     z_mul_half   = (Z * Half) + Half;
      __vpred cmp2         = __cmp_lt_pred(srHalf, Z);
      zn                   = __select(cmp2, (z_minus_half - Half), z_minus_half);
      zd                   = __select(cmp2, z_mul_half, (z_mul_half - Half_sq));

      X            = cmn_DIVDP_opt(zn, zd);
      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, X);
   }
   MATHLIB_SE0SA0Close();

   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pDst);

   for (size_t i = 0; i < numBlocks; i++) {

      X = c7x::strm_eng<0, vec>::get_adv();

      W            = X * X;
      Bd           = ((((W + b2) * W) + b1) * W) + b0;
      Cn           = (((W * a2) + a1) * W) + a0;
      Rz           = W * cmn_DIVDP_opt(Cn, Bd);
      Sa           = X + (X * Rz);
      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Sa);
   }

   MATHLIB_SE0SA0Close();

   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc0);

   __SE1_OPEN(pDst, se0Params);
   for (size_t i = 0; i < numBlocks; i++) {

      vec a = c7x::strm_eng<0, vec>::get_adv();
      Sa    = c7x::strm_eng<1, vec>::get_adv();

      Y                   = a;
      c7x::long_vec exp_  = c7x::as_long_vec((c7x::as_ulong_vec(Y) << 1) >> 53);
      c7x::long_vec N_vec = exp_ - 1022;

      c7x::ulong_vec upper = c7x::as_ulong_vec(Y) & (0x000FFFFF00000000u);
      upper                = 0x3FE0000000000000u | upper;

      Z = c7x::as_double_vec((0x00000000FFFFFFFFu & c7x::as_ulong_vec(Y)) | upper);

      __vpred cmp1 = __cmp_eq_pred(exp_, long_zero_vec);
      Z            = __select(cmp1, double_zero_vec, Z);

      __vpred cmp2 = __cmp_lt_pred(srHalf, Z);

      N_vec = __select(cmp2, N_vec, (N_vec - 1));

      Cn = __low_int_to_double(c7x::as_int_vec(N_vec));
      Da = ((Cn * c2_vec) + Sa) + (Cn * c1_vec);

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      __vpred cmp_min_vec = __cmp_lt_pred(Y, MINe);
      Da                  = __select(cmp_min_vec, -MAXe, Da);

      __vpred cmp_max_vec = __cmp_lt_pred(MAXe, Y);
      Da                  = __select(cmp_max_vec, outMAX, Da);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Da);
   }
   __SE1_CLOSE();
   MATHLIB_SE0SA0Close();
}

template <>
inline MATHLIB_STATUS
MATHLIB_pow_cond<double>(size_t length, double *restrict pSrc0, double *restrict pSrc1, double *restrict pDst)
{
   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<double>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SE_TEMPLATE_v1 se1Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   __SE_ELETYPE SE_ELETYPE;
   __SE_VECLEN  SE_VECLEN;
   __SA_VECLEN  SA_VECLEN;

   // assign SE and SA params based on vector type
   SE_VECLEN  = c7x::se_veclen<vec>::value;
   SA_VECLEN  = c7x::sa_veclen<vec>::value;
   SE_ELETYPE = c7x::se_eletype<vec>::value;

   se0Params.ICNT0   = length;
   se0Params.ELETYPE = SE_ELETYPE;
   se0Params.VECLEN  = SE_VECLEN;
   se0Params.DIMFMT  = __SE_DIMFMT_1D;

   se1Params.DIMFMT        = __SE_DIMFMT_3D;
   se1Params.ICNT0         = c7x::element_count_of<vec>::value;
   se1Params.ELETYPE       = SE_ELETYPE;
   se1Params.VECLEN        = SE_VECLEN;
   se1Params.ICNT1         = 2;
   se1Params.ICNT2         = numBlocks;
   se1Params.DIM2          = c7x::element_count_of<vec>::value;
   se1Params.DECDIM1       = __SE_DECDIM_DIM2;
   se1Params.DECDIM1_WIDTH = length;
   se1Params.DIM1          = ((double *) pSrc1) - ((double *) pSrc0);

   sa0Params.ICNT0  = length;
   sa0Params.DIM1   = length;
   sa0Params.VECLEN = SA_VECLEN;
   sa0Params.DIMFMT = __SA_DIMFMT_1D;

   __SE0_OPEN(pDst, se0Params);
   __SE1_OPEN(pSrc0, se1Params);
   __SA0_OPEN(sa0Params);

   vec zeroVec, X2;

   zeroVec                   = (vec) 0.0f;
   c7x::long_vec zero_int    = (c7x::long_vec)(0);
   c7x::long_vec one_int     = (c7x::long_vec)(1);
   c7x::int_vec  Sign        = (c7x::int_vec)(1);
   c7x::int_vec  negate_Sign = (c7x::int_vec)(-1);

   c7x::long_vec valueL1 = (c7x::long_vec)(0X7fffffffffffffff);
   c7x::long_vec valueL2 = (c7x::long_vec)(0X7ff0000000000000);

   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec0 = c7x::strm_eng<1, vec>::get_adv();
      vec inVec1 = c7x::strm_eng<1, vec>::get_adv();
      vec W      = c7x::strm_eng<0, vec>::get_adv();

      c7x::int_vec y = __double_to_int(inVec1);

      __vpred cmp_cond1 = __cmp_lt_pred(inVec0, zeroVec);
      __vpred cmp_cond2 = __cmp_eq_pred(inVec1, __low_int_to_double(y));

      cmp_cond2 = __and(cmp_cond2, cmp_cond1);

      c7x::long_vec y_long    = c7x::as_long_vec(y);
      __vpred       cmp_cond3 = __cmp_eq_pred((y_long & one_int), zero_int);

      cmp_cond3 = __and(cmp_cond2, __negate(cmp_cond3));

      Sign = __select(cmp_cond3, negate_Sign, Sign);

      X2 = __low_int_to_double(Sign) * W;

      __vpred cmp_cond5 = __cmp_eq_pred(inVec1, __low_int_to_double(y));
      cmp_cond5         = __and(cmp_cond1, __negate(cmp_cond5));
      X2                = __select(cmp_cond5, c7x::as_double_vec(valueL1), X2);

      __vpred cmp_cond6 = __cmp_eq_pred(inVec0, zeroVec);
      __vpred cmp_cond7 = __cmp_lt_pred(inVec1, zeroVec);
      cmp_cond7         = __and(cmp_cond6, cmp_cond7);
      X2                = __select(cmp_cond7, c7x::as_double_vec(valueL2), X2);

      // Store log computation result in
      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, X2);
   }
   MATHLIB_SE0SE1SA0Close();
   return status;
}

template <>
inline MATHLIB_STATUS MATHLIB_pow_exp<double>(__SE_TEMPLATE_v1 *restrict se0Params,
                                              __SA_TEMPLATE_v1 *restrict sa0Params,
                                              size_t length,
                                              double *restrict pSrc1,
                                              double *restrict pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<double>::type vec;

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }

   // open SE0, SE1, and SA0 for reading and writing operands
   MATHLIB_SE0SA0Open(se0Params, sa0Params, pDst);

   __SE1_OPEN(pSrc1, *se0Params);

   vec Halfe, Maxe, LnMaxe, LnMine, a0e, a1e, a2e, b0e, b1e, b2e, c1e, C2e, L2e, Zero;

   Halfe  = (vec) 0.5;
   Maxe   = (vec) 1.7976931348623157e+308;
   LnMaxe = (vec) 709.78271289338;
   LnMine = (vec) -708.3964185322641;
   a0e    = (vec) 0.249999999999999993;
   a1e    = (vec) 0.694360001511792852e-2;
   a2e    = (vec) 0.165203300268279130e-4;
   b0e    = (vec) 0.5;
   b1e    = (vec) 0.555538666969001188e-1;
   b2e    = (vec) 0.495862884905441294e-3;
   c1e    = (vec) 0.693359375; /* 355/512 */
   C2e    = (vec) -2.1219444005469058277e-4;
   L2e    = (vec) 1.4426950408889634074;
   Zero   = (vec) 0.0f;

   vec Ye, Xe, We, Re, Se, Be, Ce, De;

   // compute loop to perform vector pow
   for (size_t i = 0; i < numBlocks; i++) {
      vec invec = c7x::strm_eng<0, vec>::get_adv();
      vec b     = c7x::strm_eng<1, vec>::get_adv();

      Ye = invec * b;
      Ce = Ye * L2e;

      c7x::int_vec Ne = __double_to_int(Ce);

      Se = __low_int_to_double(Ne);
      Xe = (Ye - (Se * c1e)) - (Se * C2e); /*  range reduction  */

      We = Xe * Xe;
      Be = (((b2e * We) + b1e) * We) + b0e;        /* denominator  */
      De = ((((a2e * We) + a1e) * We) + a0e) * Xe; /* numerator  */
      Re = Halfe + cmn_DIVDP_opt(De, Be - De);

      Ye = invec;

      Ce = Ye * L2e;

      c7x::int_vec upper = 1024 + (Ne);
      upper              = c7x::as_int_vec((c7x::as_uint_vec(upper) << 20));
      Se                 = c7x::as_double_vec(c7x::as_long_vec(upper) << 32);

      Ce = Re * Se;

      __vpred cmp_Max = __cmp_lt_pred(LnMaxe, invec);
      Ce              = __select(__negate(cmp_Max), Ce, Maxe);

      __vpred cmp_Min = __cmp_lt_pred(invec, LnMine);
      Ce              = __select(cmp_Min, Zero, Ce);

      // Store exp computation result in
      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Ce);
   }

   MATHLIB_SE0SA0Close();
   __SE1_CLOSE();
   return status;
}

template <>
inline void
MATHLIB_pow_vector<double>(size_t length, double *restrict pSrc0, double *restrict pSrc1, double *restrict pDst)
{
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<double>::type vec;

   __SE_TEMPLATE_v1 se0Params = __gen_SE_TEMPLATE_v1();
   __SA_TEMPLATE_v1 sa0Params = __gen_SA_TEMPLATE_v1();

   // calculate compute loop's iteration counter
   numBlocks    = length / c7x::element_count_of<vec>::value;
   remNumBlocks = length % c7x::element_count_of<vec>::value;
   if (remNumBlocks) {
      numBlocks++;
   }
   MATHLIB_SE0SA01DSequentialInit(&se0Params, &sa0Params, length, pSrc0, pDst);

   /**********************************************************************/
   /* Create and assign values for constants employed on pow computation */
   /**********************************************************************/

   MATHLIB_pow_log_opt(length, pSrc0, pSrc1, pDst);

   MATHLIB_pow_exp(&se0Params, &sa0Params, length, pSrc1, pDst);

   MATHLIB_pow_cond(length, pSrc0, pSrc1, pDst);
}

// this method performs power computation of input vectors
template <typename T> MATHLIB_STATUS MATHLIB_pow(size_t length, T *restrict pSrc0, T *restrict pSrc1, T *restrict pDst)
{
   // variables
   MATHLIB_STATUS status = MATHLIB_SUCCESS; // return function status

   status = MATHLIB_checkParams(length, pSrc0, pSrc1, pDst);

   if (status == MATHLIB_SUCCESS) {
      if (length < 2) {
         pDst[0] = MATHLIB_pow_scalar_ci<T>(pSrc0[0], pSrc1[0]);
      }
      else {
         MATHLIB_pow_vector<T>(length, pSrc0, pSrc1, pDst);
      }
   }

   return status;
}

// single precision
template MATHLIB_STATUS
MATHLIB_pow<float>(size_t length, float *restrict pSrc0, float *restrict pSrc1, float *restrict pDst);

// double precision
template MATHLIB_STATUS
MATHLIB_pow<double>(size_t length, double *restrict pSrc0, double *restrict pSrc1, double *restrict pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_pow_sp(size_t length, float *restrict pSrc0, float *restrict pSrc1, float *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_pow<float>(length, pSrc0, pSrc1, pDst);
   return status;
}

// double-precision wrapper
MATHLIB_STATUS MATHLIB_pow_dp(size_t length, double *restrict pSrc0, double *restrict pSrc1, double *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_pow<double>(length, pSrc0, pSrc1, pDst);
   return status;
}

} // extern "C"
