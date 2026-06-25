// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include "MATHLIB_test.h"
#if !defined(_HOST_BUILD)
extern const uint64_t pte_lvl0[512];
#endif
void MATHLIB_test_init()
{
#if !defined(_HOST_BUILD)
   // enable MMU
   MATHLIB_enable_cache_mmu((uint64_t) pte_lvl0);
#endif

   // invalidate TLB
   MATHLIB_invalidate_tlb();

#ifndef MATHLIB_FIXED_SEED
   time_t t;
   time(&t);
   srand((unsigned) t);
#else
   srand((unsigned) MATHLIB_FIXED_SEED);
#endif

#if defined(_HOST_BUILD)
#if defined(MATHLIB_DEBUGPRINT)
   streaming_engine::set_debug_level(MATHLIB_DEBUGPRINT);
   sa_generator::set_debug_level(MATHLIB_DEBUGPRINT);
#endif
#endif
}
