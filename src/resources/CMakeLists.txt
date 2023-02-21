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
#   Kathleen Biagas, Tue Jan 31, 2023
#   Use custom target with custom commands to copy resources on Windows.
#
#   Kathleen Biagas, Tue Feb 21, 2023
#   Fix destination of copied resources folders on Windows.
#
#*****************************************************************************

# Do a little extra in the hosts directory.
add_subdirectory(hosts)

# Directories that we want installed wholesale to the "resources" directory.
set(RESOURCE_DIRECTORIES colortables help hosts images movietemplates translations usage clients)

# Copy the resources directory into the out of source build

if(WIN32)
    # create a custom target, with custom commands to copy the directories.
    add_custom_target(copy_resources ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/clients
                    ${VISIT_EXECUTABLE_DIR}/resources/clients
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/colortables
                    ${VISIT_EXECUTABLE_DIR}/resources/colortables
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/help
                    ${VISIT_EXECUTABLE_DIR}/resources/help
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/hosts
                    ${VISIT_EXECUTABLE_DIR}/resources/hosts
            COMMAND ${CMAKE_COMMAND} -E rm -f
                    ${VISIT_EXECUTABLE_DIR}/resources/hosts/CMakeLists.txt
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/images
                    ${VISIT_EXECUTABLE_DIR}/resources/images
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/movietemplates
                    ${VISIT_EXECUTABLE_DIR}/resources/movietemplates
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/translations
                    ${VISIT_EXECUTABLE_DIR}/resources/translations
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/usage
                    ${VISIT_EXECUTABLE_DIR}/resources/usage)
    visit_add_to_util_builds(copy_resources)
else()

    foreach(ITEM ${RESOURCE_DIRECTORIES})
        get_filename_component(ITM_RP ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}
                           REALPATH)

        file(COPY ${ITM_RP}
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
             PATTERN "CMakeLists.txt" EXCLUDE)
    endforeach()
endif()

# Install these directories in their entirety to the installed "resources"
# directory.
install(DIRECTORY ${RESOURCE_DIRECTORIES}
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
