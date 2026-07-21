# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

set(VISIT_NSIS_SEARCHPATH)

if (WIN32)
    set(PF1 "PROGRAMFILES")
    set(PF2 "PROGRAMFILES(X86)")
    set(VISIT_NSIS_SEARCHPATH "$ENV{${PF1}}/NSIS"
                              "$ENV{${PF2}}/NSIS")
    set(PF1)
    set(PF2)
endif()

find_program(MAKENSIS
    makensis
    PATHS ${VISIT_NSIS_SEARCHPATH}
    DOC "Location of the NSIS executable")

string(COMPARE NOTEQUAL ${MAKENSIS} "MAKENSIS-NOTFOUND" MAKENSIS_FOUND)

