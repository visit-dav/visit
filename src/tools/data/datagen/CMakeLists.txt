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
#   Kathleen Biagas, Tue Jan 31, 2023
#   Clean up logic, modernize cmake usage, fix issues with some targets
#   on Windows.
#
#****************************************************************************

message(STATUS "Configuring VisIt Data Generation Targets")

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
function(BUILD_DATA_TARGET_IMPL target_name do_parallel)
    set(target_SOURCES "")
    set(target_INCLUDE_DIRS "")
    set(target_LIB_DIRS "")
    set(target_LIBS "")
    set(target_DEFS "")

    # cmake gymnastics to implement optional argument sections:
    foreach(ARG ${ARGN})
        if(${ARG} STREQUAL "INCLUDE_DIRS" OR
           ${ARG} STREQUAL "LIB_DIRS" OR
           ${ARG} STREQUAL "LIBS" OR
           ${ARG} STREQUAL "DEFS")
            #switch var name to extend
            set(ARG_VAR target_${ARG})
        else()
            # add current arg into current varname
            set("${ARG_VAR}" ${${ARG_VAR}} ${ARG})
        endif()
    endforeach()

    # Automatically setup target_SOURCES if not set
    if(DEFINED ${target_name}_SOURCES)
        set(target_SOURCES "${${target_name}_SOURCES}")
    else()
        set(target_SOURCES ${target_name}.C)
    endif()

    set(tname ${target_name})
    if("${do_parallel}" STREQUAL "ON")
        if(VISIT_PARALLEL)
            set(tname "${target_name}_par")
            ADD_PARALLEL_EXECUTABLE(${tname} ${target_SOURCES})
        endif()
    else()
        add_executable(${tname} ${target_SOURCES})
    endif()
    target_include_directories(${tname} PRIVATE ${target_INCLUDE_DIRS})
    target_link_directories(${tname} PRIVATE ${target_LIB_DIRS})
    VISIT_DATA_ADD_FOLDER(datagen ${tname})

    target_compile_definitions(${tname} PRIVATE ${target_DEFS})
    target_link_libraries(${tname} PRIVATE ${target_LIBS})
    set_target_properties(${tname} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

function(BUILD_VISIT_DIST_DATA)
    file(MAKE_DIRECTORY visit_dist_data)
    if(WIN32)
        file(GLOB vdd_files "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFGINT_DIR}/*.{silo,pdb}" CONFIGURE_DEPENDS)
        file(COPY ${vdd_files} DESTINATION visit_dist_data )
    else()
        file(COPY dummy
             DESTINATION visit_dist_data
             FILES_MATCHING PATTERN "*.{silo,pdb}")
    endif()
    set(DIST_TAR_CMD "${TAR_CMD} ${TAR_ARGS} visit_dist_data.tar.xz visit_dist_data")
    execute_process(COMMAND ${DIST_TAR_CMD}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction()

function(BUILD_DATA_TARGET target_name)
    BUILD_DATA_TARGET_IMPL(${target_name} "OFF" ${ARGN})
endfunction()

function(BUILD_PARALLEL_DATA_TARGET target_name)
    BUILD_DATA_TARGET_IMPL(${target_name} "ON" ${ARGN})
endfunction()

#-----------------------------------------------------------------------------
# Data Generation Program Targets
#
# This are build as part of the default target.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Build all Basic Targets
#-----------------------------------------------------------------------------
foreach(BASIC_TARGET bov curve_builder distribution_curves mandelbulb
                     xdmf_binary)
     BUILD_DATA_TARGET(${BASIC_TARGET}
         INCLUDE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${VISIT_BINARY_DIR}/include
         ${VISIT_SOURCE_DIR}/include)
endforeach()

foreach(BASIC_TARGET mandelbulb)
     BUILD_PARALLEL_DATA_TARGET(${BASIC_TARGET}
         INCLUDE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${VISIT_BINARY_DIR}/include
         ${VISIT_SOURCE_DIR}/include)
endforeach()

#-----------------------------------------------------------------------------
# Build all VTK Targets
#-----------------------------------------------------------------------------
foreach(VTK_TARGET degen_meshes)
    if(VTK_VERSION VERSION_EQUAL "8.1.0")
        BUILD_DATA_TARGET(${VTK_TARGET}
            INCLUDE_DIRS ${VTK_INCLUDE_DIRS}
            LIB_DIRS ${VTK_LIBRARY_DIRS}
            LIBS vtkIOLegacy)
    else()
        BUILD_DATA_TARGET(${VTK_TARGET}
            LIBS VTK::IOLegacy)
    endif()
endforeach()

#-----------------------------------------------------------------------------
# If we have HDF5 build all HDF5 Data Gen Target Programs
#-----------------------------------------------------------------------------
if(HDF5_FOUND)
    foreach(HDF5_TARGET unic xdmf xdmf2d)
        BUILD_DATA_TARGET(${HDF5_TARGET}
            INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${HDF5_INCLUDE_DIR}
            LIB_DIRS ${HDF5_LIBRARY_DIR}
            LIBS ${HDF5_LIB})
    endforeach()
endif()

#-----------------------------------------------------------------------------
# If we have Silo build all Silo & PDB Data Gen Programs
#-----------------------------------------------------------------------------
if(SILO_FOUND)
    #-------------------------------------------------------------------------
    #  Setup {TARGET}_SOURCES for targets with multiple source files.
    #-------------------------------------------------------------------------
    set(correlationtest_SOURCES correlationtest.C pdbdatabase_link.C)
    set(pdbdatabase_SOURCES pdbdatabase.C pdbdatabase_link.C)
    set(streak_SOURCES streak.C pdbdatabase_link.C)
    # Build PDB Targets (The required pdb symbols live in Silo's libs.)
    foreach(PDB_TARGET correlationtest pdbdatabase streak)
        BUILD_DATA_TARGET(${PDB_TARGET}
            INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                         ${VISIT_BINARY_DIR}/include
                         ${VISIT_SOURCE_DIR}/include
                         ${VISIT_SOURCE_DIR}/common/utility
                         ${SILO_INCLUDE_DIR}
            LIB_DIRS ${PDB_LIBRARY_DIR}
            LIBS ${PDB_LIB})
    endforeach()
    #-------------------------------------------------------------------------
    #  Setup {TARGET}_SOURCES for targets with multiple source files.
    #-------------------------------------------------------------------------
    set(noise_SOURCES noise.C QuadMesh.C)
    set(multidir_SOURCES multidir.C QuadMesh.C)
    # Build Silo Targets
    foreach (SILO_TARGET arbpoly bigsil bounce big_quads csg emptydomains
                         fullframe galaxy ghost global_node globe largefile
                         lineout_test lowfrac mat_3x3_3across mat_3x3x3_3across
                         meshorigin mmadj_rect_2d multi_test multidir noise
                         noise2d non_conforming one_quad quad_disk sid97
                         specmix specmix_double testall thinplane tire wave)
        BUILD_DATA_TARGET(${SILO_TARGET}
            INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                         ${SILO_INCLUDE_DIR}
                         ${VISIT_SOURCE_DIR}/common/utility
            LIB_DIRS ${SILO_LIBRARY_DIR}
            LIBS ${SILO_LIB})
    endforeach()
    if(WIN32)
      set_target_properties(multi_test PROPERTIES LINK_FLAGS "/STACK:8000000")
    endif()
endif()

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
find_program(TAR_EXE tar)
if(TAR_EXE AND NOT WIN32)
    set(TAR_CMD ${TAR_EXE})
    set(UNTAR_ARGS "xvf")
    set(TAR_ARGS "cvfJ")
else()
    set(TAR_CMD ${CMAKE_COMMAND})
    set(UNTAR_ARGS "-E tar xvf")
    set(TAR_ARGS "-E tar cvfJ")
endif()

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

if(USE_VISIT_DIST_DATA_TARBALL)
    set(DIST_UNTAR_CMD "${TAR_CMD} ${UNTAR_ARGS} ${VISIT_SOURCE_DIR}/../data/visit_dist_data.tar.xz")
    install(CODE "execute_process(COMMAND ${DIST_UNTAR_CMD} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )")
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/visit_dist_data/
        DESTINATION data
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)

else()
    if(SILO_FOUND)
        set(DATA_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
        set(CMDS "")
        if(WIN32)
            set(DATA_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
        endif()

        set(testall_output  curv2d.silo
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
        add_custom_command(OUTPUT  ${testall_output}
                           COMMAND "$<TARGET_FILE:testall>" ${SDA}
                           DEPENDS testall
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running testall")
        set(globe_output globe.silo globe_mat0.silo globe_matcolors.silo)
        add_custom_command(OUTPUT  ${globe_output}
                           COMMAND "$<TARGET_FILE:globe>" ${SDA}
                           DEPENDS globe
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running globe")
        set(correlationtest_output dbA00.pdb dbB00.pdb dbC00.pdb)
        add_custom_command(OUTPUT  ${correlationtest_output}
                           COMMAND "$<TARGET_FILE:correlationtest>" ${SDA}
                           DEPENDS correlationtest
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running correlationtest")
        set(multi_test_output  multi_curv2d.silo
                               multi_curv3d.silo
                               multi_point2d.silo
                               multi_rect2d.silo
                               multi_rect3d.silo
                               multi_ucd3d.silo)
        add_custom_command(OUTPUT  ${multi_test_output}
                           COMMAND "$<TARGET_FILE:multi_test>" -noCycles -noTimeInvariantMultimesh ${SDA}
                           DEPENDS multi_test
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running multi_test")
        add_custom_command(OUTPUT  noise.silo
                           COMMAND "$<TARGET_FILE:noise>" ${SDA}
                           DEPENDS noise
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running noise")
        add_custom_command(OUTPUT  tire.silo
                           COMMAND "$<TARGET_FILE:tire>" ${SDA}
                           DEPENDS tire
                           WORKING_DIRECTORY ${DATA_OUT_DIR}
                           COMMENT "Running tire")
        set(all_example_output
            noise.silo tire.silo
            ${testall_output}
            ${globe_output}
            ${correlationtest_output}
            ${multi_test_output})
        add_custom_target(create_data_examples ALL DEPENDS ${all_example_output})
        if(WIN32)
            # copy silo and dependencies so that the data generators will run
            foreach(SD ${SILO_LIBRARY_DIR})
                foreach(SL ${SILO_LIB})
                    get_filename_component(SL ${SL} NAME)
                    if(EXISTS ${SD}/${SL})
                        get_filename_component(SL_WE ${SL} NAME_WE)
                        set(dllName "${SD}/${SL_WE}.dll")
                        if(NOT EXISTS ${dllName})
                            set(dllName "${SD}/../bin/${SL_WE}.dll")
                            file(REAL_PATH ${dllName} dllName)
                        endif()
                        if(EXISTS ${dllName})
                            list(APPEND SILO_LIBS ${dllName})
                        endif()
                    endif()
                endforeach()
            endforeach()
            add_custom_command(TARGET create_data_examples PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${SILO_LIBS} ${DATA_OUT_DIR})
        endif()

        # Create data install target
        if(NOT WIN32)
            string(REGEX REPLACE "([a-zA-Z0-9_\\.+]+)(;|$)" "${DATA_OUT_DIR}/\\1\\2" install_files "${all_example_output}")
        else()
            string(REGEX REPLACE "([a-zA-Z0-9_\\.+]+)(;|$)" "${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/\\1\\2" install_files "${all_example_output}")
            visit_add_to_util_builds(create_data_examples)
        endif()
        install(FILES ${install_files} DESTINATION data
           PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)

        # Create a target to build visit_dist_data.tar.xz
        file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build_visit_dist_data.cmake CONTENT "
            file(MAKE_DIRECTORY visit_dist_data)
            file(COPY ${all_example_output} DESTINATION visit_dist_data)
            execute_process(COMMAND ${TAR_CMD} ${TAR_ARGS} visit_dist_data.tar.xz visit_dist_data
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")
        add_custom_command(OUTPUT visit_dist_data.tar.xz
                           COMMAND ${CMAKE_COMMAND} -P build_visit_dist_data.cmake
                           DEPENDS "${all_example_output}"
                           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                           COMMENT "Generating visit_dist_data.tar.xz")
        add_custom_target(visit_dist_data DEPENDS visit_dist_data.tar.xz)
        VISIT_DATA_ADD_FOLDER(datagen visit_dist_data)

    else()
        message(STATUS "Data files not included since visit_dist_data.tar.xz is missing")
        message(STATUS "and VisIt is not being built with Silo support.")
    endif()
endif()

