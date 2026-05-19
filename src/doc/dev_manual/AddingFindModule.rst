.. _dev_adding_find_module:

Adding a Find Module for Third Party Libraries
==============================================

VisIt_ uses custom `Find` modules for most of its dependent third party libraries.
The `Find` modules live in ``src/CMake`` and most of them utilize special functions that live in ``src/CMake/SetUpThirdParty.cmake``.

Starting with VisIt_ version 3.5.1, new functions were introduced to simplify the logic needed in a `Find` module.


Version 3.5.1 and above
-----------------------

visit_import_third_party
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cmake

    visit_import_third_party(pkg
                     LIBS               [lib1 [lib2 ...]]
                     LIBNAMES           [libname1 libname2 ...]
                     HEADER_ONLY        [TRUE | FALSE (default)]
                     DEFINES            [define1 [define2 ...]]
                     WIN32DEFINES       [win32define1 [win32define2 ...]]
                     INCDIR             [includesubdir]
                     LIBDIR             [libsubdir]
                     ADD_GLOBAL_INCLUDE [TRUE | FALSE]
                     ADD_GLOBAL_LIBRARY [TRUE | FALSE])

The function utilizes `blt_import_library` to create exportable CMake targets that have all the information needed for VisIt_ to compile against and link with a thirdparty library.
The first argument must be the uppercase name of the library to be found.
A CMake target with a name that is the lowercase version of the first argument will be created if the library is found.

LIBS: Required if HEADER_ONLY OR LIBNAMES not specified.
  A list of libraries to be found and added to the target.

LIBNAMES: Required if LIBS or HEADER_ONLY not specified.
  For single-library packages if the names may be different depending on platform/version/build options.

HEADER_ONLY: Required if LIBS or LIBNAMES not specified.
  The packge is a header-only library.

DEFINES: Optional
  List of compiler definitions.

WIN32DEFINES: Optional
  List of compiler definitions specific to Windows.

INCDIR: Optional
  Path beyond the `root-package-dir/include` where includes may be found.
  For example, if the headers are in `include/<pkgname>/<pkgversion>`.

LIBDIR: Optional
  Path beyond the `root-package-dir/lib` or `root-package-dir/lib64` where libraries may be found.
  `lib` and `lib64` will automatically be searched.

ADD_GLOBAL_INCLUDE: Optional
  Specifies that PKG_INCLUDE_DIR cache var should be created.
  This may be needed to aid another thirdparty library's find, if it depends on this package.

ADD_GLOBAL_LIBRARY: Optional
  Specifies that PKG_LIBRARY cache var should be created.
  This may be needed to aid another thirdparty library's find, if it depends on this package.


This function uses the <PKG>_DIR hint from the host config-site file as the location hint to search for specified libraries and include directories.
`Find` modules that use this function will be have their created targets listed in VisIt_'s installed `visitTargets.cmake` for use with tasks such as building a private plugin against an installed version of VisIt_.

.. note::
  This helper function should be used only with those libraries that don't provide their own exported .cmake files, or if the library's provided .cmake files cannot be used due to problems like hard-coded paths.

.. note::
  HAVE_<PKG> CMake cache var will be created if the package is found.


Heres are a few examples:

.. code-block:: cmake

    visit_import_third_party(XDMF LIBS Xdmf)

.. code-block:: cmake

    visit_import_third_party(JPEG LIBNAMES jpeg libjpeg ADD_GLOBAL_INCLUDE ADD_GLOBAL_LIBRARY)




Library packages that provide their own `.cmake` modules
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For libraries that provide useable `.cmake` files, use `find_package`, along with ``INSTALL`` commands and `create_lib_setup_cmake`.

Example:

.. literalinclude:: ../../CMake/FindHDF5.cmake
    :language: CMake
    :start-at: find_package(HDF5 CONFIG PATHS ${HDF5_DIR} NO_DEFAULT_PATH)


create_lib_setup_cmake
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cmake

    create_lib_setup_cmake(
                     NAME               [libraryname]
                     NAMESPACE          [Namespace used by NAME]
                     ITEMS              [target1 [target2 ...]]
                     INCBASE            [includesubdir]
                     SIMPLE_INCLUDE     [TRUE | FALSE (default)]
                     SKIP_HEADER        [TRUE | FALSE (default)])


Used for complex thirdparty libraries for which `blt_import_library` cannot be used with the EXPORTABLE flag on.


NAME: Required
    Name of the thirdparty library (will be used for the filename).

NAMESPACE: Required
    Namespace used by NAME.
    If none used, repeat the NAME

ITEMS: Required
    Initial list of targets to be written.

INCBASE: Required
    Base of include directory
    Most often this will be `include`, but may be `include/<pkg>/<version>`.

SIMPLE_INCLUDE: Optional
    If true, don't get includes from targets.
    Fast track for libraries where all headers are in the `include` subdir.

NESTED_INCLUDES: Optional
    If true, targets may have deeply nested includes.

SKIP_HEADER: Optional
    Don't write the header.
    Useful for multi-namespace libraries that need to call this function twice (like Qt).

This function queries the targets listed in ``ITEMS`` for library locations, link dependencies, includes, defines, etc, and writes the info to a `Setup${NAME}.cmake` file.
The created setup file is used when importing VisIt_ (like when building a private plugin against an installed version) to create CMake targets that satisfy VisIt_'s dependencies.


Version 3.5.0 and below
-----------------------

For pre-3.5.1 VisIt, the most important function in `SetUpThirdParty.cmake` is ``SET_UP_THIRD_PARTY``, as it does the bulk of the work in determining platform-specific extensions, handles ``.so`` versioning and Windows dll's and import libraries.

The first argument to the function is the name of the package (eg ``FOO``), and it expects a capitalized package name that corresponds to a ``VISIT_<PKG>_DIR`` entry in VisIt_'s config-site files.
This function also uses these keyword arguments:

LIBS: required
   The name(s) of the library(ies) without platform-specific prefixes, suffixes or extensions.
   For example: ``SETUP_THIRD_PARTY(FOO LIBS foo1 foo2)``, not ``SETUP_THIRD_PARTY(FOO LIBS libfoo1.so libfoo2.so)``.

LIBDIR: optional
    The paths beyond ``VISIT_<PKG>_DIR`` where the libraries may be found.
    ``VISIT_<PKG>_DIR/lib`` or  ``VISIT_<PKG>_DIR>/lib64`` will be assumed if this is not provided.

INCDIR: optional
    The path beyond ``VISIT_<PKG>_DIR`` where the headers may be found.
    ``VISIT_<PKG>_DIR/include`` will be assumed if this is not provided.


There may be situations where this simple solution is not sufficient: a package may have already created its own `Find` module and duplication of effort isn't desired; or the package may be complex with many components all of which may not necessarily be needed by VisIt_ (e.g. VTK).
For situations where this function is not sufficient, standard CMake find calls (`find_path`, `find_library`, `find_package`) may be used along with the necessary special ``INSTALL`` commands for ensuring the package's libraries (and possibly headers) are installed alongside VisIt_.


INSTALL commands 
----------------

The ``INSTALL`` commands that VisIt_ uses are custom functions:

THIRD_PARTY_INSTALL_LIBRARY(LIBFILE):
    Installs the library file.

    `LIBFILE` is the full path to a library file.


THIRD_PARTY_INSTALL_INCLUDE(pkg incdir)
    Installs pkg's headers.

    `pkg` is the name of the package.

    `incdir` is the full path to the headers (root of include tree if it involves multiple subdirectories).





