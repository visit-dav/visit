#/usr/gapps/visit/cmake/2.6.4/aix_xlc_32/bin/cmake
##
## ./build_visit generated host.cmake
## created: Wed Feb 17 11:40:29 PST 2010
## system: AIX up041 3 5 00C5D6DD4C00
## by: brugger

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /usr/gapps/visit)
SET(VISITARCH aix_xlc_64)

##
## Specify the location of the mesa.
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.5/${VISITARCH})

##
## Specify the location of the vtk.
##
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/5.0.0d/${VISITARCH}/lib/vtk-5.0/)

##
## Specify the Qt4 binary dir. 
## (qmake us used to locate & setup Qt4 dependencies)
##
VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${VISITHOME}/qt/4.4.3/${VISITARCH}/bin)

##
## Specify the location of the python.
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.5/${VISITARCH})

##
## Use the xlc compiler.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER xlc)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER xlC)

##
## Turn on dynamic cast code.
##
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS "-I/usr/vacpp/include")
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS "-qrtti=dynamiccast -I/usr/vacpp/include")

##
## We only get 256MB of memory by default, thus the maxdata flag
## We can only use 2GB/node without large pages (-blpdata)
##
VISIT_OPTION_DEFAULT(VISIT_EXE_LINKER_FLAGS "-Wl,-blpdata -Wl,-bmaxdata:0x70000000")

##
## Enable verbose makefiles (show compiler & linker calls)
##
VISIT_OPTION_DEFAULT(VISIT_VERBOSE_MAKEFILE TRUE)

##
## Make sure we use the right MPI library
##
VISIT_OPTION_DEFAULT(PARALLEL ON)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-I/usr/lpp/ppe.poe/include/")
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS "-I/usr/lpp/ppe.poe/include/")
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS "-L/usr/lpp/ppe.poe/lib/threads -L/usr/lpp/ppe.poe/lib -L/usr/lpp/ppe.poe/lib/ip -Wl,-binitfini:poe_remote_main")
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS mpi_r vtd_r lapi_r pthreads)

##############################################################
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
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.4/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)

##
## Mili
##
VISIT_OPTION_DEFAULT(VISIT_MILI_DIR ${VISITHOME}/mili/1.10.0/${VISITARCH})

##
## NetCDF
##
VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR ${VISITHOME}/netcdf/3.6.3/${VISITARCH})

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.7.2/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP})

