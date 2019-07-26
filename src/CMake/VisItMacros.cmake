# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
#  A place to store macros used by plugins, so that PluginVsInstall.cmake.in
#  will not have to rewrite them.  Should reduce maintenance headache for
#  that file.  This file should only include those that 
#  PluginVsInstall.cmake.in also uses without modification.
#
#*****************************************************************************

IF (WIN32)
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES -DNOMINMAX)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  ADD_DEFINITIONS(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

  # cmake automatically uses _DEBUG and MDd for Debug flags, but our
  # third-party libs are NOT debug versions, so we won't be able to
  # link against them if those defines are used.  Replace those flags:
  # When we allow users to supply their own third-party libs, then
  # this will have to be rethought -- they may want to be able to c
  # change them and this currently forces.

  MACRO(REPLACE_FLAG OLD_FLAG NEW_FLAG FLAG_TYPE FLAG_STRING)
      STRING(REPLACE "${OLD_FLAG}" "${NEW_FLAG}" TMP "${${FLAG_TYPE}}")
      SET(${FLAG_TYPE} "${TMP}" CACHE STRING "${FLAG_STRING}" FORCE)
  ENDMACRO(REPLACE_FLAG)

  # Remove /_DEBUG From debug builds
  if (VISIT_REPLACE_DEBUG_FLAGS)
    REPLACE_FLAG("/D_DEBUG" "" CMAKE_CXX_FLAGS_DEBUG
                 "Flags used by the compiler during debug builds")
    REPLACE_FLAG("/D_DEBUG" "" CMAKE_C_FLAGS_DEBUG
                 "Flags used by the compiler during debug builds")
    REPLACE_FLAG("/D_DEBUG" "" CMAKE_EXE_LINKER_FLAGS_DEBUG
                 "Flags used by the linker during debug builds")
    REPLACE_FLAG("/D_DEBUG" "" CMAKE_MODULE_LINKER_FLAGS_DEBUG
                 "Flags used by the linker during debug builds")
  # Change /MDd to /MD for debug builds
    REPLACE_FLAG("/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG
                 "Flags used by the compiler during debug builds")
    REPLACE_FLAG("/MDd" "/MD" CMAKE_C_FLAGS_DEBUG
                 "Flags used by the compiler during debug builds")
    REPLACE_FLAG("/MDd" "/MD" CMAKE_EXE_LINKER_FLAGS_DEBUG
                 "Flags used by the linker during debug builds")
    REPLACE_FLAG("/MDd" "/MD" CMAKE_MODULE_LINKER_FLAGS_DEBUG
                 "Flags used by the linker during debug builds")
  endif ()
ENDIF (WIN32)

FUNCTION(ADD_TARGET_INCLUDE target)
      set_property(TARGET ${target} 
                   APPEND
                   PROPERTY INCLUDE_DIRECTORIES ${ARGN})
ENDFUNCTION(ADD_TARGET_INCLUDE)

FUNCTION(ADD_TARGET_DEFINITIONS target newDefs)
        set_property(TARGET ${target}
                     APPEND
                     PROPERTY COMPILE_DEFINITIONS ${newDefs})
ENDFUNCTION(ADD_TARGET_DEFINITIONS)

FUNCTION(ADD_PARALLEL_LIBRARY target)
    ADD_LIBRARY(${target} ${ARGN})

    IF(UNIX)
      IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH(X)
        set_property(TARGET ${target}
                     APPEND PROPERTY COMPILE_FLAGS ${PAR_COMPILE_FLAGS})
        IF(VISIT_PARALLEL_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH(X ${VISIT_PARALLEL_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH(X)
            set_property(TARGET ${target}
                     APPEND PROPERTY LINK_FLAGS ${PAR_LINK_FLAGS})
        ENDIF(VISIT_PARALLEL_LINKER_FLAGS)

        IF(VISIT_PARALLEL_RPATH)
            SET(PAR_RPATHS "")
            FOREACH(X ${CMAKE_INSTALL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            FOREACH(X ${VISIT_PARALLEL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            set_property(TARGET ${target}
                     APPEND PROPERTY INSTALL_RPATH ${PAR_RPATHS})
        ENDIF(VISIT_PARALLEL_RPATH)
      ENDIF(VISIT_PARALLEL_CXXFLAGS)

    ELSE(UNIX)
      ADD_TARGET_INCLUDE(${target} ${VISIT_PARALLEL_INCLUDE})
      ADD_TARGET_DEFINITIONS(${target} ${VISIT_PARALLEL_DEFS})
    ENDIF(UNIX)
    IF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
        TARGET_LINK_LIBRARIES(${target} ${VISIT_PARALLEL_LIBS})
    ENDIF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
ENDFUNCTION(ADD_PARALLEL_LIBRARY)

MACRO(VISIT_PLUGIN_TARGET_RTOD type) 
    IF(WIN32)
        SET_TARGET_PROPERTIES(${ARGN} PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY_RELEASE
                "${VISIT_PLUGIN_DIR}/${type}"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG
                "${VISIT_PLUGIN_DIR}/${type}"
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                "${VISIT_PLUGIN_DIR}/${type}"
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
                "${VISIT_PLUGIN_DIR}/${type}"
        )
    ENDIF(WIN32)
ENDMACRO(VISIT_PLUGIN_TARGET_RTOD)


