.. _Common Build Scenarios:

Common Build Scenarios
----------------------

Building VisIt_ is an involved process and even with ``build_visit``,
just determining the correct selection of options can sometimes be
daunting. To help, here are the steps used to build VisIt_ on a collection
of different platforms that may serve as a starting point for your system.

In each of the scenarios below, the result is a distribution file that can
be used with ``visit-install`` to install VisIt_. Furthermore, in all these
scenarios, ``build_visit`` was used to build the third party libraries and
the initial config site file. VisIt_ was then manually built as outlined
by doing an out of source build. The advantage to building VisIt_ manually
is that you have more control over the build and its easier to troubleshoot
failures. The advantage to an out of source build is that you can easily
restart the build simply by deleting the build directory.
   
Kickit, a RedHat Enterprise Linux 7 system
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``build_visit`` was run to generate the third party libraries. In this
case all the required and optional libraries build without problem, so
``--required --optional`` could be used. Also, in this case there wasn't
a system MPI installed so ``--mpich`` was specified to use MPICH. The
``--osmesa`` flag was also included so that VisIt could do off screen
rendering.

.. code:: bash

   ./build_visit3_0_1 --required --optional --mpich --osmesa --no-visit \
   --thirdparty-path /usr/gapps/visit/thirdparty_shared/3.0.1 --makeflags -j4

This built the third party libraries and generated a ``kickit.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/gapps/visit/thirdparty_shared/3.0.1)
   SET(VISITARCH linux-x86_64_gcc-4.8)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.1.tar.gz
   cp kickit.cmake visit3.0.1/src/config-site
   cd visit3.0.1
   mkdir build
   cd build
   /usr/gapps/visit/thirdparty_shared/3.0.1/cmake/3.9.3/linux-x86_64_gcc-4.8/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON \
   -DVISIT_ENABLE_XDB:BOOL=ON -DVISIT_PARADIS:BOOL=ON
   make -j 4 package

Quartz, a Linux X86_64 TOSS3 cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. In this case, all the required and optional third party
libraries build without problem, so ``--required --optional`` could be
used. Also, the system OpenGL implementation was outdated and ``--mesagl``
had to be included to provide an OpenGL implementation suitable for
VisIt_. Lastly, the Uintah library was built to enable building the
Uintah reader.

.. code:: bash

   env PAR_COMPILER=/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/bin/mpicc \
       PAR_COMPILER_CXX=/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/bin/mpicxx \
       PAR_INCLUDE=-I/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/include \
       PAR_LIBS=-lmpl \
   ./build_visit3_0_1 --required --optional --mesagl --uintah --parallel \
   --no-visit --thirdparty-path /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.1/toss3 \
   --makeflags -j16

This built the third party libraries and generated a ``quartz386.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/gapps/visit/thirdparty_shared/3.0.1)
   SET(VISITARCH linux-x86_64_gcc-4.8)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

The ``Parallel build Setup.`` section was changed to

.. code:: bash

   ##
   ## Parallel Build Setup.
   ##
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
   VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/lib -Wl,-rpath=/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/lib" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich mpl)
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/usr/tce/packages/mvapich2/mvapich2-2.3-gcc-4.9.3/lib")

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.1.tar.gz
   cp kickit.cmake visit3.0.1/src/config-site
   cd visit3.0.1
   mkdir build
   cd build
   /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.1/toss3/cmake/3.9.3/linux-x86_64_gcc-4.9/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON -DVISIT_PARADIS:BOOL=ON
   make -j 16 package

Lassen, a Linux Power9 BlueOS cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. In this case, a few of the optional third party libraries
do not build on the system so all the desired optional third party libraries
had to be explicitly listed. Also, the system OpenGL implementation was
outdated and ``--mesagl`` had to be included to provide an OpenGL
implementation suitable for VisIt_. Lastly, the Uintah library was built
to enable building the Uintah reader.

.. code:: bash

   env PAR_COMPILER=/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-gcc-4.9.3/bin/mpicc \
    PAR_COMPILER_CXX=/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-gcc-4.9.3/bin/mpicxx \
    PAR_INCLUDE=-I/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/include \
    ./build_visit3_0_1 \
    --no-thirdparty --no-visit \
    --cmake --python --vtk --qt --qwt \
    --adios --adios2 --advio --boost --cfitsio --cgns --conduit \
    --gdal --glu --h5part --hdf5 --icet --llvm --mfem \
    --mili --moab --mxml --netcdf --openssl --p7zip \
    --silo --szip --vtkm --vtkh --xdmf --zlib \
    --mesagl --uintah --parallel \
    --thirdparty-path /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.1/blueos \
    --makeflags -j16

This built the third party libraries and generated a ``lassen708.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.1/blueos)
   SET(VISITARCH linux-ppc64le_gcc-4.9)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)


The ``Parallel build Setup.`` section was changed to

.. code:: bash

   ##
   ## Parallel Build Setup.
   ##
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
   VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/lib -Wl,-rpath=/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/lib" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpi_ibm)
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/lib")

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.1.tar.gz
   cp lassen708.cmake visit3.0.1/src/config-site
   cd visit3.0.1
   mkdir build
   cd build
   /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.1/blueos/cmake/3.9.3/linux-ppc64le_gcc-4.9/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON
   make -j 16 package

Cori, a Cray KNL cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The system is set up to support the Intel compiler by default so we need
to swap out the Intel environment for the GNU environment.

.. code:: bash

   module swap PrgEnv-intel/6.0.4 PrgEnv-gnu/6.0.4

The Cray compiler wrappers are set up to do static linking, which causes
a problem with building parallel hdf5. The linking can be changed to
link dynamically by setting a couple of environment variables.

.. code:: bash

   export XTPE_LINK_TYPE=dynamic
   export CRAYPE_LINK_TYPE=dynamic

The linker has a bug that prevents VTK from building, which is fixed with
the linker in binutils 2.32. Binutils was then manually built with the
following steps.

.. code:: bash

   wget https://mirrors.ocf.berkeley.edu/gnu/binutils/binutils-2.32.tar.gz
   mkdir /project/projectdirs/visit/thirdparty_shared/3.0.1/binutils
   tar zxf binutils-2.32.tar.gz
   cd binutils-2.32
   ./configure --prefix=/project/projectdirs/visit/thirdparty_shared/3.0.1/binutils
   make
   make install

The following lines in ``build_visit``

.. code:: bash

   vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
   vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""

were changed to

.. code:: bash

   vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS} -B/project/projectdirs/visit/thirdparty_shared/3.0.1/binutils/bin\""
   vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS} -B/project/projectdirs/visit/thirdparty_shared/3.0.1/binutils/bin\""

to build VTK with the linker from binutils 2.32.

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. In this case, all the required and optional third party
libraries built without problem, so ``--required --optional`` could be
used. Also, the system OpenGL implementation was outdated and ``--mesagl``
had to be included to provide an OpenGL implementation suitable for
VisIt_. Lastly, the Uintah library was built to enable building the
Uintah reader.

.. code:: bash

   env PAR_COMPILER=/opt/cray/pe/craype/2.5.15/bin/cc \
       PAR_COMPILER_CXX=/opt/cray/pe/craype/2.5.15/bin/CC \
       PAR_INCLUDE=-I/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/include \
       PAR_LIBS="-L/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/lib -Wl,-rpath=/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/lib -lmpich" \
    ./build_visit3_0_1 --required --optional --mesagl --uintah --parallel \
    --no-visit --thirdparty-path /project/projectdirs/visit/thirdparty_shared/3.0.1 \
    --makeflags -j8

This built the third party libraries and generated a ``cori08.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /project/projectdirs/visit/thirdparty_shared/3.0.1)
   SET(VISITARCH linux-x86_64_gcc-7.3)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

The ``VISIT_C_FLAGS`` and ``VISIT_CXX_FLAGS`` were changed to

.. code:: bash

   VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -fvisibility=hidden -B/project/projectdirs/visit/thirdparty_shared/3.0.1/binutils/bin" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC -fvisibility=hidden -B/project/projectdirs/visit/thirdparty_shared/3.0.1/binutils/bin" TYPE STRING)

The ``Parallel build Setup.`` section was changed to

.. code:: bash

   ##
   ## Parallel Build Setup.
   ##
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
   VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/lib -Wl,-rpath=/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/lib" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich)
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/opt/cray/pe/mpt/7.7.3/gni/mpich-gnu/7.1/lib")

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.1.tar.gz
   cp cori08.cmake visit3.0.1/src/config-site
   cd visit3.0.1
   mkdir build
   cd build
   /project/projectdirs/visit/thirdparty_shared/3.0.1/cmake/3.9.3/linux-x86_64_gcc-7.3/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON -DVISIT_PARADIS:BOOL=ON
   make -j 8 package

Summit, a Linux Power9 BlueOS cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The system is set up to support the IBM XL compiler by default so we need
to swap out the XL compiler for the GNU compiler.

.. code:: bash

   module swap xl/16.1.1-3 gcc/6.4.0

There was an error building CMake, so we used the system CMake after
module loading CMake 3.9.2.

.. code:: bash

   module load cmake/3.9.2

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. In this case, a few of the optional third party libraries
do not build on the system so all the desired optional third party libraries
had to be explicitly listed. Also, the system OpenGL implementation was
outdated and ``--mesagl`` had to be included to provide an OpenGL
implementation suitable for VisIt_. Lastly, the Uintah library was built
to enable building the Uintah reader.

.. code:: bash

   env PAR_COMPILER=/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/bin/mpicc \
       PAR_COMPILER_CXX=/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/bin/mpicxx \
       PAR_INCLUDE=-I/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/include \
       ./build_visit3_0_1 \
       --no-thirdparty --no-visit \
       --system-cmake --python --vtk --qt --qwt \
       --adios --adios2 --advio --boost --cfitsio --cgns --conduit \
       --gdal --glu --h5part --hdf5 --icet --llvm --mfem \
       --mili --moab --mxml --netcdf --openssl --p7zip \
       --silo --szip --xdmf --zlib \
       --mesagl --uintah --parallel \
       --thirdparty-path /autofs/nccs-svm1_home1/brugger1/visit/thirdparty_shared/3.0.1 \
       --makeflags -j8

This built the third party libraries and generated a ``login1.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /autofs/nccs-svm1_home1/brugger1/visit/thirdparty_shared/3.0.1)
   SET(VISITARCH linux-ppc64le_gcc-6.4)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

The ``Parallel build Setup.`` section was changed to

.. code:: bash

   ##
   ## Parallel Build Setup.
   ##
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
   VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/lib -Wl,-rpath=/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/lib" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpi_ibm)
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/autofs/nccs-svm1_sw/summit/.swci/1-compute/opt/spack/20180914/linux-rhel7-ppc64le/gcc-6.4.0/spectrum-mpi-10.3.0.1-20190611-cyaenjgora6now2nusxzkfli4mzjnudx/lib")

The compiler didn't like one of the boost header files, so it was manually
patched.

.. code:: bash

   vi /autofs/nccs-svm1_home1/brugger1/visit/thirdparty_shared/3.0.1/boost/1_67_0/linux-ppc64le_gcc-6.4/include/boost/numeric/interval/detail/ppc_rounding_control.hpp

   line 99:
    namespace detail {

    typedef union {
   -   ::boost::long_long_type imode;
   +   ::boost::ulong_long_type imode;
      double dmode;
    } rounding_mode_struct;

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.1.tar.gz
   cp login1.cmake visit3.0.1/src/config-site
   cd visit3.0.1
   mkdir build
   cd build
   /autofs/nccs-svm1_sw/summit/.swci/0-core/opt/spack/20171006/linux-rhel7-ppc64le/gcc-4.8.5/cmake-3.9.2-lnpnk356fyio3b6rq5bdhr2djjirtsxk/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON
   make -j 8 package

Trinity, a Cray KNL cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The system is set up to support the Intel compiler by default so we need
to swap out the Intel environment for the GNU environment.

.. code:: bash

   module swap PrgEnv-intel/6.0.8 PrgEnv-gnu/6.0.8

The Cray compiler wrappers are set up to do static linking, which causes
a problem with building parallel hdf5. The linking can be changed to
link dynamically by setting a couple of environment variables.

.. code:: bash

   export XTPE_LINK_TYPE=dynamic
   export CRAYPE_LINK_TYPE=dynamic

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. In this case, all the required and optional third party
libraries built except for Sphinx and Mili, so ``--no-sphinx --no-mili``
in addition to ``--required --optional`` could be used. Also, the system
OpenGL implementation was outdated and ``--mesagl`` had to be included to
provide an OpenGL implementation suitable for VisIt_.

.. code:: bash

   env PAR_COMPILER=/opt/cray/pe/craype/2.7.0/bin/cc \
       PAR_COMPILER_CXX=/opt/cray/pe/craype/2.7.0/bin/CC \
       PAR_INCLUDE=-I/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/include \
       PAR_LIBS="-L/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/lib -Wl,-rpath=/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/lib -lmpich" \
    ./build_visit3_1_3 --required --optional --no-sphinx --no-mili \
    --mesagl --parallel --no-visit \
    --thirdparty-path /usr/projects/views/visit/thirdparty_shared/3.1.3 \
    --makeflags -j6

This built the third party libraries and generated a ``tr-fe2.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/projects/views/visit/thirdparty_shared/3.1.3)
   SET(VISITARCH linux-x86_64_gcc-9.3)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

The ``Parallel build Setup.`` section was changed to

.. code:: bash

   ##
   ## Parallel Build Setup.
   ##
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
   VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/lib -Wl,-rpath=/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/lib" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich)
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/opt/cray/pe/mpt/7.7.15/gni/mpich-gnu/8.2/lib")

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.1.3.tar.gz
   cp tr-fe2.cmake visit3.1.3/src/config-site
   cd visit3.1.3
   mkdir build
   cd build
   /usr/projects/views/visit/thirdparty_shared/3.1.3/cmake/3.9.3/linux-x86_64_gcc-9.3/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON -DVISIT_PARADIS:BOOL=ON
   make -j 8 package

