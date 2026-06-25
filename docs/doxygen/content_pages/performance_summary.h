// Copyright (C) 2026 Texas Instruments Incorporated
//
// SPDX-License-Identifier: Apache-2.0

#if defined(__C7100__)
/**
 \page performance_summary Performance Summary

Performance data was obtained on the J721E EVM. EVM warm cycle obtained by
profiling the kernel's compute code execution after a cold run of the same code.
Please refer to the kernel's documentation for more information about the parameters
in the tables shown below.

<BR><HR>

\section MATHLIB MATHLIB kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels.

\htmlinclude ./html/benchmark_c7100.html

\section MATHLIB_DP MATHLIB double precision kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels for double precision
implementation.

\htmlinclude ./html/benchmark_dp_c7100.html

<BR>
*/

#elif defined(__C7120__)

/**
 \page performance_summary Performance Summary

Performance data was obtained on the J721S2 EVM. EVM warm cycle obtained by
profiling the kernel's compute code execution after a cold run of the same code.
Please refer to the kernel's documentation for more information about the parameters
in the tables shown below.

<BR><HR>

\section MATHLIB MATHLIB kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels.

\htmlinclude ./html/benchmark_c7120.html

\section MATHLIB_DP MATHLIB double precision kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels for double precision
implementation.

\htmlinclude ./html/benchmark_dp_c7120.html

<BR>

*/

#elif defined(__C7504__)

/**
 \page performance_summary Performance Summary

Performance data was obtained on the AM62A EVM. EVM warm cycle obtained by
profiling the kernel's compute code execution after a cold run of the same code.
Please refer to the kernel's documentation for more information about the parameters
in the tables shown below.

<BR><HR>

\section MATHLIB MATHLIB kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels.

(*) = Indicates algorithms that utilize scalar algorithm implementations due to lack of hardware support for lookup
tables.

\htmlinclude ./html/benchmark_c7504.html

\section MATHLIB_DP MATHLIB double precision kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels for double precision
implementation.

\htmlinclude ./html/benchmark_dp_c7504.html

<BR>

*/

#elif defined(__C7524__)

/**
 \page performance_summary Performance Summary

Performance data was obtained on the J722S EVM. EVM warm cycle obtained by
profiling the kernel's compute code execution after a cold run of the same code.
Please refer to the kernel's documentation for more information about the parameters
in the tables shown below.

(*) = Indicates algorithms that utilizes ILUT.

<BR><HR>

\section MATHLIB MATHLIB kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels.

\htmlinclude ./html/benchmark_c7524.html

\section MATHLIB_DP MATHLIB double precision kernels
This section contains tables that depict the expected performance numbers for the MATHLIB kernels for double precision
implementation.

\htmlinclude ./html/benchmark_dp_c7524.html

<BR>

*/

#endif

// clang-format on
