# Copyright (C) 2026 Texas Instruments Incorporated
#
# SPDX-License-Identifier: Apache-2.0

"""
File I/O utilities for MATHLIB_recip test data generation
"""

import numpy as np
import os


def write_header_file_single_input(input_data, output_data, testCase, headerFileName):
    """Write input and output data to a C header file.

    Args:
        input_data: numpy array of input values
        output_data: numpy array of output values (reciprocals)
        testCase: dict with test case parameters
        headerFileName: output header file name
    """

    testId = testCase["ID"]
    length = testCase["length"]
    dTypeIn = testCase["dTypeIn"]

    # Create test_data directory if it doesn't exist
    output_dir = "../../MATHLIB_recip/test_data"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    filepath = os.path.join(output_dir, headerFileName)

    with open(filepath, "w") as f:
        # Write header guard and includes
        f.write("/* Auto-generated test data for MATHLIB_recip */\n")
        f.write("/* Test case ID: %s, length: %d */\n\n" % (testId, length))

        guard_name = "STATIC_REF_CASE%s_H" % testId
        f.write("#ifndef %s\n" % guard_name)
        f.write("#define %s\n\n" % guard_name)

        # Determine C type
        if dTypeIn == "MATHLIB_FLOAT32":
            c_type = "float"
            fmt_str = "%.8ef"
        else:
            c_type = "double"
            fmt_str = "%.16e"

        # Write input array
        f.write("static %s staticRefInCase%s[%d] = {\n" % (c_type, testId, length))
        for i, val in enumerate(input_data):
            if i < len(input_data) - 1:
                f.write("    %s,\n" % (fmt_str % val))
            else:
                f.write("    %s\n" % (fmt_str % val))
        f.write("};\n\n")

        # Write output array
        f.write("static %s staticRefOutCase%s[%d] = {\n" % (c_type, testId, length))
        for i, val in enumerate(output_data):
            if i < len(output_data) - 1:
                f.write("    %s,\n" % (fmt_str % val))
            else:
                f.write("    %s\n" % (fmt_str % val))
        f.write("};\n\n")

        f.write("#endif /* %s */\n" % guard_name)

    print("Generated header file: %s" % filepath)
