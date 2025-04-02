# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Mon Oct 16 09:12:31 PDT 2017
#   I modified the file to no longer use BSD style pseudo terminals on
#   MacOSX, since it doesn't work on newer Mac operating sytems.
#
#   Kathleen Biagas, Thu Mar 21, 2025 
#   Removed AIX logic.
#
#****************************************************************************/

if(NOT WIN32)
    try_compile(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/TestPTY.cpp
            OUTPUT_VARIABLE outvar)
    if(tmpVar)
        MESSAGE(STATUS "System V style pseudo-tty")
        SET(PTY_SYSV 1 CACHE BOOL "Defined if we have System V style pty functions")
    else()
        message(STATUS "BSD style pseudo-tty")
    endif()
endif()
