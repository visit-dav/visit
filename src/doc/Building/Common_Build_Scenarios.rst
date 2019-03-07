.. _Common Build Scenarios

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
``--required --optional`` could be used.

.. code:: bash

   ./build_visit3_0_0b --required --optional --no-visit \
   --thirdparty-path /usr/gapps/visit/thirdparty_shared/3.0.0b --makeflags -j4

This built the third party libraries and generated a ``kickit.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/gapps/visit/thirdparty_shared/3.0.0b)
   SET(VISITARCH linux-x86_64_gcc-4.8)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.0b.tar.gz
   cp kickit.cmake visit3.0.0b/src/config-site
   cd visit3.0.0b
   mkdir build
   cd build
   /usr/gapps/visit/thirdparty_shared/3.0.0b/cmake/3.9.3/linux-x86_64_gcc-4.8/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON \
   -DVISIT_ENABLE_XDB:BOOL=ON -DVISIT_PARADIS:BOOL=ON
   make -j 4 package

Quartz, a Linux X86_64 TOSS3 cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. In this case all the required and optional third party
libraries build without problem, so ``--required --optional`` could be
used. Also, the system OpenGL implementation was outdated and ``--mesagl``
had to be included to provide an OpenGL implementation suitable for
VisIt_. Lastly, the Uintah library was built to enable building the
Uintah reader.

.. code:: bash

   env PAR_COMPILER=/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/bin/mpicc \
       PAR_COMPILER_CXX=/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/bin/mpicxx \
       PAR_INCLUDE=-I/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/include \
       PAR_LIBS=-lmpl \
   ./build_visit3_0_0b --required --optional --mesagl --uintah --parallel \
   --no-visit --thirdparty-path /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.0b/toss3 \
   --makeflags -j16

This built the third party libraries and generated a ``quartz386.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/gapps/visit/thirdparty_shared/3.0.0b)
   SET(VISITARCH linux-x86_64_gcc-4.8)
   VISIT_OPTION_DEFAULT(VISIT_SLIVR TRUE TYPE BOOL)

The ``Parallel build Setup.`` section was changed to

.. code:: bash

   ##
   ## Parallel Build Setup.
   ##
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)
   VISIT_OPTION_DEFAULT(VISIT_MPI_CXX_FLAGS -I/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_C_FLAGS   -I/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/include TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LD_FLAGS  "-L/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/lib -Wl,-rpath=/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/lib" TYPE STRING)
   VISIT_OPTION_DEFAULT(VISIT_MPI_LIBS     mpich mpl)
   VISIT_OPTION_DEFAULT(VISIT_PARALLEL_RPATH  "/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/lib")

VisIt_ was then manually built with the following steps.

.. code:: bash

   tar zxf visit3.0.0b.tar.gz
   cp kickit.cmake visit3.0.0b/src/config-site
   cd visit3.0.0b
   mkdir build
   cd build
   /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.0b/toss3/cmake/3.9.3/linux-x86_64_gcc-4.9/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON -DVISIT_PARADIS:BOOL=ON
   make -j 16 package

Lassen, a Linux Power9 BlueOS cluster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``build_visit`` was run to generate the third party libraries. In this
case the system MPI was used, so information about the system MPI had to
be provided with environment variables and the ``--parallel`` flag had
to be specified. A few of the optional third party libraries do not build
on the system so all the desired optional third party libraries had to be
explicitly listed. Also, the system OpenGL implementation was outdated and
``--mesagl`` had to be included to provide an OpenGL implementation
suitable for VisIt_. Lastly, the Uintah library was built to enable
building the Uintah reader.

.. code:: bash

   env PAR_COMPILER=/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-gcc-4.9.3/bin/mpicc \
    PAR_COMPILER_CXX=/usr/tce/packages/spectrum-mpi/spectrum-mpi-rolling-release-gcc-4.9.3/bin/mpicxx \
    PAR_INCLUDE=-I/usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-rolling-release/include \
    ./build_visit3_0_0b \
    --no-thirdparty --no-visit \
    --cmake --python --vtk --qt --qwt \
    --adios --adios2 --advio --boost --cfitsio --cgns --conduit \
    --gdal --glu --h5part --hdf5 --icet --llvm --mfem \
    --mili --moab --mxml --netcdf --openssl --p7zip --pidx \
    --silo --szip --vtkm --vtkh --xdmf --zlib \
    --mesagl --uintah --parallel \
    --thirdparty-path /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.0b/blueos \
    --makeflags -j16

This built the third party libraries and generated a ``lassen708.cmake``
config site file. The ``Setup VISITHOME & VISITARCH variables.`` section
was changed to

.. code:: bash

   ##
   ## Setup VISITHOME & VISITARCH variables.
   ##
   SET(VISITHOME /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.0b/blueos)
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

   tar zxf visit3.0.0b.tar.gz
   cp kickit.cmake visit3.0.0b/src/config-site
   cd visit3.0.0b
   mkdir build
   cd build
   /usr/workspace/wsa/visit/visit/thirdparty_shared/3.0.0b/blueos/cmake/3.9.3/linux-ppc64le_gcc-4.9/bin/cmake \
   ../src -DCMAKE_BUILD_TYPE:STRING=Release \
   -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON
   make -j 16 package
