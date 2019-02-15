.. _Advanced Usage:

Advanced Usage
--------------

``build_visit`` comes with many features for building a parallel version,
overcoming issues with OpenGL, a rendering library used by VisIt_ to
render images, and controlling precisely what libraries VisIt_ is built
with.

Building a parallel version
~~~~~~~~~~~~~~~~~~~~~~~~~~~

One of powerful capabilities of VisIt_ is running in parallel on large
parallel clusters. VisIt_ runs in parallel using a library called MPI, which
stands for Message Passing Interface. There are a couple of ways in which you
can build a parallel version of VisIt_ using MPI. If your system doesn't
already have MPI installed on it, which is typically the case with a desktop
system or small cluster, then you can use MPICH, which is an open source
implementation of MPI. The following example builds a parallel version using
MPICH.

.. code:: bash

  ./build_visit3_0_0b --mpich --makeflags -j4

If your system already has MPI installed on it, which is typically the case
with a large system at a computer center, you can set several environment
variables that specify the location of the MPI libraries and header files.
The following example uses a system installed MPI library.

.. code:: bash

   env PAR_COMPILER=/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/bin/mpicc \
       PAR_COMPILER_CXX=/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/bin/mpicxx \
       PAR_INCLUDE=-I/usr/tce/packages/mvapich2/mvapich2-2.2-gcc-4.9.3/include \
       PAR_LIBS=-lmpl \
  ./build_visit3_0_0b --parallel --makeflags -j4

When running in parallel, the user will typically use scalable rendering for
rendering images in parallel. VisIt does this through the use of the Mesa 3D
graphics library. Because of this you will want to include Mesa 3D when
building a parallel version. In the following example we have included
building with the Mesa 3D library.

.. code:: bash

  ./build_visit3_0_0b --mpich --osmesa --makeflags -j4

Building with Mesa as the OpenGL implementation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Mesa 3D is also an implementation of OpenGL and it can be used in place of
the system OpenGL when building VisIt_. There are a couple of reasons you
would want to use Mesa 3D instead of the system OpenGL. The first is when you
don't have a system OpenGL, which typically occurs when building in a container
or on a virtual machine. The second is when your system implementation of
OpenGL is too old to support VTK. In the following example we use Mesa 3D
instead of the system OpenGL.

.. code:: bash

  ./build_visit3_0_0b --mesagl --makeflags -j4

The difference between ``--mesagl`` and ``--osmesa``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you specify ``--mesagl`` VTK will be built against Mesa 3D. When you 
specify ``--osmesa`` VTK is built against the system OpenGL and the Mesa 3D
library is substituted at run time for OpenGL when running the parallel
engine to enable scalable rendering. If you specify ``--mesagl`` then
``--osmesa`` is unnecessary and ignored if specified.

Building on a system without internet access
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you want to build visit on a system without internet access, you can
use ``build_visit`` to download the third party libraries and source code
to a system that has internet access and then move those files to your
machine without access. The following example downloads all the optional
libraries and mpich and osmesa.

.. code:: bash

  ./build_visit3_0_0b --optional --mpich --osmesa --download-only

Unfortunately, due to the way the code that builds Python is implemented,
some Python libraries will not be downloaded. Here is the list of commands
to download those additional libraries.

.. code:: bash

  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/Imaging-1.1.7.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/setuptools-28.0.0.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/Cython-0.25.2.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/numpy-1.14.1.zip
  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/pyparsing-1.5.2.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/requests-2.5.1.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.0b/third_party/seedme-python-client-v1.2.4.zip

It's possible that the list could change and the above list becomes outdated.
In this case you can run ``build_visit`` to just build Python and that will
end up downloading all the files you need. The following command builds only
Python.

.. code:: bash

  ./build_visit3_0_0b --no-thirdparty --no-visit --python --makeflags -j4

Different versions of ``build_visit``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you use a version of ``build_visit`` that has a version number in it,
for example ``build_visit3_0_0b`` then it build that tagged version of
VisIt_. If the version of ``build_visit`` was from the develop branch of
VisIt_, then it will grab the latest version of VisIt_ from the devlop
branch. If the version of ``build_visit`` came from a branch corresponding
to release candidate corresponding to a major release, for example the
v3.0 branch, then it will grab the latest version of VisIt_ from that branch.
