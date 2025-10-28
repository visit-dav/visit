# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Mon Mar 31, 2025
#   Utilize visit_import_third_party.
#
#****************************************************************************/

# Uses the TIFF_DIR hint from the config-site .cmake file

if(WIN32 AND VISIT_TIFF_DIR)
    visit_import_third_party(TIFF LIBS tiff)
    # for now, satisfy the vtk interface this way
    add_library(TIFF::TIFF ALIAS tiff)
endif()

