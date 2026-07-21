# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtdatabase_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDomainBoundaries.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGhostNodeGenerator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIsenburgSGG.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalStructuredDomainBoundaries.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNekDomainBoundaries.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStreamingGhostGenerator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredDomainBoundaries.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredDomainNesting.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnstructuredDomainBoundaries.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnstructuredPointBoundaries.C
          ${CMAKE_CURRENT_SOURCE_DIR}/StructuredBoundary.C)

visit_append_list(
    NAME  avtdatabase_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDomainBoundaries.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDomainNesting.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGhostNodeGenerator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIsenburgSGG.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalStructuredDomainBoundaries.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNekDomainBoundaries.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStreamingGhostGenerator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredDomainBoundaries.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredDomainNesting.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnstructuredDomainBoundaries.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnstructuredPointBoundaries.h)

visit_append_list(
    NAME  avtdatabase_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Database/Ghost>)

