#/opt/local/bin/cmake
##
## ../visit_svn/2.6RC_trunk/src/svn_bin/build_visit generated host.cmake
## created: Fri Dec  7 17:18:25 PST 2012
## system: Darwin bart.dhcp.lbl.gov 12.2.0 Darwin Kernel Version 12.2.0: Sat Aug 25 00:48:52 PDT 2012; root:xnu-2050.18.24~1/RELEASE_X86_64 x86_64
## by: ghweber

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /Users/ghweber/devel/visit_3rdparty)
SET(VISITARCH i386-apple-darwin13_clang)

VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/local/visit)
SET(CMAKE_BUILD_TYPE Release CACHE STRING "")
VISIT_OPTION_DEFAULT(VISIT_PYTHON_FILTERS ON)
VISIT_OPTION_DEFAULT(VISIT_CREATE_APPBUNDLE_PACKAGE ON)

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER clang TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER clang++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-fno-common -fexceptions -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-fno-common -fexceptions -fvisibility=hidden" TYPE STRING)

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
## (configured w/ mpi compiler wrapper)
VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER /opt/local/bin/mpicxx TYPE FILEPATH)

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
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /opt/local/Library/Frameworks/Python.framework/Versions/2.7)
VISIT_OPTION_DEFAULT(PYTHON_INCLUDE_PATH /opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 )
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY /opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib)
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY_DIR /opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib)
VISIT_OPTION_DEFAULT(PYTHON_VERSION 2.7)
SET(VISIT_PYTHON_SKIP_INSTALL ON)

##
## Qt
##
VISIT_OPTION_DEFAULT(QT_QTUITOOLS_INCLUDE_DIR /opt/local/include/QtUiTools)
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /opt/local/bin)
SET(VISIT_QT_SKIP_INSTALL ON)

##
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/1.0.7/${VISITARCH})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.2.0/${VISITARCH})

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /opt/local)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP /opt/local/lib sz /opt/local/lib z)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR /opt/local)
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /opt/local)

##
## SZIP
##
VISIT_OPTION_DEFAULT(SZIP_DIR /opt/local)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP /usr/lib z TYPE STRING)
#VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## PySide
##
VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR /opt/local)
SET(SHIBOKEN_PYTHON_SUFFIX "-python2.7")
SET(SHIBOKEN_INCLUDE_DIR "/opt/local/include/shiboken-2.7")
SET(SHIBOKEN_LIBRARY "/opt/local/lib/libshiboken-python2.7.dylib")
SET(SHIBOKEN_PYTHON_INCLUDE_DIR "/opt/local/include/shiboken-2.7")
SET(SHIBOKEN_PYTHON_INTERPRETER "/opt/local/bin/python2.7")
SET(SHIBOKEN_PYTHON_LIBRARIES "-undefined dynamic_lookup")
SET(SHIBOKEN_BINARY "/opt/local/bin/shiboken-2.7")
SET(Shiboken_FOUND 1)
SET(PYSIDE_INCLUDE_DIR "/opt/local/include/PySide-2.7")
SET(PYSIDE_LIBRARY "/opt/local/lib/libpyside-python2.7.dylib")
SET(PYSIDE_PYTHONPATH "/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages")
SET(PYSIDE_TYPESYSTEMS "/opt/local/share/PySide-2.7/typesystems")
SET(PySide_FOUND 1)
