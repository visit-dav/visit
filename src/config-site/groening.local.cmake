#/opt/local/bin/cmake
##
## ../visit_svn/2.3RC/src/svn_bin/build_visit generated host.cmake
## created: Fri Jul 22 16:38:17 PDT 2011
## system: Darwin groening.local 11.0.0 Darwin Kernel Version 11.0.0: Sat Jun 18 12:56:35 PDT 2011; root:xnu-1699.22.73~1/RELEASE_X86_64 x86_64
## by: ghweber

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /Users/ghweber/devel/visit_3rdparty)
SET(VISITARCH i386-apple-darwin11_gcc-4.2)

##
## Specify the location of the mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.8.2/${VISITARCH})

##
## Specify the location of the vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0i/${VISITARCH}/lib/vtk-5.0/)

##
## Specify the Qt4 binary dir. 
## (qmake is used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN /opt/local/bin)

##
## Specify the location of the python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr)

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-fno-common -fexceptions -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-fno-common -fexceptions -fvisibility=hidden" TYPE STRING)

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
## SZIP
##
VISIT_OPTION_DEFAULT(SZIP_DIR /opt/local)

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
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /opt/local)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
