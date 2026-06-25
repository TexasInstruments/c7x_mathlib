// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include <c7x.h>
#include <cfloat>
#include <cstdint>
#include <mathlib.h>
#include <stdio.h>

#include "MATHLIB_ilut.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_memory.h"
#include "MATHLIB_pow_testParams.h"
#include "MATHLIB_profile.h"
#include "MATHLIB_types.h"

#if ENABLE_LDRA_COVERAGE
#include "MATHLIB_ldra.h"
#endif

/******************************************************************************/
/*                                                                            */
/* Defines                                                                    */
/*                                                                            */
/******************************************************************************/

#ifdef WIN32
#if defined(__C7504__) || defined(__C7524__)
int8_t l2auxBuffer[MATHLIB_L2_BUFFER_SIZE];
int8_t ddrBuffer[2048 * 1024];
#else
int8_t msmcBuffer[MATHLIB_L3_RESULTS_BUFFER_SIZE];
int8_t ddrBuffer[2048 * 1024];
#endif
#else
#if defined(__C7504__) || defined(__C7524__)
__attribute__((section(".l2sramaux"), aligned(64))) int8_t l2auxBuffer[MATHLIB_L2_BUFFER_SIZE];
__attribute__((section(".ddrData"), aligned(64))) int8_t   ddrBuffer[2048 * 1024];

#else
__attribute__((section(".msmcData"), aligned(64))) int8_t msmcBuffer[MATHLIB_L3_RESULTS_BUFFER_SIZE];
__attribute__((section(".ddrData"), aligned(64))) int8_t  ddrBuffer[2048 * 1024];

#endif
#endif // WIN32

/******************************************************************************/
/*                                                                            */
/* MATHLIB_pow_runTest                                                         */
/*                                                                            */
/******************************************************************************/

// this method tests the kernel for a given test case
template <typename T>
int32_t MATHLIB_pow_runTest(MATHLIB_pow_testParams_t *restrict pCurrPrm,
                            size_t length,
                            T *restrict pSrc0,
                            T *restrict pSrc1,
                            T *restrict pDst,
                            uint32_t *restrict pProfile)
{

   // variables
   uint32_t           testNum;                               // test number
   uint64_t           archCycles = 0;                        // theoretical maximum for C7x
   uint64_t           estCycles  = 0;                        // estimated cycle count based on asm report
   int32_t            status     = MATHLIB_TEST_KERNEL_FAIL; // pass or fail flag for test
   MATHLIB_TestMetric testMetric = MATHLIB_PERCENT_ERROR;    // tolerance type

   // assign test case number to testID
   testNum = pCurrPrm->testID;

   // create buffers in L2SRAM for test case
   pSrc0 = (T *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(T));
   pSrc1 = (T *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(T));
   pDst  = (T *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(T));

   // fill buffer with test case data from DDR
   MATHLIB_fillBuffer((T *) pCurrPrm->staticIn0, pSrc0, length);
   MATHLIB_fillBuffer((T *) pCurrPrm->staticIn1, pSrc1, length);

   /**************************** VECTOR *******************************/
   /*******************************************************************/

   int32_t status_v = MATHLIB_TEST_KERNEL_FAIL; // pass or fail flag for test

   // cold run
   MATHLIB_profile_start(MATHLIB_PROFILE_KERNEL_OPT_COLD);
   MATHLIB_pow(length, pSrc0, pSrc1, pDst);
   MATHLIB_profile_stop();

   // warm run
   MATHLIB_profile_start(MATHLIB_PROFILE_KERNEL_OPT_WARM);
   MATHLIB_pow(length, pSrc0, pSrc1, pDst);
   MATHLIB_profile_stop();

   status_v = MATHLIB_compare_mem(pDst, (T *) pCurrPrm->staticOut, length, MATHLIB_POW_PERCENT_ERROR, testMetric);

   /**************************** INLINED SCALAR ***********************/
   /*******************************************************************/

   int32_t status_inline = MATHLIB_TEST_KERNEL_FAIL; // pass or fail flag for test

   /* Cold run */
   pDst[0] = MATHLIB_pow_scalar_ci(pSrc0[0], pSrc1[0]);
   pDst[0] = MATHLIB_pow_scalar_ci(pSrc0[0], pSrc1[0]);

   /* Warm run */
   for (size_t ii = 0; ii < length; ii++) {
      MATHLIB_profile_start(MATHLIB_PROFILE_KERNEL_CN);
      pDst[ii] = MATHLIB_pow_scalar_ci(pSrc0[ii], pSrc1[ii]);
      MATHLIB_profile_stop();
   }

   status_inline = MATHLIB_compare_mem(pDst, (T *) pCurrPrm->staticOut, length, MATHLIB_POW_PERCENT_ERROR, testMetric);

   // sanity check
   if ((status_v == MATHLIB_TEST_KERNEL_PASS) && (status_inline == MATHLIB_TEST_KERNEL_PASS)) {
      status = MATHLIB_TEST_KERNEL_PASS;
   }
   else {
      status = MATHLIB_TEST_KERNEL_FAIL;
   }
   // free L2SRAM memory
   MATHLIB_align_free(pSrc0);
   MATHLIB_align_free(pSrc1);
   MATHLIB_align_free(pDst);

   // print test case results
   sprintf(desc, "dataType=%d, length=%ld", pCurrPrm->dType, pCurrPrm->length);
   MATHLIB_profile_add_test(testNum++, length, archCycles, estCycles, status, desc);

   return status;
}

/******************************************************************************/
/*                                                                            */
/* MATHLIB_pow_testBench                                                       */
/*                                                                            */
/******************************************************************************/

// this method performs regression testing for the MATHLIB_exp kernel
int MATHLIB_pow_testBench(uint32_t *restrict pProfile, uint8_t LevelOfFeedback)
{
   // variables to keep track of test cases
   size_t                    tpi = 0; // test case iteration variable
   MATHLIB_pow_testParams_t *prm;     // pointer to test params struct
   MATHLIB_pow_testParams_t  currPrm; // current test case

   // variables to track test cases status
   int32_t status          = MATHLIB_TEST_KERNEL_FAIL; // fail or pass flag
   int32_t currentTestFail = 0;                        // status of current test case
   int32_t fail            = 0;                        // track number failing test cases

   // kernel-specific variables
   size_t length = 0;    // vector length
   void  *pSrc0  = NULL; // pointer to input vector0; assigned later
   void  *pSrc1  = NULL; // pointer to input vector1; assigned later
   void  *pDst   = NULL; // pointer to output vector; assigned later

   // get total number of test case
   MATHLIB_pow_getTestParams(&prm, &test_cases);

   // name of profile
   MATHLIB_profile_init((char *) "MATHLIB_pow");

   // initialize LUT
#if defined(__C7100__) || defined(__C7120__)
   MATHLIB_LUTInit();
#elif defined(__C7524__)
   MATHLIB_ILUTInit();
#endif

   // loop through all test cases
   for (tpi = 0; tpi < (size_t) test_cases; tpi++) {

      currPrm = prm[tpi];
      length  = currPrm.length;

      // determine datatype of test case and call function to run test case
      if (currPrm.dType == MATHLIB_FLOAT32) {

         status = MATHLIB_pow_runTest(&currPrm, length, (float *) pSrc0, (float *) pSrc1, (float *) pDst, &pProfile[0]);
      }
      else if (currPrm.dType == MATHLIB_FLOAT64) {

         status =
             MATHLIB_pow_runTest(&currPrm, length, (double *) pSrc0, (double *) pSrc1, (double *) pDst, &pProfile[0]);
      }

      // update profiling information
      pProfile[MATHLIB_PROFILE_TEST_TYPES * tpi] =
          (int32_t) MATHLIB_profile_get_cycles(MATHLIB_PROFILE_KERNEL_OPT_COLD);
      pProfile[MATHLIB_PROFILE_TEST_TYPES * tpi + 1] =
          (int32_t) MATHLIB_profile_get_cycles(MATHLIB_PROFILE_KERNEL_OPT_WARM);

      // update status of current test case and updated fail counter appropriately
      currentTestFail = (status == MATHLIB_TEST_KERNEL_FAIL) ? 1 : 0;
      fail += currentTestFail;
   }

   return fail;
}

int coverage_test_main(uint32_t *restrict pProfile)
{
   int32_t        testNum         = 1000;
   int32_t        currentTestPass = 0;
   MATHLIB_STATUS status;
   int            fail   = 0;
   size_t         length = 16;

   float  *pSrc0_sp = (float *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(float));
   float  *pSrc1_sp = (float *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(float));
   float  *pDst_sp  = (float *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(float));
   double *pSrc0_dp = (double *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(double));
   double *pSrc1_dp = (double *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(double));
   double *pDst_dp  = (double *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(double));

   while (testNum <= 1018) {
      switch (testNum) {
      case 1000:
         status          = MATHLIB_pow(16, (float *) NULL, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_ERR_NULL_POINTER);
         break;
      case 1001:
         status          = MATHLIB_pow(16, pSrc0_sp, (float *) NULL, pDst_sp);
         currentTestPass = (status == MATHLIB_ERR_NULL_POINTER);
         break;
      case 1002:
         status          = MATHLIB_pow(16, pSrc0_sp, pSrc1_sp, (float *) NULL);
         currentTestPass = (status == MATHLIB_ERR_NULL_POINTER);
         break;
      case 1003:
         status          = MATHLIB_pow(0, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_ERR_INVALID_DIMENSION);
         break;
      case 1004:
         pSrc0_sp[0]     = FLT_MAX;
         pSrc1_sp[0]     = FLT_MAX;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
      case 1005:
         pSrc0_sp[0]     = -1.0;
         pSrc1_sp[0]     = 5.0;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1006:
         pSrc0_sp[0]     = -1.0;
         pSrc1_sp[0]     = 5.660;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1007:
         pSrc0_sp[0]     = 0.0;
         pSrc1_sp[0]     = 1.0;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1008:
         pSrc0_sp[0]     = 0.0;
         pSrc1_sp[0]     = -1.0;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1009:
         pSrc0_sp[0]     = -88.33654475f;
         pSrc1_sp[0]     = -88.33654475f;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1010:
         pSrc0_sp[0]     = -1.0;
         pSrc1_sp[0]     = 6.0;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1011:
         pSrc0_sp[0]     = -1.0;
         pSrc1_sp[0]     = 0.0;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1012:
         pSrc0_sp[0]     = 1.0f / 0.0f;
         pSrc1_sp[0]     = 1.0;
         status          = MATHLIB_pow_sp(1, pSrc0_sp, pSrc1_sp, pDst_sp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1013:
         pSrc0_dp[0]     = -1.0f;
         pSrc1_dp[0]     = 1.0f;
         status          = MATHLIB_pow_dp(1, pSrc0_dp, pSrc1_dp, pDst_dp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1014:
         pSrc0_dp[0]     = 0.0;
         pSrc1_dp[0]     = -1.0;
         status          = MATHLIB_pow_dp(1, pSrc0_dp, pSrc1_dp, pDst_dp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1015:
         pSrc0_dp[0]     = -1.0f / 0.0f;
         pSrc1_dp[0]     = 0.5f;
         status          = MATHLIB_pow_dp(1, pSrc0_dp, pSrc1_dp, pDst_dp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1016:
         pSrc0_dp[0]     = 1.0f;
         pSrc1_dp[0]     = 0.5f;
         status          = MATHLIB_pow_dp(1, pSrc0_dp, pSrc1_dp, pDst_dp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1017:
         pSrc0_dp[0]     = -1.0f;
         pSrc1_dp[0]     = 2.0f;
         status          = MATHLIB_pow_dp(1, pSrc0_dp, pSrc1_dp, pDst_dp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      case 1018:
         pSrc0_dp[0]     = DBL_MAX;
         pSrc1_dp[0]     = -1.0 / 0.0f;
         status          = MATHLIB_pow_dp(1, pSrc0_dp, pSrc1_dp, pDst_dp);
         currentTestPass = (status == MATHLIB_SUCCESS);
         break;
      default:
         break;
      }

      fail = ((fail == 1) || (currentTestPass == 0)) ? 1 : 0;
      sprintf(desc, "%s", "COVERAGE TEST");
      MATHLIB_profile_add_test(testNum++, 0, 0, 0, currentTestPass, desc);
   }
#if defined(ENABLE_LDRA_COVERAGE)
   /* For every call of DSPLIB_add_getHandleSize() the execution history is pushed
      as this function is the anchor point for LDRA in .cpp kernel files.
      Therefore calling DSPLIB_add_getHandleSize() to push the execution history
      at the end of coverage test cases. */
   MATHLIB_ldra_hook();
   printf("!!! Pushing final execution history for MATHLIB_pow()\n");
#endif
   MATHLIB_align_free(pSrc0_sp);
   MATHLIB_align_free(pSrc1_sp);
   MATHLIB_align_free(pDst_sp);
   MATHLIB_align_free(pSrc0_dp);
   MATHLIB_align_free(pSrc1_dp);
   MATHLIB_align_free(pDst_dp);
   return fail;
}

/******************************************************************************/
/*                                                                            */
/* main                                                                       */
/*                                                                            */
/******************************************************************************/

int main(void)
{
   // variables
   uint32_t profile[MATHLIB_PROFILE_MAX_TESTS * MATHLIB_PROFILE_TEST_TYPES]; // collect profiling data for test cases
   int      test_fail = 0, cov_fail = 0;                                     // track the number of failed test cases

   MATHLIB_test_init();

   // call test bench to run the regression test
   test_fail = MATHLIB_pow_testBench(&profile[0], 0);
   cov_fail  = coverage_test_main(&profile[0]);

   // check if regression passed or failed
   if ((test_fail == 0) && (cov_fail == 0)) {
      printf("All Test Pass!\n");
   }
   else {
      printf("Test Fail!\n");
   }

   return test_fail || cov_fail;
}
