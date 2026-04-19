# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
#  Modifications:
#    Eric Brugger, Mon Sep 22 16:41:58 PDT 2025
#    I changed the logic to create the INSTALL_RPATH for parallel
#    executables to generate a semicolon separated list instead of a
#    space separated list.
#
#****************************************************************************

function(ADD_PARALLEL_EXECUTABLE target)
    add_executable(${target} ${ARGN})
     if(UNIX AND VISIT_PARALLEL_RPATH)
         set(PAR_RPATHS "")
         foreach(X ${CMAKE_INSTALL_RPATH})
             list(APPEND PAR_RPATHS ${X})
         endforeach()
         foreach(X ${VISIT_PARALLEL_RPATH})
             list(APPEND PAR_RPATHS ${X})
         endforeach()
         set_target_properties(${target} PROPERTIES
                INSTALL_RPATH "${PAR_RPATHS}")
    endif()

    if(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES AND TARGET MPI::MPI_C)
        target_link_libraries(${target} MPI::MPI_C)
    endif()
    if(VISIT_PARALLEL_DEFINES)
        target_compile_definitions(${target} PUBLIC ${VISIT_PARALLEL_DEFINES})
    endif()

endfunction()

function(ADD_PARALLEL_FORTRAN_EXECUTABLE target)
    add_executable(${target} ${ARGN})

    if(VISIT_PARALLEL_RPATH)
        set(PAR_RPATHS "")
        foreach(X ${CMAKE_INSTALL_RPATH})
            list(APPEND PAR_RPATHS ${X})
        endforeach()
        foreach(X ${VISIT_PARALLEL_RPATH})
            list(APPEND PAR_RPATHS ${X})
        endforeach()
        set_target_properties(${target} PROPERTIES
                    INSTALL_RPATH ${PAR_RPATHS})
    endif()
    target_link_libraries(${target} ${VISIT_EXE_LINKER_FLAGS} MPI::MPI_FORTRAN) 
endfunction()

function(PARALLEL_EXECUTABLE_LINK_LIBRARIES target)
    if(VISIT_NOLINK_MPI_WITH_LIBRARIES)
        target_link_libraries(${target} ${ARGN} link_mpi_libs)
    else()
        if(VISIT_STATIC)
            target_link_libraries(${target} ${ARGN} ${VISIT_PARALLEL_LIBS})
        else()
            target_link_libraries(${target} ${ARGN})
        endif()
    endif()
endfunction()




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


