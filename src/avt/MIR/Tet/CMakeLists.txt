# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtmir_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/Edge.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Face.C
          ${CMAKE_CURRENT_SOURCE_DIR}/mat3d_tet.C
          ${CMAKE_CURRENT_SOURCE_DIR}/MaterialTetrahedron.C
          ${CMAKE_CURRENT_SOURCE_DIR}/MaterialTriangle.C
          ${CMAKE_CURRENT_SOURCE_DIR}/QuadraticHash.C
          ${CMAKE_CURRENT_SOURCE_DIR}/TetMIR.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Tetrahedralizer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Triangulator.C)

visit_append_list(
    NAME  avtmir_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/Edge.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Face.h
          ${CMAKE_CURRENT_SOURCE_DIR}/mat3d_tet.h
          ${CMAKE_CURRENT_SOURCE_DIR}/MaterialTetrahedron.h
          ${CMAKE_CURRENT_SOURCE_DIR}/MaterialTriangle.h
          ${CMAKE_CURRENT_SOURCE_DIR}/QuadraticHash.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Tet.h
          ${CMAKE_CURRENT_SOURCE_DIR}/TetList.h
          ${CMAKE_CURRENT_SOURCE_DIR}/TetMIR.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Tetrahedralizer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Tri.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Triangulator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/TriList.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Wedge.h
          ${CMAKE_CURRENT_SOURCE_DIR}/WedgeList.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoneCntAndVF.h)

visit_append_list(
    NAME  avtmir_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/MIR/Tet>)
