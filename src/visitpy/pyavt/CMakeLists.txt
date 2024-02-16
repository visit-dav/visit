# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Cyrus Harrison, Wed Mar 10 11:05:26 PST 2010
#  Use CMAKE_LIBRARY_OUTPUT_DIRECTORY instead of LIBRARY_OUTPUT_DIRECTORY.
#
#  Cyrus Harrison, Wed Apr 11 14:21:57 PDT 2012
#  Use new cmake commands for distutils setup.
#
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Use new cmake commands for pip setup.
#
#****************************************************************************

# deps for pyavt
SET(pyavt_sources py_src/__init__.py
                  py_src/filters.py
                  py_src/templates/advanced_expression.py
                  py_src/templates/advanced_query.py
                  py_src/templates/simple_expression.py
                  py_src/templates/simple_query.py)

PYTHON_ADD_PIP_SETUP(pyavt_py_setup
                     site-packages
                     setup.py
                     ${pyavt_sources})



