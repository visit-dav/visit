# Copyright (c) 2011, Allinea
# All rights reserved.
#
# This file has been contributed to the VisIt project, which is
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(DDTSIM_SOURCES
visitsim.cpp
visitdata.cpp
ddtsim.cpp
visitmesh.cpp
visitvariable.cpp
)

# Sources needed only when using dynamic linking
SET(DDTSIM_DYNAMIC_SOURCES
mpicompat/openmpi.c
mpicompat/mpich.c
dllibrary.cpp
libsimlibrary.cpp
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/mpicompat
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/sim/V2/lib
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
)

# Not much point in this library if it can't be read by a debugger!
ADD_DEFINITIONS(-g)

IF(NOT VISIT_STATIC)
	ADD_PARALLEL_LIBRARY(ddtsim SHARED ${DDTSIM_SOURCES} ${DDTSIM_DYNAMIC_SOURCES})
	INSTALL(TARGETS ddtsim
		ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ELSE(NOT VISIT_STATIC)
	ADD_DEFINITIONS(-DVISIT_STATIC)
	ADD_PARALLEL_LIBRARY(ddtsim_static STATIC ${DDTSIM_SOURCES})
	TARGET_LINK_LIBRARIES(ddtsim_static
		simV2_static_par
	)
	IF(NOT APPLE)
		SET_TARGET_PROPERTIES(ddtsim_static PROPERTIES INSTALL_RPATH "$ORIGIN")
	ENDIF(NOT APPLE)
	
	INSTALL(TARGETS ddtsim_static
		ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(NOT VISIT_STATIC)

