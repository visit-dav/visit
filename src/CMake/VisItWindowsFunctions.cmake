# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

function(visit_create_windows_installer)
    if(VISIT_MAKE_NSIS_INSTALLER)
        include(${VISIT_SOURCE_DIR}/CMake/FindNSIS.cmake)

        if(${MAKENSIS_FOUND})
            set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${VISIT_INSTALLED_VERSION_BIN})
            include("InstallRequiredSystemLibraries")
            # create a custom target that will run our nsis scripts.
            # Make sure it won't run unless specifically called.
            #
            file(TO_NATIVE_PATH ${VISIT_SOURCE_DIR} VSD_NATIVE)
            file(TO_NATIVE_PATH ${VISIT_WINDOWS_DIR} VWD_NATIVE)
            file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION} CIP_NATIVE)
            file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} CBD_NATIVE)
            if(CODESIGN_HASH)
               set(codesign "/DCODESIGN_HASH=${CODESIGN_HASH}")
            endif()
            add_custom_command(OUTPUT visit${VISIT_VERSION}.exe
                COMMAND "${VISIT_WINDOWS_DIR}/MSVC2017/p7zip/18.05/7z.exe" a -t7z -m0=lzma2 -mx9  visit${VISIT_VERSION}.7z VisIt${VISIT_VERSION}
                COMMAND ${MAKENSIS}
                        /DVISIT_SOURCE_DIR=${VSD_NATIVE}
                        /DVISIT_WINDOWS_DIR=${VWD_NATIVE}
                        /DINSTALL_PREFIX=${CIP_NATIVE}
                        /DBIN_DIR=${CBD_NATIVE}
                        ${VISIT_WINDOWS_DIR}/distribution/installation/CreateDBSections.nsi
                COMMAND ${MAKENSIS}
                        ${codesign}
                        /DVisItVersion=${VISIT_VERSION}
                        /DCOMPILER=${VISIT_MSVC_VERSION}
                        /DVISIT_SOURCE_DIR=${VSD_NATIVE}
                        /DVISIT_WINDOWS_DIR=${VWD_NATIVE}
                        /DINSTALL_PREFIX=${CIP_NATIVE}
                        /DBIN_DIR=${CBD_NATIVE}
                        /DMPI_VERSION=${VISIT_MSMPI_VERSION}
                        ${VISIT_WINDOWS_DIR}/distribution/installation/binaryinstallation.nsi
                DEPENDS ${VISIT_WINDOWS_DIR}/distribution/installation/CreateDBSections.nsi
                        ${VISIT_WINDOWS_DIR}/distribution/installation/binaryinstallation.nsi
            )
            add_custom_target(_PACKAGE ALL DEPENDS visit${VISIT_VERSION}.exe)
            if(NOT CODESIGN_HASH)
                # not building a signed release
                # make INSTALL be a prereq for _PACKAGE.
                add_custom_command(TARGET _PACKAGE PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=${CMAKE_CFG_INTDIR} -P cmake_install.cmake)
            endif()
            set_target_properties(_PACKAGE PROPERTIES
                                  EXCLUDE_FROM_DEFAULT_BUILD 1
                                  EXCLUDE_FROM_ALL 1 )

            if(DEFINED ARCHIVER_EXE)
                get_filename_component(VDEP ${VWD_NATIVE} PATH)
                file(TO_NATIVE_PATH ${VDEP} VDEP_NATIVE)
                set(zipname visit_windowsdev_${VISIT_VERSION}.zip)

                file(TO_NATIVE_PATH ${ARCHIVER_EXE} ARCHIVER_NATIVE)
                add_custom_command(OUTPUT ${CBD_NATIVE}/${zipname}
                    COMMAND cd ${VDEP_NATIVE}
                    COMMAND "${ARCHIVER_NATIVE}" a -m0=LZMA -mx=9
                        ${CBD_NATIVE}/${zipname}
                        windowsbuild/distribution
                        windowsbuild/MSVC2022
                        windowsbuild/thirdparty-projects
                    COMMAND cd ${CBD_NATIVE}
                    WORKING_DIRECTORY ${DEP_NATIVE})
                add_custom_target(_WINDEV_PACKAGE ALL DEPENDS ${CBD_NATIVE}/${zipname})
                set_target_properties(_WINDEV_PACKAGE PROPERTIES
                                  EXCLUDE_FROM_DEFAULT_BUILD 1
                                  EXCLUDE_FROM_ALL 1)
                unset(VDEP)
                unset(VDEP_NATIVE)
                unset(zipname)
            endif()
            unset(VSD_NATIVE)
            unset(VWD_NATIVE)
            unset(CIP_NATIVE)
            unset(CBD_NATIVE)
        else()
            message(FATAL_ERROR "NSIS installer package requested, but 'makensis' could not be found.")
        endif()
    endif()
endfunction()


#-----------------------------------------------------------------------------
# Set up resource files for Windows applications.
#-----------------------------------------------------------------------------
function(visit_setup_windows_resources)
    if(EXISTS ${VISIT_WINDOWS_DIR}/distribution/resources/visit.rc)
        set(RESOURCEDIR distribution/resources)
    elseif(EXISTS ${VISIT_WINDOWS_DIR}/visitresources/visit.rc)
        set(RESOURCEDIR visitresources)
    else()
        message(FATAL_ERROR "Cannot find visit.rc in either ${VISIT_WINDOWS_DIR}/distribution/resources or ${VISIT_WINDOWS_DIR}/visitresources")
    endif()

    set(VISIT_VISIT_RESOURCE_FILE      ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/visit.rc)
    set(VISIT_GUI_RESOURCE_FILE        ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/gui.rc)
    set(VISIT_VIEWER_RESOURCE_FILE     ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/viewer.rc)
    set(VISIT_MDSERVER_RESOURCE_FILE   ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/mdserver.rc)
    set(VISIT_ENGINE_SER_RESOURCE_FILE ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/engine_ser.rc)
    set(VISIT_ENGINE_PAR_RESOURCE_FILE ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/engine_par.rc)
    set(VISIT_VCL_RESOURCE_FILE        ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/vcl.rc)
    set(VISIT_CLI_RESOURCE_FILE        ${VISIT_WINDOWS_DIR}/${RESOURCEDIR}/cli.rc)
    message(STATUS "VISIT_VISIT_RESOURCE_FILE = ${VISIT_VISIT_RESOURCE_FILE}")

    #-------------------------------------------------------------------
    # Create versionvars.rc to include version info into resource files.
    #-------------------------------------------------------------------
    string(REPLACE "b" "" filteredver1 "${VISIT_VERSION}")
    string(REPLACE "." "," filteredver2 "${filteredver1}")
    set(VER_FILEVERSION "${filteredver2},0")
    set(VER_PRODUCTVERSION "${filteredver2},0")
    configure_file(${VISIT_WINDOWS_DIR}/distribution/resources/versionvars.rc.in
                   ${VISIT_BINARY_DIR}/include/versionvars.rc @ONLY IMMEDIATE)
endfunction()

# Function to add dependencies to visit_util_builds
function(visit_add_to_util_builds tgt)
    if(VISIT_NO_ALLBUILD)
        add_dependencies(visit_util_builds ${tgt})
        set_target_properties(${tgt} PROPERTIES FOLDER util_builds)
    endif()
endfunction()

function(visit_setup_windows)

   set(CMAKE_SKIP_RPATH ON CACHE INTERNAL "VisIt builds with rpath set." FORCE)

    # VISIT_WINDOWS_DIR needed elsewhere, ensure it is defined and the location exists
    if(NOT VISIT_WINDOWS_DIR)
        message(FATAL_ERROR "VISIT_WINDOWS_DIR is not defined.")
    endif()
    if(NOT EXISTS ${VISIT_WINDOWS_DIR})
        message(FATAL_ERROR "${VISIT_WINDOWS_DIR} does not exist.")
    endif()

    if(VISIT_NO_ALLBUILD)
        # Create a custom target for things like copying files.
        # Only needed if the CMake auto-generated ALL_BUILD project
        # isn't created.
        add_custom_target(visit_util_builds ALL)
    endif()

    visit_setup_windows_resources()

endfunction()
