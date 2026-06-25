<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://www.ti.com/content/dam/ticom/images/identities/ti-brand/ti-logo-hz-1c-white.svg" width="300">
  <img alt="Texas Instruments Logo" src="https://www.ti.com/content/dam/ticom/images/identities/ti-brand/ti-hz-2c-pos-rgb.svg" width="300">
</picture>

# MATHLIB — C7<sup>™</sup> Mathematical Functions Library

[Summary](#summary) | [Features](#features) | [Operators](#operators) | [Supported Devices](#supported-devices) | [Setup Instructions](#setup-instructions) | [Build Instructions](#build-instructions) | [Related Repos](#related-repos) | [Licensing](#licensing) | [Contributions](#contributions) | [Developer Resources](#developer-resources)

</div>

## Summary

MATHLIB is TI's optimized mathematical functions library for the **C7<sup>™</sup>** DSP architecture.
It provides a suite of scalar and vector transcendental math kernels targeting the C7<sup>™</sup>
vector and streaming engines (SE/SA), enabling high-throughput computation of trigonometric,
hyperbolic, exponential, logarithmic, and arithmetic functions on TI SoCs such as AM62D and AM275.

Each kernel is implemented as a C++ template function supporting `float` and `double` precision,
with C interfaces `_sp` (single precision) and `_dp` (double precision).

Each kernel has a corresponding test driver (`_test.cpp`) with test parameters in `_testParams.cpp`.
Some kernels include a Python reference implementation under `test/MATHLIB_testParams_gen/MATHLIB_<kernel>/` for test vector generation.


## Features

- **24 kernels** spanning trigonometric, hyperbolic, exponential, logarithmic, and arithmetic functions
- **C7<sup>™</sup> streaming engine optimized** — leverages SE/SA hardware for peak throughput across all kernels
- **CMake-based build** with presets for PC simulation and on-target deployment
- **Multi-type support** — `float`, `double`


## Operators

### Trigonometric
| Kernel | Description |
|--------|-------------|
| `MATHLIB_sin` | Sine |
| `MATHLIB_cos` | Cosine |
| `MATHLIB_tan` | Tangent |
| `MATHLIB_asin` | Arc sine |
| `MATHLIB_acos` | Arc cosine |
| `MATHLIB_atan` | Arc tangent |
| `MATHLIB_atan2` | Two-argument arc tangent |

### Hyperbolic
| Kernel | Description |
|--------|-------------|
| `MATHLIB_sinh` | Hyperbolic sine |
| `MATHLIB_cosh` | Hyperbolic cosine |
| `MATHLIB_tanh` | Hyperbolic tangent |
| `MATHLIB_asinh` | Inverse hyperbolic sine |
| `MATHLIB_acosh` | Inverse hyperbolic cosine |
| `MATHLIB_atanh` | Inverse hyperbolic tangent |

### Exponential & Logarithmic
| Kernel | Description |
|--------|-------------|
| `MATHLIB_exp` | Natural exponential (e^x) |
| `MATHLIB_exp2` | Base-2 exponential (2^x) |
| `MATHLIB_exp10` | Base-10 exponential (10^x) |
| `MATHLIB_log` | Natural logarithm |
| `MATHLIB_log2` | Base-2 logarithm |
| `MATHLIB_log10` | Base-10 logarithm |
| `MATHLIB_pow` | Power function (x^y) |

### Arithmetic
| Kernel | Description |
|--------|-------------|
| `MATHLIB_sqrt` | Square root |
| `MATHLIB_rsqrt` | Reciprocal square root |
| `MATHLIB_recip` | Reciprocal |
| `MATHLIB_div` | Division |


## Supported Devices

| Device | C7<sup>™</sup> Core |
|--------|----------|
| AM62D | C7504 |
| AM275 | C7524 |


## Setup Instructions

### Prerequisites

- **Linux** host (Ubuntu 20.04 or later recommended)
- **CMake** ≥ 3.21
- **TI C7000 Code Generation Tools** (`ti-cgt-c7000`) — install to `~/ti/`

  Download from [TI CGT C7000](https://www.ti.com/tool/C7000-CGT).
  Default expected path: `~/ti/ti-cgt-c7000_5.0.0.LTS/`

  Or set the environment variable before building:
  ```bash
  export CGT7X_ROOT=/path/to/ti-cgt-c7000_<version>
  ```


## Build Instructions

MATHLIB uses **CMake presets** to manage build configurations.

### Available Presets

| Preset | SoC | Platform |
|--------|-----|----------|
| `release-autotest-am62d-pc` | AM62D | PC simulation + tests |
| `release-autotest-am62d-target` | AM62D | On-device + tests |
| `release-autotest-am275-pc` | AM275 | PC simulation + tests |
| `release-autotest-am275-target` | AM275 | On-device + tests |
| `release-buildlib-am62d-pc` | AM62D | PC (library only) |
| `release-buildlib-am62d-target` | AM62D | On-device (library only) |
| `release-buildlib-am275-pc` | AM275 | PC (library only) |
| `release-buildlib-am275-target` | AM275 | On-device (library only) |

### Quick Start — PC simulation (AM62D)

```bash
# Configure
cmake -S . -B build --preset=release-autotest-am62d-pc

# Build
cmake --build build -- -j$(nproc)

# Build test binaries (ctest not supported; run binaries directly on target)
cmake --build build -- -j$(nproc)
```

### On-Target Build (AM62D)

```bash
cmake -S . -B build --preset=release-autotest-am62d-target
cmake --build build -- -j$(nproc)
```

### Library-Only Build

```bash
cmake -S . -B build --preset=release-buildlib-am62d-pc
cmake --build build -- -j$(nproc)
```


## Related Repos

- [MCU+ SDK](https://github.com/TexasInstruments/mcupsdk-core) — SoC drivers and middleware for AM2x/AM6x devices


## Licensing

This repository is licensed under the **Apache License, Version 2.0**.
See [LICENSE](LICENSE) for the full text.

All source files carry an SPDX `Apache-2.0` identifier.


## Contributions

This repository is not currently accepting community contributions.

Bug reports and feature requests are welcome via [TI E2E Community Forums](https://e2e.ti.com).


---

## Developer Resources

[TI E2E™ design support forums](https://e2e.ti.com)
