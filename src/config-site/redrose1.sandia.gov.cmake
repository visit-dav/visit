#/what/is/the/path/to/bin/cmake

##
## Configuration File for the "redrose" Machine at Sandia
##
## NOTES:
##

SET(VISITHOME /projects/llnl/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)


##
## If MESA is not set, use VisIt's mesa.
##
#if test -z "$MESA"; then
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/6.4.2/linux_x86_64_icc_9.1)
#fi

##
## If VTK is not set, use VisIt's vtk.
##
#if test -z "$VTK"; then
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux_x86_64_icc_9.1/lib/vtk-5.0)
#fi

##
## If QT is not set, use VisIt's Qt.
##
#if test -z "$VISIT_QT_BIN"; then
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux_x86_64_icc_9.1/bin)
#fi
#if test -z "$QT_INCLUDE"; then
#fi
#if test -z "$QT_LIB"; then
#fi

##
## Use VisIt's Python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux_x86_64_icc_9.1)

## Turn off warnings for deprecated features.
##
SET(VISIT_C_FLAGS "-fPIC")
SET(VISIT_CXX_FLAGS "-Wno-deprecated -fPIC")

##
## Add parallel arguments.
##
SET(VISIT_MPI_CXX_FLAGS "-I/usr/include -I/apps/x86_64/mpi/openmpi/intel-9.1/openmpi-1.1.2-ofed/include -I/usr/include -I/apps/x86_64/mpi/openmpi/intel-9.1/openmpi-1.1.2-ofed/include/openmpi -pthread")
SET(VISIT_MPI_LD_FLAGS "-L/apps/x86_64/mpi/openmpi/intel-9.1/openmpi-1.1.2-ofed/lib -L/usr/ofed/lib -L/usr/ofed/lib64/infiniband -L/usr/ofed/lib64 -L/apps/x86_64/mpi/openmpi/intel-9.1/openmpi-1.1.2-ofed/lib/openmpi-Wl,-rpath,/apps/x86_64/mpi/openmpi/intel-9.1/openmpi-1.1.2-ofed/lib -Wl,--export-dynamic")
SET(VISIT_MPI_LIBS mpi mpi_cxx orte opal ibverbs rt nsl util dl)


##
## Database reader plugin support libraries
##
###############################################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/linux_x86_64_icc_9.1)
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/linux_x86_64_icc_9.1)

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/linux_x86_64_icc_9.1)

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/linux_x86_64_icc_9.1)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.2/linux_x86_64_icc_9.1)

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.1/linux_x86_64_icc_9.1)
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux_x86_64_icc_9.1)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.10.0/linux_x86_64_icc_9.1)

##
## netCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux_x86_64_icc_9.1)

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/linux_x86_64_icc_9.1)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## ViSUS 
##
##DEFAULT_VISUS_INCLUDE=$VISITHOME/visus/linux_x86_64_icc_9.1/include
##DEFAULT_VISUS_LIB=$VISITHOME/visus/linux_x86_64_icc_9.1/lib
