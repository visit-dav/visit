#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## If MESA is not set, use VisIt's mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux_rhel3_gcc_3.2.2)

##
## If VTK is not set, use VisIt's vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_rhel3_gcc_3.2.2/lib/vtk-5.0)

##
## If QT is not set, use VisIt's Qt.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.2/linux_rhel3_gcc_3.2.2/bin)

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/linux_rhel3_gcc_3.2.2)

##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
##DEFAULT_BOXLIB2D_INCLUDE=/usr/gapps/visit/boxlib/chaos_3_x86_elan3_gcc_3.2.3/include/2D
##DEFAULT_BOXLIB2D_LIBS=/usr/gapps/visit/boxlib/chaos_3_x86_elan3_gcc_3.2.3/lib
##DEFAULT_BOXLIB3D_INCLUDE=/usr/gapps/visit/boxlib/chaos_3_x86_elan3_gcc_3.2.3/include/3D
##DEFAULT_BOXLIB3D_LIBS=/usr/gapps/visit/boxlib/chaos_3_x86_elan3_gcc_3.2.3/lib

##
## HDF5
##
##DEFAULT_HDF5_INCLUDE=/usr/global/tools/hdf5/installs/chaos_3_x86_elan3/hdf5-1.6.5/serial_gcc/include
##DEFAULT_HDF5_LIB=/usr/global/tools/hdf5/installs/chaos_3_x86_elan3/hdf5-1.6.5/serial_gcc/lib

##
## Mili
##
##VISIT_MILI_DIR=/usr/gapps/visit/mili/current/linux_chaos_gcc_3.3.3

##
## netCDF
##
##DEFAULT_NETCDF_INCLUDE=/usr/gapps/visit/netcdf/3.6.0/linux_chaos_gcc_3.3.3/include
##DEFAULT_NETCDF_LIB=/usr/gapps/visit/netcdf/3.6.0/linux_chaos_gcc_3.3.3/lib

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/visit/silo/4.7/i686_Linux_ELsmp)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
