#*****************************************************************************
# Copyright (c) 2014 Intelligent Light. All rights reserved.
# Work partially supported by DOE Grant SC0007548.
# FieldView XDB Export is provided expressly for use within VisIt.
# All other uses are strictly forbidden.
#*****************************************************************************

#
# Install macro that makes sure that a binary library gets installed.
#
MACRO(INSTALL_XDB_LIBRARY libfile)
    IF(WIN32)
	SET(XDB_INSTALL_DIR ${VISIT_INSTALLED_VERSION_BIN})
    ELSE(WIN32)
	SET(XDB_INSTALL_DIR ${VISIT_INSTALLED_VERSION_LIB})
    ENDIF(WIN32)
    INSTALL(FILES ${libfile}
            DESTINATION ${XDB_INSTALL_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE
           )
ENDMACRO(INSTALL_XDB_LIBRARY)

#
# On Mac, we have to add back in VTK and MPICH paths to the VXDB binaries 
# so they will run properly in a development build.
#
MACRO(PATCH_VTK_AND_MPICH_PATHS target libfile)
IF(APPLE)
    ADD_CUSTOM_TARGET(${target} ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Fixing VTK and MPICH paths in ${libfile}"
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/vxdbfixup.py -from-rpath ${libfile} ${VISIT_LIBRARY_DIR} ${VTK_LIBRARY_DIRS} ${VISIT_MPICH_DIR}/lib
    )
    ADD_DEPENDENCIES(${target} copy_vxdb_binaries_to_lib)
ENDIF(APPLE)
ENDMACRO(PATCH_VTK_AND_MPICH_PATHS)

#
# See if we have a supported platform with binaries.
#
IF(WIN32)
    IF(${VISIT_MSVC_VERSION} STREQUAL "MSVC2010-x64")
        SET(XDB_PLATFORM Win64/MSVC2010-x64)
        SET(XDBLIB_LIB XDBLib1)
    ELSEIF(${VISIT_MSVC_VERSION} STREQUAL "MSVC2012-x64")
        SET(XDB_PLATFORM Win64/MSVC2012-x64)
        SET(XDBLIB_LIB XDBLib1)
    ELSE(${VISIT_MSVC_VERSION} STREQUAL "MSVC2010-x64")
        MESSAGE(STATUS "XDB binaries for the ${VISIT_MSVC_VERSION} platform are not provided.")
    ENDIF(${VISIT_MSVC_VERSION} STREQUAL "MSVC2010-x64")
ELSE(WIN32)
    SET(LIBPREFIX lib)
    IF(${VISIT_INSTALL_PLATFORM} STREQUAL "darwin-x86_64")
        SET(XDB_PLATFORM Mac64)
        SET(XDBLIB_LIB XDBLib)
    ELSEIF(${VISIT_INSTALL_PLATFORM} STREQUAL "linux-x86_64")
        SET(XDB_PLATFORM Linux64)
        SET(XDBLIB_LIB XDBLib)
    ELSE(${VISIT_INSTALL_PLATFORM} STREQUAL "darwin-x86_64")
        MESSAGE(STATUS "XDB binaries for the ${VISIT_INSTALL_PLATFORM} platform are not provided.")
    ENDIF(${VISIT_INSTALL_PLATFORM} STREQUAL "darwin-x86_64")
ENDIF(WIN32)

#
# If we have a supported platform, build the plugin.
#
IF(XDB_PLATFORM)
    SET(VXDB_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/VXDB_src)
    SET(VXDB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    SET(VXDB_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${XDB_PLATFORM})
    SET(VXDB_LIB_SER     VXDB_ser)
    SET(VXDB_LIB_PAR     VXDB_par)

    SET(XDBLIB_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/XDBLib)
    SET(XDBLIB_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${XDB_PLATFORM})

    # Build the FieldView XDB plugin.
    MESSAGE(STATUS "The FieldView XDB database plugin will be built.")
    ADD_SUBDIRECTORY(FieldViewXDB)

    IF(WIN32)
        SET(XDB_LIB_DEST ${VISIT_EXECUTABLE_DIR})
    ELSE(WIN32)
        SET(XDB_LIB_DEST ${VISIT_LIBRARY_DIR})
    ENDIF(WIN32)

    # Copy the XDBLib binaries into VISIT_LIBRARY_DIR.
IF(APPLE)
    ADD_CUSTOM_TARGET(copy_xdb_binaries_to_lib ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${XDBLIB_LIBRARY_DIR}/${LIBPREFIX}${XDBLIB_LIB}${VISIT_PLUGIN_EXTENSION} ${XDB_LIB_DEST}
        COMMAND install_name_tool -id ${XDB_LIB_DEST}/${LIBPREFIX}${XDBLIB_LIB}${VISIT_PLUGIN_EXTENSION} ${XDB_LIB_DEST}/${LIBPREFIX}${XDBLIB_LIB}${VISIT_PLUGIN_EXTENSION} 
    )
ELSE(APPLE)
    ADD_CUSTOM_TARGET(copy_xdb_binaries_to_lib ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${XDBLIB_LIBRARY_DIR}/${LIBPREFIX}${XDBLIB_LIB}${VISIT_PLUGIN_EXTENSION} ${XDB_LIB_DEST}
    )
ENDIF(APPLE)
    INSTALL_XDB_LIBRARY(${XDBLIB_LIBRARY_DIR}/${LIBPREFIX}${XDBLIB_LIB}${VISIT_PLUGIN_EXTENSION})

    IF(EXISTS ${VXDB_DIR})
        # We have sources for private VXDB library so let's build it.

        # We will have XDBLib headers.
        SET(XDBLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/XDBLib/include)

        # If the private sources exist then build them.
        MESSAGE(STATUS "The VXDB database plugin support library will be built.")
        ADD_SUBDIRECTORY(VXDB_src)
    ELSE(EXISTS ${VXDB_DIR})
        # Copy the VXDB binaries into VISIT_LIBRARY_DIR.
        IF(VISIT_PARALLEL)
            ADD_CUSTOM_TARGET(copy_vxdb_binaries_to_lib ALL
                COMMAND ${CMAKE_COMMAND} -E copy ${VXDB_LIBRARY_DIR}/${LIBPREFIX}${VXDB_LIB_SER}${VISIT_PLUGIN_EXTENSION} ${XDB_LIB_DEST}
                COMMAND ${CMAKE_COMMAND} -E copy ${VXDB_LIBRARY_DIR}/${LIBPREFIX}${VXDB_LIB_PAR}${VISIT_PLUGIN_EXTENSION} ${XDB_LIB_DEST}
            )
            # This is needed to make sure that the VXDB libraries work in a development build on APPLE.
            PATCH_VTK_AND_MPICH_PATHS(patch_vtk_VXDB_ser ${XDB_LIB_DEST}/${LIBPREFIX}${VXDB_LIB_SER}${VISIT_PLUGIN_EXTENSION})
            PATCH_VTK_AND_MPICH_PATHS(patch_vtk_VXDB_par ${XDB_LIB_DEST}/${LIBPREFIX}${VXDB_LIB_PAR}${VISIT_PLUGIN_EXTENSION})
        ELSE(VISIT_PARALLEL)
            ADD_CUSTOM_TARGET(copy_vxdb_binaries_to_lib ALL
                COMMAND ${CMAKE_COMMAND} -E copy ${VXDB_LIBRARY_DIR}/${LIBPREFIX}${VXDB_LIB_SER}${VISIT_PLUGIN_EXTENSION} ${XDB_LIB_DEST}
            )
            # This is needed to make sure that the VXDB libraries work in a development build on APPLE.
            PATCH_VTK_AND_MPICH_PATHS(patch_vtk_VXDB_ser ${XDB_LIB_DEST}/${LIBPREFIX}${VXDB_LIB_SER}${VISIT_PLUGIN_EXTENSION})
        ENDIF(VISIT_PARALLEL)
    ENDIF(EXISTS ${VXDB_DIR})

    INSTALL_XDB_LIBRARY(${VXDB_LIBRARY_DIR}/${LIBPREFIX}${VXDB_LIB_SER}${VISIT_PLUGIN_EXTENSION})
    IF(VISIT_PARALLEL)
        INSTALL_XDB_LIBRARY(${VXDB_LIBRARY_DIR}/${LIBPREFIX}${VXDB_LIB_PAR}${VISIT_PLUGIN_EXTENSION})
    ENDIF(VISIT_PARALLEL)
ENDIF(XDB_PLATFORM)
