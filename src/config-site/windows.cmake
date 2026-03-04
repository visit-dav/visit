
set(VISIT_MSVC_VERSION "MSVC2022")

message(STATUS "Prebuilt library directory name: ${VISIT_MSVC_VERSION}")

if(NOT DEFINED VISIT_WINDOWS_DIR)
    # this works for development builds
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../visit-deps/windowsbuild)
        cmake_path(SET VISIT_WINDOWS_DIR NORMALIZE ${CMAKE_CURRENT_SOURCE_DIR}/../../visit-deps/windowsbuild)
    # this works for our released source distros
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../windowsbuild)
        cmake_path(SET VISIT_WINDOWS_DIR NORMALIZE ${CMAKE_CURRENT_SOURCE_DIR}/../windowsbuild)
    else()
        message(FATAL_ERROR "Please set VISIT_WINDOWS_DIR to location of 'windowsbuild' files")
    endif()
endif()

set(VISITHOME ${VISIT_WINDOWS_DIR}/${VISIT_MSVC_VERSION})

# Create the NSIS installer package (override default from root CMakeLists.txt
option(VISIT_MAKE_NSIS_INSTALLER "Create an installer package using NSIS." ON)

##############################################################
##
## Required libraries
##
##############################################################

##
## BOOST
##
# headers only, MSVC version doesn't matter
VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR ${VISIT_WINDOWS_DIR}/MSVC2022/boost_minimal_headers/1.67.0)

##
## OSPRay
##
SETUP_APP_VERSION(OSPRAY 3.0.0)
VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR ${VISITHOME}/ospray/${OSPRAY_VERSION})

SETUP_APP_VERSION(ANARI 0.15.0)
VISIT_OPTION_DEFAULT(VISIT_ANARI_DIR  ${VISITHOME}/anari/${ANARI_VERSION})

##
## VTK
##
SETUP_APP_VERSION(VTK 9.5.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR  ${VISITHOME}/vtk/${VTK_VERSION})

##
## QT
##
SETUP_APP_VERSION(QT 6.4.2)
VISIT_OPTION_DEFAULT(VISIT_QT_DIR     ${VISITHOME}/Qt/${QT_VERSION})


##
## QWT
##
VISIT_OPTION_DEFAULT(VISIT_QWT_DIR     ${VISITHOME}/Qwt/6.3.0)


##
## PYTHON
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/3.13.9)


##############################################################
##
## Some misc libraries Database readers may depend upon
##
##############################################################

##
## JPEG
##
set(JPEG_LIBNAME_PREFIX_LIB false CACHE INTERNAL "Is 'lib' prefixed to jpeg libname")
VISIT_OPTION_DEFAULT(VISIT_JPEG_DIR     ${VISITHOME}/jpeg/9e)

###
### TIFF
###
VISIT_OPTION_DEFAULT(VISIT_TIFF_DIR     ${VISITHOME}/tiff/4.5.0)


##
## ZLIB
##
set(ZLIB_LIB_NAME "zlib")
VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR     ${VISITHOME}/zlib/1.3.1)

##############################################################
##
## Database reader plugin support libraries
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.
##############################################################


##
## HDF5
##
set(VISIT_HDF5_VERSION "2.0.0")
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR     ${VISITHOME}/hdf5/${VISIT_HDF5_VERSION})

##
## CONDUIT
##
SETUP_APP_VERSION(CONDUIT 0.9.4)
VISIT_OPTION_DEFAULT(VISIT_CONDUIT_DIR ${VISITHOME}/conduit/${CONDUIT_VERSION})
VISIT_OPTION_DEFAULT(VISIT_CONDUIT_LIBDEP HDF5_LIB TYPE STRING)

##
## NETCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR   ${VISITHOME}/netcdf/4.9.3)
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIB TYPE STRING)

##
## BOXLIB
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/BoxLib/1.3.5)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR  ${VISITHOME}/cfitsio/4.6.3)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR     ${VISITHOME}/gdal/2.2.4)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR     ${VISITHOME}/cgns/4.1.0)
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIB TYPE STRING)


##
## H5PART
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR   ${VISITHOME}/h5part/1.6.6)
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIB TYPE STRING)

##
## ICET
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/77c708f9)

##
## MFEM
##
VISIT_OPTION_DEFAULT(VISIT_MFEM_DIR     ${VISITHOME}/mfem/4.8)
VISIT_OPTION_DEFAULT(VISIT_MFEM_INCDEP ZLIB_INCLUDE_DIR CONDUIT_INCLUDE_DIR TYPE STRING)

##
## MILI
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR     ${VISITHOME}/Mili/24.1.1)

##
## OpenEXR
##
SETUP_APP_VERSION(OPENEXR 3.3.4)
VISIT_OPTION_DEFAULT(VISIT_OPENEXR_DIR ${VISITHOME}/openexr/3.3.4)


##
## SILO
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR     ${VISITHOME}/silo/4.12.0)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP  HDF5_LIB TYPE STRING)

##
##  VTKM
##
VISIT_OPTION_DEFAULT(VISIT_VTKM_DIR ${VISITHOME}/vtkm/2.3.0)

##
## XDMF
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/xdmf/2.1.1)
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIB VTK::libxml2 TYPE STRING)


##
## ADIOS2
##
SETUP_APP_VERSION(ADIOS2 2.10.0-rc1)
VISIT_OPTION_DEFAULT(VISIT_ADIOS2_DIR ${VISITHOME}/adios2/${ADIOS2_VERSION})

unset(VISITHOME)
