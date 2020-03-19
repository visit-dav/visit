.. _Prerequisites:

Prerequisites
-------------

VisIt's Source Code
~~~~~~~~~~~~~~~~~~~

For a released version
""""""""""""""""""""""

If you want to build a released version of VisIt, you can download a windows 
installer that contains all that is necessary from the `source code downloads 
page <https://visit.llnl.gov/source>`_. Look for the *VisIt Windows sources*  
link for the particular version you want.

For the latest development version
""""""""""""""""""""""""""""""""""
If you want to build the latest development version from our repository,
you need to obtain source from the 
`visit repo <https://github.com/visit-dav/visit>`_, and the pre-built third 
party dependencies from the 
`visit-deps repo <https://github.com/visit-dav/visit-deps>`_ on GitHub.


Other Software
~~~~~~~~~~~~~~

1. `CMake <https://cmake.org/download>`_ version 3.8 or greater.

   * Don't use the CMake included with cygwin if you plan on using the pre-built thirdparty libraries.

2. Visual Studio 2017 64-bit

   * Needed if you want to use our pre-built thirdparty libraries.

3. `NSIS <http://www.nsis.sourceforge.net>`_ *Optional*

   * For creating an installer for VisIt. NSIS 2 is known to work. NSIS 3 hasn't been tested.

4. `7zip <http://7-zip.org>`_ *Optional*

   * Used to untar testdata files.

5. `Microsoft MPI <https://www.microsoft.com/en-us/download/details.aspx?id=57467>`_. *Optional*

   * For building VisIt's parallel engine.  Redistributable binaries and SDK's are needed, so download and install both msmpisdk.msi and msmpisetup.exe.
