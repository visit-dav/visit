# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the UINTAH_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

# Note UINTAH_INTERFACES_LIB is called in the reader using dlopen
# so the true library name is need for the compiled operating system.
# This variable MUST BE the FIRST variable in the library list.
# Here the base name is set and passed to the third party set up
SET(UINTAH_INTERFACES_LIB
  VisIt_interfaces
)

IF (${VISIT_USE_SYSTEM_UINTAH})
  SET(INCLUDE_TMP_DIR ../src)
ELSE (${VISIT_USE_SYSTEM_UINTAH})
  SET(INCLUDE_TMP_DIR include)
ENDIF (${VISIT_USE_SYSTEM_UINTAH})


SET(REQ_UINTAH_MODS 
  CCA_Components_DataArchiver
  CCA_Components_LoadBalancers
  CCA_Components_ProblemSpecification
  CCA_Components_Schedulers
  CCA_Ports
  Core_Containers
  Core_DataArchive
  Core_Datatypes
  Core_Disclosure
  Core_Exceptions
  Core_Geometry
  Core_GeometryPiece
  Core_Grid
  Core_IO
  Core_Malloc
  Core_Math
  Core_OS
  Core_Parallel
  Core_ProblemSpec
  Core_Util
  )

SET_UP_THIRD_PARTY(UINTAH lib ${INCLUDE_TMP_DIR}
  ${UINTAH_INTERFACES_LIB}
  ${REQ_UINTAH_MODS}
)

# Note UINTAH_INTERFACES_LIB is called in the reader using dlopen
# so the true library is need for the compiled operating system.
# This variable MUST BE the FIRST variable in the library list.
# Here the true name is retrieved from the third party set up
LIST(GET UINTAH_LIB 0 UINTAH_INTERFACES_LIB)

GET_FILENAME_COMPONENT(UINTAH_INTERFACES_LIB ${UINTAH_INTERFACES_LIB} NAME)

MESSAGE(STATUS "UINTAH_INTERFACES_LIB = ${UINTAH_INTERFACES_LIB}")

UNSET(INCLUDE_TMP_DIR)
