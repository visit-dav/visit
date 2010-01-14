#/Users/hchilds/visit_trial//cmake/2.6.4/i386-apple-darwin9_gcc-4.0/bin/cmake
##
## ./build_visit generated host.cmake
## created: Wed Jan 13 12:36:50 PST 2010
## system: Darwin capri.dhcp.lbl.gov 9.7.0 Darwin Kernel Version 9.7.0: Tue Mar 31 22:52:17 PDT 2009; root:xnu-1228.12.14~1/RELEASE_I386 i386
## by: hchilds

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /Users/hchilds/visit_trial)
SET(VISITARCH i386-apple-darwin9_gcc-4.0)

##
## Specify the location of the mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of the vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0d/${VISITARCH}/lib/vtk-5.0/)

##
## Specify the Qt4 binary dir. 
## (qmake us used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/${VISITARCH}/bin)

##
## Specify the location of the python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.6.4/${VISITARCH})

##
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-O2 -fno-common -fexceptions")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-O2 -fno-common -fexceptions")

##############################################################
##
## Database reader plugin support libraries
##
##############################################################

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.2/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

