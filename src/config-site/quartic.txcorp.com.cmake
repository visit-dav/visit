#/contrib/visit/cmake/2.6.4/linux-x86_64_gcc-4.3.0/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## build_vist generated host.conf
## created: Wed Aug 27 14:51:58 MDT 2008
## system: Linux quartic.txcorp.com 2.6.25.14-108.fc9.x86_64 #1 SMP Mon Aug 4 13:46:35 EDT 2008 x86_64 x86_64 x86_64 GNU/Linux
## by: hankchilds

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /contrib/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux-x86_64_gcc-4.3.0)


##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-4.3.0/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux-x86_64_gcc-4.3.0/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-4.3.0)

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
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-4.3.0)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## SILO (must be after HDF5 symbols to reference them correctly)
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/linux-x86_64_gcc-4.3.0)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
