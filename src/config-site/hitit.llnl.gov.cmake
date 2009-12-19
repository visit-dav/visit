#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Use the g++ 3.2.3 compiler.
##
SET(VISIT_C_COMPILER /usr/gapps/visit/gcc/3.2.3/linux_redhat7/bin/gcc)
SET(VISIT_CXX_COMPILER /usr/gapps/visit/gcc/3.2.3/linux_redhat7/bin/g++)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/current/linux_redhat7_gcc_3.2.3)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_redhat7_gcc_3.2.3/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux_redhat7_gcc_3.2.3/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux_redhat7_gcc_3.2.3)

##
## Database reader plugin support libraries
##
###############################################################################

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR /usr/gapps/visit/hdf4/2.4.1/linux_redhat7_gcc_3.2.3)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.8.1/i686_Linux_ELsmp)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/1.10/linux_redhat7_gcc_3.2.3)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /usr/gapps/visit/netcdf/3.6.0/linux_redhat7_gcc_3.2.3)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.7/i686_Linux_ELsmp)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
