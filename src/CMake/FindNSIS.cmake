# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

SET(VISIT_NSIS_SEARCHPATH)

IF (WIN32)
    SET(PF1 "PROGRAMFILES")
    SET(PF2 "PROGRAMFILES(X86)")
    SET(VISIT_NSIS_SEARCHPATH "$ENV{${PF1}}/NSIS" 
                              "$ENV{${PF2}}/NSIS")
    UNSET(PF1)
    UNSET(PF2)
ENDIF (WIN32)

FIND_PROGRAM(MAKENSIS
    makensis
    PATHS ${VISIT_NSIS_SEARCHPATH}
    DOC "Location of the NSIS executable"
)

STRING(COMPARE NOTEQUAL ${MAKENSIS} "MAKENSIS-NOTFOUND" MAKENSIS_FOUND)

