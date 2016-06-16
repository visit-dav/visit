#/apps/visit/thirdparty/visit/cmake/3.0.2/linux-x86_64_gcc-5.3/bin/cmake
##
## ./build_visit generated host.cmake
## created: Wed Apr 20 17:59:15 EDT 2016
## system: Linux whoopingcough 4.4.0-18-generic #34-Ubuntu SMP Wed Apr 6 14:01:02 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
## by: pugmire

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /apps/visit/thirdparty/visit)
SET(VISITARCH linux-x86_64_gcc-5.3)

## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

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
#VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.7.11/${VISITARCH})

VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR /usr)
VISIT_OPTION_DEFAULT(PYTHON_INCLUDE_PATH /usr/include/python2.7 )
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY /usr/lib/x86_64-linux-gnu/libpython2.7.so)
VISIT_OPTION_DEFAULT(PYTHON_LIBRARY_DIR /usr/lib/x86_64-linux-gnu)
VISIT_OPTION_DEFAULT(PYTHON_VERSION 2.7)
SET(VISIT_PYTHON_SKIP_INSTALL ON)


##
## Qt
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.8.3/${VISITARCH}/bin)

##
## QWT
##
SETUP_APP_VERSION(QWT 6.1.2)
VISIT_OPTION_DEFAULT(VISIT_QWT_DIR ${VISITHOME}/qwt/${QWT_VERSION}/${VISITARCH})

##
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})
##

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP /usr/lib/x86_64-linux-gnu z TYPE STRING)


##
## ADIOS
##
VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR /apps/adios)


##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/4.1.1/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.14/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib/x86_64-linux-gnu z TYPE STRING)

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/icet-master-0d08b037/${VISITARCH})
