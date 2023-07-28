# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#
#    Kathleen Biagas, Tue Jul 13 14:58:28 PDT 2021
#    Remove avtScalarMetaData and avtSimulationCommandSpecification from
#    code gen targets list. They cannot be auto-gen'd.
#
#    Kathleen Biagas, Tue Dec 13, 2022
#    Moved setting of sources to subdirs.
#
#****************************************************************************

add_library(avtdbatts)

add_subdirectory(MetaData)
add_subdirectory(SIL)

target_link_libraries(avtdbatts visitcommon)
target_include_directories(avtdbatts PUBLIC ${VISIT_COMMON_INCLUDES})

VISIT_INSTALL_TARGETS(avtdbatts)

