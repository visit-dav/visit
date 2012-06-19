#/opt/local/bin/cmake
##
## ../visit_svn/trunk/src/svn_bin/build_visit generated host.cmake
## created: Mon Jan  4 19:13:25 PST 2010
## system: Darwin fry.lbl.gov 11.3.0 Darwin Kernel Version 11.3.0: Thu Jan 12 18:47:41 PST 2012; root:xnu-1699.24.23~1/RELEASE_X86_64 x86_64
## by: ghweber

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /Users/ghweber/devel/visit_3rdparty)
SET(VISITARCH i386-apple-darwin11_gcc-4.2)
#SET(VISIT_VERBOSE_MAKEFILE TRUE)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/local/visit)
SET(CMAKE_BUILD_TYPE Release CACHE STRING "")
VISIT_OPTION_DEFAULT(VISIT_PYTHON_FILTERS ON)
VISIT_OPTION_DEFAULT(VISIT_CREATE_APPBUNDLE_PACKAGE ON)

##
## Specify the location of Mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.8.2/${VISITARCH})

##
## Specify the location of VTK.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.8.0/${VISITARCH})

##
## Specify the Qt4 binary dir. 
## (qmake is used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /opt/local/bin)

##
## Specify the location of Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /opt/local/Library/Frameworks/Python.framework/Versions/2.7)
VISIT_OPTION_DEFAULT(PYTHON_INCLUDE_PATH /opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 )
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY /opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib)
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY_DIR /opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib)

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-fno-common -fexceptions -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-fno-common -fexceptions -fvisibility=hidden" TYPE STRING)

##
##  Parallel flags
##
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS "-DOMPI_SKIP_MPICXX -I/opt/local/include/openmpi")
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-DOMPI_SKIP_MPICXX -I/opt/local/include/openmpi")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS "/opt/local/lib/libmpi.dylib")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)

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
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/0.1.8/${VISITARCH})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.2.0/${VISITARCH})

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
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /opt/local)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
