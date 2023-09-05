.. _dev_adding_find_module:

Adding a Find Module for Third-Party Libraries
===============================================

VisIt_ uses custom `Find` modules for most of its dependent third-party libraries.
The `Find` modules live in ``src/CMake`` and most of them utilize special functions that live in ``src/CMake/SetUpThirdParty.cmake``.
The most important of these is ``SET_UP_THIRD_PARTY``, as it does the bulk of the work in determining platform-specific extensions, handles ``.so`` versioning and Windows dll's and import libraries.

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

The ``INSTALL`` commands that VisIt_ uses are custom functions:

THIRD_PARTY_INSTALL_LIBRARY(LIBFILE):
    Installs the library file.

    `LIBFILE` is the full path to a library file.


THIRD_PARTY_INSTALL_INCLUDE(pkg incdir)
    Installs pkg's headers.

    `pkg` is the name of the package.

    `incdir1` is the full path to the headers (root of include tree if it involves multiple subdirectories).





