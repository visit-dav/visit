#/what/is/the/path/to/bin/cmake

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
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/Linux_gcc-3.3.4/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux-x86_64/bin)

##
## Specify the location of the python include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux-x86_64)

##
## Set the flags for compilation
##
SET(VISIT_C_FLAGS "-fPIC")
SET(VISIT_CXX_FLAGS "-fPIC -Wno-deprecated")

##
## Add parallel arguments.
##

# This is for the vendor Quadrics MPI, but it has problems....
SET(VISIT_MPI_CXX_FLAGS "-I/usr/lib/mpi/mpi_gcc334/include")
SET(VISIT_MPI_LD_FLAGS "-L/usr/lib/mpi/mpi_gcc334/lib")
SET(VISIT_MPI_LIBS mpi)

# ... so instead, we're using MPICH.
#LDFLAGS="-L/na/home/ahern/work/mpich-1.2.7p1/lib $LDFLAGS"
#CPPFLAGS="-I/na/home/ahern/work/mpich-1.2.7p1/include $CPPFLAGS"
#MPI_LIBS="-lmpich"

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
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /apps/netcdf/3.6.0/hawk_x86_64)

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux-x86_64)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux-x86_64)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /apps/HDF5/1.6.5/hawk_x86_64)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5.1/linux-x86_64)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

