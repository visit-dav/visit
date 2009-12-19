#/what/is/the/path/to/bin/cmake

##
## build_vist generated host.conf
## created: Tue Oct 16 16:35:45 EDT 2007
## system: Darwin muon.ornl.gov 8.10.2 Darwin Kernel Version 8.10.2: Wed Jun 20 21:49:49 PDT 2007; root:xnu-792.22.6~1/RELEASE_I386 i386 i386
## by: js9

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /apps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.4.2/i386-apple-darwin8)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/i386-apple-darwin8/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/i386-apple-darwin8/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/i386-apple-darwin8)

## Compiler flage.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-fno-common -fexceptions")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-fno-common -fexceptions")

##
## Database reader plugin support libraries
##
##############################################################

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/i386-apple-darwin8)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/i386-apple-darwin8)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/i386-apple-darwin8)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/i386-apple-darwin8)

##
## SILO (must be after HDF5 symbols to reference them correctly)
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.5.1/i386-apple-darwin8)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
