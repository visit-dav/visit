# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

PYTHON_ADD_DISTUTILS_SETUP(visit_utils_py_setup
                           site-packages
                           setup.py
                           src/__init__.py
                           # core mods
                           src/common.py
                           src/encoding.py
                           src/engine.py
                           src/exprs.py
                           src/host_profile.py
                           src/moab.py
                           src/property_tree.py
                           src/query.py
                           src/slurm.py
                           src/status.py
                           src/ult.py
                           src/windows.py
                           # builtin module
                           src/builtin/__init__.py
                           src/builtin/convert2to3.py
                           src/builtin/evalfuncs.py
                           src/builtin/flatten.py
                           src/builtin/writescript.py
                           # qannote module
                           src/qannote/__init__.py
                           src/qannote/canvas.py
                           src/qannote/items.py
                           src/qannote/xinit.py
                           # qplot module
                           src/qplot/__init__.py
                           src/qplot/plots.py
                           src/qplot/scene.py
                       )

# gen scripts that help us test the visit_utils module
set(_VISIT_UTILS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set(_VISIT_UTILS_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} )

set(_VISIT_EXECUTABLE ${PROJECT_BINARY_DIR}/bin/visit)

# NOTE: These still run in source  ... 

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/run_python_tests.sh.in"
                "${CMAKE_CURRENT_BINARY_DIR}/run_python_tests.sh" @ONLY)

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/run_visit_tests.sh.in"
                "${CMAKE_CURRENT_BINARY_DIR}/run_visit_tests.sh" @ONLY)

