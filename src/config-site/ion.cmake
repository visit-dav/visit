#/apps/visit/cmake/2.6.4/linux-x86_64/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /apps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux-x86_64)

##
## Specify the location of the vtk include files and libraries.
##
#VTK=$VISITHOME/vtk/2003.10.28c/linux-x86_64
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux-x86_64/bin)

##
## Specify the location of the python include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux-x86_64)

##
## Tell VisIt where to find cmake in case we want to build slivr.
##


##
## Set the flags for compilation
##
SET(VISIT_C_FLAGS "-fPIC")
SET(VISIT_CXX_FLAGS "-fPIC -Wno-deprecated")

##
## Add parallel arguments.
##

# use mpich for parallel
SET(VISIT_MPI_CXX_FLAGS "-DMPICH_SKIP_MPICXX -I/usr/local/include")
SET(VISIT_MPI_LD_FLAGS "-L/usr/local/lib")
SET(VISIT_MPI_LIBS mpich)

##
## If enable_viewer_mesa_stub in not set, then disable it.
##
#if test -z "$enable_viewer_mesa_stub"; then
#not done -- enable_viewer_mesa_stub = no
#fi

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
##DEFAULT_BOXLIB2D_INCLUDE=$VISITHOME/boxlib/linux-x86_64/include/2D
##DEFAULT_BOXLIB2D_LIBS=$VISITHOME/boxlib/linux-x86_64/lib
##DEFAULT_BOXLIB3D_INCLUDE=$VISITHOME/boxlib/linux-x86_64/include/3D
##DEFAULT_BOXLIB3D_LIBS=$VISITHOME/boxlib/linux-x86_64/lib

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2r1)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/hawk_x86_64)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/linux-x86_64)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
