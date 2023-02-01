.. _Advanced Usage:

Advanced Usage
--------------

``build_visit`` comes with many options for features such as building a
parallel version, overcoming issues with OpenGL, a rendering library used
by VisIt_ to render images, and controlling precisely what libraries
VisIt_ is built with.

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

  ./build_visit3_0_1 --mpich

If your system already has MPI installed on it, which is typically the case
with a large system at a computer center, you can set several environment
variables that specify the location of the MPI libraries and header files.
The following example uses a system installed MPI library.

.. code:: bash

   env PAR_COMPILER=/usr/packages/mvapich2/bin/mpicc \
       PAR_COMPILER_CXX=/usr/packages/mvapich2/bin/mpicxx \
       PAR_INCLUDE=-I/usr/packages/mvapich2/include \
       PAR_LIBS=-lmpl \
  ./build_visit3_0_1 --parallel

When running in parallel, the user will typically use scalable rendering for
rendering images in parallel. VisIt does this through the use of the Mesa 3D
graphics library. Because of this you will want to include Mesa 3D when
building a parallel version. In the following example we have included
building with the Mesa 3D library.

.. code:: bash

  ./build_visit3_0_1 --mpich --osmesa

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

  ./build_visit3_0_1 --mesagl

The difference between ``--mesagl`` and ``--osmesa``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you specify ``--mesagl`` VTK will be built against Mesa 3D. When you 
specify ``--osmesa`` VTK is built against the system OpenGL and the Mesa 3D
library is substituted at run time for OpenGL when running the parallel
engine to enable scalable rendering. If you specify ``--mesagl`` then
``--osmesa`` is unnecessary and ignored if specified.

Building VisIt_ with Pre-Installed (e.g. System) Libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On many systems, some libraries VisIt_ needs (e.g. Qt, VTK, Python OpenGL, HDF5, etc.) come pre-installed.
Can a user just use those pre-installed libraries to build VisIt_?

**Please don't!**
In all likelihood this will not work at all or, worse, it will only partially work and fail in subtle ways that are nearly impossible to diagnose.
In the unlikely chance it appears to work upon reporting any issues our first question will be, how was VisIt_ configured/built?
If VisIt_ is built in a way that is not consistent with how developers routinely build, run and test it, we will not be able to reproduce the issue, debug it, identify work-arounds or otherwise provide sufficient support.

Apart from the general issues of reproducibility and support, there are many reasons building VisIt_ with pre-installed libraries will likely not work.
Below, we breifly summarize various compatability issues with trying to use pre-installed libraries.

Version Compatability : Pre-installed libraries are not the version VisIt_ requires
    Often, users notice a *newer* version of a library VisIt_ needs is pre-installed on their system and expect VisIt_ will run *better* with this newer version.
    However, having a newer version of VTK, for example, pre-installed does not mean VisIt_ will build or run properly with that version.
    Major versions of VTK, for example, (e.g. 8.0 and 9.0) are not compatable.
    Incompatabilities sometimes exist even between minor versions of some libraries.
    Incorrect library versions may cause VisIt_ to either fail to build or fail to run properly.

Patch Compatability : Pre-installed libaries are missing patches VisIt_ requires
    In some cases, the libraries VisIt_ needs are patched to work around various issues building or running VisIt_.
    Such patches are almost certainly not in any pre-installed version of the library.
    Missing patches may cause VisIt_ to either fail to build or fail to run properly.

Configuration Compatability : Pre-installed libraries are not configured in a way VisIt_ requires
    Libraries often have many build options which enable or disable certain features.
    The Qt library, for example, has hundreds of build options.
    Some build options VisIt_ may not care about.
    Other build options, however, VisIt_ may require to be enabled and still other options to be disabled.
    Incorrect library configuration may cause VisIt_ to either fail to build or fail to run properly.

Dependency Compatability : Pre-installed libraries are not built with dependencies VisIt_ requires
    Libraries often have dependencies on still other libraries.
    For example, Qt and VTK can both depend on OpenGL.
    In some cases, however, VisIt_ may require a specific implementation of OpenGL called MesaGL.
    Incorrect dependencies may cause VisIt_ to either fail to build or fail to run properly.
    Such dependencies complicate things significantly because it means all of the aforementioned compatability issues apply, recursively, to any libraries a pre-installed library depends on.

Compiler (Run-Time) Compatability : Pre-installed libraries are not built with a compiler (run-time) VisIt_ requires
    For some situations, building VisIt_ and its dependencies requires a specific compiler.
    The compiler (run-time) used for pre-installed libraries may not be compatable with the compiler (run-time) VisIt_ requires.

There are likely other subtle compatability issues that can arise which we have neglected to mention here.
A fully featured build of VisIt_ can involve 35+ libraries, many of which may come pre-installed (Qt, VTK, Python, HDF5, netCDF, OpenGL, MPI to name a few) on any particular platform.
Bottom line, the number of ways pre-installed libraries can be built such that they will cause VisIt_ to either fail to build or fail to run properly are almost boundless.
For this reason, we discourage users from attempting to build VisIt_ using pre-installed libraries and warn users that in all likelihood we will not have sufficient resources to help address any resulting issues that may arise.

Building on a system without internet access
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you want to build visit on a system without internet access, you can
use ``build_visit`` to download the third party libraries and source code
to a system that has internet access and then move those files to your
machine without access. The following example downloads the optional
libraries, mpich and osmesa.

.. code:: bash

  ./build_visit3_0_1 --optional --mpich --osmesa --download-only

Unfortunately, due to the way the code that builds Python is implemented,
some Python libraries will not be downloaded. Here is the list of commands
to download those additional libraries.

.. code:: bash

  wget http://portal.nersc.gov/project/visit/releases/3.0.1/third_party/Imaging-1.1.7.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.1/third_party/setuptools-28.0.0.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.1/third_party/Cython-0.25.2.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.1/third_party/numpy-1.14.1.zip
  wget http://portal.nersc.gov/project/visit/releases/3.0.1/third_party/pyparsing-1.5.2.tar.gz
  wget http://portal.nersc.gov/project/visit/releases/3.0.1/third_party/requests-2.5.1.tar.gz

It's possible that the list could change and the above list becomes outdated.
In this case you can run ``build_visit`` to just build Python and that will
end up downloading all the files you need. The following command builds only
Python.

.. code:: bash

  ./build_visit3_0_1 --no-thirdparty --no-visit --python

Different versions of ``build_visit``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you use a version of ``build_visit`` that has a version number in it,
for example ``build_visit3_0_1`` then it builds that tagged version of
VisIt_. If the version of ``build_visit`` was from the develop branch of
VisIt_, then it will grab the latest version of VisIt_ from the devlop
branch. If the version of ``build_visit`` came from a release candidate
branch, for example the v3.0 branch, then it will grab the latest version
of VisIt_ from that branch.

Troubleshooting ``build_visit`` failures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``build_visit`` runs, it generates a log file with ``_log`` added to
the name of the script. For example, if you are running ``build_visit3_0_1``
then the log file will be named ``build_visit3_0_1_log``. The error that
caused the failure should be near the end of the log file. When ``build_visit``
finishes running, it will leave the directories that it used to build
the packages intact. You can go into the directory of the package that
failed and correct the issue and finish building and installing the package.
You can then execute the ``build_visit`` command again to have it continue
the build.

Why can't I use the Qt, Python, VTK, Mesa/GL, etc. that came on my system?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As much as we might like to believe it, large, complex libraries like Qt, Python and VTK are rarely 100% compatible between newer or older versions.
Furthermore, for large libraries like these, there are often many, many different installation options for a given platform.
It is highly unlikely that a given installation of VTK for example, is not only of a version compatibile with a given release of VisIt_ but also configured and installed on your system in exactly the way VisIt_ needs it.
In addition, VisIt_ gets developed and thoroughly tested on specific versions and configurations of various libraries meaning that when users encounter issues in other configurations, we are not always able to reproduce them.
In some cases, VisIt_ developers have had to work-around a bug in a library or, worse, had to patch the actual library itself to address an issue that might be specific to just one platform.
Together, these issues result in a situation where VisIt_ often must be compiled with precisely the libraries it is released on and rarely, if ever, can take advantage of an installation that came as part of the system VisIt_ is being built on.
Lastly, it becomes almost impossible to duplicate and diagnose issues reported by users when users are running VisIt_ in configurations substantially different from that which is being developed and routinely tested.
