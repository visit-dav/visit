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
#   Kathleen Biagas, Mon May 20, 2024
#   Utilize visit_add_library.
#
#****************************************************************************

visit_add_library(
    NAME       bow
    SOURCES    bow.C fwav.C gzlib.C
    HEADERS    bow.h fwav.h gzlib.h bow_exports.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/third_party_builtin/bow>
    DEPENDS_ON zlib
    FOLDER     thirdparty_builtin/bow)

