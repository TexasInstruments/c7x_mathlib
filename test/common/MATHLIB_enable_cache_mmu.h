// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_ENABLE_CACHE_MMU_H_
#define MATHLIB_ENABLE_CACHE_MMU_H_

#include "MATHLIB_test_c7xecr.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void MATHLIB_enable_cache_mmu(uint64_t ttbr);

#ifdef __cplusplus
}
#endif // extern "C"

#endif
