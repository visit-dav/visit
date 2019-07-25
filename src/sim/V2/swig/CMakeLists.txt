# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

IF(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
    ADD_SUBDIRECTORY(python)
ENDIF(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
