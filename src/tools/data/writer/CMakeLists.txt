# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Kathleen Biagas, Thu Sep 27 11:35:34 PDT 2018
#   Change PYTHON_LIBRARIES to PYTHON_LIBRARY for consistency throughout visit.
#
#****************************************************************************

SET(WRITER_SOURCES
visit_writer.c
py_visit_writer.c
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${PYTHON_INCLUDE_PATH}
# for python 2 + 3 helper header
${VISIT_INCLUDE_DIR}/visitpy/common
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

# Create the visit_writer module
PYTHON_ADD_MODULE(visit_writer
${WRITER_SOURCES}
)
SET_TARGET_PROPERTIES(visit_writer PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES(visit_writer
${PYTHON_LIBRARY}
)

VISIT_INSTALL_TARGETS(visit_writer)
VISIT_DATA_ADD_FOLDER(visit_writer)
