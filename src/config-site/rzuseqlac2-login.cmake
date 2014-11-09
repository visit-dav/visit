#/usr/gapps/visit/thirdparty_static/2.7.0/cmake/2.8.10.2/linux-ppc64_gcc-4.4/bin/cmake
##
## ./build_visit generated host.cmake
## created: Fri May 31 12:16:29 PDT 2013
## system: Linux rzuseqlac2 2.6.32-358.6chaos.bgq62.ppc64 #1 SMP Thu Apr 4 11:21:55 PDT 2013 ppc64 ppc64 ppc64 GNU/Linux
## by: whitlocb

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/gapps/visit/thirdparty_static/2.7.0)
SET(VISITARCH linux-ppc64_gcc-4.4)
MESSAGE(status "************* login node build")
## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER   /usr/bin/gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER /usr/bin/g++ TYPE FILEPATH)

##
## Static build
##
VISIT_OPTION_DEFAULT(VISIT_STATIC ON TYPE BOOL)

## server components only
VISIT_OPTION_DEFAULT(VISIT_SERVER_COMPONENTS_ONLY ON TYPE BOOL)

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
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr/gapps/visit/thirdparty_static/2.7.0/python/2.7.3/linux-ppc64_gcc-4.4)

##
## VTK
##
SETUP_APP_VERSION(VTK 6.0.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})
##

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.7/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib z TYPE STRING)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.11.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.9.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

