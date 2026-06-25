// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_ilut.h"
#include "MATHLIB_log10_scalar.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_permute.h"
#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"

// Division function for double precision
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

// for length greater than 1 and less than 33 use split - 1 function for better optimization
static inline void MATHLIB_log10_vector_split1(size_t length, double *restrict pSrc, double *restrict pDst)
{

   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<double>::type vec;

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
   /* Create and assign values for constants employed on log10 computation */
   /***********************************************************************/

   vec Half, MAXe, srHalf, Half_sq, MINe, a0, a1, a2, b0, b1, b2, c1, c2, c10e, W, X, Y, Z, zn, zd, Rz, Sa, Bd, Cn, Da;

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
   c1      = (vec) 0.693359375;                /*  355/512      */
   c2      = (vec) -2.121944400546905827679e-4;
   c10e    = (vec) 0.43429448190325182765; /* log (base 10) of e */

   c7x::long_vec long_zero_vec   = (c7x::long_vec) 0;
   vec           double_zero_vec = (vec) 0.0;
   vec           outMAX          = (vec) (308.254715974092);

   // compute loop to perform vector log10
   for (size_t i = 0; i < numBlocks; i++) {

      vec a = c7x::strm_eng<0, vec>::get_adv();

      Y                  = a;
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

      X  = cmn_DIVDP_opt(zn, zd);
      W  = X * X;
      Bd = ((((W + b2) * W) + b1) * W) + b0;
      Cn = (((W * a2) + a1) * W) + a0;
      Rz = W * cmn_DIVDP_opt(Cn, Bd);
      Sa = X + (X * Rz);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Sa);
   }

   MATHLIB_SE0SA0Close();

   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);
   __SE1_OPEN(pDst, se0Params);

   for (size_t i = 0; i < numBlocks; i++) {

      vec a = c7x::strm_eng<0, vec>::get_adv();
      Sa    = c7x::strm_eng<1, vec>::get_adv();

      Y                  = a;
      c7x::long_vec exp_ = c7x::as_long_vec((c7x::as_ulong_vec(Y) << 1) >> 53);
      c7x::long_vec N    = exp_ - 1022;

      c7x::ulong_vec upper = c7x::as_ulong_vec(Y) & (0x000FFFFF00000000u);
      upper                = 0x3FE0000000000000u | upper;

      Z = c7x::as_double_vec((0x00000000FFFFFFFFu & c7x::as_ulong_vec(Y)) | upper);

      __vpred cmp1 = __cmp_eq_pred(exp_, long_zero_vec);
      Z            = __select(cmp1, double_zero_vec, Z);

      __vpred cmp2 = __cmp_lt_pred(srHalf, Z);

      N = __select(cmp2, N, (N - 1));

      Cn = __low_int_to_double(c7x::as_int_vec(N));
      Da = ((Cn * c2) + Sa) + (Cn * c1);
      Da = c10e * Da;

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      __vpred cmp_min = __cmp_lt_pred(Y, MINe);
      Da              = __select(cmp_min, -MAXe, Da);

      __vpred cmp_max = __cmp_lt_pred(MAXe, Y);
      Da              = __select(cmp_max, outMAX, Da);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Da);
   }

   __SE1_CLOSE();
   MATHLIB_SE0SA0Close();
}

// for length greater than 32 use split - 2 function for better optimization
static inline void MATHLIB_log10_vector_split2(size_t length, double *restrict pSrc, double *restrict pDst)
{
   // variables
   size_t numBlocks    = 0; // compute loop's iteration count
   size_t remNumBlocks = 0; // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<double>::type vec;

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
   /* Create and assign values for constants employed on log10 computation */
   /***********************************************************************/

   vec Half, MAXe, srHalf, Half_sq, MINe, a0, a1, a2, b0, b1, b2, c1, c2, c10e, W, X, Y, Z, zn, zd, Rz, Sa, Bd, Cn, Da;

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
   c1      = (vec) 0.693359375;                /*  355/512      */
   c2      = (vec) -2.121944400546905827679e-4;
   c10e    = (vec) 0.43429448190325182765; /* log (base 10) of e */

   c7x::long_vec long_zero_vec   = (c7x::long_vec) 0;
   vec           double_zero_vec = (vec) 0.0;
   vec           outMAX          = (vec) (308.254715974092);

   // compute loop to perform vector log10
   for (size_t i = 0; i < numBlocks; i++) {

      vec a = c7x::strm_eng<0, vec>::get_adv();

      Y                  = a;
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

      X = cmn_DIVDP_opt(zn, zd);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, X);
   }

   MATHLIB_SE0SA0Close();

   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pDst);

   for (size_t i = 0; i < numBlocks; i++) {

      X = c7x::strm_eng<0, vec>::get_adv();

      W  = X * X;
      Bd = ((((W + b2) * W) + b1) * W) + b0;
      Cn = (((W * a2) + a1) * W) + a0;
      Rz = W * cmn_DIVDP_opt(Cn, Bd);
      Sa = X + (X * Rz);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Sa);
   }

   MATHLIB_SE0SA0Close();

   MATHLIB_SE0SA0Open(&se0Params, &sa0Params, pSrc);

   __SE1_OPEN(pDst, se0Params);

   for (size_t i = 0; i < numBlocks; i++) {

      vec a = c7x::strm_eng<0, vec>::get_adv();
      Sa    = c7x::strm_eng<1, vec>::get_adv();

      Y                  = a;
      c7x::long_vec exp_ = c7x::as_long_vec((c7x::as_ulong_vec(Y) << 1) >> 53);
      c7x::long_vec N    = exp_ - 1022;

      c7x::ulong_vec upper = c7x::as_ulong_vec(Y) & (0x000FFFFF00000000u);
      upper                = 0x3FE0000000000000u | upper;

      Z = c7x::as_double_vec((0x00000000FFFFFFFFu & c7x::as_ulong_vec(Y)) | upper);

      __vpred cmp1 = __cmp_eq_pred(exp_, long_zero_vec);
      Z            = __select(cmp1, double_zero_vec, Z);

      __vpred cmp2 = __cmp_lt_pred(srHalf, Z);

      N = __select(cmp2, N, (N - 1));

      Cn = __low_int_to_double(c7x::as_int_vec(N));
      Da = ((Cn * c2) + Sa) + (Cn * c1);
      Da = c10e * Da;

      /**********************************************************************/
      /* Bounds checking                                                    */
      /**********************************************************************/

      __vpred cmp_min = __cmp_lt_pred(Y, MINe);
      Da              = __select(cmp_min, -MAXe, Da);

      __vpred cmp_max = __cmp_lt_pred(MAXe, Y);
      Da              = __select(cmp_max, outMAX, Da);

      __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
      vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
      __vstore_pred(tmp, addr, Da);
   }

   __SE1_CLOSE();

   MATHLIB_SE0SA0Close();
}

template <typename T> static inline void MATHLIB_log10_vector(size_t length, T *pSrc, T *pDst);

#if defined(__C7504__)
// this method performs log10 computation of input vector
template <> inline void MATHLIB_log10_vector<float>(size_t length, float *restrict pSrc, float *restrict pDst)
{
   // Call scalar code
   for (uint32_t i = 0; i < length; i++) {
      pDst[i] = MATHLIB_log10_scalar_ci<float>(pSrc[i]);
   }
}
#elif defined(__C7100__) || defined(__C7120__) || defined(__C7524__)
// this method performs log10 computation of input vector
template <> inline void MATHLIB_log10_vector<float>(size_t length, float *restrict pSrc, float *restrict pDst)
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
   /* Create and assign values for constants employed on log10 computation */
   /***********************************************************************/

   vec             C1, C2, C3, C4, C5, eMax, outVecMin, outVecMax;
   c7x::double_vec ln2, base;
   c7x::uint_vec   zero;
   zero = (c7x::uint_vec) 0;

   ln2       = (c7x::double_vec) 0.693147180559945;
   base      = (c7x::double_vec) 0.4342944819033f;
   C1        = (vec) -0.2302894f;
   C2        = (vec) 0.1908169f;
   C3        = (vec) -0.2505905f;
   C4        = (vec) 0.3333164f;
   C5        = (vec) -0.5000002f;
   eMax      = (vec) 3.402823466e+38f;
   outVecMin = (vec) 0xFF800000u;
   outVecMax = (vec) 308.2547f;

   // compute loop to perform vector log10
   for (size_t i = 0; i < numBlocks; i++) {
      vec inVec = c7x::strm_eng<0, vec>::get_adv();

      /**********************************************************************/
      /* Create variables employed on log10 computation                      */
      /**********************************************************************/
      vec             pol, r1, r2, r3, r4;
      c7x::double_vec inVecVals_odd, inVecVals_even, inVecVals_oddReciprocal, inVecVals_evenReciprocal,
          inVecReciprocalApprox_8_15, inVecReciprocalApprox_0_7, inVecVals_8_15, inVecVals_0_7, rVals_0_7, rVals_8_15,
          TVals_8_15, TVals_0_7, NVals_odd, NVals_even, NVals_0_7, NVals_8_15, outVec_8_15, outVec_0_7;
      c7x::uint_vec inVecReciprocal_32_63, inVecReciprocalClr_32_63, inVecReciprocalApprox_32_63, indexT;
      c7x::int_vec  N;
      vec           outVec;

      /**********************************************************************/
      /* Calculate Taylor series approximation for log10                     */
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
      indexT = (((inVecReciprocalApprox_32_63 << 12) >> 29) + MATHLIB_LOGTABLE_OFFSET);

      // Read from LUT or ILUT and reconstruct double values split into two vectors
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

// this method performs log10 computation of input vector
template <> inline void MATHLIB_log10_vector<double>(size_t length, double *restrict pSrc, double *restrict pDst)
{
   if (length < 33) {
      MATHLIB_log10_vector_split1(length, pSrc, pDst);
   }
   else {
      MATHLIB_log10_vector_split2(length, pSrc, pDst);
   }
}

// this method performs exponential computation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_log10(size_t length, T *restrict pSrc, T *restrict pDst)
{
   MATHLIB_STATUS status = MATHLIB_SUCCESS; // return function status

   // check for null pointers and non-zero length
   status = MATHLIB_checkParams(length, pSrc, pDst);

   // for length equal to 1 use scalar operations othewise use vector operations for better optimization
   if (status == MATHLIB_SUCCESS) {
      if (length < 2) {
         pDst[0] = MATHLIB_log10_scalar_ci<T>(pSrc[0]);
      }
      else {
         MATHLIB_log10_vector<T>(length, pSrc, pDst);
      }
   }
   return status;
}

template MATHLIB_STATUS MATHLIB_log10<float>(size_t length, float *pSrc, float *pDst);
template MATHLIB_STATUS MATHLIB_log10<double>(size_t length, double *pSrc, double *pDst);

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_log10_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_log10<float>(length, pSrc, pDst);
   return status;
}

// double-precision wrapper
MATHLIB_STATUS MATHLIB_log10_dp(size_t length, double *pSrc, double *pDst)
{
   MATHLIB_STATUS status = MATHLIB_log10<double>(length, pSrc, pDst);
   return status;
}

} // extern "C"
