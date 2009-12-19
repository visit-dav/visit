#/soft/apps/visit/cmake/2.6.4/linux-sles10-x86_64_gcc-4.1.2/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
##
## build_vist generated host.conf
## created: Wed Nov  5 12:17:12 CST 2008
## system: Linux login1 2.6.16.53-0.16-smp #1 SMP Tue Oct 2 16:57:49 UTC 2007 x86_64 x86_64 x86_64 GNU/Linux
## by: childs
##
##  This is for the Argonne system, login1.eureka.alcf.anl.gov.
##
#not done -- VISITARCH = linux-sles10-x86_64_gcc-4.1.2

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /soft/apps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/linux-sles10-x86_64_gcc-4.1)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-sles10-x86_64_gcc-4.1.2/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/${VISITARCH}/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-sles10-x86_64_gcc-4.1.2)


##
## Silo
##

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC")
SET(VISIT_MPI_C_FLAGS "-I/soft/apps/mpich-mx-1.2.7..7-1/include")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC")
SET(VISIT_MPI_CXX_FLAGS "-I/soft/apps/mpich-mx-1.2.7..7-1/include")

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

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.2/linux-sles10-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##
##
## SZIP
##
##
## CCMIO
##

##
##
## SILO (must be after HDF5 symbols to reference them correctly)
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/linux-sles10-x86_64_gcc-4.1.2)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
