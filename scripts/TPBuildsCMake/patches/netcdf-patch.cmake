# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


set(pd ${PATCH_DIR}/netcdf-${NETCDF_VERSION}-patch-files)
if(EXISTS ${pd})
    file(GLOB patchfiles ${pd}/*.patch)
    foreach(patch_file ${patchfiles})
        execute_process(COMMAND ${PATCH_CMD} -u --verbose -p0 -i ${patch_file})
    endforeach()
else()
    message(STATUS "No patches exist for netcdf version: ${NETCDF_VERSION}")
endif()

