Building with Spack
===================

Overview
--------

Spack is a multi-platform package manager that builds and installs multiple versions and configurations of software.
It works on Linux, macOS, and many supercomputers.
Spack is non-destructive: installing a new version of a package does not break existing installations, so many configurations of the same package can coexist.

Spack offers a simple "spec" syntax that allows users to specify versions and configuration options.
Package files are written in pure Python, and specs allow package authors to write a single script for many different builds of the same package.
With Spack, you can build your software all the ways you want to.

The complete documentation can be found `here <https://spack.readthedocs.io/en/latest/index.html>`__.

The VisIt_ spack package
------------------------

The spack package for VisIt_ currently only builds a subset of the libraries that ``build_visit`` builds.
It builds the core libraries needed by VisIt_ and the following I/O libraries:

* HDF5
* Silo_

The rest of the I/O libraries needed by the VisIt_ readers are currently not built.
The expectation is that the list of I/O libraries built will grow over time.

Building VisIt_ with spack
--------------------------

The first step is to clone spack from GitHub. ::

    # Using https.
    git clone https://github.com/spack/spack

    # Using ssh.
    git clone ssh://git@github.com/spack/spack
 
Now you need to set up your spack environment. ::

    cd spack

    # bash shell
    . share/spack/setup-env.sh

    # c shell
    source share/spack/setup-env.csh

Now you need to have spack find the compilers available on your system. ::

    spack compilers

This will also create a ``.spack`` directory in your home directory.
The ``.spack`` directory also contains a ``compilers.yaml`` file, which contains properties about your compilers.
You can modify the ``compilers.yaml`` file if you need to change some properties of one or more of your compilers, such as additional rpaths.
This is also the directory that you would put a ``package.yaml`` file for specifying the version of mpi to use.

If there is a compiler you want to use and spack didn't find it, you can tell spack about the compiler. ::

    spack compiler find path_to_compiler_install_dir

If you would like to find a list of all the packages that spack will build you can use the spec command. ::

    spack spec visit ^python+shared ^mesa+opengl

To build VisIt_ with the default compiler. ::

    spack install visit ^python+shared ^mesa+opengl

To build VisIt_ with a specific compiler. ::

    spack install visit%gcc@11.2.0 ^python+shared ^mesa+opengl

To build VisIt_ and specify a specific version of a dependent library. ::

    spack install visit%gcc@11.2.0 ^python+shared ^mesa+opengl ^llvm@11.0.1

To uninstall a package (the ``--dependents`` uninstalls all the packages that depend on the uninstalled packages). ::

    spack uninstall --dependents visit vtk

To uninstall all the packages. ::

    spack uninstall --all

The package files for a package are located in ``var/spack/repos/builtin/package``.
Here are some examples. ::

    ./var/spack/repos/builtin/packages/visit
    ./var/spack/repos/builtin/packages/vtk

The installed packages are stored in ``opt/spack/<architecture>/<compiler>``.
Here are some examples. :: 

    opt/spack/cray-sles15-zen2/gcc-11.2.0/hdf5-1.8.22-c3djozhlmrvy7wpu46f36qeakemiactw
    opt/spack/cray-sles15-zen2/gcc-11.2.0/cmake-3.14.7-nnahgnkkl2d2ty2us46we75pnjepci35

Building VisIt with the development version of spack
----------------------------------------------------

The following 3 patches should be applied.
These are patches to `package.py` files that cannot be patched as part of the VisIt_ spack package.

::

    diff --git a/var/spack/repos/builtin/packages/libxcb/package.py b/var/spack/repos/builtin/packages/libxcb/package.py
    index 1db0f5de5a..f19f1856de 100644
    --- a/var/spack/repos/builtin/packages/libxcb/package.py
    +++ b/var/spack/repos/builtin/packages/libxcb/package.py
    @@ -50,3 +50,4 @@ def configure_args(self):

         def patch(self):
             filter_file("typedef struct xcb_auth_info_t {", "typedef struct {", "src/xcb.h")
    +        filter_file("python python2 python3", "python3", "configure")

::

diff --git a/var/spack/repos/builtin/packages/visit/package.py b/var/spack/repos/builtin/packages/visit/package.py
index 290280e17d..19bda1b1ed 100644
--- a/var/spack/repos/builtin/packages/visit/package.py
+++ b/var/spack/repos/builtin/packages/visit/package.py
@@ -99,7 +99,7 @@ class Visit(CMakePackage):
     patch("visit32-missing-link-libs.patch", when="@3.2")

     # Exactly one of 'gui' or 'osmesa' has to be enabled
-    conflicts("+gui", when="+osmesa")
+    #conflicts("+gui", when="+osmesa")

     depends_on("cmake@3.14.7:", type="build")

::

diff --git a/var/spack/repos/builtin/packages/vtk/package.py b/var/spack/repos/builtin/packages/vtk/package.py
index c7bec82c74..d87f61ea0b 100644
--- a/var/spack/repos/builtin/packages/vtk/package.py
+++ b/var/spack/repos/builtin/packages/vtk/package.py
@@ -61,7 +61,7 @@ class Vtk(CMakePackage):
     patch("xdmf2-hdf51.13.2.patch", when="@9:9.2 +xdmf")

     # We cannot build with both osmesa and qt in spack
-    conflicts("+osmesa", when="+qt")
+    #conflicts("+osmesa", when="+qt")

     with when("+python"):
         # Depend on any Python, add bounds below.

Building on Frontier
~~~~~~~~~~~~~~~~~~~~

The following spack command is used to build with spack.

::

    spack install visit@3.3.3%gcc@11.2.0+mpi+gui+osmesa+vtkm ^python@3.7.7+shared ^mesa@21.2.5+opengl ^vtk@8.1.0+osmesa ^kokkos@3.7.01 ^vtk-m@1.9.0+kokkos+rocm~openmp+fpic amdgpu_target=gfx90a

Working around recurring download failures
------------------------------------------

Depending on context, recurring issues downloading a particular *dependent* package may arise.
When this happens, SSL certificate handling may be the cause.
A quick work-around is to disable this `security checking feature <https://spack.readthedocs.io/en/latest/config_yaml.html?highlight=ssl%20certificates#verify-ssl>`_ in Spack by adding the ``--insecure`` command-line option as the second option *just* after ``spack``.
Alternatively, you may be able to manually download the needed files and place them in a directory for Spack to use as a `mirror <https://spack.readthedocs.io/en/latest/mirrors.html?highlight=mirror#mirrors-mirrors-yaml>`_.
For example, starting from the point of having successfully downloaded the ``Python-3.7.13.tgz`` file somewhere, here are the Spack steps... ::

    spack mirror add my_local_mirror file://`pwd`/my_local_mirror
    mkdir -p my_local_mirror/python
    cp Python-3.7.13.tgz my_local_mirror/python/python-3.7.13.tgz

Note that change in case of the file name.
Doing this will cause Spack to go get the file you manually downloaded.
The first step to add the mirror is only needed once.
To add additional files for which recurring download failures are occurring, just copy them into the mirror following the Spack naming conventions for packages.

The spack environment files
---------------------------

Spack uses two files to control the environment on a system. They are the ``compilers.yaml`` file and the ``packages.yaml`` file.

The ``compilers.yaml`` file is used to specify information about compilers on a system.
The ``spack compilers`` command, as mentioned earlier, will create one for you with all the compilers on the system.
You can then customize it.
The complete documentation on the ``compilers.yaml`` file can be found `here <https://spack.readthedocs.io/en/latest/getting_started.html#compiler-configuration>`__.

The ``packages.yaml`` file is used to specify information about external packages on a system.
By default, spack will want to build everything from scratch for your system.
If some of the packages are already installed on the system you can use those by listing them in a ``packages.yaml`` file.
Typically, you will want to use an external MPI library on most HPC systems.
The ``spack externals find`` command will create an initial ``packages.yaml`` file for you.
The ``spack externals find`` command is non-destructive and will append to an existing ``packages.yaml`` file.
You can then customize it.
The complete documentation on the ``packages.yaml`` file can be found `here <https://spack.readthedocs.io/en/latest/build_settings.html>`__.

These files are stored in your ``~/.spack`` directory. ::

    .spack
    .spack/<platform>
    .spack/<platform>/compilers.yaml
    .spack/packages.yaml

The VisIt_ repository at GitHub contains ``compilers.yaml`` and ``packages.yaml`` files for popular systems in the directory ``scripts/spack/configs``.

Here are the ``compilers.yaml`` and ``packages.yaml`` files for ``spock.olcf.ornl.gov`` for VisIt_.

``compilers.yaml``

.. literalinclude:: ../../../scripts/spack/configs/olcf/spock/compilers.yaml

``packages.yaml``

.. literalinclude:: ../../../scripts/spack/configs/olcf/spock/packages.yaml

Debugging a spack package
-------------------------

When doing a spack install and the install fails, it will automatically keep the directory where it did the work, called the `stage` directory, which will allow you debug the failure.
If you want to modify an otherwise successful install or explore the state of a successful install you can use the ``--keep-stage`` flag to the ``spack install`` command. ::

    spack install --keep-stage visit ^python+shared ^mesa+opengl

If you are developing a new package from scratch and need to create the stage directory. ::

    spack stage visit

To go to the stage directory and set up the spack environment. ::

    spack cd visit
    spack build-env visit bash

Note that this will create a new shell so you will want to do an ``exit`` when you are finished.

Spack will cache various items that will sometimes undermine changes you are making while developing a package.
If you believe this is happening then you can clear all the caches. ::

    spack clean -a

Here are some common locations of stage directories. ::

    /tmp/<username>/spack-stage
    /var/tmp/<username>/spack-stage

E4S Project
-----------

The Extreme-scale Scientific Software Stack (E4S) is a community effort to provide open source software packages for developing, deploying and running scientific applications on high-performance computing (HPC) platforms.
E4S provides from-source builds and containers of a broad collection of HPC software packages.

E4S exists to accelerate the development, deployment and use of HPC software, lowering the barriers for HPC users.
E4S provides containers and turn-key, from-source builds of more than 80 popular HPC products in programming models, such as MPI; development tools such as HPCToolkit, TAU and PAPI; math libraries such as PETSc and Trilinos; and Data and Viz tools such as HDF5 and VisIt_.

E4S packages build on most computer systems, from laptops to supercomputers by using spack as the meta-build tool for the packages.

The E4S software distribution is tested regularly on a variety of platforms, from Linux clusters to leadership platforms.

The E4S testsuite
~~~~~~~~~~~~~~~~~

As a member of E4S, VisIt_ has tests that are part of the E4S-Project testsuite repository at GitHub.

Complete information on the testsuite can be found in the README at the bottom of the testsuite repository located `here <https://github.com/E4S-Project/testsuite>`__.

Running the tests
+++++++++++++++++

The first step is to clone the testsuite from GitHub. ::

    # Using https.
    git clone https://github.com/E4S-Project/testsuite

    # Using ssh.
    git clone ssh://git@github.com/brugger1/testsuite

Now you need to set up your spack environment. ::

    # bash shell
    /path/to/spack/share/spack/setup-env.sh

    # c shell
    source /path/to/spack/share/spack/setup-env.csh

Now you are ready to run the VisIt_ tests. ::

    cd testsuite
    ./test-all.sh ./validation_tests/visit
