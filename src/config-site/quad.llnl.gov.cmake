#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/irix64_cc_7.41_64)
#fi

##
## Set VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/irix64_cc_7.41_64/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/irix64_cc_7.41_64/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/irix64_cc_7.41_64)

##
## Make a 64 bit binary
##
SET(VISIT_C_FLAGS "-64 -G 7 -OPT:Olimit=0")
SET(VISIT_CXX_FLAGS "-64 -LANG:std -G 7 -OPT:Olimit=0")

##
## Report all warnings.
##
SET(VISIT_C_FLAGS "-fullwarn -woff 1424")
SET(VISIT_CXX_FLAGS "-fullwarn -woff 1424")
SET(VISIT_MPI_CXX_FLAGS "-DMPI_NO_CPPBIND")

##
## Add parallel arguments.
##
SET(VISIT_MPI_LIBS mpi)

##
## Database reader plugin support libraries
##
###############################################################################

#
# HDF5
#
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.6.6/irix_6.5_64)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/current/irix64_cc_7.41_64)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.6/irix_6.5_64)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
