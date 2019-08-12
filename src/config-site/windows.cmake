
if(NOT MSVC_VERSION OR MSVC_VERSION LESS 1910 OR NOT CMAKE_CL_64)
    send_message(FATAL_ERROR "You have chosen a compiler version not currently supported by this config-site file.  You will need to specify \"Visual Studio 15 2017 Win64\" as your generator or create a new config-site file pointing to the locations of third party libraries on your system.")
endif()

set(VISIT_MSVC_VERSION "MSVC2017")

message(STATUS "Prebuilt library directory name: ${VISIT_MSVC_VERSION}")

if(NOT DEFINED VISIT_WINDOWS_DIR)
    # this works for development builds
    if(EXISTS ${VISIT_SOURCE_DIR}/../../visit-deps/windowsbuild)
        set(VISIT_WINDOWS_DIR ${VISIT_SOURCE_DIR}/../../visit-deps/windowsbuild)
    # this works for our released source distros
    elseif(EXISTS ${VISIT_SOURCE_DIR}/../windowsbuild)
        set(VISIT_WINDOWS_DIR ${VISIT_SOURCE_DIR}/../windowsbuild)
    else()
        message(FATAL_ERROR "Please set VISIT_WINDOWS_DIR to location of 'windowsbuild' files")
    endif()
endif()
get_filename_component(VISIT_WINDOWS_DIR ${VISIT_WINDOWS_DIR} ABSOLUTE)
set(VISITHOME ${VISIT_WINDOWS_DIR}/${VISIT_MSVC_VERSION})


# Create the NSIS installer package (override default from root CMakeLists.txt
option(VISIT_MAKE_NSIS_INSTALLER "Create an installer package using NSIS." ON)
VISIT_OPTION_DEFAULT(VISIT_USE_7ZIP true TYPE BOOL)

VISIT_OPTION_DEFAULT(VISIT_SPHINX true TYPE BOOL)

##############################################################
##
## Required libraries
##
##############################################################

##
## BOOST
##
VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR ${VISITHOME}/boost_minimal_headers/1.67.0)



##
## ISPC
##
VISIT_OPTION_DEFAULT(VISIT_ISPC_DIR ${VISITHOME}/ispc/1.9.2)

##
## EMBREE
##
VISIT_OPTION_DEFAULT(VISIT_EMBREE_DIR ${VISITHOME}/embree/3.4.0/)

##
## TBB
##
VISIT_OPTION_DEFAULT(TBB_ROOT ${VISITHOME}/tbb/2018_20171205oss)
VISIT_OPTION_DEFAULT(VISIT_TBB_DIR ${VISITHOME}/tbb/2018_20171205oss)

##
## OSPRay
##
VISIT_OPTION_DEFAULT(VISIT_OSPRAY ON TYPE BOOL)
SETUP_APP_VERSION(OSPRAY 1.6.1)
VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR ${VISITHOME}/ospray/1.6.1)


##
## VTK
##
SETUP_APP_VERSION(VTK 8.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR  ${VISITHOME}/vtk/${VTK_VERSION})

##
## QT
##
VISIT_OPTION_DEFAULT(VISIT_QT_DIR     ${VISITHOME}/Qt/5.10.1)


##
## QWT
##
VISIT_OPTION_DEFAULT(VISIT_QWT_DIR     ${VISITHOME}/Qwt/6.1.2)

##
## PYTHON
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.7.14)

##
## PYSIDE
##
#VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR ${VISITHOME}/PySide/2.0.0-2017.08.30)


##############################################################
##
## Some misc libraries Database readers may depend upon
##
##############################################################

##
## JPEG
##
set(JPEG_LIBNAME_PREFIX_LIB false CACHE INTERNAL "Is 'lib' prefixed to jpeg libname")
VISIT_OPTION_DEFAULT(VISIT_JPEG_DIR     ${VISITHOME}/jpeg/9b)

##
## SZIP
##
set(SZIP_LIB_NAME "szip")
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR     ${VISITHOME}/szip/2.1.1)

##
## ZLIB
##
set(ZLIB_LIB_NAME "zlib")
VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR     ${VISITHOME}/zlib/1.2.11)

##############################################################
##
## Database reader plugin support libraries
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.
##############################################################

##
## HDF4
##
set(HDF4_LIBNAMES_AFFIX_DLL false CACHE INTERNAL "Is 'dll' appended to hdf4 libnames")
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR     ${VISITHOME}/hdf/4.2.13)

VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP  
      JPEG_LIBRARY_DIR JPEG_LIB
      SZIP_LIBRARY_DIR SZIP_LIB
      ZLIB_LIBRARY_DIR ZLIB_LIBRARY
      TYPE STRING)

##
## HDF5
##
set(VISIT_HDF5_VERSION "1.8.19")

VISIT_OPTION_DEFAULT(HDF5_LIBNAMES_AFFIX_DLL OFF TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR     ${VISITHOME}/hdf5/${VISIT_HDF5_VERSION})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP  
    SZIP_LIBRARY_DIR SZIP_LIB 
    ZLIB_LIBRARY_DIR ZLIB_LIBRARY
    TYPE STRING)

##
## CONDUIT
##
VISIT_OPTION_DEFAULT(VISIT_CONDUIT_DIR ${VISITHOME}/conduit/0.4.0)
VISIT_OPTION_DEFAULT(VISIT_CONDUIT_LIBDEP
    HDF5_LIBRARY_DIR HDF5_LIB ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## NETCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR   ${VISITHOME}/netcdf/4.5.0)
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP 
    HDF5_LIBRARY_DIR HDF5_LIB ${VISIT_HDF5_LIBDEP}
    TYPE STRING)

##
## BOXLIB
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/BoxLib/1.3.5)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR  ${VISITHOME}/cfitsio/3.420)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR     ${VISITHOME}/cgns/3.2.1)
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP 
    HDF5_LIBRARY_DIR HDF5_LIB ${VISIT_HDF5_LIBDEP}
    TYPE STRING)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR     ${VISITHOME}/gdal/2.2.4)

##
## H5PART
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR   ${VISITHOME}/h5part/1.6.6)
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP 
    HDF5_LIBRARY_DIR HDF5_LIB ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)

##
## MFEM
##
VISIT_OPTION_DEFAULT(VISIT_MFEM_DIR     ${VISITHOME}/mfem/3.4)
VISIT_OPTION_DEFAULT(VISIT_MFEM_INCDEP ZLIB_INCLUDE_DIR CONDUIT_INCLUDE_DIR TYPE STRING)

##
## MILI
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR     ${VISITHOME}/Mili/15.1)

##
## OpenEXR
##
VISIT_OPTION_DEFAULT(VISIT_OPENEXR_DIR ${VISITHOME}/OpenEXR/2.2.0)


##
## SILO
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR     ${VISITHOME}/silo/4.10.3)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP  
    HDF5_LIBRARY_DIR HDF5_LIB ${VISIT_HDF5_LIBDEP}
    TYPE STRING
)

##
## XDMF
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1)
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP 
    HDF5_LIBRARY_DIR HDF5_LIB ${VISIT_HDF5_LIBDEP}
    ${VISIT_VTK_DIR}/lib vtklibxml2-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}
    TYPE STRING)

