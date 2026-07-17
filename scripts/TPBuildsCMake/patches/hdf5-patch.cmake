# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


set(pd ${PATCH_DIR}/hdf5-${HDF5_VERSION}-patch-files)
if(EXISTS ${pd})
    message("HDF5 patch file dir exists")
    file(GLOB patchfiles ${pd}/*.patch)
    foreach(patch_file ${patchfiles})
        message("HDF5 patch file: ${patch_file}")
        execute_process(COMMAND ${PATCH_CMD} -u --verbose -p0 -i ${patch_file})
    endforeach()
else()
    message(STATUS "No patches exist for hdf5 version: ${HDF5_VERSION}")
endif()

