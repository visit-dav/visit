#/usr/gapps/visit/cmake/2.6.4/linux-x86_64_gcc-4.1.2/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## build_vist generated host.conf
## created: Wed Dec  3 11:05:37 PST 2008
## system: Linux palpatine.llnl.gov 2.6.18-92.1.6.el5xen #1 SMP Fri Jun 20 02:51:52 EDT 2008 x86_64 x86_64 x86_64 GNU/Linux
## by: whitlocb

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux-x86_64_gcc-4.1.2)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-4.1.2/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/linux-x86_64_gcc_4.1.2/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-4.1.2)


## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC")

##
## If enable_viewer_mesa_stub in not set, then disable it.
##
#if test -z "$enable_viewer_mesa_stub"; then
#not done -- enable_viewer_mesa_stub = no
#fi

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux-x86_64_gcc-4.1.2)

##
## CGNS
##

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/linux-x86_64_gcc-4.1.2)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/linux-x86_64_gcc-4.1.2)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.0/linux-x86_64_gcc-4.1.2)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/linux-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
#not done -- DEFAULT_VISIT_MILI_DIR = $VISITHOME/mili/1.10.0/linux-x86_64_gcc-4.1.2

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux-x86_64_gcc-4.1.2)

##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/linux-x86_64_gcc-4.1.2)

##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR ${VISITHOME}/ccmio/2.6.1/linux-x86_64_gcc-4.1.2)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.2/linux-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
