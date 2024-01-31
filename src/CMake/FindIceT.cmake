# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed logic handling windows differently than other platforms.
#
#   Kathleen Biagas, Thu Jan 7, 2021
#   Only process if VISIT_ICET_DIR is set.
#
#****************************************************************************/

# Ice-T provides an ICETConfig.cmake file that we could include but it appears
# to have errors in the computed library and include paths. If those problems
# get resolved, we can uncomment these lines and remove our own detection code.
#IF(VISIT_ICET_DIR)
#    INCLUDE(${VISIT_ICET_DIR}/lib/ICETConfig.cmake)
#ENDIF(VISIT_ICET_DIR)

# Use the ICET_DIR hint from the config-site .cmake file

if(VISIT_ICET_DIR)
    SET_UP_THIRD_PARTY(ICET LIBS IceTCore IceTGL IceTMPI)
endif()

