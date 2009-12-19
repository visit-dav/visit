#/Users/ghweber/devel/visit_3rdparty/cmake/2.6.4/i386-apple-darwin10_gcc-4.2/bin/cmake

##
## ../visit_svn/trunk/src/svn_bin/build_visit generated host.conf
## created: Tue Oct 20 15:05:35 PDT 2009
## system: Darwin parallax.lbl.gov 10.0.0 Darwin Kernel Version 10.0.0: Fri Jul 31 22:47:34 PDT 2009; root:xnu-1456.1.25~1/RELEASE_I386 i386
## by: ghweber

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /Users/ghweber/devel/visit_3rdparty)
SET(VISITARCH i386-apple-darwin10_gcc-4.2)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/${VISITARCH}/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /opt/local/libexec/qt4-mac/bin)
#VISIT_QT_BIN=$VISITHOME/qt/4.6.0-beta1/$VISITARCH/bin
#not done -- QT_INCLUDE = /opt/local/libexec/qt4-mac/include
#QT_INCLUDE=$VISITHOME/qt/4.6.0-beta1/$VISITARCH/include
#not done -- QT_LIB = /opt/local/libexec/qt4-mac/lib
#QT_LIB=$VISITHOME/qt/4.6.0-beta1/$VISITARCH/lib

##
## Specify the location of the python include and libraries.
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
##  Parallel Args
##
SET(VISIT_C_FLAGS "-D_REENTRANT")
SET(VISIT_MPI_C_FLAGS "-DOMPI_SKIP_MPICXX")

##
## Database reader plugin support libraries
##
##############################################################

##
## AdvIO
##

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/${VISITARCH})

##
## CFITSIO
##

##
## CGNS
##

##
## Exodus
##

##
## GDAL
##

##
## H5Part
##

##
## FastBit
##

##
## HDF4
##

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.3/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##

##
## NetCDF
##

##
## SZIP
##
SET(SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## CCMIO
##
##

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
