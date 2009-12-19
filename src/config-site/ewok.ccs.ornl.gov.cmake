#/sw/sources/visit/thirdparty/ewok/visit/cmake/2.6.4/linux-x86_64_gcc-3.4.6/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## build_vist generated host.conf
## created: Wed Nov 19 09:30:24 EST 2008
## system: Linux ewok002.ccs.ornl.gov 2.6.9-42.0.10.EL_lustre-1.4.10.1smp #1 SMP Wed Apr 25 12:52:57 MDT 2007 x86_64 x86_64 x86_64 GNU/Linux
## by: pugmire
#
# NOTE: Compile this on ewok080!

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /sw/sources/visit/thirdparty/ewok/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux-x86_64_gcc-3.4.6)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-3.4.6/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux-x86_64_gcc-3.4.6/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-3.4.6)

##
## Silo
##
#not done -- DEFAULT_SILO_INCLUDES = $VISITHOME/silo/4.6.1/linux-x86_64_gcc-3.4.6/include
#not done -- DEFAULT_SILO_LIBRARY = $VISITHOME/silo/4.6.1/linux-x86_64_gcc-3.4.6/lib

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
#CXXFLAGS=" -m64 -fPIC $CXXFLAGS"


#CXXFLAGS=" -m64 -fPIC -DUSE_STDARG -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDARG_H=1 -DUSE_STDARG=1 -DMALLOC_RET_VOID=1 -Msignextend -B "
#MPI_LIBS="-L//sw/ewok/ompi/1.3a1r19727/sl5_pgi7.2.4/lib -lmpi"

#MPI_LIBS="-Wl,-rpath-link -Wl,///usr/ofed/mpi/pgi/mvapich-0.9.7-mlx2.2.0/lib/shared -L///usr/ofed/mpi/pgi/mvapich-0.9.7-mlx2.2.0/lib/shared -L///usr/ofed/mpi/pgi/mvapich-0.9.7-mlx2.2.0/lib -lpmpich++ -lmpich -L//usr/ofed/lib64 -libverbs -lpthread -Msignextend -B -lpthread -lrt "

#MPI_LIBS="-Wl,-rpath-link -Wl,///usr/ofed/mpi/pgi/mvapich-0.9.7-mlx2.2.0/lib/shared -L///usr/ofed/mpi/pgi/mvapich-0.9.7-mlx2.2.0/lib/shared -L///usr/ofed/mpi/pgi/mvapich-0.9.7-mlx2.2.0/lib -lpmpich++ -lmpich -L//usr/ofed/lib64 -libverbs -lpthread -Msignextend -B -lpthread -lrt "


SET(VISIT_CXX_FLAGS "-m64 -fPIC -fPIC -D_REENTRANT")
SET(VISIT_MPI_CXX_FLAGS "-I/sw/ewok/ompi/1.2.8/sl5.2_gnu4.1.2/include/")

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux-x86_64_gcc-3.4.6)

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux-x86_64_gcc-3.4.6)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux-x86_64_gcc-3.4.6)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/linux-x86_64_gcc-3.4.6)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/linux-x86_64_gcc-3.4.6)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.0/linux-x86_64_gcc-3.4.6)

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux-x86_64_gcc-3.4.6)
##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/linux-x86_64_gcc-3.4.6)
##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR ${VISITHOME}/ccmio/2.6.1/linux-x86_64_gcc-3.4.6)

##
