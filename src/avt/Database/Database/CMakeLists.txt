# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtdatabase_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabase.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabaseFactory.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetCollection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetDatabase.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFileDescriptorManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGenericDatabase.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIOInformation.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPolygonToTrianglesTesselator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPolyhedralSplit.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSILGenerator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromDatabase.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTransformManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableCache.C)

visit_append_list(
    NAME  avtdatabase_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabase.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabaseFactory.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetCollection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetDatabase.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFileDescriptorManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGenericDatabase.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIOInformation.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPolygonToTrianglesTesselator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPolyhedralSplit.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSILGenerator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromDatabase.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTransformManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableCache.h
          ${CMAKE_CURRENT_SOURCE_DIR}/database_exports.h)

visit_append_list(
    NAME  avtdatabase_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Database/Database>)

