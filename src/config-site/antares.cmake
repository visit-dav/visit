#/d/visguests/oruebel/devel/visit/cmake/2.6.4/linux-x86_64_gcc-4.1.2/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## build_vist generated host.conf
## created: Sat Apr 12 12:21:18 PDT 2008
## system: Linux antares 2.6.18.8-0.9-default #1 SMP Sun Feb 10 22:48:05 UTC 2008 x86_64 x86_64 x86_64 GNU/Linux
## by: oruebel

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /d/visguests/oruebel/devel/visit)
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
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux-x86_64_gcc-4.1.2/bin)

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
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##

##
## CFITSIO
##

##
## CGNS
##

##
## Exodus
##

##
## GDAL
##

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.0/linux-x86_64_gcc-4.1.2)

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit-Prabhat)

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/linux-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Mili
##

##
## NetCDF
##
##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/linux-x86_64_gcc-4.1.2)
##
## CCMIO
##

