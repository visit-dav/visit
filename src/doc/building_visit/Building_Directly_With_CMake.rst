.. _Building Directly with CMake:

Building Directly with CMake
----------------------------
If the third party libraries have already been installed using ``build_visit``, 
If a *config site file* is available for the platform you wish to build on, VisIt_ can often be built without the use of the ``build_visit`` script, with these 
steps.

.. code:: bash

  git clone git@github.com:visit-dav/visit.git
  mkdir visit/build
  cd visit/build

``build_visit`` should have created a cmake file specific to your machine.
CMake needs to be simply pointed at it. Examples can be found in the 
``src/config-site`` directory.

.. code:: bash

  /path/to/cmake ../src/ -DVISIT_CONFIG_SITE="/path/to/your_computer.cmake"
  make -j
