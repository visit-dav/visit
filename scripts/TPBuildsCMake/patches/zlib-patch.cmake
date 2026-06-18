# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


if(EXISTS ${PATCH_DIR}/zlib-${ZLIB_VERSION}-patched-files.zip)
    file(ARCHIVE_EXTRACT
         INPUT ${PATCH_DIR}/zlib-${ZLIB_VERSION}-patched-files.zip
         VERBOSE)
else()
    message(STATUS "No patches exist for zlib version: ${ZLIB_VERSION}")
endif()

