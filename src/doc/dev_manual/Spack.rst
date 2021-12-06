Building with Spack
===================

Overview
--------

Spack is a multi-platform package manager that builds and installs multiple versions and configurations of software. It works on Linux, macOS, and many supercomputers. Spack is non-destructive: installing a new version of a package does not break existing installations, so many configurations of the same package can coexist.

Spack offers a simple "spec" syntax that allows users to specify versions and configuration options. Package files are written in pure Python, and specs allow package authors to write a single script for many different builds of the same package. With Spack, you can build your software all the ways you want to.

The complete documentation can be found `here <https://spack.readthedocs.io/en/latest/index.html>`__.

The VisIt_ spack package
------------------------

The spack package for VisIt_ currently only builds a subset of the libraries that build_visit builds.
It builds the core libraries needed by VisIt_ and the following I/O libraries:

* HDF5
* Silo.

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

    pack install visit%gcc@11.2.0 ^python+shared ^mesa+opengl

To build VisIt_ and specify a specific version of a dependent library. ::

    pack install visit%gcc@11.2.0 ^python+shared ^mesa+opengl ^llvm@11.0.1

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

The VisIt_ repository at GitHub contains ``compilers.yaml`` and ``packages.yaml`` files for popular systems.

Here are the ``compilers.yaml`` and ``packages.yaml`` files for ``spock.olcf.ornl.gov`` for VisIt_.

``compilers.yaml`` ::

    - compiler:
        spec: gcc@11.2.0
        paths:
          cc: cc
          cxx: CC
          f77: ftn
          fc: ftn
        flags: {}
        operating_system: sles15
        target: any
        modules:
        - PrgEnv-gnu
        - gcc/11.2.0
        environment: {}
        extra_rpaths: [/opt/cray/libfabric/1.11.0.4.75/lib64]

``packages.yaml`` ::

    packages:
      autoconf:
        externals:
        - spec: autoconf@2.69
          prefix: /usr
      automake:
        externals:
        - spec: automake@1.15.1
          prefix: /usr
      bison:
        externals:
        - spec: bison@3.0.4
          prefix: /usr
      cmake:
        externals:
        - spec: cmake@3.17.0
          prefix: /usr
      cpio:
        externals:
        - spec: cpio@2.12
          prefix: /usr
      diffutils:
        externals:
        - spec: diffutils@3.6
          prefix: /usr
      file:
        externals:
        - spec: file@5.32
          prefix: /usr
      findutils:
        externals:
        - spec: findutils@4.6.0
          prefix: /usr
      flex:
        externals:
        - spec: flex@2.6.4+lex
          prefix: /usr
      gawk:
        externals:
        - spec: gawk@4.2.1
          prefix: /usr
      gcc:
        externals:
        - spec: gcc@7.5.0 languages=c,c++,fortran
          prefix: /usr
          extra_attributes:
            compilers:
              c: /usr/bin/gcc-7
              cxx: /usr/bin/g++
              fortran: /usr/bin/gfortran-7
      ghostscript:
        externals:
        - spec: ghostscript@9.52
          prefix: /usr
      git:
        externals:
        - spec: git@2.26.2~tcltk
          prefix: /usr
      gmake:
        externals:
        - spec: gmake@4.2.1
          prefix: /usr
      groff:
        externals:
        - spec: groff@1.22.3
          prefix: /usr
      libtool:
        externals:
        - spec: libtool@2.4.6
          prefix: /usr
      m4:
        externals:
        - spec: m4@1.4.18
          prefix: /usr
      ncurses:
        externals:
        - spec: ncurses@6.1.20180317+termlib abi=6
          prefix: /usr
      openssh:
        externals:
        - spec: openssh@8.1p1
          prefix: /usr
      openssl:
        externals:
        - spec: openssl@1.1.1d
          prefix: /usr
      perl:
        externals:
        - spec: perl@5.26.1~cpanm+shared+threads
          prefix: /usr
      pkg-config:
        externals:
        - spec: pkg-config@0.29.2
          prefix: /usr
      python:
        externals:
        - spec: python@2.7.18+bz2+ctypes~dbm~lzma+nis+pyexpat+readline+sqlite3+ssl~tix~tkinter+uuid+zlib
          prefix: /usr
        - spec: python@3.6.13+bz2+ctypes~dbm+lzma+nis+pyexpat+readline+sqlite3+ssl~tix~tkinter+uuid+zlib
          prefix: /usr
      rsync:
        externals:
        - spec: rsync@3.1.3
          prefix: /usr
      ruby:
        externals:
        - spec: ruby@2.5.9
          prefix: /usr
      sed:
        externals:
        - spec: sed@4.4
          prefix: /usr
      tar:
        externals:
        - spec: tar@1.30
          prefix: /usr
      texinfo:
        externals:
        - spec: texinfo@6.5
          prefix: /usr
      xz:
        externals:
        - spec: xz@5.2.3
          prefix: /usr
      all:
        compiler: [gcc, cce]
        providers:
          mpi: [cray-mpich]
      cray-mpich:
        buildable: false
        externals:
        - prefix: /opt/cray/pe/mpich/8.1.10/ofi/gnu/9.1
          spec: cray-mpich@8.1.10%gcc
          modules:
          - cray-mpich/8.1.10
            cray-pmi/6.0.14
            cray-pmi-lib/6.0.14
            libfabric/1.11.0.4.75
        - prefix: /opt/cray/pe/mpich/8.1.10/ofi/cray/10.0
          spec: cray-mpich@8.1.10%cce
          modules:
          - cray-mpich/8.1.10
            cray-pmi/6.0.14
            cray-pmi-lib/6.0.14
            libfabric/1.11.0.4.75

Debugging a spack package
-------------------------

When doing a spack install and the install fails, it will automatically keep the stage directory, which will allow you debug the failure.
If you want to modify an otherwise successful install or explore the state of a successful install you can use the ``--keep-stage`` flag to the ``install`` command. ::

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
