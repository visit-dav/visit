#/what/is/the/path/to/bin/cmake

##
## build_vist generated host.conf
## created: Fri Oct 19 02:03:36 CEST 2007
## system: Linux socre13.ocre.cea.fr 2.6.9-42.ELsmp #1 SMP Wed Jul 12 23:27:17 EDT 2006 i686 i686 i386 GNU/Linux
## by: childs

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /cea/home/gpocre/childs/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.4.2/linux-i686_gcc-3.4.6)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-i686_gcc-3.4.6/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux-i686_gcc-3.4.6/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-i686_gcc-3.4.6)

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-O2")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-O2")

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
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5.1/linux-i686_gcc-3.4.6)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## SZIP
##
