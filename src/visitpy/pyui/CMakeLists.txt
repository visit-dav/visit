# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

IF(HAVE_LIBPYSIDE OR HAVE_LIBPYQT)
    ADD_SUBDIRECTORY(common)
ENDIF()

IF(HAVE_LIBPYSIDE)
    ADD_SUBDIRECTORY(pyside)
ENDIF()

IF(HAVE_LIBPYQT AND VISIT_SIP_DIR)
    ADD_SUBDIRECTORY(pyqt)
ENDIF(HAVE_LIBPYQT AND VISIT_SIP_DIR)
