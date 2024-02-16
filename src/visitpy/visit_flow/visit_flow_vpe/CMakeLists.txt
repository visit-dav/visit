# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Use new cmake commands for pip setup.
#
#****************************************************************************

PYTHON_ADD_PIP_SETUP(visit_flow_vpe_py_setup
                     site-packages
                     setup.py
                     src/__init__.py
                     src/visit_flow_exec.vpe
                     src/visit_flow_vpe.py)





