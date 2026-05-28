# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Kathleen Biagas, Mon Jun 3, 2024
#   Modified to use visit_append_list.
#
#****************************************************************************

visit_append_list(
    NAME   avtdbatts_SOURCES
    ITEMS  ${CMAKE_CURRENT_SOURCE_DIR}/avtSIL.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILArray.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILCollection.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILEnumeratedNamespace.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILMatrix.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILNamespace.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILRangeNamespace.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILRestriction.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILRestrictionTraverser.C
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILSet.C)

visit_append_list(
    NAME   avtdbatts_HEADERS
    ITEMS  ${CMAKE_CURRENT_SOURCE_DIR}/avtSIL.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILArray.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILCollection.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILEnumeratedNamespace.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILMatrix.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILNamespace.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILRangeNamespace.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILRestriction.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILRestrictionTraverser.h
           ${CMAKE_CURRENT_SOURCE_DIR}/avtSILSet.h)

visit_append_list(
    NAME   avtdbatts_INCLUDES
    ITEMS  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
           $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/DBAtts/SIL>)

