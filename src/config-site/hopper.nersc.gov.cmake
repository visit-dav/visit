#/usr/common/graphics/installs/visit_3rdparty/cmake/2.8.0/linux-x86_64_gcc-4.5/bin/cmake
##
## ../visit_svn/2.1RC_trunk/src/svn_bin/build_visit generated host.cmake
## created: Mon Nov 15 17:10:11 PST 2010
## system: Linux hopper02 2.6.27.48-0.12-default #1 SMP 2010-09-20 11:03:26 -0400 x86_64 x86_64 x86_64 GNU/Linux
## by: ghweber

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/common/graphics/installs/visit_3rdparty)
SET(VISITARCH linux-x86_64_gcc-4.5)
SET(VISIT_VERBOSE_MAKEFILE TRUE)
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /usr/common/graphics/visit)
VISIT_OPTION_DEFAULT(CMAKE_BUILD_TYPE Release)

##
## Do not build Tuvok
##
VISIT_OPTION_DEFAULT(VISIT_TUVOK OFF)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.8.2/${VISITARCH})

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0i/${VISITARCH}/lib/vtk-5.0)

##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.6.1/${VISITARCH}/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.6.4/${VISITARCH})

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC -DVIZSCHEMA_DECOMPOSE_DOMAINS")
SET(VISIT_MPI_C_FLAGS "-I/opt/cray/mpt/default/xt/gemini/mpich2-gnu/include")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC -DVIZSCHEMA_DECOMPOSE_DOMAINS")
#SET(VISIT_EXE_LINKER_FLAGS "-Wl,--rpath,/opt/gcc/4.3.3/snos/lib64")
# Get these via CC -v
SET(VISIT_MPI_CXX_FLAGS "-I/opt/cray/mpt/default/xt/gemini/mpich2-gnu/include")
SET(VISIT_MPI_LD_FLAGS "-L/opt/cray/mpt/default/xt/gemini/mpich2-gnu/lib -L/opt/cray/pmi/default/lib64 -L/opt/cray/stat/default/lib/alps")
VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH "/opt/cray/mpt/default/xt/gemini/mpich2-gnu/lib;/opt/cray/pmi/default/lib64;/opt/cray/stat/default/lib/alps")
SET(VISIT_MPI_LIBS mpich pmi alpslli alpsutil pthread rt)
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)
VISIT_OPTION_DEFAULT(VISIT_CREATE_SOCKET_RELAY_EXECUTABLE ON)

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
## HDF4
##
VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR ${VISITHOME}/hdf4/4.2.5/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz ${VISITHOME}/vtk/5.0.0i/${VISITARCH}/lib vtkjpeg)

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib z)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/4.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIBRARY_DIR hdf5_hl HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## AdvIO
##
VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR ${VISITHOME}/AdvIO/1.2/${VISITARCH}/)

##
## Boxlib
##
VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR ${VISITHOME}/boxlib/2011.04.28/${VISITARCH})

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
VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR ${VISITHOME}/cgns/3.0.8/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Exodus
##
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR ${VISITHOME}/exodus/4.98/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_EXODUSII_LIBDEP NETCDF_LIBRARY_DIR netcdf ${VISIT_NETCDF_LIBDEP})

##
## FastBit
##
VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR ${VISITHOME}/fastbit/1.2.0/${VISITARCH})

##
## GDAL
##
VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR ${VISITHOME}/gdal/1.7.1/${VISITARCH})

##
## H5Part
##
VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR ${VISITHOME}/h5part/1.6.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.8/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})


##
## ITAPS
##
## MOAB implementation
ITAPS_INCLUDE_DIRECTORIES(MOAB ${VISITHOME}/itaps/1.2/MOAB/4.0.1RC2/${VISITARCH}/include)
ITAPS_FILE_PATTERNS(MOAB *.cub)
ITAPS_LINK_LIBRARIES(MOAB iMesh MOAB hdf5 sz z netcdf_c++ netcdf vtkGraphics)
ITAPS_LINK_DIRECTORIES(MOAB  ${VISITHOME}/itaps/1.2/MOAB/4.0.1RC2/${VISITARCH}/lib  ${VISITHOME}/hdf5/1.8.4/${VISITARCH}/lib  ${VISITHOME}/szip/2.1/${VISITARCH}/lib  ${VISITHOME}/netcdf/4.1.1/${VISITARCH}/lib)
## FMDB implementation
ITAPS_INCLUDE_DIRECTORIES(FMDB ${VISITHOME}/itaps/1.2/FMDB/1.2/${VISITARCH}/include)
ITAPS_FILE_PATTERNS(FMDB *.sms)
ITAPS_LINK_LIBRARIES(FMDB FMDB SCORECModel SCORECUtil vtkGraphics)
ITAPS_LINK_DIRECTORIES(FMDB ${VISITHOME}/itaps/1.2/FMDB/1.2/${VISITARCH}/lib)
## GRUMMP implementation
ITAPS_INCLUDE_DIRECTORIES(GRUMMP ${VISITHOME}/itaps/1.2/GRUMMP/0.6.3/${VISITARCH}/include)
ITAPS_FILE_PATTERNS(GRUMMP *.bdry *.smesh *.vmesh)
ITAPS_LINK_LIBRARIES(GRUMMP iMesh_GRUMMP GR_3D GR_surf GR_2D GR_geom GR_base SUMAAlog_lite OptMS vtkGraphics cgm dl)
ITAPS_LINK_DIRECTORIES(GRUMMP ${VISITHOME}/itaps/1.2/GRUMMP/0.6.3/${VISITARCH}/lib)
