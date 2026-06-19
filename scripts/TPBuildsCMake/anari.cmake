# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(ANARI VERSION 0.15.0)

# set global variables used by other libraries
set(visit_anari_root ${visit_install_root}/anari/${ANARI_VERSION})
set(visit_anari_library_dir ${visit_anari_root}/lib)
set(visit_anari_config_dir ${visit_anari_library_dir}/cmake/anari-${ANARI_VERSION})
set(visit_anari_include_dir ${visit_anari_root}/include/anari)

# has this already been built
if(EXISTS ${visit_anari_include_dir}/anari.h AND
   EXISTS ${visit_anari_library_dir}/anari.lib)
    if(BUILD_ALL OR ENABLE_ANARI)
        message(STATUS "ANARI already installed.")
    endif()
    set(ANARI_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_ANARI))
  return()
endif()

message(STATUS "ANARI being added to the build")


set(ANARI_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreadedDLL
    -DBUILD_TESTING:STRING=OFF
    -DBUILD_EXAMPLES:STRING=OFF
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/anari/${ANARI_VERSION})

set(ANARI_DEPENDS)

list(APPEND ANARI_CMAKE_OPTIONS
    -DPython3_EXECUTABLE:FILEPATH=${visit_python_executable}
    -DPython3_INCLUDE_DIR:PATH=${visit_python_include}
    -DPython3_LIBRARY:FILEPATH=${visit_python_library})

if(NOT PYTHON_PREBUILT)
    list(APPEND ANARI_DEPENDS PYTHON)
endif()


set(ANARI_ZIP_NAME "ANARI-SDK-${ANARI_VERSION}_with_deps.zip")

ExternalProject_Add(ANARI
    PREFIX           ANARI
    URL              ${visit_tp_url}/${ANARI_ZIP_NAME}
    URL_HASH         SHA256=6b07d04e00571a381921a9d95aa969e3c62f8b0efbf57a3cf2da84ea9daa802b
    INSTALL_DIR      ${visit_anari_root}
    CMAKE_CACHE_ARGS ${ANARI_CMAKE_OPTIONS}
    DEPENDS          ${ANARI_DEPENDS}
    ${LOGGING_OPTIONS})

