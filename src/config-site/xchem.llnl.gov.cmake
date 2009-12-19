#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /opt/llnl/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux_fedora4_gcc_4.0.2)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_fedora4_gcc_4.0.2/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux_fedora4_gcc_4.0.2/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux_fedora4_gcc_4.0.2)

##
## Add parallel arguments.
##
SET(VISIT_C_FLAGS "-fPIC")
SET(VISIT_MPI_CXX_FLAGS "-DMPICH_IGNORE_CXX_SEEK")

##
## Turn off warnings for deprecated features on g++
##
SET(VISIT_CXX_FLAGS "-Wno-deprecated")

##
## Database reader plugin support libraries
##
###############################################################################
 
##
## Boxlib
##
##DEFAULT_BOXLIB2D_INCLUDE=$VISITHOME/boxlib/linux_fedora4_gcc_4.0.2/include/2D
##DEFAULT_BOXLIB2D_LIBS=$VISITHOME/boxlib/linux_fedora4_gcc_4.0.2/lib
##DEFAULT_BOXLIB3D_INCLUDE=$VISITHOME/boxlib/linux_fedora4_gcc_4.0.2/include/3D
##DEFAULT_BOXLIB3D_LIBS=$VISITHOME/boxlib/linux_fedora4_gcc_4.0.2/lib

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/linux_fedora4_gcc_4.0.2)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux_fedora4_gcc_4.0.2)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/2.4.1/linux_fedora4_gcc_4.0.2)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.1/linux_fedora4_gcc_4.0.2)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux_fedora4_gcc_4.0.2)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.2/linux_fedora4_gcc_4.0.2)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
