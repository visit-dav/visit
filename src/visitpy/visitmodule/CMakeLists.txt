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
#****************************************************************************

PYTHON_ADD_PIP_SETUP(visitmodule_py_setup
                     site-packages
                     setup.py
                     py_src/__init__.py
                     py_src/frontend.py
                    )





