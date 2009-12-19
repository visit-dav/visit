#/what/is/the/path/to/bin/cmake

## This config site file is for tdevi, an LLNL
## Itanium development cluster

SET(VISITHOME /home/meredith/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Set the VISITHOME environment variable.
##

##
## Specify the location of the silo include files and library.
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})


##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/lib/vtk-5.0)

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Specify the location of the python include files and libraries.
##
SET(VISIT_CXX_FLAGS "-fpic")

