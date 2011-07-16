#/opt/local/bin/cmake
##
## ../visit_svn/trunk/src/svn_bin/build_visit generated host.cmake
## created: Mon Jan  4 19:13:25 PST 2010
## system: Darwin fry.local 10.2.0 Darwin Kernel Version 10.2.0: Tue Nov  3 10:37:10 PST 2009; root:xnu-1486.2.11~1/RELEASE_I386 i386
## by: ghweber

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /Users/ghweber/devel/visit_3rdparty)
SET(VISITARCH i386-apple-darwin10_gcc-4.2)
SET(VISIT_VERBOSE_MAKEFILE TRUE)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/local/visit)
SET(CMAKE_BUILD_TYPE Debug CACHE STRING "")
VISIT_OPTION_DEFAULT(VISIT_PYTHON_FILTERS ON)

##
## Specify the location of Mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.10.2/${VISITARCH})

##
## Specify the location of VTK.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0i/${VISITARCH}/lib/vtk-5.0/)

##
## Specify the Qt4 binary dir. 
## (qmake is used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /opt/local/bin)

##
## Specify the location of Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr)

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-fno-common -fexceptions -fvisibility=hidden")
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-fno-common -fexceptions -fvisibility=hidden -D__USE_ISOC99")

##
##  Parallel flags
##
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS "-D_REENTRANT -DOMPI_SKIP_MPICXX")
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-D_REENTRANT -DOMPI_SKIP_MPICXX")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS "-Wl,-u,_munmap -Wl,-multiply_defined,suppress /usr/lib/libmpi.dylib /usr/lib/libopen-rte.dylib /usr/lib/libopen-pal.dylib /usr/lib/libutil.dylib")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)

##############################################################
##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/2011.04.28/${VISITARCH})

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
#VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.2/${VISITARCH})
#VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

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
