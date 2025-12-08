#!/bin/env python
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.
"""
VisIt's test suite shim.
"""
import os
os.environ['VISIT_VLD_ELEMENT_PARTITIONING'] = '1'

from visit_testing import run_main

if __name__ == "__main__":
    run_main()

