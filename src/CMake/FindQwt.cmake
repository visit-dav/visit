# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kevin Griffin, Thu Jan  4 12:45:28 PST 2018
#   Changed the linked directory lib/qwt.framework/Headers to the actual path
#   it was linked to. This fixes the make install symlink error.
#
#   Kathleen Biagas, Thu Feb  8 08:30:19 PST 2018
#   Set QWT_LIBRARY to full path, for use in target_link_libraries.
#
#*****************************************************************************

if(NOT EXISTS ${VISIT_QWT_DIR})
    message(FATAL_ERROR "Qwt installation directory is not specified or does not exist")
endif()

if(APPLE)
    if(VISIT_STATIC)
        SET_UP_THIRD_PARTY(QWT LIBS qwt)
    else()
        SET_UP_THIRD_PARTY(QWT
            INCDIR lib/qwt.framework/Versions/Current/Headers
            LIBS qwt)
    endif()
else()
    SET_UP_THIRD_PARTY(QWT LIBS qwt)
endif()

if(NOT QWT_FOUND)
    message(FATAL_ERROR "Qwt installation could not be used.")
endif()

SET(QWT_LIBRARY ${QWT_LIBRARY_DIR}/${QWT_LIB} CACHE FILEPATH "full path to qwt library" FORCE)

