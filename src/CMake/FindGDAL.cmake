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
#   Kathleen Biagas, Wed Mar 4, 2026
#   Reduce version check on Windows.
#
#****************************************************************************/

# Use the GDAL_DIR hint from the config-site .cmake file

if(WIN32)
    SET_UP_THIRD_PARTY(GDAL LIBS gdal_i)
    # normally handled in InstallThirdParty.cmake, but gdal has a weird
    # naming convention on windows
    if(EXISTS ${GDAL_LIBRARY_DIR}/gdal224.dll)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                ${GDAL_LIBRARY_DIR}/gdal224.dll
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        install(FILES ${GDAL_LIBRARY_DIR}/gdal224.dll
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
    else()
        message(WARNING "Could not find gdal 224 dll")
    endif()
else()
    SET_UP_THIRD_PARTY(GDAL LIBS gdal)
endif()

