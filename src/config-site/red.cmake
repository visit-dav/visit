#/what/is/the/path/to/bin/cmake

## ----------------------------------------------------------------------------
##   config-site file for red.slac.stanford.edu, an SGI Altix machine at SLAC
## ----------------------------------------------------------------------------


##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /nfs/slac/g/ki/ki04/visit/libraries)
SET(VISIT_VERBOSE_MAKEFILE TRUE)


##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux_redhat_gcc_3.2.3)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_redhat_gcc_3.2.3/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux_redhat_gcc_3.2.3/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux_redhat_gcc_3.2.3)

##
## Turn off warnings for deprecated features.
##
SET(VISIT_CXX_FLAGS "-Wno-deprecated")

##
## Gernerate position independent code.
##
SET(VISIT_CXX_FLAGS "-fPIC")
SET(VISIT_MPI_CXX_FLAGS "-DMPI_NO_CPPBIND")

##
## Add parallel arguments.
##
SET(VISIT_MPI_LIBS mpi)

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux_redhat_gcc_3.2.3)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux_redhat_gcc_3.2.3)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/2.0/linux_ia64_chaos_gcc_3.3.3)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.0/linux_redhat_gcc_3.2.3)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5/linux_redhat_gcc_3.2.3)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
