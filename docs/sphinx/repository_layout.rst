:icon: material/file-tree

Repository Layout
=================

Top-level structure of the MATHLIB repository:

.. raw:: html

   <style>
   .repo-tree summary::-webkit-details-marker { display: none; }
   .repo-tree summary::before {
     content: "\25B6";
     font-size: 0.65em;
     color: #007C8C;
     flex-shrink: 0;
     transition: transform 0.15s;
     margin-right: 0.3em;
   }
   .repo-tree details[open] > summary::before { transform: rotate(90deg); }
   .rt-root {
     padding: 0.4rem 0.8rem;
     background: #115566;
     color: #ffffff;
     font-weight: 700;
     font-size: 0.88rem;
     letter-spacing: 0.03em;
     border-radius: 3px;
     margin-bottom: 0.3em;
   }
   .rt-l1 { padding-left: 1.2rem; }
   .rt-l2 { padding-left: 2.4rem; }
   details.rt-folder > summary {
     font-weight: 600;
     color: #0D3040;
     cursor: pointer;
     display: flex;
     align-items: baseline;
     gap: 0.3em;
     padding: 0.15em 0;
   }
   details.rt-folder > summary .rt-name { color: #007C8C; }
   details.rt-folder > summary .rt-desc {
     color: #547880;
     font-weight: 400;
     font-size: 0.8rem;
   }
   .rt-file {
     display: flex;
     align-items: baseline;
     gap: 0.4em;
     padding: 0.15em 0.5em;
     border-radius: 3px;
   }
   .rt-file:hover { background: #EAF4F6; }
   .rt-file .rt-icon { color: #547880; font-size: 0.85em; }
   .rt-file .rt-name { color: #0D3040; }
   .rt-file .rt-desc { color: #547880; font-size: 0.79rem; margin-left: 0.2em; }
   details.rt-folder[open] > .rt-children {
     border-left: 1px solid #E0E0E0;
     margin-left: 1.4rem;
     padding-left: 0.5rem;
   }
   </style>

   <div class="repo-tree">
   <div class="rt-root">mathlib/</div>

   <!-- top-level leaf files -->
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">CMakeLists.txt</span><span class="rt-desc">Top-level CMake entry point</span></div>
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">CMakePresets.json</span><span class="rt-desc">Shared, tracked build presets</span></div>
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">README.md</span><span class="rt-desc">Overview, features, kernel list</span></div>
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">LICENSE</span><span class="rt-desc">Apache-2.0 license text</span></div>
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">CONTRIBUTING.md</span><span class="rt-desc">Contribution policy</span></div>
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">makefile.am62dx.c75x.ti-c7000</span><span class="rt-desc">Standalone makefile (AM62D / C7504)</span></div>
   <div class="rt-file rt-l1"><span class="rt-icon">&#128196;</span><span class="rt-name">makefile.am275x.c75x.ti-c7000</span><span class="rt-desc">Standalone makefile (AM275 / C7524)</span></div>

   <!-- cmake/ (leaf folder, no children shown) -->
   <div class="rt-file rt-l1"><span class="rt-icon">&#128193;</span><span class="rt-name">cmake/</span><span class="rt-desc">CMake helper modules (toolchains, docs, tests)</span></div>

   <!-- src/ -->
   <details class="rt-folder rt-l1" open>
     <summary><span class="rt-name">src/</span> <span class="rt-desc">Kernel implementations &#8212; one folder per kernel</span></summary>
     <div class="rt-children">
       <div class="rt-file rt-l2"><span class="rt-icon">&#128193;</span><span class="rt-name">common/</span><span class="rt-desc">Shared types, lookup tables, utility headers</span></div>
       <div class="rt-file rt-l2"><span class="rt-icon">&#128193;</span><span class="rt-name">MATHLIB_&lt;kernel&gt;/</span><span class="rt-desc">Per-kernel sources + public header</span></div>
     </div>
   </details>

   <!-- test/ -->
   <details class="rt-folder rt-l1" open>
     <summary><span class="rt-name">test/</span> <span class="rt-desc">Test drivers and generated parameter files</span></summary>
     <div class="rt-children">
       <div class="rt-file rt-l2"><span class="rt-icon">&#128193;</span><span class="rt-name">MATHLIB_testParams_gen/</span><span class="rt-desc">Python test-parameter generators</span></div>
       <div class="rt-file rt-l2"><span class="rt-icon">&#128193;</span><span class="rt-name">MATHLIB_&lt;kernel&gt;/</span><span class="rt-desc">Per-kernel test driver + parameters</span></div>
     </div>
   </details>

   <!-- examples/ (leaf folder, no children shown) -->
   <div class="rt-file rt-l1"><span class="rt-icon">&#128193;</span><span class="rt-name">examples/</span><span class="rt-desc">Minimal usage examples</span></div>

   <!-- docs/ -->
   <details class="rt-folder rt-l1" open>
     <summary><span class="rt-name">docs/</span> <span class="rt-desc">Documentation</span></summary>
     <div class="rt-children">
       <div class="rt-file rt-l2"><span class="rt-icon">&#128193;</span><span class="rt-name">doxygen/</span><span class="rt-desc">Doxyfiles, content pages, performance HTML</span></div>
       <div class="rt-file rt-l2"><span class="rt-icon">&#128193;</span><span class="rt-name">sphinx/</span><span class="rt-desc">Sphinx site (conf.py, rst, TI theme)</span></div>
     </div>
   </details>

   </div>

Kernel sources — ``src/MATHLIB_<kernel>/``
------------------------------------------

Each kernel lives in its own folder. The public header carries the Doxygen
documentation that this site renders.

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - File
     - Contents
   * - ``MATHLIB_<kernel>.h``
     - Public API — C++ template + C interface prototypes and Doxygen documentation
   * - ``MATHLIB_<kernel>.cpp``
     - C7™ SE/SA optimized implementation
   * - ``MATHLIB_<kernel>_scalar.h``
     - Scalar reference implementation helper — not part of the public API
   * - ``CMakeLists.txt``
     - Build rules for the kernel

``src/common/`` holds cross-kernel definitions: ``MATHLIB_types.h`` (status
codes, shared type definitions), ``MATHLIB_lut.h`` / ``MATHLIB_ilut.h``
(lookup tables), ``MATHLIB_scalarTables.h``, ``MATHLIB_permute.h``,
``MATHLIB_debugPrint.h``, and ``MATHLIB_utility.h``.

Tests — ``test/MATHLIB_<kernel>/``
-----------------------------------

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - File
     - Contents
   * - ``MATHLIB_<kernel>_d.cpp``
     - Test driver (entry point) for the kernel
   * - ``MATHLIB_<kernel>_testParams.cpp``
     - Generated test parameters (see :doc:`getting_started`)

Test-parameter generators — ``test/MATHLIB_testParams_gen/<kernel>/``
----------------------------------------------------------------------

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - File
     - Contents
   * - ``gen_data.py``
     - Generates ``_testParams.cpp`` for the kernel
   * - ``test_cases_list.csv``
     - Parametric test-case definitions (sizes, types, options)

Other folders
-------------

- ``cmake/`` — build-system modules: toolchain files (``c7x.cmake``, ``gcc.cmake``),
  documentation (``doxygen.cmake``), and test/vector generation
  (``test.cmake``, ``gen_test_cases.cmake``).
- ``docs/doxygen/`` — Doxygen configuration, narrative content pages, and the
  prebuilt performance HTML tables.
- ``docs/sphinx/`` — this documentation site (``conf.py``, ``.rst`` sources, and
  the TI-branded theme assets under ``_static/``).
