# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtdbatts_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtSIL.C
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
    NAME  avtdbatts_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/include/avt/DBatts/SIL>)

