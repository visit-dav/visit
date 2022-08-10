# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Fri Jan 22 16:51:39 PST 2010
#   I added the data generator unic, which generates UNIC test data.
#
#   Mark C. Miller, Fri Jan 22 16:40:43 PST 2010
#   Added function LINK_DATA_TARGETS and invokation to create sesame.pdb.lnk
#   and largefile.silo.lnk.
#
#   Mark C. Miller, Fri Jan 22 17:27:47 PST 2010
#   Removed preceding change to satisfy purists ;)
#
#   Cyrus Harrison, Mon Feb  1 12:46:56 PST 2010
#   Added missing visit includes to basic targets.
#
#   Brad Whitlock, Wed Feb 10 14:44:23 PST 2010
#   I added Velodyne test data.
#
#   Eric Brugger, Thu Mar 11 15:04:16 PST 2010
#   I added logic to create the example data files for a visit distribution
#   if the file visit_dist_data.tar.gz was not present.
#
#   Eric Brugger, Fri Mar 12 13:08:21 PST 2010
#   I added dependencies and additional outputs to the custom commands I
#   added yesterday so that parallel builds would work and make clean would
#   remove all the files.
#
#   Mark C. Miller, Fri Apr 23 23:51:46 PDT 2010
#   Use glob to produce list of archived targets. That way, don't have to
#   edit this makefile every time one is added or removed.
#
#   Brad Whitlock, Thu Apr 29 11:50:42 PDT 2010
#   I added xdmf2d.C.
#
#   Eric Brugger, Thu Jun 17 15:24:51 PDT 2010
#   I added logic to only create the Silo data files if VisIt is being
#   built with the Silo library.
#
#   Kathleen Bonnell, Mon Nov 15 15:46:11 MST 2010
#   Add support for building on windows.
#
#   Cyrus Harrison, Wed Feb  9 11:20:26 PST 2011
#   Fix install of data files from out of source build.
#
#   Eric Brugger, Fri Nov 18 14:52:52 PST 2011
#   I added xdmf_binary.C.
#
#   Eric Brugger, Fri Oct 18 14:55:19 PDT 2013
#   I changed the name of the test target to testdata, since test no longer
#   works with cmake 2.8.10.2.
#
#   Kathleen Biagas, Tue Nov 25 18:06:23 MST 2014
#   Add /STACK Link flag to multi_test target on Windows so it will run
#   properly.  Reworked the create_data_examples targets so things work
#   correctly on Windows.
#
#   Eric Brugger, Wed Sep 20 17:35:37 PDT 2017
#   Modify the script to untar files of the name "*test_data*.tar.gz" instead
#   of just "*test_data.tar.gz".
#
#   Mark C. Miller, Thu Aug 16 11:32:45 PDT 2018
#   Although we use 7z for data archives for VisIt testing, we don't wanna
#   force all users/consumers of VisIt to have to have/use 7z. So, for
#   example data we package with VisIt distributions, we still use good 'ole
#   tar and gzip.
#
#   Kathleen Biagas, Thu Dec  6 14:21:19 PST 2018
#   Add VISIT_DEFAULT_SILO_DRIVER cmake arg, which if blank will allow
#   driver to be specified at make time.
#
#   Mark C. Miller, Wed Mar 23 21:12:34 PDT 2022
#   Elim use of 7z in favor of vanilla tar
#
#   Kathleen Biagas, Fri Mar 25 11:41:48 PDT 2022
#   Test the size of visit_dist_data.tar.xz to ensure its the real one
#   and not the lfs'd version. Otherwise untar will fail.
#
#****************************************************************************

MESSAGE(STATUS "Configuring VisIt Data Generation Targets")

#-----------------------------------------------------------------------------
# BUILD_DATA_TARGET cmake function
# Helper function that compiles & links a data generator program.
# Usage:
#  BUILD_DATA_TARGET( target_name
#                    INCLUDE_DIRS
#                     path/to/includes
#                     path/to/more/includes
#                    LIB_DIRS
#                     path/to/libs
#                     path/to/more/libs
#                    LIBS
#                     lib_names
#                    DEFS
#                     definitions (eg. Preprocessor for windows)
#
# Source files are detected in two ways:
#  If ${target_name}_SOURCES is set:  its contents are used.
#  Otherwise: ${target_name}.C is used.
#
#-----------------------------------------------------------------------------
FUNCTION(BUILD_DATA_TARGET_IMPL target_name do_parallel)
    SET(target_SOURCES "")
    SET(target_INCLUDE_DIRS "")
    SET(target_LIB_DIRS "")
    SET(target_LIBS "")
    SET(target_DEFS "")

    # cmake gymnastics to implement optional argument sections:
    FOREACH(ARG ${ARGN})
        IF(${ARG} STREQUAL "INCLUDE_DIRS" OR
           ${ARG} STREQUAL "LIB_DIRS" OR
           ${ARG} STREQUAL "LIBS" OR
           ${ARG} STREQUAL "DEFS")
            #switch var name to extend
            SET(ARG_VAR target_${ARG})
        ELSE()
            # add current arg into current varname
            SET("${ARG_VAR}" ${${ARG_VAR}} ${ARG})
        ENDIF()
    ENDFOREACH(ARG ${ARGN})

    INCLUDE_DIRECTORIES(${target_INCLUDE_DIRS})
    LINK_DIRECTORIES(${target_LIB_DIRS})

    # Automatically setup target_SOURCES if not set
    IF(DEFINED ${target_name}_SOURCES)
        SET(target_SOURCES "${${target_name}_SOURCES}")
    ELSE()
        SET(target_SOURCES ${target_name}.C)
    ENDIF()

    SET(tname ${target_name})
    IF("${do_parallel}" STREQUAL "ON")
        IF(VISIT_PARALLEL)
            SET(tname "${target_name}_par")
            ADD_PARALLEL_EXECUTABLE(${tname} ${target_SOURCES})
        ENDIF()
    ELSE()
        ADD_EXECUTABLE(${tname} ${target_SOURCES})
    ENDIF()
    VISIT_DATA_ADD_FOLDER(datagen ${tname})

    ADD_DEFINITIONS(${target_DEFS})
    TARGET_LINK_LIBRARIES(${tname} ${target_LIBS})
    SET_TARGET_PROPERTIES(${tname}
                           PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                          )
ENDFUNCTION(BUILD_DATA_TARGET_IMPL target_name)

FUNCTION(BUILD_VISIT_DIST_DATA)
    FILE(MAKE_DIRECTORY visit_dist_data)
if(WIN32)
    file(GLOB vdd_files "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFGINT_DIR}/*.{silo,pdb}" CONFIGURE_DEPENDS)
    FILE(COPY ${vdd_files} DESTINATION visit_dist_data )
else()
    FILE(COPY dummy DESTINATION visit_dist_data FILES_MATCHING PATTERN "*.{silo,pdb}")
endif()
    SET(DIST_TAR_CMD "${TAR_CMD} ${TAR_ARGS} visit_dist_data.tar.xz visit_dist_data")
    EXECUTE_PROCESS(COMMAND ${DIST_TAR_CMD} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
ENDFUNCTION(BUILD_VISIT_DIST_DATA)

FUNCTION(BUILD_DATA_TARGET target_name)
    BUILD_DATA_TARGET_IMPL(${target_name} "OFF" ${ARGN})
ENDFUNCTION(BUILD_DATA_TARGET target_name)

FUNCTION(BUILD_PARALLEL_DATA_TARGET target_name)
    BUILD_DATA_TARGET_IMPL(${target_name} "ON" ${ARGN})
ENDFUNCTION(BUILD_PARALLEL_DATA_TARGET target_name)

#-----------------------------------------------------------------------------
# Data Generation Program Targets
#
# This are build as part of the default target.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Basic Data Gen Programs
#-----------------------------------------------------------------------------
SET(BASIC_TARGETS
bov
curve_builder
distribution_curves
mandelbulb
xdmf_binary
)

SET(BASIC_TARGETS_PAR
mandelbulb
)

#-----------------------------------------------------------------------------
# VTK Gen Programs
#-----------------------------------------------------------------------------
SET(VTK_TARGETS degen_meshes)

#-----------------------------------------------------------------------------
# HDF5 Data Gen Programs
#-----------------------------------------------------------------------------
SET(HDF5_TARGETS unic xdmf xdmf2d)

#-----------------------------------------------------------------------------
# PDB Data Gen Programs
#-----------------------------------------------------------------------------
SET(PDB_TARGETS
correlationtest
pdbdatabase
streak
)

#-----------------------------------------------------------------------------
#  Setup {TARGET}_SOURCES for targets with multiple source files.
#-----------------------------------------------------------------------------
SET(correlationtest_SOURCES correlationtest.C pdbdatabase_link.C)
SET(pdbdatabase_SOURCES pdbdatabase.C pdbdatabase_link.C)
SET(streak_SOURCES streak.C pdbdatabase_link.C)

#-----------------------------------------------------------------------------
# Silo Data Gen Programs
#-----------------------------------------------------------------------------
SET(SILO_TARGETS
arbpoly
bigsil
bounce
big_quads
csg
emptydomains
fullframe
galaxy
ghost
global_node
globe
largefile
lineout_test
lowfrac
mat_3x3_3across
mat_3x3x3_3across
meshorigin
mmadj_rect_2d
multi_test
multidir
noise
noise2d
non_conforming
one_quad
quad_disk
sid97
specmix
specmix_double
testall
thinplane
tire
wave
)

#-----------------------------------------------------------------------------
#  Setup {TARGET}_SOURCES for targets with multiple source files.
#-----------------------------------------------------------------------------
SET(noise_SOURCES noise.C QuadMesh.C)
SET(multidir_SOURCES multidir.C QuadMesh.C)


#-----------------------------------------------------------------------------
# Build all Basic Targets
#-----------------------------------------------------------------------------
FOREACH(BASIC_TARGET ${BASIC_TARGETS})
   BUILD_DATA_TARGET(${BASIC_TARGET}
                     INCLUDE_DIRS
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${VISIT_BINARY_DIR}/include
                     ${VISIT_SOURCE_DIR}/include)
ENDFOREACH(BASIC_TARGET ${BASIC_TARGETS})

FOREACH(BASIC_TARGET ${BASIC_TARGETS_PAR})
   BUILD_PARALLEL_DATA_TARGET(${BASIC_TARGET}
                     INCLUDE_DIRS
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${VISIT_BINARY_DIR}/include
                     ${VISIT_SOURCE_DIR}/include)
ENDFOREACH(BASIC_TARGET ${BASIC_TARGETS_PAR})

#-----------------------------------------------------------------------------
# Build all VTK Targets
#-----------------------------------------------------------------------------
FOREACH(VTK_TARGET ${VTK_TARGETS})
    if(VTK_VERSION VERSION_EQUAL "8.1.0")
        BUILD_DATA_TARGET(${VTK_TARGET}
            INCLUDE_DIRS ${VTK_INCLUDE_DIRS}
            LIB_DIRS ${VTK_LIBRARY_DIRS}
            LIBS vtkIOLegacy)
    else()
        BUILD_DATA_TARGET(${VTK_TARGET}
            LIBS VTK::IOLegacy)
    endif()
ENDFOREACH(VTK_TARGET ${VTK_TARGETS})

#-----------------------------------------------------------------------------
# If we have HDF5 build all HDF5 Data Gen Target Programs
#-----------------------------------------------------------------------------
IF(HDF5_FOUND)
    FOREACH(HDF5_TARGET ${HDF5_TARGETS})
        BUILD_DATA_TARGET(${HDF5_TARGET}
            INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIR}
            LIB_DIRS ${HDF5_LIBRARY_DIR}
            LIBS ${HDF5_LIB})
    ENDFOREACH()
ENDIF(HDF5_FOUND)

#-----------------------------------------------------------------------------
# If we have Silo build all Silo & PDB Data Gen Programs
#-----------------------------------------------------------------------------
IF(SILO_FOUND)
    # Build PDB Targets (The required pdb symbols live in Silo's libs.)
    FOREACH(PDB_TARGET ${PDB_TARGETS})
        BUILD_DATA_TARGET(${PDB_TARGET}
            INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_BINARY_DIR}/include ${VISIT_SOURCE_DIR}/include
            LIB_DIRS ${PDB_LIBRARY_DIR}
            LIBS ${PDB_LIB})
    ENDFOREACH()
    # Build Silo Targets
    FOREACH (SILO_TARGET ${SILO_TARGETS})
        BUILD_DATA_TARGET(${SILO_TARGET}
            INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${SILO_INCLUDE_DIR}
                ${VISIT_SOURCE_DIR}/common/utility
            LIB_DIRS ${SILO_LIBRARY_DIR}
            LIBS ${SILO_LIB})
    ENDFOREACH()
    if(WIN32)
      set_target_properties(multi_test PROPERTIES LINK_FLAGS "/STACK:8000000")
    endif()
ENDIF(SILO_FOUND)

#-----------------------------------------------------------------------------
# Setup the Data Generation Target.
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# Discover the proper untar command.
#
# "cmake -E tar" supports untaring for normal & gzipped tarballs.
# Unfortunately it is very slow so we only use it as a fallback. It also
# fails on Windows for many of our .tar.gz data files.
#
#  Kathleen Biagas, Wed Oct  2 14:19:36 PDT 2013
#  Added PROGRAM6432 env var to search as I discovered that 32-bit cmake
#  running on 64-bit Windows 8 reports 'Program Files (x86)' for
#  'ENV${PROGRAMFILES}'.
#
#-----------------------------------------------------------------------------
FIND_PROGRAM(TAR_EXE tar)
IF(TAR_EXE AND NOT WIN32)
    SET(TAR_CMD ${TAR_EXE})
    SET(UNTAR_ARGS "xvf")
    SET(TAR_ARGS "cvfJ")
ELSE()
    SET(TAR_CMD ${CMAKE_COMMAND})
    SET(UNTAR_ARGS "-E tar xvf")
    SET(TAR_ARGS "-E tar cvfJ")
ENDIF()

#-----------------------------------------------------------------------------
# CMake variable names for make variables used in convenience targets below.
# Note the {} surrounding the make variables allowing them to be expanded at
# make time, not cmake time. Allows for `make SILO_DRIVER=DB_HDF5` to build
# all Silo data using HDF5 driver.
#-----------------------------------------------------------------------------
if(VISIT_DEFAULT_SILO_DRIVER STREQUAL "HDF5" OR
   VISIT_DEFAULT_SILO_DRIVER STREQUAL "PDB")
    set(SDA "DB_${VISIT_DEFAULT_SILO_DRIVER}")
else()
    message(STATUS "VISIT_DEFAULT_SILO_DRIVER not specified, leaving driver designation blank for use at make time.")
    set(SILO_DRIVER_ARG {SILO_DRIVER})
    set(SDA "$${SILO_DRIVER_ARG}")
endif()

#-----------------------------------------------------------------------------
# Add install command for contents of "data"
#-----------------------------------------------------------------------------

set(USE_VISIT_DIST_DATA_TARBALL false)

if(EXISTS ${VISIT_SOURCE_DIR}/../data/visit_dist_data.tar.xz)
    # Need to test the size of visit_dist_data, because untar will fail if
    # its the lfs'd version of the file and not the real one. This happens
    # with build_visit,  because lfs content isn't pulled.

    file(SIZE ${VISIT_SOURCE_DIR}/../data/visit_dist_data.tar.xz VDD_SIZE)
    if(VDD_SIZE GREATER 200) # lfs'd files are generally ~130 bytes.
        set(USE_VISIT_DIST_DATA_TARBALL true)
    endif()
endif()

IF(USE_VISIT_DIST_DATA_TARBALL)
    SET(DIST_UNTAR_CMD "${TAR_CMD} ${UNTAR_ARGS} ${VISIT_SOURCE_DIR}/../data/visit_dist_data.tar.xz")
    INSTALL(CODE "execute_process(COMMAND ${DIST_UNTAR_CMD} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )")
    INSTALL(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/visit_dist_data/
        DESTINATION data
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

ELSE()

    IF(SILO_FOUND)
        SET(EXE_EXT "")
        SET(DATA_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
        SET(CMDS "")
        if(WIN32)
            set(EXE_EXT ".exe")
            set(DATA_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
            foreach(SD ${SILO_LIBRARY_DIR})
              foreach(SL ${SILO_LIB})
                get_filename_component(SL ${SL} NAME)
                if(EXISTS ${SD}/${SL})
                  get_filename_component(SL_WE ${SL} NAME_WE)
                  set(dllName "${SD}/${SL_WE}.dll")
                  if(NOT EXISTS ${dllName})
                      set(dllName "${SD}/../bin/${SL_WE}.dll")
                  endif()
                  if(EXISTS ${dllName})
                    foreach(CFG ${CMAKE_CONFIGURATION_TYPES})
                      file(COPY        ${dllName}
                           DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CFG})
                    endforeach()
                  endif()
                endif()
              endforeach()
            endforeach()
        endif()

        SET(testall_output  curv2d.silo
                            curv2d_colmajor.silo
                            curv3d.silo
                            curv3d_colmajor.silo
                            poly3d.silo
                            rect2d.silo
                            rect3d.silo
                            ucd2d.silo
                            ucd3d.silo
                            ucd_lines2d.silo
                            ucd_lines3d.silo)
        ADD_CUSTOM_COMMAND(OUTPUT  ${testall_output}
                           COMMAND "$<TARGET_FILE:testall>" ${SDA}
                           DEPENDS testall
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running testall")
        SET(globe_output globe.silo globe_mat0.silo globe_matcolors.silo)
        ADD_CUSTOM_COMMAND(OUTPUT  ${globe_output}
                           COMMAND "$<TARGET_FILE:globe>" ${SDA}
                           DEPENDS globe
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running globe")
        SET(correlationtest_output dbA00.pdb dbB00.pdb dbC00.pdb)
        ADD_CUSTOM_COMMAND(OUTPUT  ${correlationtest_output}
                           COMMAND "$<TARGET_FILE:correlationtest>" ${SDA}
                           DEPENDS correlationtest
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running correlationtest")
        SET(multi_test_output  multi_curv2d.silo
                               multi_curv3d.silo
                               multi_point2d.silo
                               multi_rect2d.silo
                               multi_rect3d.silo
                               multi_ucd3d.silo)
        ADD_CUSTOM_COMMAND(OUTPUT  ${multi_test_output}
                           COMMAND "$<TARGET_FILE:multi_test>" -noCycles -noTimeInvariantMultimesh ${SDA}
                           DEPENDS multi_test
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running multi_test")
        ADD_CUSTOM_COMMAND(OUTPUT  noise.silo
                           COMMAND "$<TARGET_FILE:noise>" ${SDA}
                           DEPENDS noise
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running noise")
        ADD_CUSTOM_COMMAND(OUTPUT  tire.silo
                           COMMAND "$<TARGET_FILE:tire>" ${SDA}
                           DEPENDS tire
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running tire")
        SET(all_example_output
            noise.silo tire.silo
            ${testall_output}
            ${globe_output}
            ${correlationtest_output}
            ${multi_test_output})
        ADD_CUSTOM_TARGET(create_data_examples ALL DEPENDS ${all_example_output})
        VISIT_DATA_ADD_FOLDER(datagen create_data_examples)

        # Create data install target
        if(NOT WIN32)
            STRING(REGEX REPLACE "([a-zA-Z0-9_\\.+]+)(;|$)" "${DATA_OUT_DIR}/\\1\\2" install_files "${all_example_output}")
        else()
            if(VISIT_NO_ALLBUILD)
                add_dependencies(visit_special_builds create_data_examples)
            endif()
            STRING(REGEX REPLACE "([a-zA-Z0-9_\\.+]+)(;|$)" "${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/\\1\\2" install_files "${all_example_output}")
        endif()
        INSTALL(FILES ${install_files} DESTINATION data
           PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)

        # Create a target to build visit_dist_data.tar.xz
        FILE(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build_visit_dist_data.cmake CONTENT "
            FILE(MAKE_DIRECTORY visit_dist_data)
            FILE(COPY ${all_example_output} DESTINATION visit_dist_data)
            EXECUTE_PROCESS(COMMAND ${TAR_CMD} ${TAR_ARGS} visit_dist_data.tar.xz visit_dist_data
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")
        ADD_CUSTOM_COMMAND(OUTPUT visit_dist_data.tar.xz
                           COMMAND ${CMAKE_COMMAND} -P build_visit_dist_data.cmake
                           DEPENDS "${all_example_output}"
                           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                           COMMENT "Generating visit_dist_data.tar.xz")
        ADD_CUSTOM_TARGET(visit_dist_data DEPENDS visit_dist_data.tar.xz)
        VISIT_DATA_ADD_FOLDER(datagen visit_dist_data)

    ELSE(SILO_FOUND)
        MESSAGE(STATUS "Data files not included since visit_dist_data.tar.xz")
        MESSAGE(STATUS "is missing and VisIt is not being built with Silo")
        MESSAGE(STATUS "support.")
    ENDIF(SILO_FOUND)
ENDIF()


