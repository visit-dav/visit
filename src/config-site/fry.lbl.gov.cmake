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

##
## Specify the location of Mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of VTK.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0d/${VISITARCH}/lib/vtk-5.0)

##
## Specify the Qt4 binary dir. 
## (qmake is used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /opt/local/libexec/qt4-mac/bin)

##
## Specify the location of Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr)

##
## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-O2 -fno-common -fexceptions")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-O2 -fno-common -fexceptions -D__USE_ISOC99")

##
##  Parallel flags
##
SET(VISIT_MPI_C_FLAGS "-D_REENTRANT -DOMPI_SKIP_MPICXX")
SET(VISIT_MPI_CXX_FLAGS "-D_REENTRANT -DOMPI_SKIP_MPICXX")
SET(VISIT_PARALLEL_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup")
SET(VISIT_MPI_LIBS "-Wl,-u,_munmap -Wl,-multiply_defined,suppress -lmpi -lopen-rte -lopen-pal -lutil")

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
