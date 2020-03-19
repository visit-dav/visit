# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${SILO_INCLUDE_DIR}
)

# Add link directories
LINK_DIRECTORIES(${SILO_LIBRARY_DIR})

#********************************* SERIAL ************************************
ADD_EXECUTABLE(mrgtree2dot mrgtree2dot.C)
TARGET_LINK_LIBRARIES(mrgtree2dot ${SILO_LIB} )
VISIT_INSTALL_TARGETS(mrgtree2dot)
VISIT_DATA_ADD_FOLDER(mrgtree2dot mrgtree2dot)
