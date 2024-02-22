# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Use new cmake commands for pip setup.
#
#****************************************************************************


PYTHON_ADD_PIP_SETUP(NAME visit_flow_vpe_py_setup
                     DEST_DIR lib/site-packages
                     PY_MODULE_DIR visit_flow_vpe
                     PY_SETUP_FILE setup.py
                     PY_SOURCES  src/__init__.py
                                 src/visit_flow_exec.vpe
                                 src/visit_flow_vpe.py)



