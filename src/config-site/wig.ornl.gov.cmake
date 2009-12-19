#/Users/pugmire/work/visit/thirdparty/cmake/2.6.4/i386-apple-darwin9_gcc-4.0.1/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## build_vist generated host.conf
## created: Tue Sep  2 10:37:41 EDT 2008
## system: Darwin wig.ornl.gov 9.4.0 Darwin Kernel Version 9.4.0: Mon Jun  9 19:30:53 PDT 2008; root:xnu-1228.5.20~1/RELEASE_I386 i386
## by: pugmire

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /Users/pugmire/work/visit/thirdparty)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.4.2/i386-apple-darwin9_gcc-4.0.1)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/i386-apple-darwin9_gcc-4.0.1/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/i386-apple-darwin9_gcc-4.0.1/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/i386-apple-darwin9_gcc-4.0.1)

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-O2 -fno-common -fexceptions")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-O2 -fno-common -fexceptions")

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/i386-apple-darwin9_gcc-4.0.1)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/i386-apple-darwin9_gcc-4.0.1)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/i386-apple-darwin9_gcc-4.0.1)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/i386-apple-darwin9_gcc-4.0.1)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/i386-apple-darwin9_gcc-4.0.1)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/i386-apple-darwin9_gcc-4.0.1)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.0/i386-apple-darwin9_gcc-4.0.1)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/i386-apple-darwin9_gcc-4.0.1)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/i386-apple-darwin9_gcc-4.0.1)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/i386-apple-darwin9_gcc-4.0.1)
##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/i386-apple-darwin9_gcc-4.0.1)
##
## CCMIO
##

##

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/i386-apple-darwin9_gcc-4.0.1)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

