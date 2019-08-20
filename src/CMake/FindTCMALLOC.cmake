# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the TCMALLOC_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF(NOT WIN32)
  # There is no include directory presently so add lib again as the 2nd dir.
  SET_UP_THIRD_PARTY(TCMALLOC lib lib tcmalloc)

  # Hack for hoth's tcmalloc installation
  IF(TCMALLOC_FOUND)
      IF(EXISTS ${TCMALLOC_LIBRARY_DIR}/libtcmalloc.so.0)
          INSTALL(FILES ${TCMALLOC_LIBRARY_DIR}/libtcmalloc.so.0
              DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
              PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
          )
      ENDIF(EXISTS ${TCMALLOC_LIBRARY_DIR}/libtcmalloc.so.0)
  ENDIF(TCMALLOC_FOUND)
ENDIF(NOT WIN32)

