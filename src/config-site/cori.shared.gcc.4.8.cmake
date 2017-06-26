#/global/homes/w/whitlocb/Development/thirdparty_shared_cori/2.12.0/cmake/3.0.2/linux-x86_64_gcc-4.8/bin/cmake
##
## ./build_visit generated host.cmake
## created: Fri Jun 23 16:37:26 PDT 2017
## system: Linux cori04 3.12.60-52.63.1.12215.0.PTF.1017941-default #1 SMP Thu Jan 5 05:33:02 UTC 2017 (afd16ea) x86_64 x86_64 x86_64 GNU/Linux
## by: whitlocb

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /global/homes/w/whitlocb/Development/thirdparty_shared_cori/2.12.0)
SET(VISITARCH linux-x86_64_gcc-4.8)

## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)

##
## VisIt install location.
##
VISIT_OPTION_DEFAULT(CMAKE_INSTALL_PREFIX /global/homes/w/whitlocb/Development/install_cori_shared TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_INSTALL_PROFILES_TO_HOSTS "nersc" TYPE STRING)

##
## Parallel Build Setup.
##
VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
## (configured w/ user provided CXX (PAR_INCLUDE) & LDFLAGS (PAR_LIBS) flags)
VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   "-I/opt/cray/pe/mpt/default/gni/mpich-gnu/49/include -I/opt/cray/pe/libsci/16.09.1/GNU/5.1/x86_64/include -I/opt/cray/pe/mpt/7.4.4/gni/mpich-gnu/5.1/include -I/opt/cray/pe/mpt/7.4.4/gni/sma/include -I/opt/cray/rca/2.1.6_g2c60fbf-2.265/include -I/opt/cray/alps/6.3.4-2.21/include -I/opt/cray/xpmem/2.1.1_gf9c9084-2.38/include -I/opt/cray/gni-headers/5.0.11-2.2/include -I/opt/cray/dmapp/7.1.1-39.37/include -I/opt/cray/pe/pmi/5.0.10-1.0000.11050.0.0.ari/include -I/opt/cray/ugni/6.0.15-2.2/include -I/opt/cray/udreg/2.3.2-7.54/include -I/opt/cray/wlm_detect/1.2.1-3.10/include -I/opt/cray/krca/2.1.3_g923f58b-2.244/include -I/opt/cray-hss-devel/8.0.0/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS "-I/opt/cray/pe/mpt/default/gni/mpich-gnu/49/include -I/opt/cray/pe/libsci/16.09.1/GNU/5.1/x86_64/include -I/opt/cray/pe/mpt/7.4.4/gni/mpich-gnu/5.1/include -I/opt/cray/pe/mpt/7.4.4/gni/sma/include -I/opt/cray/rca/2.1.6_g2c60fbf-2.265/include -I/opt/cray/alps/6.3.4-2.21/include -I/opt/cray/xpmem/2.1.1_gf9c9084-2.38/include -I/opt/cray/gni-headers/5.0.11-2.2/include -I/opt/cray/dmapp/7.1.1-39.37/include -I/opt/cray/pe/pmi/5.0.10-1.0000.11050.0.0.ari/include -I/opt/cray/ugni/6.0.15-2.2/include -I/opt/cray/udreg/2.3.2-7.54/include -I/opt/cray/wlm_detect/1.2.1-3.10/include -I/opt/cray/krca/2.1.3_g923f58b-2.244/include -I/opt/cray-hss-devel/8.0.0/include" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  " -L/opt/cray/pe/libsci/16.09.1/GNU/5.1/x86_64/lib -L/opt/cray/dmapp/default/lib64 -L/opt/cray/pe/mpt/7.4.4/gni/mpich-gnu/5.1/lib -L/opt/cray/dmapp/default/lib64 -L/opt/cray/pe/mpt/7.4.4/gni/mpich-gnu/5.1/lib -L/opt/cray/pe/mpt/7.4.4/gni/sma/lib64 -L/opt/cray/rca/2.1.6_g2c60fbf-2.265/lib64 -L/opt/cray/alps/6.3.4-2.21/lib64 -L/opt/cray/xpmem/2.1.1_gf9c9084-2.38/lib64 -L/opt/cray/dmapp/7.1.1-39.37/lib64 -L/opt/cray/pe/pmi/5.0.10-1.0000.11050.0.0.ari/lib64 -L/opt/cray/ugni/6.0.15-2.2/lib64 -L/opt/cray/udreg/2.3.2-7.54/lib64 -L/usr/common/software/darshan/3.1.4/lib -L/opt/cray/pe/atp/2.1.0/libApp -L/lib64 -L/opt/cray/wlm_detect/1.2.1-3.10/lib64 -Wl,--no-as-needed,-lAtpSigHandler,-lAtpSigHCommData -Wl,--undefined=_ATP_Data_Globals -Wl,--undefined=__atpHandlerInstall -Wl,@/usr/common/software/darshan/3.1.4/share/ld-opts/darshan-base-ld-opts -Wl,-rpath,/opt/cray/pe/libsci/16.09.1/GNU/5.1/x86_64/lib -Wl,-rpath,/opt/cray/dmapp/default/lib64 -Wl,-rpath,/opt/cray/pe/mpt/7.4.4/gni/mpich-gnu/5.1/lib -Wl,-rpath,/opt/cray/dmapp/default/lib64 -Wl,-rpath,/opt/cray/pe/mpt/7.4.4/gni/mpich-gnu/5.1/lib -Wl,-rpath,/opt/cray/pe/mpt/7.4.4/gni/sma/lib64 -Wl,-rpath,/opt/cray/rca/2.1.6_g2c60fbf-2.265/lib64 -Wl,-rpath,/opt/cray/alps/6.3.4-2.21/lib64 -Wl,-rpath,/opt/cray/xpmem/2.1.1_gf9c9084-2.38/lib64 -Wl,-rpath,/opt/cray/dmapp/7.1.1-39.37/lib64 -Wl,-rpath,/opt/cray/pe/pmi/5.0.10-1.0000.11050.0.0.ari/lib64 -Wl,-rpath,/opt/cray/ugni/6.0.15-2.2/lib64 -Wl,-rpath,/opt/cray/udreg/2.3.2-7.54/lib64 -Wl,-rpath,/usr/common/software/darshan/3.1.4/lib -Wl,-rpath,/opt/cray/pe/atp/2.1.0/libApp -Wl,-rpath,/lib64 -Wl,-rpath,/opt/cray/wlm_detect/1.2.1-3.10/lib64" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS         pthread fmpich mpichcxx -Wl,-Bstatic -Wl,--start-group -ldarshan -ldarshan-stubs -Wl,--end-group -Wl,-Bdynamic z sci_gnu_51_mpi sci_gnu_51 mpichcxx_gnu_51 rt ugni pthread pmi mpich_gnu_51 rt ugni pthread pmi sma pmi dmapp pthread xpmem udreg pmi pthread alpslli pthread wlm_detect ugni pthread alpsutil pthread rca TYPE STRING)

##
## Server components only
##
VISIT_OPTION_DEFAULT(VISIT_SERVER_COMPONENTS_ONLY ON TYPE BOOL)

##
## XDB
##
VISIT_OPTION_DEFAULT(VISIT_ENABLE_XDB ON TYPE BOOL)

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

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
## Mesa
##
VISIT_OPTION_DEFAULT(VISIT_MESA_DIR ${VISITHOME}/mesa/7.10.2/${VISITARCH})

##
## Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/2.7.11/${VISITARCH})

##
## VTK
##
SETUP_APP_VERSION(VTK 6.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})
##

##
## SZIP
##
VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR ${VISITHOME}/szip/2.1/${VISITARCH})

##
## HDF5
##
VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR ${VISITHOME}/hdf5/1.8.14/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz /usr/lib z TYPE STRING)

##
## Ice-T
##
VISIT_OPTION_DEFAULT(VISIT_ICET_DIR ${VISITHOME}/icet/1.0.0/${VISITARCH})

##
## Silo
##
VISIT_OPTION_DEFAULT(VISIT_SILO_DIR ${VISITHOME}/silo/4.10.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} /usr/lib z TYPE STRING)

##
## Xdmf
##
VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR ${VISITHOME}/Xdmf/2.1.1/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5  VTK_LIBRARY_DIRS vtklibxml2-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}  TYPE STRING)

