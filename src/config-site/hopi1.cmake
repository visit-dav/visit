#/what/is/the/path/to/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/chaos_3_x86_64_ib_gcc_3.4.4)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/chaos_3_x86_64_ib_gcc_3.4.4/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/chaos_4_x86_64_ib_gcc_4.1.2/bin)

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/chaos_3_x86_64_ib_gcc_3.4.4)

##
## Add parallel arguments.
##
SET(VISIT_C_FLAGS "-I/usr/local/tools/mvapich-gnu/include -fPIC")
SET(VISIT_CXX_FLAGS "-I/usr/local/tools/mvapich-gnu/include -fPIC")

##
## Turn off warnings for deprecated features on g++
##
SET(VISIT_CXX_FLAGS "-Wno-deprecated")

##
## Database reader plugin support libraries
##
###############################################################################
 
##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/chaos_3_x86_64_ib_gcc_3.4.4)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/chaos_3_x86_64_ib_gcc_3.4.4)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR /usr/gapps/visit/cgns/2.4/chaos_3_x86_64_ib_gcc_3.4.4)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR /usr/gapps/visit/gdal/1.3.0/chaos_3_x86_64_ib_gcc_3.4.4)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR /usr/gapps/visit/hdf4/2.4.1/chaos_3_x86_64_ib_gcc_3.4.4)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR /usr/gapps/silo/hdf5/1.8.1/chaos_4_x86_64_ib)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR /usr/gapps/visit/mili/current/chaos_3_x86_64_ib_gcc_3.4.4)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR /usr/gapps/visit/netcdf/3.6.0/chaos_3_x86_64_ib_gcc_3.4.4)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR /usr/gapps/silo/4.7/chaos_4_x86_64_ib)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## ViSUS
##
VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR /usr/gapps/visus/1.0a5/chaos_3_x86_64_ib_gcc3.4.4)
