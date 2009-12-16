#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/
cmake_minimum_required(VERSION 2.6)

#-----------------------------------------------------------------------------
# If this directory exists the root CMakeLists.txt adds this directroy as a 
# subdirectory to the main VisIt project.
#
# Prevent users from running cmake directy in this directory.
#-----------------------------------------------------------------------------
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(FATAL_ERROR "VisIt's \"data\" directory cannot be configured "
                    "independently from the main \"src\" directory. Please "
                    "run cmake on VisIt's \"src\" directory.")
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")


MESSAGE(STATUS "Configuring VisIt Data Targets")

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
#
# Source files are detected in two ways:
#  If ${target_name}_SOURCES is set:  its contents are used.
#  Otherwise: ${target_name}.C is used.
#
#-----------------------------------------------------------------------------
FUNCTION(BUILD_DATA_TARGET target_name)
    SET(target_SOURCES "")
    SET(target_INCLUDE_DIRS "")
    SET(target_LIB_DIRS "")
    SET(target_LIBS "")

    # cmake gymnastics to implement optional argument sections:
    FOREACH(ARG ${ARGN})
        IF("${ARG}" STREQUAL "INCLUDE_DIRS" OR
           "${ARG}" STREQUAL "LIB_DIRS" OR
           "${ARG}" STREQUAL "LIBS")
            #switch var name to extend
            SET(ARG_VAR target_${ARG})
        ELSE("${ARG}" STREQUAL "INCLUDE_DIRS" OR
             "${ARG}" STREQUAL "LIB_DIRS" OR
             "${ARG}" STREQUAL "LIBS")
            # add current arg into current varname
            SET("${ARG_VAR}" ${${ARG_VAR}} ${ARG})
        ENDIF("${ARG}" STREQUAL "INCLUDE_DIRS" OR
              "${ARG}" STREQUAL "LIB_DIRS" OR
              "${ARG}" STREQUAL "LIBS")
    ENDFOREACH(ARG ${ARGN})

    INCLUDE_DIRECTORIES(${target_INCLUDE_DIRS})
    LINK_DIRECTORIES(${target_LIB_DIRS})

    # Automatically setup target_SOURCES if not set
    IF(DEFINED ${target_name}_SOURCES)
        SET(target_SOURCES "${${target_name}_SOURCES}")
    ELSE(DEFINED ${target_name}_SOURCES)
        SET(target_SOURCES ${target_name}.C)
    ENDIF(DEFINED ${target_name}_SOURCES)

    ADD_EXECUTABLE(${target_name} ${target_SOURCES})
    TARGET_LINK_LIBRARIES(${target_name} ${target_LIBS})
    SET_TARGET_PROPERTIES(${target_name}
                           PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                          )

ENDFUNCTION(BUILD_DATA_TARGET target_name)


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
)

#-----------------------------------------------------------------------------
# VTK Gen Programs
#-----------------------------------------------------------------------------
SET(VTK_TARGETS degen_meshes)

#-----------------------------------------------------------------------------
# HDF5 Data Gen Programs
#-----------------------------------------------------------------------------
SET(HDF5_TARGETS xdmf)

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
one_quad
quad_disk
sid97
specmix
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
    BUILD_DATA_TARGET(${BASIC_TARGET})
ENDFOREACH(BASIC_TARGET ${BASIC_TARGETS})

#-----------------------------------------------------------------------------
# Build all VTK Targets
#-----------------------------------------------------------------------------
FOREACH(VTK_TARGET ${VTK_TARGETS})
BUILD_DATA_TARGET(${VTK_TARGET}
                  INCLUDE_DIRS
                    ${VTK_INCLUDE_DIRS}
                  LIB_DIRS
                    ${VTK_LIBRARY_DIRS}
                  LIBS
                    vtkIO
                 )
ENDFOREACH(VTK_TARGET ${VTK_TARGETS})

#-----------------------------------------------------------------------------
# If we have HDF5 build all HDF5 Data Gen Target Programs
#-----------------------------------------------------------------------------
IF(HDF5_FOUND)
FOREACH(HDF5_TARGET ${HDF5_TARGETS})
    BUILD_DATA_TARGET(${HDF5_TARGET}
                      INCLUDE_DIRS
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${HDF5_INCLUDE_DIR}
                      LIB_DIRS
                        ${HDF5_LIBRARY_DIR}
                      LIBS
                        ${HDF5_LIB}
                     )
ENDFOREACH(HDF5_TARGET ${HDF5_TARGETS})
ENDIF(HDF5_FOUND)

#-----------------------------------------------------------------------------
# If we have Silo build all Silo & PDB Data Gen Programs
#-----------------------------------------------------------------------------
IF(SILO_FOUND)
    # Build PDB Targets (The required pbd symbols live in Silo's libs.)
    FOREACH(PDB_TARGET ${PDB_TARGETS})
        BUILD_DATA_TARGET(${PDB_TARGET}
                          INCLUDE_DIRS
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${VISIT_BINARY_DIR}/include
                            ${VISIT_SOURCE_DIR}/include
                          LIB_DIRS
                            ${PDB_LIBRARY_DIR}
                        LIBS
                            ${PDB_LIB}
                        )
    ENDFOREACH(PDB_TARGET ${PDB_TARGETS})
    # Build Silo Targets
    FOREACH (SILO_TARGET ${SILO_TARGETS})
        BUILD_DATA_TARGET(${SILO_TARGET}
                          INCLUDE_DIRS
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${SILO_INCLUDE_DIR}
                            ${VISIT_SOURCE_DIR}/common/utility
                        LIB_DIRS
                            ${SILO_LIBRARY_DIR}
                        LIBS
                            ${SILO_LIB}
                        )
    ENDFOREACH(SILO_TARGET ${SILO_TARGETS})
ENDIF(SILO_FOUND)

#-----------------------------------------------------------------------------
# Setup the Data Generation Target.
#-----------------------------------------------------------------------------
# We have a single target "data" that generates all of our test data.
# This untars all archived datasets.
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# Discover the proper untar command.
#
# "cmake -E tar" supports untaring for normal & gzipped tarballs.
# Unfortunately it is very slow so we only use it as a fallback.
#
#-----------------------------------------------------------------------------
FIND_PROGRAM(TAR_EXE tar)
IF(NOT "${TAR_EXE}" STREQUAL "TAR_EXE-NOTFOUND")
    SET(UNTAR_CMD ${TAR_EXE})
    SET(UNTAR_ARGS "-xzvf")
ELSE(NOT "${TAR_EXE}" STREQUAL "TAR_EXE-NOTFOUND")
    SET(UNTAR_CMD "cmake")
    SET(UNTAR_ARGS "-E tar -xzvf")
ENDIF(NOT "${TAR_EXE}" STREQUAL "TAR_EXE-NOTFOUND")


#-----------------------------------------------------------------------------
# Create the list of archived data targets.
#
# Each archived target extracts data into a directory matching the root name
# of the source tarball. This makes tracking the dependencies simple.
#
#-----------------------------------------------------------------------------
SET(ARCHIVED_TARGETS
ANALYZE_test_data
ANSYS_test_data
Adventure_test_data
CGNS_test_data
Cale_test_data
Chombo_test_data
EnSight_test_data
Enzo_test_data
FITS_test_data
FLASH_test_data
Fluent_test_data
GDAL_test_data
GGCM_test_data
Image_test_data
KullLite_test_data
NASTRAN_test_data
OpenFOAM_test_data
PATRAN_test_data
ProteinDataBank_test_data
RAW_test_data
Rect_test_data
Vista_test_data
bov_test_data
boxlib_test_data
chgcar_test_data
claw_test_data
csg_test_data
curve_test_data
ddcmd_test_data
exodus_test_data
h5nimrod_test_data
h5part_test_data
iTaps_test_data
lines_test_data
mili_test_data
miranda_test_data
nek3d_test_data
netcdf_test_data
okc_test_data
old_silo_test_data
overlink_test_data
pdb_test_data
pixie_test_data
sami_test_data
samrai_test_data
shapefile_test_data
silo_amr_test_data
silo_hdf5_test_data
silo_pdb_test_data
tecplot_test_data
visitdiff_test_data
visit_dist_data
visus_test_data
vtk_cylindrical_test_data
vtk_test_data
vtk_xml_test_data
xdmf_test_data
xyz_test_data
zipwrapper_test_data
)



#-----------------------------------------------------------------------------
# Add a custom command to extract each archived dataset & collect
# output directory names in a dependency list.
#-----------------------------------------------------------------------------
SET(DATA_TARGET_DEPENDS "")

FOREACH(DATASET_TARGET ${ARCHIVED_TARGETS})
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DATASET_TARGET}
                       COMMAND ${UNTAR_CMD}
                       ARGS ${UNTAR_ARGS} ${CMAKE_CURRENT_SOURCE_DIR}/${DATASET_TARGET}.tar.gz
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      )
    SET( DATA_TARGET_DEPENDS ${DATA_TARGET_DEPENDS} ${CMAKE_CURRENT_BINARY_DIR}/${DATASET_TARGET})
ENDFOREACH(DATASET_TARGET IN ${ARCHIVED_DATASETS})

#-----------------------------------------------------------------------------
# Add custom targets: "data" & "test"
#-----------------------------------------------------------------------------
ADD_CUSTOM_TARGET(data DEPENDS ${DATA_TARGET_DEPENDS})
ADD_CUSTOM_TARGET(test)
ADD_DEPENDENCIES(test data)

#-----------------------------------------------------------------------------
# Add install command for contents of "visit_dist_data"
#-----------------------------------------------------------------------------

SET(DIST_UNTAR_CMD "${UNTAR_CMD} ${UNTAR_ARGS} ${CMAKE_CURRENT_SOURCE_DIR}/visit_dist_data.tar.gz")

INSTALL(CODE "execute_process(COMMAND ${DIST_UNTAR_CMD} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )")

INSTALL(DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/visit_dist_data/
        DESTINATION
            data
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)


