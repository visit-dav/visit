# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************/

# Use the TIFF_DIR hint from the config-site .cmake file

if(WIN32 AND VISIT_TIFF_DIR)
    if(EXISTS ${VISIT_TIFF_DIR}/TiffTargets.cmake)
        include(${VISIT_TIFF_DIR}/TiffTargets.cmake)
    endif()

    if(TARGET TIFF::tiff)
        if(VISIT_INSTALL_THIRD_PARTY)
           get_target_property(ilr TIFF::tiff IMPORTED_IMPLIB_RELEASE)
           THIRD_PARTY_INSTALL_LIBRARY(${ilr})
        endif()
    endif()
endif()

