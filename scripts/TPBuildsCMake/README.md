# Build Third party libraries for VisIt with CMake on Windows

CMake ExternalProject build system for VisIt's third party library dependencies.

Designed for Windows and Visual Studio.

##  How to use

The basic usage is to open a Visual Studio enabled terminal window, then issue the CMake commands for building:

Tell CMake where the source and build dirs are:

```
cmake -S /path/to/CMakeLists.txt -B Build
```

`-S` is used to specify the Source location, the place where this Build system's CMakeLists.txt lives.

`-B` is used to specify the location of the Build.
If not wanting it to be a subdir from the current location, specify a full path.
This is the location where everything will be downloaded and compiled.


Tell CMake to perform the build:

```
cmake --build Build --config Release
```

Because this build system uses CMake's `ExternalProject_Add`, there is no need to issue an install command.
The build and install commands happen together.

The installation directory by default will be peer to the build directory, so if build is /path1/to/build, the install will be /path1/to/INSTALL.
The installation location can be changed by passing arguments (described below) to the first command.

### Command line options

The following options can be passed on the command line:

    visit_install_root  Directory where you want the libraries to be installed
                        Default: peer to BUILD directory
                        -Dvisit_install_root:PATH=/path/to/install

    visit_patch_command Full path to patch command.
                        Default: C:/Program Files/Git/user/bin/patch.exe
                        -Dvisit_patch_command:FILEPATH=/path/to/patch.exe

    BUILD_ALL           Build all supported libraries.
                        Default: ON
                        -DBUILD_ALL:BOOL=OFF

    ENABLE_XXX          Enable a specific library when BUILD_ALL is OFF where XXX is the capitalized name of the library.
                        See list of libraries below for all available options.

     VISIT_LOGGING      If ON, enable logging of all steps (download, verify, patch, configure build, install) for each library being built.
                        The logs are library-specific and are written to that library's subdirectory in the build.
                        The logs can be helpful when there are issues with a library.




### PATH environment variable requirements

Paths to the following executables must be part of your PATH environment varaiable:
* CMake (for python module installation)
* Ninja (for qt)
* nasm  (for openssl (needed by qt))
* perl  (for openssl (needed by qt))

### Supported Libraries

* ADIOS2
* ANARI
* CFITSIO
* CGNS
* CONDUIT
* GDAL
* HDF5
* H5PART
* ICET
* JPEG
* MFEM
* MILI
* NETCDF
* OPENSSL
* OPENXR
* OSPRAY
* PNG
* PYTHON
* QT
* QWT
* TIFF
* VTK
* VTKM
* XDMF
* ZLIB

### To use the generated libraries when building VisIt.

If the chosen INSTALL location is not the standard location within visit-deps/windowsbuild/MSVC2022, then add `-DVISIT_THIDPARTY_PATH:PATH=/path/to/INSTALL` when configuring VisIt with CMake.

## Caveats

This build system does not currently allow linking to system installed versions of the libraries.

All libraries are installed to subdirectories of a common installation directory.
If any library is determined to already exist in the installation directory, it will not be re-built.
This allows for updating specific libraries without necessarily needing to rebuild any libraries it depends upon.

The build options for each library were chosen with the specific purpose of being used for VisIt on Windows.

##  Updating

These modules should be updated whenever build_visit modules are updated.
The same version of libraries should be used whenever possible.
Only generic patches are needed, not patches specific to Linux or Macos.
When updating any library that has other libraries depend on (like hdf5-> silo), the other libraries' build should be tested along with the library being updated.

