.. _dev_create_bv_module:

Creating a build_visit Module
=============================

To create the skeleton for a build_visit module, run ``construct_build_visit_module.sh <module>``, where ``<module>`` is the name of the library you wish to build.
The script is located in ``src/tools/dev/scripts/bv_support``.
For purposes of the rest of this section, ``<module>`` will be named ``foo``.

Running ``construct_build_visit_module.sh foo`` will create the file: ``bv_foo.sh`` containing most of the code needed, and only a few functions will require modification.

There are three functions in the file that absolutely need to be fleshed out further: ``bv_foo_info``, ``bv_foo_host_profile``, and ``build_foo``.
Other build_visit modules may be helpful as examples for filling these out.
If the package being built has a decent CMake build system, please use that in the `build_foo` function.

Here is a list of all the functions that will be defined (listed in order of appearance in the file):

bv_foo_initialize
    Initialize any variables controlled by command line options.

bv_foo_enable
    Enables the module (sets `DO_FOO` to `yes`).

bv_foo_disable
    Disables the module (sets `DO_FOO` to `no`).

bv_foo_depends_on
    What other modules does `foo` depend on.
    For example, the `osmesa` module returns `llvm`.

bv_foo_info
    Name of the `foo` tarball, the version, etc.

bv_foo_print
    Prints info about the `foo` module.

bv_foo_print_usage
    Prints how to enable the module and any other relevant command line args.

bv_foo_host_profile
    Adds the necessary information about the `foo` module to config-site host profile cmake file.
    Most often this is just the insall location, but could also be version information.

bv_foo_initialize_vars
    Set vars possibly needed by other modules, such as install location for the module (can use EXPORT).
    This is an optional function and can be removed if not needed.

bv_foo_ensure
    Ensure the module has been downloaded and extracted properly.

build_foo
    Where all the steps for building and installing the module reside.

bv_foo_is_enabled
    Returns true if the module is enabled, and false otherwise

bv_foo_is_installed
    Returns true if the module is installed, and false otherwise

bv_foo_build
    Checks if `foo` is already installed, and calls `build_foo` if not.

Most of the above referenced functions will suit fine as originally written by the construction script.

If your module has dependencies on other build_visit modules, then also modify ``bv_foo_depends_on``.

If you want to allow use of a system version of your module, then ``bv_foo_initialize`` needs work to ensure extra command line arguments are added.
See ``bv_qt.sh`` as an example of allowing system or also an alternate (already installed but not system) qt.
``bv_qt_system_qt`` and ``bv_alt_qt_dir`` were added and other functions were modified to support this for qt.


Once ``bv_foo.sh`` has been updated appropriately, add the module name to ``bv_support/modules.xml`` under the appropriate categories.
Then run the build_visit script to ensure that your module builds and installs correctly, and that the host profile entry is correct.


Order of execution of the functions:

#. bv_foo_initialize
#. bv_foo_info
#. bv_foo_[enable|disable]
#. bv_foo_is_enabled
#. bv_foo_initialize_vars (if defined)
#. bv_foo_ensure
#. bv_foo_is_enabled
#. bv_foo_is_installed
#. bv_foo_depends_on
#. bv_foo_build
#. bv_foo_host_profile



