# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Use new cmake commands for pip setup.
#
#  Kathleen Biagas, Tue Mar 5, 2024
#  For PYTHON_ADD_PIP_SETUP change DEST_DIR from lib/site-packages to
#  simply site-packages. The lib part handled in PYTHON_ADD_PIP_SETUP.
#
#****************************************************************************


PYTHON_ADD_PIP_SETUP(NAME visit_flow_vpe_py_setup
                     DEST_DIR site-packages
                     PY_MODULE_DIR visit_flow_vpe
                     PY_SETUP_FILE setup.py
                     PY_SOURCES  src/__init__.py
                                 src/visit_flow_exec.vpe
                                 src/visit_flow_vpe.py)



