# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

if(VISIT_PYTHON_SCRIPTING AND NOT VISIT_STATIC)
    add_subdirectory(python)
endif()
