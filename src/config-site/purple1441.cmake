#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Use the xlc compiler.
##
SET(VISIT_C_COMPILER xlc)
SET(VISIT_CXX_COMPILER xlC)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/aix_5_xlc_64)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/aix_5_64_fed_xlc_64/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/aix_5_xlc_64/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/aix_5_xlc_7.0.0.3_64)

##
## Turn off warnings for deprecated features.
##

##
## Turn on dynamic cast code.
##
SET(VISIT_CXX_FLAGS "-qrtti=dynamiccast")

##
## Turn on fake exceptions.
##
##FAKE_EXCEPTIONS_BY_DEFAULT="yes"

##
## We only get 256MB of memory by default, thus the maxdata flag
## We can only use 2GB/node without large pages (-blpdata)
##
SET(VISIT_EXE_LINKER_FLAGS "-Wl,-blpdata -Wl,-bmaxdata:0x70000000")

##
## Make sure we use the right MPI library (05/04/01 JSM)
##
SET(VISIT_MPI_CXX_FLAGS "-I/usr/lpp/ppe.poe/include/")
SET(VISIT_MPI_LD_FLAGS "-L/usr/lpp/ppe.poe/lib/threads -L/usr/lpp/ppe.poe/lib -L/usr/lpp/ppe.poe/lib/ip -Wl,-binitfini:poe_remote_main")
SET(VISIT_MPI_LIBS mpi_r vtd_r lapi_r pthreads)
#if test -z "$MPI_LIBS"; then
#fi

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR /usr/gapps/visit/boxlib/aix_5_xlc_64)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR /usr/gapps/visit/boxlib/aix_5_xlc_64)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.8.1/aix_5_64_fed)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/current/aix_5_xlc_64)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.7/aix_5_64_fed)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## ViSUS
##
VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR /usr/gapps/visus/1.0a5/aix_5_64_fed)
