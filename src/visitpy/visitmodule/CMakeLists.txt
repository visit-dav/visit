# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Tue May  1 17:02:34 PDT 2012
#   Add writescript.py
#
#   Cyrus Harrison, Wed Feb 24 10:12:20 PST 2021
#   Moved PySide logic into visit_utils.builtin
#
#   Kathleen Biagas, Wed Feb 28, 2024
#   Changed PY_MODULE_DIR from visit_flow_vpe to visit.
#
#   Kathleen Biagas, Tue Mar 5, 2024
#   For PYTHON_ADD_PIP_SETUP change DEST_DIR from lib/site-packages to
#   simply site-packages. The lib part handled in PYTHON_ADD_PIP_SETUP.
#
#****************************************************************************

PYTHON_ADD_PIP_SETUP(NAME visitmodule_py_setup
                     DEST_DIR site-packages
                     PY_MODULE_DIR visit
                     PY_SETUP_FILE setup.py
                     PY_SOURCES  py_src/__init__.py
                                 py_src/frontend.py)




