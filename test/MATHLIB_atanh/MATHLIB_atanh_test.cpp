// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include <c7x.h>
#include <cstdint>
#include <mathlib.h>
#include <stdio.h>

#include "MATHLIB_atanh_testParams.h"
#include "MATHLIB_ilut.h"
#include "MATHLIB_lut.h"
#include "MATHLIB_memory.h"
#include "MATHLIB_profile.h"
#include "MATHLIB_types.h"

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
/* MATHLIB_atanh_runTest                                                       */
/*                                                                            */
/******************************************************************************/

// this method tests the kernel for a given test case
template <typename T>
int32_t MATHLIB_atanh_runTest(MATHLIB_atanh_testParams_t *pCurrPrm, size_t length, T *pSrc, T *pDst, uint32_t *pProfile)
{

   // variables
   uint32_t           testNum;                                 // test number
   uint64_t           archCycles = 0;                          // theoretical maximum for C7x
   uint64_t           estCycles  = 0;                          // estimated cycle count based on asm report
   int32_t            status     = MATHLIB_TEST_KERNEL_FAIL;   // pass or fail flag for test
   MATHLIB_TestMetric testMetric = MATHLIB_ABSOLUTE_TOLERANCE; // tolerance type

   // assign test case number to testID
   testNum = pCurrPrm->testID;

   // create buffers in L2SRAM for test case
   pSrc = (T *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(T));
   pDst = (T *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(T));

   // fill buffer with test case data from DDR
   MATHLIB_fillBuffer((T *) pCurrPrm->staticIn, pSrc, length);

   // cold run
   MATHLIB_profile_start(MATHLIB_PROFILE_KERNEL_OPT_COLD);
   MATHLIB_atanh(length, pSrc, pDst);
   MATHLIB_profile_stop();

   // warm run
   MATHLIB_profile_start(MATHLIB_PROFILE_KERNEL_OPT_WARM);
   MATHLIB_atanh(length, pSrc, pDst);
   MATHLIB_profile_stop();

   // sanity check
   status = MATHLIB_compare_mem(pDst, (T *) pCurrPrm->staticOut, length, 10e-2, testMetric);

   // free L2SRAM memory
   MATHLIB_align_free(pSrc);
   MATHLIB_align_free(pDst);

   // print test case results
   sprintf(desc, "dataType=%d, length=%ld", pCurrPrm->dType, pCurrPrm->length);
   MATHLIB_profile_add_test(testNum++, length, archCycles, estCycles, status, desc);

   return status;
}

/******************************************************************************/
/*                                                                            */
/* MATHLIB_atanh_testBench                                                     */
/*                                                                            */
/******************************************************************************/

// this method performs regression testing for the MATHLIB_atanh kernel
int MATHLIB_atanh_testBench(uint32_t *pProfile, uint8_t LevelOfFeedback)
{
   // variables to keep track of test cases
   size_t                      tpi = 0; // test case iteration variable
   MATHLIB_atanh_testParams_t *prm;     // pointer to test params struct
   MATHLIB_atanh_testParams_t  currPrm; // current test case

   // variables to track test cases status
   int32_t status          = MATHLIB_TEST_KERNEL_FAIL; // fail or pass flag
   int32_t currentTestFail = 0;                        // status of current test case
   int32_t fail            = 0;                        // track number failing test cases

   // kernel-specific variables
   size_t length = 0;    // vector length
   void  *pSrc   = NULL; // pointer to input vector0; assigned later
   void  *pDst   = NULL; // pointer to output vector; assigned later

   // get total number of test case
   MATHLIB_atanh_getTestParams(&prm, &test_cases);

   // name of profile
   MATHLIB_profile_init((char *) "MATHLIB_atanh");

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

         status = MATHLIB_atanh_runTest(&currPrm, length, (float *) pSrc, (float *) pDst, &pProfile[0]);
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

int coverage_test_main(uint32_t *pProfile)
{
   int32_t        testNum         = 1000;
   int32_t        currentTestPass = 0;
   MATHLIB_STATUS status;
   int            fail   = 0;
   uint32_t       i      = 0;
   size_t         length = 16;

   float *pSrc = (float *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(float));
   float *pDst = (float *) MATHLIB_memalign(MATHLIB_L2DATA_ALIGNMENT, length * sizeof(float));
   for (i = 0; i < 16; i++) {
      pSrc[i] = 1.0f;
   }

   while (testNum <= 1003) {
      switch (testNum) {
      case 1000:
         status          = MATHLIB_atanh(16, (float *) NULL, pDst);
         currentTestPass = (status == MATHLIB_ERR_NULL_POINTER);
         break;
      case 1001:
         status          = MATHLIB_atanh(16, pSrc, (float *) NULL);
         currentTestPass = (status == MATHLIB_ERR_NULL_POINTER);
         break;
      case 1002:
         status          = MATHLIB_atanh(0, pSrc, pDst);
         currentTestPass = (status == MATHLIB_ERR_INVALID_DIMENSION);
         break;
      case 1003:
         status          = MATHLIB_atanh_sp(16, pSrc, pDst);
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
   MATHLIB_atanh(0, pSrc, pDst);
   printf("!!! Pushing final execution history for MATHLIB_atanh()\n");
#endif

   MATHLIB_align_free(pSrc);
   MATHLIB_align_free(pDst);
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
   test_fail = MATHLIB_atanh_testBench(&profile[0], 0);
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
