.. _Setup:

Setup
-----

Masonry Scripts
~~~~~~~~~~~~~~~

The masonry scripts are bundled with VisIt_'s source code. You will need to download 
the source code and extract masonry from ``visit/src/tools/dev``. There are a few options 
for downloading the source code. If you want a released version of VisIt_ then go to the
`source code downloads page <https://visit.llnl.gov/source>`_ and look for the *VisIt sources*
link. The other option is to download from the `git repository <https://github.com/visit-dav/visit>`_.
Once you have the source code, copy ``visit/src/tools/dev/masonry`` to a location of your choosing.

Configuration
~~~~~~~~~~~~~

1. In the ``opts`` directory copy one of the ``*.json`` files and rename it as desired.
   For example: ``cp mb-3.1.1-darwin-10.14-x86_64-release.json mb-3.1.2-darwin-10.14-x86_64-release.json``

2. Open the JSON configuration file (see :numref:`Figure %s<masonry_config_file>`)  created in **step 1** and modify or add the following options as needed:
   
``version``: **required** 
   The version of VisIt you are building.

``build_types``: **required** 
   A list of builds for masonry to create.

``build_dir``: **optional** 
   The directory to place all of the files generated from the build process. If this option isn't specified the build directory will default to ``build-<json_base>`` (e.g., build-mb-3.1.2-darwin-10.14-x86_64-release) in your current working directory. 

``branch``: **required** 
   The git branch to checkout and build.

``arch``: **required**
   The build architecture (e.g., darwin-x86_64).

``cert``: **optional** 
   The Developer ID signing certificate **Common Name**.

``make_nthreads``: **optional** 
   The number of parallel threads to use when building the source code.

``skip_checkout``: **optional**
   if you have to restart masonry and already have the source code checked out you can skip that step by setting this option to *yes*. 

``boost_dir``: **optional**
   The path to boost if installed on your system. This also triggers the setting of two CMake options (**VISIT_USE_BOOST:BOOL** and **BOOST_ROOT:PATH**).

``git``: **required** 
   ``mode``: **required** - set this option to **ssh** or **https**
   
   ``git_uname``: **optional** - github username

   ``depth``: **optional** - specify an integer value for a shallow clone with a history truncated to the specified number of commits.

``build_visit``: **required** 
   Allows you to set the ``build_visit`` options.

   ``cmake_ver``: **required** - the CMake version to use

   ``args``: **optional** - arguments for ``build_visits``

   ``libs``: **optional** - third-party libraries to build

   ``make_flags``: **optional** - Make flags

``force_clean``: **optional**
   Removes all files and directories from your build folder.

``c_compiler``: **optional**
   Specify the C compiler

``cxx_compiler``: **optional**
   Specify the C++ compiler

``tarball``: **optional**
   Specify the path to the source tar file. This option is currently not being used.

``build_xdb``: **optional**
   Set the **VISIT_ENABLE_XDB:BOOL** option to *ON* if true. 

``cmake_extra_args``: **optional**
   Specify extra arguments for CMake.

``config_site``: **optional**
   Specify a path for the config site file.

``platform``: **optional**
   Specify the platform (**osx** or **linux**)

.. _masonry_config_file:

.. figure:: images/config.png
   :width: 50%
   :align: center

   Masonry's JSON config file

Signing macOS Builds
~~~~~~~~~~~~~~~~~~~~
To `code sign <https://developer.apple.com/library/archive/technotes/tn2206/_index.html>`_ your VisIt_ build, you must be enrolled in the `Apple Developer Program <https://developer.apple.com/programs/>`_ and have a valid Developer ID certificate. Below are simple steps to get started, reference the links for more detailed information.

1. Enroll in the Apple Developer Program, if needed, and create your Developer ID certificates.

2. Install Apple certificates into your keychain

   * From **Xcode** go to the account preferences (``Xcode->Preferences->Account``) and select the **Manage Certificates...** button.

   * Click the **+** to add your certificates (see :numref:`Figure %s<xcode_certs_image>`).

3. Add the Developer ID signing certificate **Common Name** to the **cert** option in the masonry JSON configuration file.

.. _xcode_certs_image:

.. figure:: images/certs.png
   :width: 50%
   :align: center

   Xcode Manage Certificates Dialog

