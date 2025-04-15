# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

if(VISIT_HAS_PYSIDE OR VISIT_HAS_PYQT)
    add_subdirectory(common)
endif()

if(VISIT_HAS_PYSIDE)
    add_subdirectory(pyside)
endif()

if(VISIT_HAS_PYQT AND VISIT_SIP_DIR)
    add_subdirectory(pyqt)
endif()
