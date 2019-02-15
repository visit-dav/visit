.. _Basic Usage
      

Basic Usage
-----------

When using ``build_visit`` without any arguments it will do a minimal build
of VisIt_ downloading the VisIt_ source code by making an anonymous git clone
from GitHub and the source code for the third party libraries from NERSC.
It will build a serial version of the code without any of the optional
I/O libraries. This will result in only the file readers that require no
external dependencies to be built. Buiding VisIt_ in this fashion will give
you the highest probability of success.

.. code:: bash

  ./build_visit3_0_0b

When ``build_visit`` is run it will build the code using a single core. This
may take a half a day or longer. Modern computers have anywhere from 4 to 80
cores at the time of writing chapter. You can speed up the build process by
specify that ``build_visit`` use more cores. If you using a shared resource
you probably shouldn't use all the cores in consideration of other users of
the system. The following example specifies using 4 cores.

.. code:: bash

  ./build_visit3_0_0b --makeflags -j4

Some of the more common I/O libraries that will result in building a larger
number of file readers are HDF5 and Silo. The following example specifies
building HDF5 and Silo.

.. code:: bash

  ./build_visit3_0_0b --hdf5 --silo --makeflags -j4

If you are feeling lucky you can have ``build_visit`` build all of the optional
I/O libraries that have a high probability of building. The following example
specifies building more reliable of the optional I/O libraries.

.. code:: bash

  ./build_visit3_0_0b --optional --makeflags -j4
