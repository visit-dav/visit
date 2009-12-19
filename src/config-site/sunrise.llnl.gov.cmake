#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Use the g++ 3.2 compiler.
##
SET(VISIT_C_COMPILER /usr/local/gnu/gcc-3.2/bin/gcc)
SET(VISIT_CXX_COMPILER /usr/local/gnu/gcc-3.2/bin/g++)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/current/solaris_gcc_3.2)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/solaris_gcc_3.2/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/solaris_gcc_3.2/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/solaris_gcc_3.2)

##
## Add pthread arguments.
##
#not done -- PTHREAD_LIB = -lthread

##
## Database reader plugin support libraries
##
###############################################################################

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/solaris_gcc_3.2)
                                                                                
##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/solaris_gcc_3.2)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/solaris_gcc_3.2)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.0/solaris_gcc_3.2)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR /usr/gapps/hdf4/1.4.0/SunOS/gcc/static/debug)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

#
# HDF5
#
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.6.6/sparc-sun-solaris-gcc-2.95)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/current/sunos_57)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /usr/gapps/visit/netcdf/3.6.0/sunos_57)

##
## Use the correct versions of the silo include and library files
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.7/sparc_SunOS_57)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
