# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Notes:
#   This directory may later contain additional subdirectories that provide
#   useful content. For example, we could have a "logos" directory that contains
#   VisIt project logos or commonly used Lab logos. We could have some VTK
#   datasets such as globes, etc. 
#
# Modifications:
#   Brad Whitlock, Thu Sep  6 11:50:09 PDT 2012
#   Add hosts to the directories that get installed to resources.
#
#   Kathleen Biagas, Wed Sep 12 11:57:06 PDT 2012
#   Add CMakeLists.txt and cmake-generated files to exclusion lists.
#
#   Brad Whitlock, Fri Oct 12 16:32:28 PDT 2012
#   Move the help directory to here.
#
#   Kathleen Biagas, Mon Oct 23 17:07:52 MST 2017
#   Change how Resources dir is handled for build on Windows.
#
#*****************************************************************************

# Do a little extra in the hosts directory.
ADD_SUBDIRECTORY(hosts)

# Directories that we want installed wholesale to the "resources" directory.
SET(RESOURCE_DIRECTORIES colortables help hosts images movietemplates translations usage clients)

# Copy the resources directory into the out of source build
FOREACH(ITEM ${RESOURCE_DIRECTORIES})
    GET_FILENAME_COMPONENT(ITM_RP ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}
                           REALPATH)

    if(NOT WIN32)
        FILE(COPY ${ITM_RP}
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
             PATTERN ".svn" EXCLUDE
             PATTERN "CMakeLists.txt" EXCLUDE)
    else()
        foreach(CFG Release Debug RelWithDebInfo MinSizeRel)
            file(COPY ${ITM_RP}
                 DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CFG}/resources
                 FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                  GROUP_READ GROUP_WRITE
                                  WORLD_READ
                 PATTERN ".svn" EXCLUDE
                 PATTERN "CMakeLists.txt" EXCLUDE)
        endforeach()
    endif()
ENDFOREACH(ITEM)

# Install these directories in their entirety to the installed "resources" 
# directory.
INSTALL(DIRECTORY ${RESOURCE_DIRECTORIES}
    DESTINATION ${VISIT_INSTALLED_VERSION_RESOURCES}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE 
                     GROUP_READ GROUP_WRITE 
                     WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                          GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                          WORLD_READ             WORLD_EXECUTE
    PATTERN ".svn" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "Makefile" EXCLUDE
    PATTERN "cmake_install.cmake" EXCLUDE
)
