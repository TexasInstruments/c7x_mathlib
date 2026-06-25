// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATHLIB_SCALARTABLES_H_
#define MATHLIB_SCALARTABLES_H_ 1

const double MATHLIB_kTable[4] = {
    1.000000000, /* 2^(0/4) */
    1.189207115, /* 2^(1/4) */
    1.414213562, /* 2^(2/4) */
    1.681792831  /* 2^(3/4) */
};

const double MATHLIB_jTable[4] = {
    1.000000000, /* 2^(0/16) */
    1.044273782, /* 2^(1/16) */
    1.090507733, /* 2^(2/16) */
    1.138788635  /* 2^(3/16) */
};

const double MATHLIB_logTable[8] = {0.0000000000,  -0.1177830356, -0.2231435513, -0.3184537311,
                                    -0.4054651081, -0.4855078157, -0.5596157879, -0.6286086594};

#endif // MATHLIB_SCALARTABLES_H_
