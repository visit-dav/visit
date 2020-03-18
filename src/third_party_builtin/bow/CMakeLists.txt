# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:22:08 PST 2010
#   Add folder property.
#
#   Brad Whitlock, Thu Apr 18 14:40:01 PDT 2013
#   Provide a macro name for zlib.
#
#****************************************************************************

SET(BOW_SOURCES
bow.C
fwav.C
gzlib.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
)

ADD_LIBRARY(bow ${BOW_SOURCES})
TARGET_LINK_LIBRARIES(bow ${ZLIB_LIBRARY})

VISIT_INSTALL_TARGETS(bow)
SET_TARGET_PROPERTIES(bow PROPERTIES FOLDER thirdparty_builtin/bow)
