# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Mon Oct 16 09:12:31 PDT 2017
#   I modified the file to no longer use BSD style pseudo terminals on
#   MacOSX, since it doesn't work on newer Mac operating sytems.
#
#****************************************************************************/

IF(NOT WIN32)
    IF(CMAKE_SYSTEM_NAME STREQUAL AIX)
        MESSAGE(STATUS "BSD style pseudo-tty")
    ELSE(CMAKE_SYSTEM_NAME STREQUAL AIX)
        TRY_COMPILE(tmpVar
                ${CMAKE_CURRENT_BINARY_DIR}
                ${VISIT_SOURCE_DIR}/CMake/TestPTY.cpp
                OUTPUT_VARIABLE outvar)
        IF(tmpVar)
            MESSAGE(STATUS "System V style pseudo-tty")
            SET(PTY_SYSV 1 CACHE BOOL "Defined if we have System V style pty functions")
        ELSE(tmpVar)
            MESSAGE(STATUS "BSD style pseudo-tty")
        ENDIF(tmpVar)
    ENDIF(CMAKE_SYSTEM_NAME STREQUAL AIX)
ENDIF(NOT WIN32)
