# Configuration file for the Sphinx documentation builder.
#
# This file is part of the MATHLIB documentation build.
#
# Theme and CSS override approach (html_theme + theme_overrides.css) are
# deliberately mirrored from TexasInstruments/processor-sdk-doc:
#   html_theme = 'sphinx_rtd_theme'
#   html_logo  pointing at _static/img/ti_logo.svg
#   html_show_sphinx = False
#   def setup(app): app.add_css_file("css/theme_overrides.css")
#
# All DEVFAMILY / OS / tag-loader machinery present in processor-sdk-doc has
# been omitted — MATHLIB is a single-target C library and does not need it.
#
# Doxygen XML is produced by the CMake build (not by Exhale).  Pass the
# output directory via the MATHLIB_DOXYGEN_XML environment variable before
# invoking sphinx-build, e.g.:
#   export MATHLIB_DOXYGEN_XML=/path/to/build/docs/xml
#   sphinx-build -b html docs/sphinx docs/sphinx/_build/html

import os
import sys

# ---------------------------------------------------------------------------
# Project information
# ---------------------------------------------------------------------------

project   = "MATHLIB"
author    = "Texas Instruments Incorporated"
copyright = "2026, Texas Instruments Incorporated"

# Show the source branch under the logo so the rendered docs correlate with
# the code they were generated from. Priority: explicit env override
# (MATHLIB_DOC_REF, set by CI) -> current git branch -> "main".
import subprocess


def _doc_ref():
    ref = os.environ.get("MATHLIB_DOC_REF")
    if ref:
        return ref.strip()
    try:
        out = subprocess.run(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"],
            cwd=os.path.dirname(os.path.abspath(__file__)),
            capture_output=True, text=True, timeout=5)
        branch = out.stdout.strip()
        if branch and branch != "HEAD":
            return branch
    except Exception:
        pass
    return "main"


version = _doc_ref()
release = version

# ---------------------------------------------------------------------------
# Doxygen XML directory
# ---------------------------------------------------------------------------

_fallback_xml_dir = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "build", "docs", "xml")
)

xml_dir = os.environ.get("MATHLIB_DOXYGEN_XML", _fallback_xml_dir)

if not os.path.isdir(xml_dir):
    import warnings
    warnings.warn(
        f"[conf.py] Doxygen XML directory not found: {xml_dir!r}\n"
        "  Set MATHLIB_DOXYGEN_XML to the path produced by the CMake 'doxygen' target, "
        "or run CMake first to generate the XML before invoking sphinx-build.",
        stacklevel=2,
    )

# ---------------------------------------------------------------------------
# Extensions
# ---------------------------------------------------------------------------

extensions = [
    "breathe",
    "sphinx.ext.mathjax",
    "sphinx_design",
    "sphinx_copybutton",
    "sphinx.ext.graphviz",
    "sphinx_immaterial",
]

# ---------------------------------------------------------------------------
# Source / master document
# ---------------------------------------------------------------------------

master_doc = "index"

# ---------------------------------------------------------------------------
# Language settings
# ---------------------------------------------------------------------------
# MATHLIB is a C++ template library — each kernel is a template<typename T>
# function. Using primary_domain="c" causes Breathe to try to parse C++
# template declarations in the C parser, which triggers a warning under -W
# for every kernel group. Switch to "cpp" so template<typename T> signatures
# parse correctly while C-style structs and enums still render fine.

primary_domain     = "cpp"
highlight_language = "cpp"

# ---------------------------------------------------------------------------
# Breathe configuration
# ---------------------------------------------------------------------------

breathe_projects        = {"mathlib": xml_dir}
breathe_default_project = "mathlib"
# MATHLIB exposes C++ template functions plus plain C (_sp / _dp) interfaces.
# Let Breathe auto-detect the domain from the XML rather than forcing "c",
# since template<typename T> function declarations require C++ parsing.
breathe_domain_by_extension = {"h": "cpp", "c": "c"}

# ---------------------------------------------------------------------------
# API reference generation (per-kernel Doxygen groups)
# ---------------------------------------------------------------------------
# MATHLIB organizes each kernel as a Doxygen group (\defgroup / \addtogroup).
# The kernel OVERVIEW — including the LaTeX formulas embedded in the header
# comments — lives in the group's detailed description, which Exhale's
# file/function tree does not surface. Instead we emit one page per group
# using Breathe's ``.. doxygengroup::`` directive (which renders the overview
# prose + formulas + all member functions). This auto-scales to every kernel
# and is reused unchanged across the sibling libraries (dsplib, fftlib, ...).

import glob
import xml.etree.ElementTree as ET

_API_DIR = os.path.join(os.path.dirname(__file__), "api")

# Groups rendered without ``:members:`` because their header has an unbalanced
# Doxygen group (``@{`` with no ``@}``), which duplicates a member under ``-W``.
# Remove entries here once the corresponding header is fixed on Bitbucket dev.
_NO_MEMBERS_GROUPS = set()

# Ordered API grouping for the index page.  Each entry is
# (category_title, [group_stem, ...]) matching the README kernel taxonomy.
# Any group discovered in the XML but NOT listed here is emitted under a
# final "Other" toctree and triggers a build warning.
_API_GROUP_ORDER = [
    ("Common", [
        "MATHLIB_COMMON",
        "MATHLIB_KERNELS",
        "MATHLIB_TYPES",
    ]),
    ("Trigonometric", [
        "MATHLIB_SIN",
        "MATHLIB_COS",
        "MATHLIB_TAN",
        "MATHLIB_ASIN",
        "MATHLIB_ACOS",
        "MATHLIB_ATAN",
        "MATHLIB_ATAN2",
    ]),
    ("Hyperbolic", [
        "MATHLIB_SINH",
        "MATHLIB_COSH",
        "MATHLIB_TANH",
        "MATHLIB_ASINH",
        "MATHLIB_ACOSH",
        "MATHLIB_ATANH",
    ]),
    ("Exponential & Logarithmic", [
        "MATHLIB_EXP",
        "MATHLIB_EXP2",
        "MATHLIB_EXP10",
        "MATHLIB_LOG",
        "MATHLIB_LOG2",
        "MATHLIB_LOG10",
        "MATHLIB_POW",
    ]),
    ("Arithmetic", [
        "MATHLIB_SQRT",
        "MATHLIB_RSQRT",
        "MATHLIB_RECIP",
        "MATHLIB_DIV",
    ]),
]


def _caption_to_slug(caption):
    """Convert a caption like 'Exponential & Logarithmic' to a slug like 'exponential___logarithmic'."""
    import re
    return re.sub(r"[^a-z0-9]+", "_", caption.lower()).strip("_")


def _generate_api_pages(xml_directory, api_directory):
    """Emit api/index.rst + per-page RST files for all Doxygen groups.

    Layout produced (sphinx-immaterial landing-page-per-caption pattern):
    - One api/<group>.rst per kernel group (``.. doxygengroup:: <name>``).
    - One landing page per _API_GROUP_ORDER caption: ``api/<slug>.rst``
      (e.g. ``api/trigonometric.rst``) with a toctree of that category's stems.
    - ``api/index.rst``: single toctree (no :caption:) listing all landing pages
      so sphinx-immaterial renders each landing page as a real nav section.

    This mirrors the TISP approach: landing pages give sphinx-immaterial the
    structural anchors it needs to render the nav sections cleanly.
    """
    if not os.path.isdir(xml_directory):
        return
    os.makedirs(api_directory, exist_ok=True)

    groups = []  # list of (group_name, title)
    for path in sorted(glob.glob(os.path.join(xml_directory, "group__*.xml"))):
        try:
            root = ET.parse(path).getroot()
        except ET.ParseError:
            continue
        compound = root.find("./compounddef[@kind='group']")
        if compound is None:
            continue
        name_el = compound.find("compoundname")
        if name_el is None or not name_el.text:
            continue
        name = name_el.text.strip()
        title_el = compound.find("title")
        title = (title_el.text.strip() if title_el is not None and title_el.text
                 else name)
        groups.append((name, title))

    # One page per kernel group. ``:members:`` expands inner structs (e.g.
    # <kernel>_InitArgs) so their fields render, not just the struct name.
    #
    # Exception: a group whose header has an unclosed Doxygen group (``@{``
    # without a matching ``@}``) double-attributes a member, which trips a
    # duplicate-ID error under ``-W`` when ``:members:`` is used. Such groups
    # render without ``:members:`` until the header is fixed.
    discovered = {name for name, _title in groups}
    title_map = dict(groups)

    for name, title in groups:
        underline = "=" * max(len(title), 3)
        with open(os.path.join(api_directory, f"{name}.rst"), "w",
                  encoding="utf-8") as fh:
            fh.write(f"{title}\n{underline}\n\n")
            opts = "" if name in _NO_MEMBERS_GROUPS else "\n   :members:"
            fh.write(f".. doxygengroup:: {name}{opts}\n")

    # Build a lookup for quick membership testing.
    grouped_stems = {stem for _cat, stems in _API_GROUP_ORDER for stem in stems}

    # Ungrouped pages (new kernels not yet in _API_GROUP_ORDER).
    ungrouped = sorted(discovered - grouped_stems)
    if ungrouped:
        print(f"WARNING: ungrouped API page(s): {', '.join(ungrouped)}")

    # -----------------------------------------------------------------------
    # Emit one landing page per _API_GROUP_ORDER caption.
    # landing_index_entries: ordered list of page slugs for api/index toctree.
    # -----------------------------------------------------------------------
    landing_index_entries = []
    all_emitted = set(discovered)  # per-group pages already written above

    for category, stems in _API_GROUP_ORDER:
        # Emit only stems that were actually discovered in the XML.
        present = [s for s in stems if s in discovered]
        if not present:
            continue

        slug = _caption_to_slug(category)
        landing_file = os.path.join(api_directory, f"{slug}.rst")
        underline = "=" * max(len(category), 3)
        with open(landing_file, "w", encoding="utf-8") as fh:
            fh.write(f"{category}\n{underline}\n\n")
            fh.write(
                f".. toctree::\n"
                f"   :maxdepth: 1\n\n"
            )
            for stem in present:
                fh.write(f"   {stem}\n")
            fh.write("\n")
        landing_index_entries.append(slug)
        all_emitted.add(slug)

    if ungrouped:
        slug = "other"
        landing_file = os.path.join(api_directory, f"{slug}.rst")
        with open(landing_file, "w", encoding="utf-8") as fh:
            fh.write("Other\n=====\n\n")
            fh.write(".. toctree::\n   :maxdepth: 1\n\n")
            for stem in ungrouped:
                fh.write(f"   {stem}\n")
            fh.write("\n")
        landing_index_entries.append(slug)
        all_emitted.add(slug)

    # Remove stale RST files left from a previous build.
    all_emitted.add("index")
    for stale in glob.glob(os.path.join(api_directory, "*.rst")):
        stem = os.path.splitext(os.path.basename(stale))[0]
        if stem not in all_emitted:
            os.remove(stale)

    # -----------------------------------------------------------------------
    # api/index.rst: single toctree listing all landing pages in order.
    # -----------------------------------------------------------------------
    with open(os.path.join(api_directory, "index.rst"), "w",
              encoding="utf-8") as fh:
        fh.write(":icon: material/book-open-variant\n\n")
        fh.write("API Reference\n=============\n\n")
        fh.write("Per-kernel API documentation generated from the source "
                 "headers.\n\n")
        fh.write(
            ".. toctree::\n"
            "   :maxdepth: 2\n\n"
        )
        for entry in landing_index_entries:
            fh.write(f"   {entry}\n")
        fh.write("\n")


_generate_api_pages(xml_dir, _API_DIR)


# ---------------------------------------------------------------------------
# Performance pages (embed the prebuilt perf/datasheet HTML tables)
# ---------------------------------------------------------------------------
# The legacy Doxygen docs inlined per-device performance tables via
# ``\htmlinclude ./html/<file>.html`` inside content_pages/performance_summary.h,
# split into per-device blocks (#if __C7100__ / __C7120__ / __C7504__ / __C7524__).
# The referenced files under docs/doxygen/html/ are self-contained <table>
# fragments. We reproduce that here: one page per device, each table
# embedded via a ``.. raw:: html`` block, so the numbers render exactly as before.
#
# MATHLIB uses \section (not \subsection) in performance_summary.h, so the
# parser matches \section lines for entry titles.

_PERF_DIR = os.path.join(os.path.dirname(__file__), "perf")
_PERF_SUMMARY = os.path.join(
    os.path.dirname(__file__), "..", "doxygen", "content_pages",
    "performance_summary.h")
_PERF_HTML_DIR = os.path.join(os.path.dirname(__file__), "..", "doxygen", "html")

# Doxygen device macro -> (slug, human title)
# Only C7504 (AM62D) and C7524 (AM275) numbers are published for now.
_PERF_DEVICES = [
    ("__C7504__", "c7504", "C7504 (AM62D)"),
    ("__C7524__", "c7524", "C7524 (AM275)"),
]


def _parse_perf_summary(summary_path):
    """Return {device_macro: [(table_title, html_basename), ...]} from the .h.

    MATHLIB uses ``\\section`` (not ``\\subsection``) for the two per-device
    table entries: "MATHLIB kernels" and "MATHLIB double precision kernels".
    """
    import re
    result = {macro: [] for macro, _slug, _title in _PERF_DEVICES}
    if not os.path.isfile(summary_path):
        return result
    current = None
    title = None
    with open(summary_path, encoding="utf-8", errors="ignore") as fh:
        for line in fh:
            s = line.strip()
            m = re.match(r"#(?:el)?if\s+defined\((__C7\d+__)\)", s)
            if m:
                # Skip device blocks we do not publish (current -> None).
                current = m.group(1) if m.group(1) in result else None
                title = None
                continue
            if s.startswith("#endif"):
                current = None
                continue
            if current is None:
                continue
            # Match \section <tag> <title text>
            m = re.match(r"\\section\s+\S+\s+(.*)", s)
            if m:
                title = m.group(1).strip()
                continue
            # Also accept \subsection for future compatibility
            m = re.match(r"\\subsection\s+\S+\s+(.*)", s)
            if m:
                title = m.group(1).strip()
                continue
            # Use search (not match): some entries put the \htmlinclude on the
            # same line as a trailing \f$...\f$ formula, so it is not at column 0.
            m = re.search(r"\\htmlinclude\s+\S*?/([^/\s]+\.html)", s)
            if m:
                result[current].append((title or m.group(1), m.group(1)))
                title = None
    return result


def _generate_perf_pages(summary_path, html_dir, perf_dir):
    """Emit perf/index.rst + one perf/<device>.rst per device with embedded tables."""
    data = _parse_perf_summary(summary_path)
    if not any(data.values()):
        return
    os.makedirs(perf_dir, exist_ok=True)

    written = []  # (slug, title) for pages that actually have tables
    for macro, slug, title in _PERF_DEVICES:
        entries = data.get(macro, [])
        # keep only entries whose table file exists on disk
        entries = [(t, f) for (t, f) in entries
                   if os.path.isfile(os.path.join(html_dir, f))]
        if not entries:
            continue
        underline = "=" * len(title)
        with open(os.path.join(perf_dir, f"{slug}.rst"), "w",
                  encoding="utf-8") as fh:
            fh.write(f"{title}\n{underline}\n\n")
            for ktitle, fname in entries:
                kunder = "-" * max(len(ktitle), 3)
                fh.write(f"{ktitle}\n{kunder}\n\n")
                fh.write(".. raw:: html\n\n")
                with open(os.path.join(html_dir, fname), encoding="utf-8",
                          errors="ignore") as tbl:
                    for tline in tbl.read().splitlines():
                        fh.write(f"   {tline}\n")
                fh.write("\n")
        written.append((slug, title))

    with open(os.path.join(perf_dir, "index.rst"), "w",
              encoding="utf-8") as fh:
        fh.write(":icon: material/chart-line\n\n")
        fh.write("Performance\n===========\n\n")
        fh.write("Measured kernel performance per device. EVM warm cycles are "
                 "obtained by profiling the kernel's compute code after a cold "
                 "run. See each kernel's API page for parameter definitions.\n\n")
        fh.write(".. toctree::\n   :maxdepth: 1\n\n")
        for slug, _title in written:
            fh.write(f"   {slug}\n")


_generate_perf_pages(_PERF_SUMMARY, _PERF_HTML_DIR, _PERF_DIR)

# ---------------------------------------------------------------------------
# MathJax (version 3 is the Sphinx default; AMS extensions load automatically)
# ---------------------------------------------------------------------------
# No custom macros needed for MATHLIB — rely on MathJax 3 defaults.

# ---------------------------------------------------------------------------
# HTML output
# ---------------------------------------------------------------------------

html_theme        = "sphinx_immaterial"
html_title        = "MATHLIB Documentation"
html_static_path  = ["_static"]
html_logo         = "_static/img/ti_logo_mono.svg"

html_show_sphinx      = False
html_show_sourcelink  = False
html_copy_source      = False
html_last_updated_fmt = "%b %d, %Y"

pygments_style = "friendly"
# No maximum_signature_line_length: let signatures render single-line in the
# gray sig block (natural wrap) — the clean look. Forcing multi-line adds a
# per-line break + trailing comma that reads clumsily for long signatures.

html_theme_options = {
    # Palette: calm single light scheme with TI-adjacent neutral tones.
    # No dark-mode toggle; user can tune later.
    "palette": [
        {
            "scheme": "default",
            "primary": "red",
            "accent":  "teal",
        },
    ],
    # Features: clean navigation, no tabs (too much chrome).
    "features": [
        "navigation.top",   # back-to-top button
        "search.highlight", # highlight search terms after navigation
        "toc.follow",       # TOC sidebar tracks scroll position
    ],
    # Source repo shown in the header: GitHub logo + repo name.
    # The branch label ("main") is added via the source.html template
    # override (see _templates/partials/source.html).
    "repo_url":  "https://github.com/TexasInstruments/c7x_mathlib",
    "repo_name": "c7x_mathlib",
    "icon": {"repo": "material/github"},
}

# Template overrides (partials/source.html adds the branch badge).
templates_path = ["_templates"]

# ---------------------------------------------------------------------------
# Theme customisation hook
#
# theme_overrides.css targets .wy-* RTD classes that don't exist in
# sphinx-immaterial — load it anyway (harmless, no matching selectors).
# Add a second CSS file targeting sphinx-immaterial's .md-* selectors for:
#   - WIP amber banner via .md-content__inner::before
#   - C signature token colouring (same palette as RTD version, new selectors)
# ---------------------------------------------------------------------------

def setup(app):
    # Original RTD overrides (harmless, no matching selectors in immaterial).
    app.add_css_file("css/theme_overrides.css")
    # Immaterial-specific overrides: WIP banner + sig colouring.
    app.add_css_file("css/immaterial_overrides.css")
