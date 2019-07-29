# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

IF(APPLE)
    #
    #convert all libraries using osxfixup
    #
    INSTALL(CODE
            "
            MESSAGE(STATUS \"Running osxfixup on libraries and executables\")
            EXECUTE_PROCESS(WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}
             COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/osxfixup/osxfixup.py \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION}
             OUTPUT_VARIABLE OSXOUT)
             MESSAGE(STATUS \"\${OSXOUT}\")")
ENDIF(APPLE)
