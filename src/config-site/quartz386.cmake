#/usr/workspace/wsa/visit/visit/thirdparty_shared/3.3.0/toss3/cmake/3.18.2/linux-x86_64_gcc-7.3/bin/cmake
##
## ./build_visit3_3_0 generated host.cmake
## created: Tue May 31 14:34:31 PDT 2022
## system: Linux quartz770 3.10.0-1160.62.1.1chaos.ch6.x86_64 #1 SMP Wed Apr 6 12:39:56 PDT 2022 x86_64 x86_64 x86_64 GNU/Linux
## by: brugger

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/workspace/wsa/visit/visit/thirdparty_shared/3.3.0/toss3)
SET(VISITARCH linux-x86_64_gcc-7.3)
VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER /usr/tce/packages/gcc/gcc-7.3.0/bin/gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER /usr/tce/packages/gcc/gcc-7.3.0/bin/g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_FORTRAN_COMPILER no TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-std=c++11  -m64 -fPIC -fvisibility=hidden" TYPE STRING)

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-7.3.0/include TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-7.3.0/include TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-7.3.0/lib -Wl,-rpath=/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-7.3.0/lib" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich mpl)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-7.3.0/lib")

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

##
## Turn on DDT support.
##
VISIT_OPTION_DEFAULT(VISIT_DDT ON TYPE BOOL)

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
## ZLIB
##
SETUP_APP_VERSION(ZLIB 1.2.11)
VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR ${VISITHOME}/zlib/${ZLIB_VERSION}/${VISITARCH})

##
## Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/3.7.7/${VISITARCH})

##
## LLVM
##
VISIT_OPTION_DEFAULT(VISIT_LLVM_DIR ${VISITHOME}/llvm/6.0.1/${VISITARCH})

##
## MesaGL
##
VISIT_OPTION_DEFAULT(VISIT_MESAGL_DIR ${VISITHOME}/mesagl/17.3.9/${VISITARCH})

##
## Qt
##
SETUP_APP_VERSION(QT 5.10.1)
VISIT_OPTION_DEFAULT(VISIT_QT_DIR ${VISITHOME}/qt/${QT_VERSION}/${VISITARCH})

##
## QWT
##
SETUP_APP_VERSION(QWT 6.1.2)
VISIT_OPTION_DEFAULT(VISIT_QWT_DIR ${VISITHOME}/qwt/${QWT_VERSION}/${VISITARCH})

##
## ISPC
##
SETUP_APP_VERSION(ISPC 1.9.2)
VISIT_OPTION_DEFAULT(VISIT_ISPC_DIR ${VISITHOME}/ispc/${ISPC_VERSION}/${VISITARCH})

##
## TBB
##
VISIT_OPTION_DEFAULT(TBB_ROOT ${VISITHOME}/tbb/tbb2018_20171205oss/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_TBB_DIR ${VISITHOME}/tbb/tbb2018_20171205oss/${VISITARCH})

##
## EMBREE
##
SETUP_APP_VERSION(EMBREE 3.2.0)
VISIT_OPTION_DEFAULT(VISIT_EMBREE_DIR ${VISITHOME}/embree/${EMBREE_VERSION}/${VISITARCH})

##
## OSPRay
##
VISIT_OPTION_DEFAULT(VISIT_OSPRAY ON TYPE BOOL)
SETUP_APP_VERSION(OSPRAY 1.6.1)
VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR ${VISITHOME}/ospray/${OSPRAY_VERSION}/${VISITARCH})

##
## VTK
##
SETUP_APP_VERSION(VTK 8.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_VTK_INCDEP ZLIB_INCLUDE_DIR)
VISIT_OPTION_DEFAULT(VISIT_VTK_LIBDEP ZLIB_LIBRARY)

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.14/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_MPI_DIR ${VISITHOME}/hdf5_mpi/1.8.14/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz ${VISITHOME}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib z TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_HDF5_MPI_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz ${VISITHOME}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib z TYPE STRING)

##
## BLOSC
##
VISIT_OPTION_DEFAULT(VISIT_BLOSC_DIR ${VISITHOME}/blosc/1.21.1/${VISITARCH})

##
## ADIOS
## (configured w/ mpi compiler wrapper)
##
SETUP_APP_VERSION(ADIOS 1.13.1)
VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR ${VISITHOME}/adios/${ADIOS_VERSION}/${VISITARCH})

##
## ADIOS2
##
SETUP_APP_VERSION(ADIOS2 2.7.1)
VISIT_OPTION_DEFAULT(VISIT_ADIOS2_DIR ${VISITHOME}/adios2-ser/${ADIOS2_VERSION}/${VISITARCH})
## (configured w/ mpi compiler wrapper)
VISIT_OPTION_DEFAULT(VISIT_ADIOS2_PAR_DIR ${VISITHOME}/adios2-par/${ADIOS2_VERSION}/${VISITARCH})

##
## AdvIO
##
VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR ${VISITHOME}/AdvIO/1.2/${VISITARCH})

##
## BOOST
##
SETUP_APP_VERSION(BOOST 1_67_0)
VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR ${VISITHOME}/boost/${BOOST_VERSION}/${VISITARCH})

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/1.3.5/${VISITARCH})

##
## CFITSIO
##
VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR ${VISITHOME}/cfitsio/3006/${VISITARCH})

##
## CGNS
##
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/4.1.0/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## Conduit
##
SETUP_APP_VERSION(CONDUIT 0.8.4)
VISIT_OPTION_DEFAULT(VISIT_CONDUIT_DIR ${VISITHOME}/conduit/v0.8.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_CONDUIT_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## FMS
##
VISIT_OPTION_DEFAULT(VISIT_FMS_DIR ${VISITHOME}/fms/0.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_FMS_LIBDEP CONDUIT_LIBRARY_DIR conduit CONDUIT_LIBRARY_DIR conduit_blueprint CONDUIT_LIBRARY_DIR conduit_relay ${VISIT_CONDUIT_LIBDEP} TYPE STRING)

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/2.2.4/${VISITARCH})

##
## H5Part
##
SETUP_APP_VERSION(H5PART 1.6.6)
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/${H5PART_VERSION}/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## IceT
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/77c708f9090236b576669b74c53e9f105eedbd7e/${VISITARCH})

##
## MFEM 
##
VISIT_OPTION_DEFAULT(VISIT_MFEM_DIR ${VISITHOME}/mfem/4.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_MFEM_INCDEP CONDUIT_INCLUDE_DIR FMS_INCLUDE_DIR TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MFEM_LIBDEP ${VISIT_CONDUIT_LIBDEP} ${VISITHOME}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib z TYPE STRING)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/22.1/${VISITARCH})

##
## MOAB 
##
VISIT_OPTION_DEFAULT(VISIT_MOAB_DIR ${VISITHOME}/moab/4.9.2-RC0/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_MOAB_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MOAB_MPI_DIR ${VISITHOME}/moab_mpi/4.9.2-RC0/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_MOAB_MPI_LIBDEP HDF5_MPI_LIBRARY_DIR hdf5_mpi ${VISIT_HDF5_MPI_LIBDEP} TYPE STRING)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/4.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIBRARY_DIR hdf5_hl HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} TYPE STRING)

##
## OpenEXR
##
VISIT_OPTION_DEFAULT(VISIT_OPENEXR_DIR ${VISITHOME}/openexr/2.2.0/${VISITARCH})

##
## PIDX
##
#SETUP_APP_VERSION(PIDX 0.9.3)
#VISIT_OPTION_DEFAULT(VISIT_PIDX_DIR ${VISITHOME}/pidx/${PIDX_VERSION}/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} ZLIB_LIBRARY_DIR z TYPE STRING)

##
## Uintah
##
SETUP_APP_VERSION(UINTAH 2.6.1)
VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR ${VISITHOME}/uintah/${UINTAH_VERSION}/${VISITARCH})

##
## VTKM
##
VISIT_OPTION_DEFAULT(VISIT_VTKM_DIR ${VISITHOME}/vtkm/v1.7.0/${VISITARCH})

##
## VTKH
##
VISIT_OPTION_DEFAULT(VISIT_VTKH_DIR ${VISITHOME}/vtkh/v0.8.0/${VISITARCH})

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5  VTK_LIBRARY_DIRS vtklibxml2-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}  TYPE STRING)

