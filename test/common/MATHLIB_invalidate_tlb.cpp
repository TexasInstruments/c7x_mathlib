// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#include "MATHLIB_invalidate_tlb.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void MATHLIB_invalidate_tlb()
{
#if !defined(_HOST_BUILD)
   MATHLIB_TEST_c7xMmuTlbInvAll();
#endif
}

#ifdef __cplusplus
}
#endif // extern "C"
