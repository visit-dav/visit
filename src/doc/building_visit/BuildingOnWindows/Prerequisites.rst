.. _Prerequisites:

Prerequisites
-------------

VisIt's Source Code
~~~~~~~~~~~~~~~~~~~

For a released version
""""""""""""""""""""""

For building a released version of VisIt version 3.2.1 or earlier, you can download a windows installer that contains all that is necessary from the `downloads page <https://visit-dav.github.io/visit-website/releases-as-tables>`_.
Look for the *Win 10/8/7 development*  link for the particular version you want.
The file will be named *visitdev3.2.1.exe* (or similar, based on version chosen).

For building a released version of VisIt version 3.2.2 or newer, download the *Source* tgz file as well as the *Win 10/8/7 development* file which will be named *visit_windowsdev_3.2.2.zip* (or similar, based on version chosen).
The .zip file contains all the pre-built third-party binaries needed for building VisIt on Windows.
It is best to extract these files to the same folder so that *src* from the source tarball is peer to *windowsbuild* from the windows-dev zip file.

For the latest development version
""""""""""""""""""""""""""""""""""

If you want to build the latest development version from our repository, you need to obtain source from the `visit repo <https://github.com/visit-dav/visit>`_, and the pre-built third party dependencies from the `visit-deps repo <https://github.com/visit-dav/visit-deps>`_ on GitHub.


Other Software
~~~~~~~~~~~~~~

1. `CMake <https://cmake.org/download>`_ version 3.15 or greater.

   * Don't use the CMake included with cygwin if you plan on using the pre-built thirdparty libraries.

2. Visual Studio 2017 64-bit

   * Needed if you want to use our pre-built thirdparty libraries.

3. `NSIS <http://www.nsis.sourceforge.net>`_ *Optional*

   * For creating an installer for VisIt. NSIS 2 or 3 should work.

4. `Microsoft MPI <https://www.microsoft.com/en-us/download/details.aspx?id=57467>`_. *Optional*

   * For building VisIt's parallel engine.  Redistributable binaries and SDK's are needed, so download and install both msmpisdk.msi and msmpisetup.exe.
