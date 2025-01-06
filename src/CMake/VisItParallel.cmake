# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


function(DETECT_MPI_SETTINGS COMP mlibs mflags mlflags mrpath)
    # Unset any variables that may have been set before by FindMPI
    unset(MPI_FOUND CACHE)
    unset(MPI_INCLUDE_PATH CACHE)
    unset(MPI_LIB CACHE)
    unset(MPI_COMPILE_FLAGS CACHE)
    unset(MPI_LIBRARIES CACHE)
    unset(MPI_LIBRARY CACHE)
    unset(MPI_EXTRA_LIBRARY CACHE)
    unset(MPI_LINK_FLAGS CACHE)
    unset(MPI_COMPILER CACHE)
    unset(MPI_RPATH CACHE)

    # Set the compiler and call FindMPI
    set(MPI_COMPILER ${COMP})
    include(${CMAKE_ROOT}/Modules/FindMPI.cmake)

    # Return the values
    if(NOT MPI_FOUND)
        message(FATAL_ERROR  "Failed to setup MPI using compiler wrapper: ${COMP}")
    else()
        # Take the MPI include path and split it into -I's
        set(MPI_INCLUDE_PATH_CONV "")
        foreach(I ${MPI_INCLUDE_PATH})
            set(MPI_INCLUDE_PATH_CONV "${MPI_INCLUDE_PATH_CONV} -I${I}")
        endforeach()

        set(${mlibs}   ${MPI_LIBRARIES} CACHE STRING "MPI libraries")
        set(${mflags}  "${MPI_INCLUDE_PATH_CONV} ${MPI_COMPILE_FLAGS}" CACHE STRING "Parallel compiler flags")
        set(${mlflags} "${MPI_LINK_FLAGS}" CACHE STRING "Parallel linker flags")

        #
        # Detect all mpi library paths, we need these to keep the RPATH intact
        # for installs that link to MPI.
        #
        set(MPI_RPATH "")
        foreach(MLIB ${MPI_LIBRARIES})
            get_filename_component(MLIB_PATH ${MLIB} PATH)
            # make sure it is not an implicit path - we need to skip these
            list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${MLIB_PATH}" pidx)
            if("${pidx}" EQUAL -1)
                list(APPEND MPI_RPATH ${MLIB_PATH})
            endif()
        endforeach(MLIB ${MPI_LIBRARIES})
        list(REMOVE_DUPLICATES MPI_RPATH)
        set(${mrpath} "${MPI_RPATH}" CACHE STRING "Parallel rpath(s)")
    endif()

endfunction()


function(ADD_PARALLEL_EXECUTABLE target)
    add_executable(${target} ${ARGN})

    if(UNIX)
      if(VISIT_PARALLEL_CXXFLAGS)
        set_target_properties(${target} PROPERTIES
            COMPILE_FLAGS ${VISIT_PARALLEL_CXXFLAGS})
      endif()
      target_compile_definitions(${target} PRIVATE ${VISIT_PARALLEL_DEFINES})

      if(VISIT_PARALLEL_LINK_FLAGS)
          set_target_properties(${target} PROPERTIES
                LINK_FLAGS ${VISIT_PARALLEL_LINK_FLAGS})
      endif()
      if(VISIT_PARALLEL_LINK_DIRS)
          set_target_properties(${target} PROPERTIES
                LINK_DIRECTORIES ${VISIT_PARALLEL_LINK_DIRS})
      endif()

      if(VISIT_PARALLEL_RPATH)
          set_target_properties(${target} PROPERTIES
              INSTALL_RPATH "${VISIT_PARALLEL_RPATH};${CMAKE_INSTALL_RPATH}") 
      endif()
    else()
        target_include_directories(${target} PRIVATE ${VISIT_PARALLEL_INCLUDE})
        target_compile_definitions(${target} PRIVATE ${VISIT_PARALLEL_DEFINES})
        target_link_libraries(${target} PRIVATE ${VISIT_PARALLEL_LINK_LIBS})
    endif()

    # If we're on doing this "nolink mpi" option, we rely on the
    # PARALLEL_TARGET_LINK_LIBRARIES function to actually link the
    # target with MPI.
    if(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES) # This is a new if test
        target_link_libraries(${target} ${VISIT_EXE_LINKER_FLAGS} ${VISIT_PARALLEL_LIBS})
    endif()
endfunction()

function(ADD_PARALLEL_FORTRAN_EXECUTABLE target)
    add_executable(${target} ${ARGN})
    if(VISIT_PARALLEL_FORTRAN_FLAGS)
        set(PAR_COMPILE_FLAGS "")
        foreach(X ${VISIT_PARALLEL_FORTRAN_FLAGS})
            set(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        endforeach()
        set_target_properties(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS})
        target_compile_definitions(${target} PRIVATE ${VISIT_PARALLEL_DEFINES})

        if(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS)
            set(PAR_LINK_FLAGS "")
            foreach(X ${VISIT_PARALLEL_FORTRAN_LINKER_FLAGS})
                set(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            endforeach()
            set_target_properties(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
            if(VISIT_PARALLEL_RPATH)
                set(PAR_RPATHS "")
                foreach(X ${CMAKE_INSTALL_RPATH})
                    list(APPEND PAR_RPATHS ${X})
                endforeach()
                foreach(X ${VISIT_PARALLEL_RPATH})
                    list(APPEND PAR_RPATHS ${X})
                endforeach()
                set_target_properties(${target} PROPERTIES
                    INSTALL_RPATH ${PAR_RPATHS}
                )
            endif()
        endif()
        target_link_libraries(${target} ${VISIT_EXE_LINKER_FLAGS} ${VISIT_PARALLEL_FORTRAN_LIBS})
    endif()
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

if(VISIT_MPI_COMPILER)
    message(STATUS "Setting up MPI using compiler wrapper")

    # Detect the MPI settings that C++ wants
    DETECT_MPI_SETTINGS(${VISIT_MPI_COMPILER}
        VISIT_PARALLEL_LIBS
        VISIT_PARALLEL_CFLAGS
        VISIT_PARALLEL_LINKER_FLAGS
        VISIT_PARALLEL_RPATH)

    set(VISIT_PARALLEL_CXXFLAGS ${VISIT_PARALLEL_CFLAGS} CACHE STRING "Parallel CXXFLAGS")

    # Detect the MPI settings that Fortran wants
    if(VISIT_FORTRAN AND VISIT_MPI_FORTRAN_COMPILER)
        DETECT_MPI_SETTINGS(${VISIT_MPI_FORTRAN_COMPILER}
            VISIT_PARALLEL_FORTRAN_LIBS
            VISIT_PARALLEL_FORTRAN_DEFS
            VISIT_PARALLEL_FORTRAN_FLAGS
            VISIT_PARALLEL_FORTRAN_LINKER_FLAGS
            VISIT_PARALLEL_RPATH
            )
    endif()

else()
    if(VISIT_MPI_LIBS OR VISIT_MPI_FORTRAN_LIBS)
        message(STATUS "Setting up MPI using user defined flags")
        set(VISIT_PARALLEL_LIBS          ${VISIT_MPI_LIBS}
            CACHE STRING "MPI libraries")
        set(VISIT_PARALLEL_FORTRAN_LIBS  ${VISIT_MPI_FORTRAN_LIBS}
            CACHE STRING "MPI libraries for Fortran")

        if(VISIT_MPI_C_FLAGS)
            set(VISIT_PARALLEL_CFLAGS  "${VISIT_MPI_C_FLAGS}"
                CACHE STRING "Parallel CFLAGS")
        else()
            set(VISIT_PARALLEL_CFLAGS  ""
                CACHE STRING "Parallel CFLAGS")
        endif()

        if(VISIT_MPI_CXX_FLAGS)
            set(VISIT_PARALLEL_CXXFLAGS "${VISIT_MPI_CXX_FLAGS}"
                CACHE STRING "Parallel CXXFLAGS")
        else()
            set(VISIT_PARALLEL_CXXFLAGS  ""
                CACHE STRING "Parallel CXXFLAGS")
        endif()

        if(VISIT_MPI_FORTRAN_FLAGS)
            set(VISIT_PARALLEL_FORTRAN_FLAGS "${VISIT_MPI_FORTRAN_FLAGS}"
                CACHE STRING "Parallel Fortran flags")
        else()
            set(VISIT_PARALLEL_FORTRAN_FLAGS  ""
                CACHE STRING "Parallel flags for Fortran")
        endif()

        if(VISIT_MPI_LD_FLAGS)
            set(VISIT_PARALLEL_LINKER_FLAGS "${VISIT_MPI_LD_FLAGS}"
                CACHE STRING "Parallel LDFLAGS")
            set(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS "${VISIT_MPI_LD_FLAGS}"
                CACHE STRING "Parallel LDFLAGS")
        else()
            set(VISIT_PARALLEL_LINKER_FLAGS ""
                CACHE STRING "Parallel LDFLAGS")
            set(VISIT_PARALLEL_FORTRAN_LINKER_FLAGS ""
                CACHE STRING "Parallel LDFLAGS")
        endif()
    else()
        if(WIN32)
          if(NOT MPI_FOUND)
            include(${CMAKE_ROOT}/Modules/FindMPI.cmake)
            if(MPI_FOUND)
              set(VISIT_PARALLEL_LIBS "${MPI_LIBRARY}"
                  CACHE STRING "MPI libraries")
              set(VISIT_PARALLEL_INCLUDE "${MPI_INCLUDE_PATH}"
                  CACHE STRING "MPI include dir")
              if(MPI_EXTRA_LIBRARY)
                   list(APPEND VISIT_PARALLEL_LIBS "${MPI_EXTRA_LIBRARY}")
              endif()

              if(MPI_COMPILE_FLAGS)
                  set(VISIT_PARALLEL_CFLAGS "${MPI_COMPILE_FLAGS}"
                      CACHE STRING "Parallel CFLAGS")
                  set(VISIT_PARALLEL_CXXFLAGS "${MPI_COMPILE_FLAGS}"
                      CACHE STRING "Parallel CXXFLAGS")
              endif()
              if(MPI_LINK_FLAGS)
                  set(VISIT_PARALLEL_LINKER_FLAGS "${MPI_LINK_FLAGS}"
                      CACHE STRING "Parallel LDFLAGS")
              endif()
              find_file(HAVE_HPC_SCHEDULER "Microsoft.Hpc.Scheduler.tlb")
            else()
              message(FATAL_ERROR "To build parallel VisIt, you must at a "
                      "minimum define VISIT_MPI_COMPILER or VISIT_MPI_LIBS. "
                      "You may also define VISIT_MPI_LD_FLAGS, "
                      "VISIT_MPI_CXX_FLAGS." )
            endif()
          endif()
        else()
            message(FATAL_ERROR "To build parallel VisIt, you must at a "
                    "minimum define VISIT_MPI_COMPILER or VISIT_MPI_LIBS. "
                    "You may also define VISIT_MPI_LD_FLAGS, "
                    "VISIT_MPI_CXX_FLAGS." )
        endif()

    endif()
endif()

# prepare these for direct use in cmake calls
if(VISIT_PARALLEL_CXXFLAGS)
    string(REPLACE " " ";" VISIT_PARALLEL_CXXFLAGS ${VISIT_PARALLEL_CXXFLAGS})
endif()
if(VISIT_PARALLEL_LINKER_FLAGS)
    string(REPLACE " " ";" VISIT_PARALLEL_LINKER_FLAGS ${VISIT_PARALLEL_LINKER_FLAGS})
endif()
if(VISIT_PARALLEL_RPATH)
    string(REPLACE " " ";" VISIT_PARALLEL_RPATH ${VISIT_PARALLEL_RPATH})
endif()

# Separate link dirs from other flags
foreach(plf ${VISIT_PARALLEL_LINKER_FLAGS})
    string(SUBSTRING ${plf} 0 2 lf_type)
    if(lf_type STREQUAL "-L")
        string(SUBSTRING ${plf} 2 -1 lf_dir)
        list(APPEND VISIT_PARALLEL_LINK_DIRS ${lf_dir})
    else()
        list(APPEND VISIT_PARALLEL_LINK_FLAGS ${plf})
    endif()
endforeach()


message(STATUS "Parallel version of VisIt")
message(STATUS "    VISIT_PARALLEL_CFLAGS = ${VISIT_PARALLEL_CFLAGS}")
message(STATUS "    VISIT_PARALLEL_CXXFLAGS = ${VISIT_PARALLEL_CXXFLAGS}")
message(STATUS "    VISIT_PARALLEL_LINK_FLAGS = ${VISIT_PARALLEL_LINK_FLAGS}")
message(STATUS "    VISIT_PARALLEL_LINK_DIRS = ${VISIT_PARALLEL_LINK_DIRS}")
message(STATUS "    VISIT_PARALLEL_LIBS = ${VISIT_PARALLEL_LIBS}")
message(STATUS "    VISIT_PARALLEL_RPATH = ${VISIT_PARALLEL_RPATH}")
message(STATUS "    VISIT_PARALLEL_DEFINES = ${VISIT_PARALLEL_DEFINES}")
message(STATUS "    VISIT_PARALLEL_INCLUDE = ${VISIT_PARALLEL_INCLUDE}")

if(VISIT_FORTRAN AND VISIT_PARALLEL_FORTRAN_LIBS)
    message(STATUS "")
    message(STATUS "    VISIT_PARALLEL_FORTRAN_FLAGS = ${VISIT_PARALLEL_FORTRAN_FLAGS}")
    message(STATUS "    VISIT_PARALLEL_FORTRAN_LINKER_FLAGS = ${VISIT_PARALLEL_FORTRAN_LINKER_FLAGS}")
    message(STATUS "    VISIT_PARALLEL_FORTRAN_LIBS = ${VISIT_PARALLEL_FORTRAN_LIBS}")
endif()


