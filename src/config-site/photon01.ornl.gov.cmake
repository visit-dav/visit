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
## Set the flags for compilation
##
SET(VISIT_C_FLAGS "-fPIC")
SET(VISIT_CXX_FLAGS "-fPIC -Wno-deprecated")

##
## Add parallel arguments.
##

SET(VISIT_MPI_CXX_FLAGS "-I$MPIHOME/include")
SET(VISIT_MPI_LD_FLAGS "-L/usr/local/ofed/lib64 -L$MPIHOME/lib/shared -L$MPIHOME/lib -Wl,-rpath-link=/usr/local/ofed/mpi/gcc/mvapich-0.9.5-mlx2.0.1/lib/shared -L$MPIHOME/lib")
SET(VISIT_MPI_LIBS mpich ibverbs)


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
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/hawk_x86_64)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/hawk_x86_64)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5.1/linux-x86_64)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
