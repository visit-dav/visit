# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Dirk Schubert (Allinea Software), Mon Oct 15, 2012
#   Build symV2dyn only if VISIT_DDT is chosen.
#
#   Eric Brugger, Wed Dec 26 15:31:11 PST 2012
#   Always build symV2dyn, so that it can be used in the swig/python
#   directory.
#
#   Kathleen Biagas, Mon Jan 7 18:04:23 PST 2012
#   Don't build symV2dyn on Windows.
#
#   Kathleen Biagas, Thu Nov  7 07:57:14 PST 2013
#   Add VisItControlInterface_V2_plotting.h to files being installed.
#
#   Brad Whitlock, Fri Aug 14 12:28:12 PDT 2015
#   Add VisIt_OptionList.
#
#   Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#   Add xml tools code gen targets
#****************************************************************************

# See ../CMakeLists.txt for LIBSIM_SOURCES

SET(FORTRAN_LIBSIM_SOURCES visitfortransimV2interface.c)


# Add XML Code Gen Targets

# Note: These names are in alphabetical order

set(LIBSIM_GEN_OBJECTS 
    CommandMetaData
    CurveMetaData
    ExpressionMetaData
    MaterialMetaData
    MeshMetaData
    MessageMetaData
    NameList
    SpeciesMetaData
    VariableMetaData
    View3D)

# Libsim Objects
foreach(obj_name ${LIBSIM_GEN_OBJECTS})
    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_BINARY_DIR}/include
)

IF(NOT WIN32 AND NOT VISIT_STATIC)
  # Allinea Software:
  # Create a libsimV2.so shared library for the standard VisIt
  # interface (normally provided by statically linking with libsimV2.a).
  # -fvisibility is set to default (it is normally hidden).
  # Preferred solution would be to tag each function to be exposed in
  # the shared library individually instead of exposing everything,
  # but this will do for now. Also provide debug info.
  ADD_DEFINITIONS(-g -fvisibility=default)
  ADD_LIBRARY(simV2dyn SHARED ${LIBSIM_SOURCES})
  SET(LIBSIM_DYN simV2dyn)
ENDIF(NOT WIN32 AND NOT VISIT_STATIC)

ADD_LIBRARY(simV2 STATIC ${LIBSIM_SOURCES})
ADD_LIBRARY(simV2f STATIC ${FORTRAN_LIBSIM_SOURCES})


INSTALL(TARGETS simV2 simV2f ${LIBSIM_DYN}
    ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
    LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
INSTALL(FILES 
VisItControlInterface_V2.h
VisItControlInterface_V2_plotting.h
VisItDataInterface_V2.h
VisItInterfaceTypes_V2.h
VisIt_CSGMesh.h
VisIt_CommandMetaData.h
VisIt_CurveData.h
VisIt_CurveMetaData.h
VisIt_CurvilinearMesh.h
VisIt_DomainBoundaries.h
VisIt_DomainList.h
VisIt_DomainNesting.h
VisIt_ExpressionMetaData.h
VisIt_MaterialData.h
VisIt_MaterialMetaData.h
VisIt_MeshMetaData.h
VisIt_MessageMetaData.h
VisIt_NameList.h
VisIt_OptionList.h
VisIt_PointMesh.h
VisIt_RectilinearMesh.h
VisIt_SimulationMetaData.h
VisIt_SpeciesData.h
VisIt_SpeciesMetaData.h
VisIt_UnstructuredMesh.h
VisIt_VariableData.h
VisIt_VariableMetaData.h
VisIt_View2D.h
VisIt_View3D.h
visitfortransimV2interface.inc
    DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/include
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ 
)
