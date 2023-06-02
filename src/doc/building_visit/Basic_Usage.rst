.. _Basic Usage:

Basic Usage
-----------

Doing a minimal build
~~~~~~~~~~~~~~~~~~~~~

When using ``build_visit`` without any arguments it will do a minimal build
of VisIt_ downloading the VisIt_ source code by making an anonymous git clone
from GitHub and downloading the source code for the third party libraries
from NERSC. It will build a serial version of the code without any of the
optional I/O libraries. This will result in only the file readers that require
no external dependencies to be built. Buiding VisIt_ in this fashion will give
you the highest probability of success.

.. code:: bash

  ./build_visit3_0_1

Building with multiple cores
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``build_visit`` is run by default it will build the code using a single
core. This may take a half a day or longer. Modern computers have anywhere
from 4 to 80 cores at the time of the writing of this chapter. You can speed
up the build process by specify that ``build_visit`` use more cores. If you
are using a shared resource you probably shouldn't use all the cores in
consideration of other users of the system. The following example specifies
using 4 cores.

.. code:: bash

  ./build_visit3_0_1 --makeflags -j4

Specifying the third party library install location
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When ``build_visit`` is run by default it will install the third party
libraries in the directory ``third_party`` in the current directory. If you
would like to install the libraries in another directory for the purposes
of sharing them with other users of the system, you can have ``build_visit``
install them in a different directory. The following example specifies
installing the third party libraries in a another location.

.. code:: bash

  ./build_visit3_0_1 --thirdparty-path /usr/gapps/visit/third_party

Building with the HDF5 and Silo_ libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some of the more common I/O libraries that will result in building a larger
number of file readers are HDF5 and Silo_. The following example specifies
building HDF5 and Silo_.

.. code:: bash

  ./build_visit3_0_1 --hdf5 --silo

Building the stable optional libraries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you are feeling lucky you can have ``build_visit`` build all of the optional
I/O libraries that have a high probability of building. The following example
specifies building the more reliable of the optional I/O libraries.

.. code:: bash

  ./build_visit3_0_1 --optional

Using a VisIt_ source code tar file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also have visit use the prepackaged source code for a specific version
of VisIt_ instead of doing a git download of the source code. The tar file
should be considerably smaller than a git clone. The following example uses
the VisIt_ source code corresponding to the official 3.0.1 release of VisIt_.

.. code:: bash

  ./build_visit3_0_1 --optional --tarball visit3.0.1.tar.gz

If ``build_visit`` is interrupted
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If ``build_visit`` is interrupted while it is executing, it is suggested that
you remove the directories associated with the last package it was in the
process of building. ``build_visit`` always leaves directories intact when
it runs to aid with troubleshooting failures. Likewise, ``build_visit``
doesn't remove existing directories before starting to build a package.
This can sometimes problems when ``build_visit`` is interrupted and you
restart the build again.

Finishing up
~~~~~~~~~~~~

Once you have successfully built VisIt, there are a couple of directions
you can go. The first option is to use it in the location where it was
built. The executable can run by executing the following command:

.. code:: bash

  visit/build/bin/visit

if you built using a git clone.

.. code:: bash

  visit3.0.1/build/bin/visit

if you built using a tar file.

The second option is to create a distribution file that you can install
using ``visit-install``. This can be done by executing the following
command:

.. code:: bash

  cd visit/build
  make package

if you built using a git clone.

.. code:: bash

  cd visit3.0.1/build
  make package

if you built using a tar file.


A note about compiler versions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you encounter problems with build_visit (especially building Qt), it may be due to the compiler version being used.

If building Qt 6, then a fully compliant c++17 compiler is required (g++8 or newer on Linux) otherwise a compiler supporting c++14 is needed, with minimum g++ set to 7.3 or newer on Linux.

Sometimes the absolute latest releases of compilers will cause compile errors during the building of third_party libraries requiring patches to the library's code base, a change to an older compiler version, or an update to a newer version of the library.
If this is the case, and you would like us to support the compiler version you are using, please contact us by one of our :ref:`Contact methods <methods_of_contact>`, letting us know the OS and OS-version as well as the compiler version you are attempting to use, and the version of VisIt_ being built, along with all compile error messages.

