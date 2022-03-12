.. _Building Directly with CMake:

Building Directly with CMake
----------------------------
If a *config site file* is available for the platform you wish to build on, VisIt_ can often be built without the use of the ``build_visit`` script, with these steps.

.. code:: bash

  git clone git@github.com:visit-dav/visit.git
  mkdir visit/build
  cd visit/build

If ``build_visit`` was used to build VisIt_ on the platform in the past, it should have created a cmake file specific to your machine which we call a *config site* file. CMake simply needs to be told where to find it using the ``-DVISIT_CONFIG_SITE`` option. Examples of *config site* files for a variety of machines VisIt_ developers directly support can be found in the `config-site <https://github.com/visit-dav/visit/tree/develop/src/config-site>`_ directory.

.. code:: bash

  /path/to/cmake ../src/ -DVISIT_CONFIG_SITE="/path/to/your_computer.cmake"
  make -j
