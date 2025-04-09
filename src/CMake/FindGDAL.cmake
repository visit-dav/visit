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
#   Kathleen Biagas, Mon Mar 24, 2025
#   Utilize visit_import_third_party
#
#****************************************************************************/

# Uses the GDAL_DIR hint from the config-site .cmake file

visit_import_third_party(GDAL LIBNAMES gdal gdal_i)

if(WIN32)
    if(EXISTS ${VISIT_GDAL_DIR}/lib/gdal224.dll)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
            ${VISIT_GDAL_DIR}/lib/gdal224.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        install(FILES ${VISIT_GDAL_DIR}/lib/gdal224.dll
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ             WORLD_EXECUTE)
    endif()
endif()

