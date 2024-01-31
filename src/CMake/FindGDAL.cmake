# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Tom Fogal, Thu Mar 25 14:24:49 MDT 2010
#   Fix GDAL library naming convention on OS X.
#
#   Kathleen Bonnell, Tue Dec 21 14:45:51 MST 2010
#   Update gdal version to 1.7 on Windows.
#
#   Brad Whitlock, Fri Oct 14 10:56:28 PDT 2011
#   GDAL changed again on Mac.
#
#****************************************************************************/

# Use the GDAL_DIR hint from the config-site .cmake file

IF (WIN32)
    SET_UP_THIRD_PARTY(GDAL LIBS gdal_i)
    # normally handled in InstallThirdParty.cmake, but gdal has a weird
    # naming convention on windows
    FOREACH(VER 17 19 110 111 222 224)
        IF(EXISTS ${GDAL_LIBRARY_DIR}/gdal${VER}.dll)
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                ${GDAL_LIBRARY_DIR}/gdal${VER}.dll
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            INSTALL(FILES ${GDAL_LIBRARY_DIR}/gdal${VER}.dll
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                )
        ENDIF(EXISTS ${GDAL_LIBRARY_DIR}/gdal${VER}.dll)
    ENDFOREACH(VER)
ELSE (WIN32)
    SET_UP_THIRD_PARTY(GDAL LIBS gdal)
ENDIF (WIN32)

