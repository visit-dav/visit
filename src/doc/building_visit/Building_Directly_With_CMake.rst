.. _Building Directly with CMake:

Building Directly with CMake
----------------------------
If a *config site file* is available for the platform you wish to build on, VisIt_ can often be built without the use of the ``build_visit`` script, with these steps.

.. code:: bash

  git clone --recursive git@github.com:visit-dav/visit.git
  mkdir visit/build
  cd visit/build

If ``build_visit`` was used to build VisIt_ on the platform in the past, it should have created a cmake file specific to your machine which we call a *config site* file. 
CMake simply needs to be told where to find it using the ``-DVISIT_CONFIG_SITE`` option. 
Examples of *config site* files for a variety of machines VisIt_ developers directly support can be found in the `config-site <https://github.com/visit-dav/visit/tree/develop/src/config-site>`_ directory.

.. code:: bash

  /path/to/cmake ../src/ -DVISIT_CONFIG_SITE="/path/to/your_computer.cmake"
  make -j

.. _CMake Variables:

CMake Variables
~~~~~~~~~~~~~~~

The following CMake vars can be modified to suit your build needs.

When specified via a command-line invocation of CMake, they should be specified as: VARNAME:TYPE=value, eg 'VISIT_BUILD_ALL_PLUGINS:BOOL=ON'.

The defaults listed are the settings used if the Variable has not been set in a *config-site* file.



Controlling major components being built
""""""""""""""""""""""""""""""""""""""""

VISIT_DBIO_ONLY : BOOL : OFF
    Toggles building of only visitconvert and engine plugins.

VISIT_ENGINE_ONLY : BOOL : OFF
    Toggles building of only the compute engine and its plugins.

VISIT_SERVER_COMPONENTS_ONLY : BOOL : OFF
    Build only vcl, mdserver, engine and their plugins.

VISIT_ENABLE_LIBSIM : BOOL : ON
    Toggles building of libsim.


Controlling plugins being built
"""""""""""""""""""""""""""""""

VISIT_BUILD_ALL_PLUGINS : BOOL : OFF
    Toggles the building of all plugins.
    When turned on the following optional plugins will be added to the build:

    Database: PICS_Tester, Rect

    Operator: Context ConnCompReduce, MetricThreshold, RemoveCells, SurfCompPrep

    Plot: Topology

    Note: the list of optional plugins is subject to change.

VISIT_BUILD_MINIMAL_PLUGINS : BOOL : OFF
    Toggles the building of a minimal set of database, operator, and plot plugins.
    When turned on, only the following plugins will be built:

    Database: Curve2D, RAW, VTK, PICS_Tester

    Operator: Lineout, Slice, Threshold

    Plot: Curve, Mesh, Pseudocolor

    Note: the list of minimal plugins is subject to change.

VISIT_SELECTED_DATABASE_PLUGINS : STRING
    ';' separated list of database plugins to build, eg: VTK;Silo

     If not empty, will supersede the settings of VISIT_BUILD_MINIMAL_PLUGINS and VISIT_BUILD_ALL_PLUGINS for database plugins.

VISIT_SELECTED_OPERATOR_PLUGINS : STRING
    ';' separated list of operator plugins to build, eg: Slice;Lineout;Transform

     If not empty, will supersede the settings of VISIT_BUILD_MINIMAL_PLUGINS and VISIT_BUILD_ALL_PLUGINS for operator plugins.

VISIT_SELECTED_PLOT_PLUGINS : STRING
    ';' separated list of plot plugins to build, eg: Mesh;Pseudocolor

     If not empty, will supersede the settings of VISIT_BUILD_MINIMAL_PLUGINS and VISIT_BUILD_ALL_PLUGINS for plot plugins.


Controlling extra tools being built
"""""""""""""""""""""""""""""""""""
VISIT_ENABLE_ANNOTATION_TOOLS : BOOL : ON
    Toggles the generation of annotation tools: text2polys, time_annotation.

VISIT_ENABLE_DATAGEN : BOOL: ON
    Toggles the generation of sample data files.

VISIT_ENABLE_DATA_MANUAL_EXAMPLES: BOOL : OFF
    Toggles generation of :ref:`Getting Data Into Visit<Getting Data into VisIt_>` examples.

VISIT_ENABLE_DIAGNOSTICS : BOOL : ON
    Toggles building of diagnostic tools: exceptiontest, mpitest, networktest, osmesatest.

VISIT_ENABLE_MANUALS : BOOL : ON
    Toggles building of manuals, requires Sphinx in Python.

VISIT_ENABLE_SILO_TOOLS : BOOL : ON
    Toggles building of Silo tools: mrgtree2dot, add_visit_searchpath.

VISIT_ENABLE_UNIT_TESTS : BOOL : ON
    Toggles building of unit tests: MRUCache, Namescheme, Utilty, StringHelpers, exprconfig, exprtest.

Useful for developers
"""""""""""""""""""""


VISIT_CREATE_SOCKET_RELAY_EXECUTABLE: BOOL : ON
    Toggles creation of separate executable that forwards VisIt_'s socket connection between engine and component launcher.

VISIT_CREATE_XMLTOOLS_GEN_TARGETS : BOOL : ON
    Toggles the creation of build targets to run xmltools code generation.
    More information can be found in the :ref:`XML Tools<XML Tools>` section of the *Developer Manual*

    **Be careful on Windows**, *all of the codegen targets will be built unless you tell Visual Studio to build the ALL_BUILD project (instead of the Solution). 
    This will cause a lot of source files to be regenerated and may cause problems with the build.* 



VISIT_RPATH_RELATIVE_TO_EXECUTABLE_PATH : BOOL : OFF
    Install rpath relative to executable location using \$ORIGIN tag.

CMAKE_SUPPRESS_REGENERATION : BOOL : OFF
    When on, tells CMake to suppress regeneration of project/make files when CMakeLists.txt or .cmake files have changed.

Miscellany
""""""""""

CMAKE_BUILD_TYPE : STRING : Release
    Specifies the build type for single-configuration generators (like Makefiles).

CMAKE_INSTALL_PREFIX : PATH : *default is system dependent* 
    Specifies the location for files installed with *make install.*

IGNORE_THIRD_PARTY_LIB_PROBLEMS : BOOL : OFF
    Ignore problems finding requested third party libraries.

VISIT_CONFIG_SITE : FILEPATH : ${VISIT_SOURCE_DIR}/config-site/<localhost>.cmake
    Location of a config-site cmake file that has settings to control the build, including locations of third-party libraries.
    Created automatically by build_visit script.

VISIT_DDT : BOOL : OFF
    Toggles support for the DDT debugger.

VISIT_DEFAULT_SILO_DRIVER : STRING : PDB
    Designates the default Silo driver to use when generating silo data.
    Options: PDB, HDF5

VISIT_DISABLE_SELECT : BOOL : OFF
    Toggles the disablement for use of the select() function.

VISIT_FORCE_SSH_TUNNELING : BOOL : OFF
    Toggles use of SSH tunneling for sockets.

VISIT_FORTRAN : BOOL : OFF
    Toggles building of Fortran example programs.

VISIT_INSTALL_THIRD_PARTY : BOOL : OFF
    Intall VisIt_'s 3rd part I/O libraries and includes to permit plugin development.

VISIT_JAVA : BOOL : OFF
    Build VisIt_'s Java client interface.

VISIT_NOLINK_MPI_WITH_LIBRARIES : BOOL : OFF
    Do not link MPI with VisIt_'s parallel shared libraries; just with executables

VISIT_PARALLEL : BOOL : ON
    Build VisIt_'s parallel compute engine.

VISIT_PYTHON_SCRIPTING : BOOL : ON
    Build VisIt_ with Python scripting support.

VISIT_PYTHON_FILTERS : BOOL : ON
    Build VisIt_ with Python Engine Filter support.

VISIT_SLIVR : BOOL : ON
    Build VisIt_ with support for the SLIVR volume rendering library.

VISIT_STATIC : BOOL : OFF
    Build VisIt_ statically.

Mac OS only
"""""""""""

VISIT_CREATE_APPBUNDLE_PACKAGE : BOOL : OFF
    Toggles creation of DMG file with Mac App bundle with make package.


Windows OS only
"""""""""""""""

VISIT_MAKE_NSIS_INSTALLER : BOOL : OFF
    Toggles creation of an installer package using NSIS.

    The windows.cmake *config-site* file turns this ON.

VISIT_MESA_REPLACE_OPENGL : BOOL : OFF
    Toggles use of Mesa as a drop-in replacement for OpenGL when system OpenGL is insufficient.

    The windows.cmake *config-site* file turns this ON.

VISIT_WINDOWS_APPLICATION : BOOL : ON
    Toggles creation of Windows-style applications with no console.

VISIT_WINDOWS_DIR : PATH : 
    Specifies the location of the pre-built third party library binaries.
    See :ref:`Location of windowsbuild directory<Location of windowsbuild Directory>` for default locations.

