#/what/is/the/path/to/bin/cmake

##
## build_vist generated host.conf
## created: Fri Oct 19 18:59:40 CEST 2007
## system: Linux GraphUVSQ0 2.6.9-22.ELsmp #1 SMP Mon Sep 19 18:00:54 EDT 2005 x86_64 x86_64 x86_64 GNU/Linux
## by: childs

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /cea/home/gpocre/childs/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.4.2/linux-x86_64_gcc-3.4.4)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-3.4.4/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux-x86_64_gcc-3.4.4/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-3.4.4)

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC")

##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##

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
## HDF4
##

##
## HDF5
##

##
## Mili
##

##
## NetCDF
##

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5.1/linux-x86_64_gcc-3.4.4)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## SZIP
##
