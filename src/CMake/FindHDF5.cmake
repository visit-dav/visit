# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Mon Dec 27, 17:52:39 MST 2010
#   Added high-level hdf5 lib to search on Windows. (hdf5_hldll).
#
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#   Kathleen Biagas, Thu Jan 9 18:47:21 PDT 2014
#   Add patch from John Cary for hdf5 without 'dll' suffix on name.
#
#   Kathleen Biagas, Wed July 31, 2024
#   Add hdf5_hl to search on Linux.
#
#****************************************************************************/

# Use the HDF5_DIR hint from the config-site .cmake file


OPTION(HDF5_LIBNAMES_AFFIX_DLL "Whether HDF5 library base names end with dll" ON)
IF(WIN32)
  if(HDF5_LIB_NAME)
    SET_UP_THIRD_PARTY(HDF5 LIBS ${HDF5_LIB_NAME})
    IF(VISIT_PARALLEL)
        SET_UP_THIRD_PARTY(HDF5_MPI LIBS ${HDF5_LIB_NAME})
    ENDIF(VISIT_PARALLEL)
  else()
    if(HDF5_LIBNAMES_AFFIX_DLL)
      SET_UP_THIRD_PARTY(HDF5 LIBS hdf5dll hdf5_hldll)
      IF(VISIT_PARALLEL)
          SET_UP_THIRD_PARTY(HDF5_MPI LIBS hdf5dll hdf5_hldll)
      ENDIF(VISIT_PARALLEL)
    else()
      SET_UP_THIRD_PARTY(HDF5 LIBS hdf5 hdf5_hl)
      IF(VISIT_PARALLEL)
          SET_UP_THIRD_PARTY(HDF5_MPI LIBS hdf5 hdf5_hl)
      ENDIF(VISIT_PARALLEL)
    endif()
  endif()
ELSE()
  find_package(HDF5
    PATHS ${VISIT_HDF5_DIR}
    NO_DEFAULT_PATH
  )


# After find_package(HDF5 ...)
get_cmake_property(_vars VARIABLES)

list(SORT _vars)
foreach(v IN LISTS _vars)
  if(v MATCHES "^(HDF5|hdf5)_")
    # Avoid noisy internal variables unless you want them
    message(STATUS "${v}='${${v}}'")
  endif()
endforeach()

set(HDF5_LIBRARY_DIR ${HDF5_BUILD_DIR}/lib)
set(HDF5_INCLUDE_DIR ${HDF5_BUILD_DIR}/include)
set(HDF5_LIB hdf5-shared)

#-- HDF5_ARCHITECTURE=''
#- HDF5_BUILD_DIR='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3'
#- HDF5_BUILD_MODE='Release'
#- HDF5_CONFIG='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/cmake/hdf5-config.cmake'
#- HDF5_CONSIDERED_CONFIGS='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/cmake/hdf5-config.cmake'
#- HDF5_CONSIDERED_VERSIONS='2.0.0'
#- HDF5_C_SHARED_LIBRARY='hdf5-shared'
#- HDF5_DEFAULT_API_VERSION='v200'
#- HDF5_DIR='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/cmake'
#- HDF5_EXPORT_LIBRARIES='hdf5-shared;hdf5_tools-shared;hdf5_hl-shared'
#- HDF5_FIND_REQUIRED_shared_C='true'
#- HDF5_FOUND='1'
#- HDF5_HL_SHARED_LIBRARY='hdf5_hl-shared'
#- HDF5_INCLUDE_DIR='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/include;'
#- HDF5_INCLUDE_DIR_HL='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/include'
#- HDF5_INCLUDE_DIR_TOOLS='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/include'
#- HDF5_INSTALL_MOD_FORTRAN=''
#- HDF5_JAVA_INCLUDE_DIRS='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/lib/;/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/lib/;/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/lib/;/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/lib/'
#- HDF5_LIBDEP='/usr/WS1/visit/visit/thirdparty_shared/3.5.0-python3.13/toss4/szip/2.1/linux-x86_64_gcc-10.3/lib;sz;/usr/WS1/visit/visit/thirdparty_shared/3.5.0-python3.13/toss4/zlib/1.3.1/linux-x86_64_gcc-10.3/lib;z'
#- HDF5_LIBNAMES_AFFIX_DLL='ON'
#- HDF5_LIB_TYPE='shared'
#- HDF5_MPI_C_INCLUDE_PATH='/usr/tce/packages/mvapich2/mvapich2-2.3.7-gcc-10.3.1/include'
#- HDF5_MPI_C_LIBRARIES='/usr/tce/packages/mvapich2/mvapich2-2.3.7-gcc-10.3.1/lib/libmpi.so'
#- HDF5_MPI_DIR='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3'
#- HDF5_MPI_LIBDEP='/usr/WS1/visit/visit/thirdparty_shared/3.5.0-python3.13/toss4/szip/2.1/linux-x86_64_gcc-10.3/lib;sz;/usr/WS1/visit/visit/thirdparty_shared/3.5.0-python3.13/toss4/zlib/1.3.1/linux-x86_64_gcc-10.3/lib;z'
#- HDF5_PACKAGE_EXTLIBS='OFF'
#- HDF5_PACKAGE_NAME='HDF5'
#- HDF5_PROVIDES_CPP_LIB='OFF'
#- HDF5_PROVIDES_DEPRECATED_SYMBOLS='ON'
#- HDF5_PROVIDES_DIRECT_VFD='OFF'
#- HDF5_PROVIDES_FORTRAN='OFF'
#- HDF5_PROVIDES_HDFS_VFD='OFF'
#- HDF5_PROVIDES_HL_LIB='ON'
#- HDF5_PROVIDES_JAVA='OFF'
#- HDF5_PROVIDES_JAVA_COMPAT='ON'
#- HDF5_PROVIDES_JNI='OFF'
#- HDF5_PROVIDES_LARGE_PARALLEL_IO='ON'
#- HDF5_PROVIDES_MAP_API='OFF'
#- HDF5_PROVIDES_MIRROR_VFD='OFF'
#- HDF5_PROVIDES_NONSTANDARD_FEATURE_FLOAT16='OFF'
#- HDF5_PROVIDES_PARALLEL='ON'
#- HDF5_PROVIDES_PARALLEL_FILTERED_WRITES='ON'
#- HDF5_PROVIDES_PLUGIN_SUPPORT='OFF'
#- HDF5_PROVIDES_ROS3_VFD='OFF'
#- HDF5_PROVIDES_SHARED_LIBS='YES'
#- HDF5_PROVIDES_STATIC_LIBS='NO'
#- HDF5_PROVIDES_STATIC_TOOLS='OFF'
#- HDF5_PROVIDES_SUBFILING_VFD='OFF'
#- HDF5_PROVIDES_SZIP_SUPPORT='OFF'
#- HDF5_PROVIDES_THREADS='ON'
#- HDF5_PROVIDES_THREADSAFE='OFF'
#- HDF5_PROVIDES_TOOLS='ON'
#- HDF5_PROVIDES_ZLIB_SUPPORT='ON'
#- HDF5_SHARE_DIR='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/cmake'
#- HDF5_TOOLSET=''
#- HDF5_TOOLS_DIR='/g/g11/miller86/visit/visit/build-Xdmf/my_install/hdf5/2.0.0/linux-x86_64_gcc-10.3/bin'
#- HDF5_VALID_COMPONENTS='static;shared;C;CXX;Fortran;HL;CXX_HL;Fortran_HL;Java;Tools'
#- HDF5_VERSION='2.0.0'
#- HDF5_VERSION_COUNT='3'
#- HDF5_VERSION_MAJOR='2.0'
#- HDF5_VERSION_MINOR='0'
#- HDF5_VERSION_PATCH='0'
#- HDF5_VERSION_STRING='2.0.0'
#- HDF5_VERSION_TWEAK='0'
#- HDF5_shared_C_FOUND='1'
#- HDF5_shared_HL_FOUND='1'
#- hdf5_comp='hdf5_hl'

ENDIF()
