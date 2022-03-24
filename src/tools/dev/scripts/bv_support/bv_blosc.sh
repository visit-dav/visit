# Initialize any variables you may want to export
function bv_blosc_initialize
{
    # by default, turn blosc off
    export DO_BLOSC="no"
}

# Enables the module
function bv_blosc_enable
{
    DO_BLOSC="yes"
}

# Disables the module
function bv_blosc_disable
{
    DO_BLOSC="no"
}

# Where to get the module, the version, etc...
function bv_blosc_info
{
    c-blosc2-2.0.4.tar.gz

    export BLOSC_VERSION=${BLOSC_VERSION:-"2.0.4"}
    export BLOSC_FILE=${BLOSC_FILE:-"c-blosc2-${BLOSC_VERSION}.tar.gz"}
    export BLOSC_URL=${BLOSC_URL:-"http://releases.llvm.org/${BV_LLVM_VERSION}/"}
    export BLOSC_BUILD_DIR=${CONDUIT_BUILD_DIR:-"conduit-${CONDUIT_VERSION}"}
    export BLOSC_MD5_CHECKSUM="7f9bb79ef0a6bf42fa9bc05af3829e5b"
    export BLOSC_SHA256_CHECKSUM="460a480cf08fedbf5b38f707f94f20828798327adadb077f80dbab048fd0a07d"

    export CONDUIT_VERSION=${CONDUIT_VERSION:-"v0.7.1"}
    export CONDUIT_FILE=${CONDUIT_FILE:-"conduit-${CONDUIT_VERSION}-src-with-blt.tar.gz"}
    export CONDUIT_COMPATIBILITY_VERSION=${CONDUIT_COMPATIBILITY_VERSION:-"v0.7.1"}
    export CONDUIT_BUILD_DIR=${CONDUIT_BUILD_DIR:-"conduit-${CONDUIT_VERSION}"}
    export CONDUIT_MD5_CHECKSUM="7f9bb79ef0a6bf42fa9bc05af3829e5b"
    export CONDUIT_SHA256_CHECKSUM="460a480cf08fedbf5b38f707f94f20828798327adadb077f80dbab048fd0a07d"

    export BV_LLVM_VERSION=${BV_LLVM_VERSION:-"6.0.1"}
    export BV_LLVM_FILE=${BV_LLVM_FILE:-"llvm-${BV_LLVM_VERSION}.src.tar.xz"}
    export BV_LLVM_URL=${BV_LLVM_URL:-"http://releases.llvm.org/${BV_LLVM_VERSION}/"}
    export BV_LLVM_BUILD_DIR=${BV_LLVM_BUILD_DIR:-"llvm-${BV_LLVM_VERSION}.src"}
    export BV_LLVM_MD5_CHECKSUM="c88c98709300ce2c285391f387fecce0"
    export BV_LLVM_SHA256_CHECKSUM="b6d6c324f9c71494c0ccaf3dac1f16236d970002b42bb24a6c9e1634f7d0f4e2"

    export CMAKE_URL=${CMAKE_URL:-"https://cmake.org/files/v3.18/"}
    export CMAKE_VERSION=${CMAKE_VERSION:-"3.18.2"}
    export CMAKE_FILE=${CMAKE_FILE:-"cmake-${CMAKE_VERSION}.tar.gz"}
    export CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:-"cmake-${CMAKE_VERSION}"}
    export CMAKE_MD5_CHECKSUM="7a882b3764f42981705286ac9daa29c2"
    export CMAKE_SHA256_CHECKSUM="5d4e40fc775d3d828c72e5c45906b4d9b59003c9433ff1b36a1cb552bbd51d7e"

    export VTK_FILE=${VTK_FILE:-"VTK-8.1.0.tar.gz"}
    export VTK_VERSION=${VTK_VERSION:-"8.1.0"}
    export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"8.1"}
    export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
    export VTK_URL=${VTK_URL:-"http://www.vtk.org/files/release/${VTK_SHORT_VERSION}"}
    export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK-8.1.0"}
    export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
    export VTK_MD5_CHECKSUM="4fa5eadbc8723ba0b8d203f05376d932"
    export VTK_SHA256_CHECKSUM="6e269f07b64fb13774f5925161fb4e1f379f4e6a0131c8408c555f6b58ef3cb7"
}

# Ensure the module has been downloaded and extracted properly. Set and check variables here..
function bv_blosc_ensure
{
    echo ""
}

# What other modules does this module depend on. Example "adios" returns string "mxml"
function bv_blosc_depends_on
{
    echo ""
}

# build the module
function bv_blosc_build
{
    echo ""
}

function bv_blosc_print
{
    printf ""
}

function bv_blosc_print_usage
{
    printf ""
}

function bv_blosc_host_profile
{
    printf ""
}

function bv_blosc_dry_run
{
    printf ""
}

function bv_blosc_is_installed
{
    printf ""
}

function bv_blosc_is_enabled
{
    printf ""
}
