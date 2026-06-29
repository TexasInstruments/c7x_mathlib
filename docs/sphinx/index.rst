:icon: material/home

MATHLIB — C7x Math Library
==========================

MATHLIB is TI's optimized mathematical functions library for the **C7™** DSP architecture.
It provides a suite of scalar and vector transcendental math kernels targeting the C7™
vector and streaming engines (SE/SA), enabling high-throughput computation of trigonometric,
hyperbolic, exponential, logarithmic, and arithmetic functions on TI SoCs such as AM62D and AM275.

Each kernel is implemented as a C++ template function supporting ``float`` and ``double``
precision, with C interfaces ``_sp`` (single precision) and ``_dp`` (double precision).

.. grid:: 1 2 2 3
   :gutter: 3
   :class-container: ti-home-grid

   .. grid-item-card:: :si-icon:`material/rocket-launch` Getting Started
      :link: getting_started
      :link-type: doc

      Build, run, and generate test data — from PC host emulation to on-target EVM.

   .. grid-item-card:: :si-icon:`material/book-open-variant` API Reference
      :link: api/index
      :link-type: doc

      Every kernel, grouped by category, with signatures, parameters, and usage notes.

   .. grid-item-card:: :si-icon:`material/chart-line` Performance
      :link: perf/index
      :link-type: doc

      Measured kernel cycle counts on C7504 (AM62D) and C7524 (AM275).

   .. grid-item-card:: :si-icon:`material/file-tree` Repository Layout
      :link: repository_layout
      :link-type: doc

      A guided tour of the source tree — kernels, tests, and build files.

   .. grid-item-card:: :si-icon:`material/scale-balance` License
      :link: license
      :link-type: doc

      MATHLIB is distributed under the Apache License 2.0.

.. toctree::
   :hidden:
   :maxdepth: 2

   getting_started
   repository_layout
   api/index
   perf/index
   license
