# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

PYTHON_ADD_PIP_SETUP(NAME visit_module_py_setup
                     DEST_DIR site-packages
                     PY_MODULE_DIR visit
                     PY_SETUP_FILE setup.py
                     PY_SOURCES  py_src/__init__.py)




