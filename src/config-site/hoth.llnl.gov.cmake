#/usr/gapps/visit/cmake/2.6.4/linux-i686_gcc-3.2/bin/cmake

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISITARCH linux_rhel3_gcc_3.2.3)
SET(SILOHOME /usr/gapps/silo)
SET(SILOARCH i686_Linux_ELsmp)

SET(VISIT_VERBOSE_MAKEFILE TRUE)

SET(VISIT_CXX_FLAGS -Wno-deprecated)

# Set the location for where we find Mesa
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/linux-i686_gcc-3.2)

# Set VISIT_VTK_DIR, which is used by the FindVTK module to locate VTK)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0d/linux-i686_gcc-3.2/lib/vtk-5.0)

# Add Qt's qmake to the path so we can use CMake's Qt4 autodetection.
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/${VISITARCH}/bin)

# Set the location for where we find Python
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-i686_gcc-3.2)

# Set the location for where we find tcmalloc
VISIT_OPTION_DEFAULT(VISIT_TCMALLOC_DIR ${VISITHOME}/google-perftools/0.97/linux-i686_gcc-3.2.3)

##
## Add parallel arguments.
##
SET(VISIT_MPI_LIBRARY_DIR /misc/gapps/mpich/1.2.4/Linux/serial/64/debug/lib)
SET(VISIT_MPI_CXX_FLAGS "-I/misc/gapps/mpich/1.2.4/Linux/serial/64/debug/include")
SET(VISIT_MPI_LD_FLAGS "-L${VISIT_MPI_LIBRARY_DIR}")
SET(VISIT_MPI_LIBS mpich)

##
## If enable_viewer_mesa_stub in not set, then disable it.
##
#if test -z "$enable_viewer_mesa_stub"; then
#   enable_viewer_mesa_stub=no
#fi


##
## Database reader plugin support libraries
##
##############################################################

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB2D_DIR ${VISITHOME}/boxlib/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_BOXLIB3D_DIR ${VISITHOME}/boxlib/${VISITARCH})

##
## CCMIO
##
VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR ${VISITHOME}/ccmio/2.6.1/${VISITARCH})

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/${VISITARCH})

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/2.4/${VISITARCH})

##
## ExodusII
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.46/${VISITARCH})

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.3.0/${VISITARCH})

##
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/2.0/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP /usr/lib jpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${SILOHOME}/hdf5/1.8.1/${SILOARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${SILOHOME}/szip/2.1/${SILOARCH}/lib sz)

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.4.0/${VISITARCH})

##
## ITAPS
##
# MOAB implementation
ITAPS_INCLUDE_DIRECTORIES(MOAB ${VISITHOME}/itaps/${VISITARCH}/ITAPS-1.0/MOAB/include)
ITAPS_FILE_PATTERNS(MOAB *.cub)
ITAPS_LINK_LIBRARIES(MOAB iMesh MOAB hdf5 sz z netcdf_c++ netcdf vtkGraphics)
ITAPS_LINK_DIRECTORIES(MOAB 
    ${VISITHOME}/itaps/${VISITARCH}/ITAPS-1.0/MOAB/lib
    ${SILOHOME}/hdf5/1.8.1/${SILOARCH}/lib
    ${SILOHOME}/szip/2.1/${SILOARCH}/lib
    ${VISITHOME}/netcdf/3.6.0/${VISITARCH}/lib)
# FMDB implementation
ITAPS_INCLUDE_DIRECTORIES(FMDB ${VISITHOME}/itaps/${VISITARCH}/ITAPS-1.0/FMDB/include/FMDB)
ITAPS_FILE_PATTERNS(FMDB *.sms)
ITAPS_LINK_LIBRARIES(FMDB FMDB SCORECModel SCORECUtil vtkGraphics)
ITAPS_LINK_DIRECTORIES(FMDB ${VISITHOME}/itaps/${VISITARCH}/ITAPS-1.0/FMDB/lib)
# GRUMMP implementation
ITAPS_INCLUDE_DIRECTORIES(GRUMMP ${VISITHOME}/itaps/${VISITARCH}/ITAPS-1.0/GRUMMP/include)
ITAPS_FILE_PATTERNS(GRUMMP *.bdry *.smesh *.vmesh)
ITAPS_LINK_LIBRARIES(GRUMMP iMesh_GRUMMP GR_3D GR_surf GR_2D GR_base SUMAAlog_lite OptMS vtkGraphics)
ITAPS_LINK_DIRECTORIES(GRUMMP ${VISITHOME}/itaps/${VISITARCH}/ITAPS-1.0/GRUMMP/lib)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/current/${VISITARCH})

##
## NETCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${SILOHOME}/4.7/${SILOARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## ViSUS 
##
VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR ${VISITHOME}/visus/${VISITARCH}_new)
