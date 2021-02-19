#/usr/local/bin/cmake
##
## ../visit_svn/trunk/src/svn_bin/build_visit generated host.cmake
## created: Thu Dec  3 19:29:12 PST 2015
## system: Darwin fry.lbl.gov 15.0.0 Darwin Kernel Version 15.0.0: Sat Sep 19 15:53:46 PDT 2015; root:xnu-3247.10.11~1/RELEASE_X86_64 x86_64
## by: ghweber

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /Users/ghweber/devel/visit_3rdparty)
SET(VISITARCH i386-apple-darwin17_clang)

VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/local/visit)
SET(CMAKE_BUILD_TYPE Release CACHE STRING "")
VISIT_OPTION_DEFAULT(VISIT_PYTHON_FILTERS ON)
VISIT_OPTION_DEFAULT(VISIT_CREATE_APPBUNDLE_PACKAGE ON)

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER clang TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER clang++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-fno-common -fexceptions -DVIZSCHEMA_DECOMPOSE_DOMAINS" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-fno-common -fexceptions -DVIZSCHEMA_DECOMPOSE_DOMAINS" TYPE STRING)

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
## (configured w/ mpi compiler wrapper)
VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER /usr/local/bin/mpicxx TYPE FILEPATH)

##############################################################
##
## Database reader plugin support libraries
##
## The HDF4, HDF5 and NetCDF libraries must be first so that
## their libdeps are defined for any plugins that need them.
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.
##############################################################
##

##
## Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr/local/opt/python/Frameworks/Python.framework/Versions/2.7)
VISIT_OPTION_DEFAULT(PYTHON_INCLUDE_PATH /usr/local/opt/python/Frameworks/Python.framework/Versions/2.7/include/python2.7 )
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY /usr/local/opt/python/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib)
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY_DIR /usr/local/opt/python/Frameworks/Python.framework/Versions/2.7/lib)
VISIT_OPTION_DEFAULT(PYTHON_VERSION 2.7)
SET(VISIT_PYTHON_SKIP_INSTALL ON)

##
## Qt
##
SETUP_APP_VERSION(QT 5.10.0)
VISIT_OPTION_DEFAULT(VISIT_QT_DIR /usr/local/Cellar/qt/${QT_VERSION}_1)
SET(VISIT_QT_SKIP_INSTALL ON)

##
## QWT
##
SETUP_APP_VERSION(QWT 6.1.2)
VISIT_OPTION_DEFAULT(VISIT_QWT_DIR /usr/local)

##
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})

##
## BOOST
##
SETUP_APP_VERSION(BOOST 1_59_0)
VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR /usr/local)

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/1.3.5/${VISITARCH})

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/local)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP /usr/local/lib sz /usr/lib z)

##
## H5Part
##
#VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR /usr/local)
#VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /usr/local)

##
## SZIP
##
VISIT_OPTION_DEFAULT(SZIP_DIR /usr/local)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/local)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} /usr/lib z TYPE STRING)

##
## PySide
##
VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR /usr/local)
