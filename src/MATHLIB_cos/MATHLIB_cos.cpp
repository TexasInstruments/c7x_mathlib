// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#define ELEMENT_COUNT(x) c7x::element_count_of<x>::value

#include "MATHLIB_types.h"
#include "MATHLIB_utility.h"

// this method performs cosine coputation of input vector
template <typename T> MATHLIB_STATUS MATHLIB_cos(size_t length, T *pSrc, T *pDst);

template <> MATHLIB_STATUS MATHLIB_cos<float>(size_t length, float *pSrc, float *pDst)
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

      /**********************************************************************/
      /* Create and assign values for constants employed on cos computation */
      /**********************************************************************/

      vec InvPI, HalfPI, One, MAX, Zero, s1, s2, s3, s4, C1, C2;

      InvPI  = (vec) 0.318309886183791;
      HalfPI = (vec) 1.5707963268;
      One    = (vec) 1.0;
      MAX    = (vec) 1048576.0;

      Zero = (vec) 0.0;
      s1   = (vec) -1.666665668e-1;
      s2   = (vec) 8.333025139e-3;
      s3   = (vec) -1.980741872e-4;
      s4   = (vec) 2.601903036e-6;
      C1   = (vec) 3.140625;
      C2   = (vec) 9.67653589793e-4;

      // compute loop to perform vector cos
      for (size_t i = 0; i < numBlocks; i++) {
         vec inVec = c7x::strm_eng<0, vec>::get_adv();

         /**********************************************************************/
         /* Create and assign values for variables employed on cos computation */
         /**********************************************************************/

         vec Sign, X, Y, Z, F, G, R;

         vec_type int_one = (vec_type) 1;
         vec_type N;

         vec negativeOne = (vec) -1;
         vec negativeR;
         Sign = One;

         Y              = __abs(inVec) + HalfPI;
         __vpred cmp_gt = __cmp_lt_pred((vec) MAX, Y);
         Y              = __select(cmp_gt, HalfPI, Y);

         // X = Y * (1/PI)
         X = Y * InvPI;

         N = __float_to_int(X);
         Z = c7x::convert<vec>(N);

         /**********************************************************************/
         /* Sign checking for quadrant 3 or 4                                  */
         /**********************************************************************/

         vec_type andN         = N & int_one;
         vec      convert_andN = c7x::convert<vec>(andN);
         __vpred  cmp_mod      = __cmp_le_pred(convert_andN, Zero);
         vec      Sign_T       = __select(cmp_mod, Sign, negativeOne);

         F = (Y - (Z * C1)) - (Z * C2);
         R = F;

         negativeR     = -R;
         __vpred cmp_F = __cmp_lt_pred(F, Zero);
         R             = __select(cmp_F, R, negativeR);

         G = F * F;

         __vpred cmp_RMin = __cmp_lt_pred(R, Zero);

         vec outputRMin = R * Sign_T;
         R              = ((((((s4 * G) + s3) * G) + s2) * G) + s1) * G;
         vec outVec     = ((F + (F * R)) * Sign_T);

         outVec = __select(cmp_RMin, outVec, outputRMin);

         // outVec.print();

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, outVec);
      }

      MATHLIB_SE0SA0Close();
   }

   return status;
}

template <> MATHLIB_STATUS MATHLIB_cos<double>(size_t length, double *pSrc, double *pDst)
{

   // variables
   MATHLIB_STATUS status       = MATHLIB_SUCCESS; // return function status
   size_t         numBlocks    = 0;               // compute loop's iteration count
   size_t         remNumBlocks = 0;               // when numBlocks is not a multiple of SIMD width

   // derive c7x vector type from template typename
   typedef typename c7x::make_full_vector<c7x::double_vec>::type vec;
   typedef typename c7x::make_full_vector<c7x::int_vec>::type    vec_type;

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

      /**********************************************************************/
      /* Create and assign values for constants employed on cos computation */
      /**********************************************************************/

      vec InvPI, HalfPI, Zero, negativeOne, MAX, Sign, r1, r2, r3, r4, r5, r6, r7, r8, C1, C2;

      C1          = (vec) 3.1416015625;
      C2          = (vec) -8.908910206761537356617e-6;
      r8          = (vec) 2.7204790957888846175e-15;
      r7          = (vec) -7.6429178068910467734e-13;
      r6          = (vec) 1.6058936490371589114e-10;
      r5          = (vec) -2.5052106798274584544e-8;
      r4          = (vec) 2.7557319210152756119e-6;
      r3          = (vec) -1.9841269841201840457e-4;
      r2          = (vec) 8.3333333333331650314e-3;
      r1          = (vec) -1.6666666666666665052e-1;
      MAX         = (vec) 1.073741824e+09;
      HalfPI      = (vec) 1.57079632679489661923;
      InvPI       = (vec) 0.31830988618379067154;
      Sign        = (vec) 1.0;
      Zero        = (vec) 0;
      negativeOne = (vec) -1.0;

      vec_type int_one = (vec_type) 1;

      vec      X, Z, F, F2, G, R;
      vec_type N;

      // compute loop to perform vector cos
      for (size_t i = 0; i < numBlocks; i++) {
         vec inVec = c7x::strm_eng<0, vec>::get_adv();

         F              = __abs(inVec) + HalfPI;
         __vpred cmp_gt = __cmp_lt_pred(MAX, F);
         F              = __select(cmp_gt, HalfPI, F);

         X = F * InvPI;
         N = __double_to_int(X);
         Z = __low_int_to_double(N);

         vec_type andN         = N & int_one;
         vec      convert_andN = __low_int_to_double(andN);
         __vpred  cmp_mod      = __cmp_le_pred(convert_andN, Zero);
         vec      sign_vec     = __select(cmp_mod, Sign, negativeOne);

         F  = (F - (Z * C1)) - (Z * C2);
         R  = __abs(F);
         F2 = F * F;
         G  = F2 * F2;
         R  = ((((((G * r8) + r6) * G) + r4) * G) + r2) * G;
         X  = ((((((G * r7) + r5) * G) + r3) * G) + r1) * F2;
         R  = R + X;
         G  = (F + (F * R)) * sign_vec;

         __vpred tmp  = c7x::strm_agen<0, vec>::get_vpred();
         vec    *addr = c7x::strm_agen<0, vec>::get_adv(pDst);
         __vstore_pred(tmp, addr, G);
      }

      MATHLIB_SE0SA0Close();
   }

   return status;
}

extern "C" {

// single-precision wrapper
MATHLIB_STATUS MATHLIB_cos_sp(size_t length, float *pSrc, float *pDst)
{
   MATHLIB_STATUS status = MATHLIB_cos<float>(length, pSrc, pDst);
   return status;
}

// double-precision wrapper
MATHLIB_STATUS MATHLIB_cos_dp(size_t length, double *pSrc, double *pDst)
{
   MATHLIB_STATUS status = MATHLIB_cos<double>(length, pSrc, pDst);
   return status;
}

} // extern "C"
