# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Modifications:
#   Kathleen Biagas, Thu Feb 20, 2025
#   Removed ADD_PARALLEL_FORTRAN_EXECUTABLE target as it was only being used
#   by sim examples. Functionality replicated there.
#

set(VISIT_PARALLEL_DEFINES "PARALLEL;MPICH_IGNORE_CXX_SEEK;MPICH_SKIP_MPICXX;OMPI_SKIP_MPICXX;MPI_NO_CPPBIND" CACHE STRING "Parallel compiler defines")

set(errmsg "")
if(VISIT_MPI_COMPILER)
    set(MPI_COMPILER ${VISIT_MPI_COMPILER})
    set(errmsg "Using VISIT_MPI_COMPILER: ${VISIT_MPI_COMPILER}")
else(VISIT_MPI_HOME)
    set(MPI_HOME ${VISIT_MPI_HOME})
    set(MPI_DIR ${VISIT_MPI_DIR})
    set(errmsg "Using VISIT_MPI_HOME: ${VISIT_MPI_HOM}")
endif()

# do we still need this?
iF(VISIT_FORTRAN AND VISIT_MPI_FORTRAN_COMPILER)
   set(MPI_Fortran_COMPILER ${VISIT_MPI_FORTRAN_COMPILER})
endif()

find_package(MPI)


if(MPI_C_FOUND)
    message(STATUS "MPI_C_COMPILER:            ${MPI_C_COMPILER}")
    message(STATUS "MPI_C_COMPILE_OPTIONS:     ${MPI_C_COMPILE_OPTIONS}")
    message(STATUS "MPI_C_COMPILE_DEFINITIONS: ${MPI_C_COMPILE_DEFINITIONS}")
    message(STATUS "MPI_C_INCLUDE_DIRS:        ${MPI_C_INCLUDE_DIRS}")
    message(STATUS "MPI_C_LINK_FLAGS:          ${MPI_C_LINK_FLAGS}")
    message(STATUS "MPI_C_LIBRARIES:           ${MPI_C_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find MPI ${errmsg}")
endif()

# is this still needed?
if(VISIT_FORTRAN AND VISIT_MPI_FORTRAN_COMPILER)
    if(MPI_Fortran_FOUND)
        message(STATUS "MPI_Fortran_COMPILER:            ${MPI_Fortran_COMPILE}")
        message(STATUS "MPI_Fortran_COMPILE_OPTIONS:     ${MPI_Fortran_COMPILE_OPTIONS}")
        message(STATUS "MPI_Fortran_COMPILE_DEFINITIONS: ${MPI_Fortran_COMPILE_OPTIONS}")
        message(STATUS "MPI_Fortran_INCLUDE_DIRS:        ${MPI_Fortran_INCLUDE_DIRS}")
        message(STATUS "MPI_Fortran_LINK_FLAGS:          ${MPI_Fortran_LINK_FLAGS}")
        message(STATUS "MPI_Fortran_LIBRARIES:           ${MPI_Fortran_LIBRARIES}")
    else()
        message(FATAL_ERROR "Could not find Fortran MPI using VISIT_FORTRAN_MPI_COMPILER: ${VISIT_MPI_FORTRAN_COMPILER}")
    endif()
endif()

# make ;-separated lists, once now instead of per every parallel target added
if(VISIT_PARALLEL_RPATH)
    string(REPLACE " " ";" VISIT_PARALLEL_RPATH ${VISIT_PARALLEL_RPATH})
endif()
if(CMAKE_INSTALL_RPATH)
    string(REPLACE " " ";" CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH})
endif()

