#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /scratch/visit/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Turn off warnings for deprecated features.
##
SET(VISIT_CXX_FLAGS "-g -Wno-deprecated")

##
## Use mpich for parallel builds
##
SET(VISIT_MPI_CXX_FLAGS "-I/scratch/pkg/mpich-1.2.5.2/include")
SET(VISIT_MPI_LD_FLAGS "-L/scratch/pkg/mpich-1.2.5.2/lib")
SET(VISIT_MPI_LIBS mpich)

##
## Specify the location of the mesa include files and libraries.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.4.2/linux-i686_gcc-3.2.3)
#fi

##
## Specify the location of the vtk include files and libraries.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-i686_gcc-3.2.3/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##

#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/linux-i686_gcc-3.2.3/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Specify the location of the python include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-i686_gcc-3.2.3)

#
# HDF5
#
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR $PKGHOME/hdf5-1.8.1/install)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)


##
## Specify the location of the silo include files and library.
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR $PKGHOME/silo-4.7/install)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

#
# Mili
#
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.10.0/linux-i686_gcc-3.2.3)

#
# CGNS
#
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux-i686_gcc-3.2.3)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /scratch/pkg/netcdf-3.6.0-p1/src/install)

##
## ITAPS (MOAB installation)
##
#not done -- DEFAULT_ITAPS_IMPLS = "ITAPS_C:cub,h5m:-I/scratch/pkg/ITAPS-MOAB-3.99-20Apr09/install/include:-L/scratch/pkg/ITAPS-MOAB-3.99-20Apr09/install/lib,-liMesh,-lMOAB,-lhdf5,-lsz,-lz,-L/scratch/pkg/netcdf-3.6.0-p1/src/install/lib,-lnetcdf_c++,-lnetcdf,-lvtkGraphics"
