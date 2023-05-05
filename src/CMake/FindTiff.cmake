# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************/

# Use the TIFF_DIR hint from the config-site .cmake file

if(WIN32 AND VISIT_TIFF_DIR)
    SET_UP_THIRD_PARTY(TIFF LIBS tiff)
    set(TIFF_LIBRARY ${TIFF_LIBRARY_DIR}/${TIFF_LIB})
endif()

