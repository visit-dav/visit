#/usr/local/itaps/visit/cmake/2.6.4/linux-x86_64_gcc-4.2.3/bin/cmake

# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.
# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.

##
## build_vist generated host.conf
## created: Tue Sep  2 15:41:56 CDT 2008
## system: Linux mesh 2.6.24-19-generic #1 SMP Wed Aug 20 17:53:40 UTC 2008 x86_64 GNU/Linux
## by: mcmiller
##
## For Tim Tautge's machine, mesh.ep.wisc.edu
##
## Modifications:
##
##     Mark C. Miller, modified for ITAPS plugins.

##
## Set the VISITHOME environment variable.
##
SET(VISITHOME /usr/local/itaps/visit)
SET(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Specify the location of the mesa include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/5.0/linux-x86_64_gcc-4.2.3)

##
## Specify the location of the vtk include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0c/linux-x86_64_gcc-4.2.3/lib/vtk-5.0)

##
## Specify the location of cmake.
##


##
## Specify the location of the qt include files and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/3.3.8/linux-x86_64_gcc-4.2.3/bin)

##
## Specify the location of the python include and libraries.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/linux-x86_64_gcc-4.2.3)

## Compiler flags.
##
SET(VISIT_C_COMPILER gcc)
SET(VISIT_C_FLAGS "-m64 -fPIC")
SET(VISIT_CXX_COMPILER g++)
SET(VISIT_CXX_FLAGS "-m64 -fPIC")
SET(VISIT_MPI_CXX_FLAGS "-DMPICH_IGNORE_CXX_SEEK")

##
## Database reader plugin support libraries
##
##############################################################

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.6.5/linux-x86_64_gcc-4.2.3)
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.0/linux-x86_64_gcc-4.2.3)

##
## ITAPS
##
## Because there can often be multiple implementations of the ITAPS interfaces
## we handle ITAPS very differently. The symbol below is a space separated
## string of implementation specifications. In turn, each implementation
## specification is a colon separated string of implementation details. In turn
## each implementation detail is a comma separated string of tokens. The
## format of each implementation specification is as follows...
##
## <impl-base-name>:<ext1>,<ext2>,...:<cflag1>,<cflag2>,...:<lflag1>,<lflag2>,...
##
## note the colon and comma separation of this string.
##
## where
##
##     impl-base-name is the base name of the implementation (e.g. GRUMMP). Note that
##         the actual plugin name is the concatenation of 'ITAPS_' with the impl-base-name
##         (e.g. ITAPS_GRUMMP)
##     exti is a filename extension to be associated with the implementation's plugin. You
##         can associate as many extensions as you wish but be sure to comma separate them.
##     cflagi is a flag to pass during the compile phase of the plugin. Usually, you use
##         cflag entries to specify include file paths (be sure to include the leading '-I'
##         for them). You can specify as many cflag entries as you wish but comma separate
##         them.
##     lflagi is a flag to pass during the link phase of the plugin. Usually, you use lflag
##         entries to specify library file paths (be sure to include the leading '-L' for
##         them).
##
## When you are doing development on the ITAPS plugin, as opposed to installing it for
## possibly multiple ITAPS implementations, then use the special base-name 'ITAPS_C',
## along with whatever other entries in the implementation specification you need
## and then just work directly on the source code in src/databases/ITAPS_C.
##
#not done -- DEFAULT_ITAPS_IMPLS = "MOAB:cub,vmesh:-DITAPS_MOAB,-I/usr/local/itaps/MOABser/include:-L/usr/local/itaps/MOABser/lib,-Wl,-rpath=/usr/local/itaps/MOABser/lib,-liMesh,-lMOAB GRUMMP:bdry,smesh,vmesh:-I/usr/local/itaps/GRUMMP/include/ITAPS:-L/usr/local/itaps/GRUMMP/lib/ITAPS,-L/usr/local/itaps/GRUMMP/lib,-Wl,-rpath=/usr/local/itaps/GRUMMP/lib/ITAPS,-Wl,-rpath=/usr/local/itaps/GRUMMP/lib,-liMesh_GRUMMP,-lGR_3D,-lGR_surf,-lGR_2D,-lGR_base,-lSUMAAlog_lite,-lOptMS"

##
## SILO (must be after HDF5 symbols to reference them correctly)
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.6.1/linux-x86_64_gcc-4.2.3)
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})
