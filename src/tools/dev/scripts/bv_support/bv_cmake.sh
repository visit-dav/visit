function bv_cmake_initialize
{
    export DO_CMAKE="yes"
    export FORCE_CMAKE="no"
    export USE_SYSTEM_CMAKE="no"
    add_extra_commandline_args "cmake" "system-cmake" 0 "Use cmake found on system"
    add_extra_commandline_args "cmake" "alt-cmake-dir" 1 "Use cmake found in alternative directory"
    add_extra_commandline_args "cmake" "bin-cmake-dir" 1 "Use cmake found in alternative binary directory"
}

function bv_cmake_enable
{
    DO_CMAKE="yes"
    FORCE_CMAKE="yes"
}

function bv_cmake_disable
{
    DO_CMAKE="no"
    FORCE_CMAKE="no"
}

function bv_cmake_depends_on
{
    echo ""
}

function bv_cmake_force
{
    if [[ "$FORCE_CMAKE" == "yes" ]]; then
        return 0;
    fi
    return 1;
}

function cmake_set_vars_helper
{
    CMAKE_VERSION=`"${CMAKE_COMMAND}" --version`
    CMAKE_VERSION=${CMAKE_VERSION/cmake version }
    CMAKE_BUILD_DIR=`"${CMAKE_COMMAND}" --system-information 2>& 1 | grep _CMAKE_INSTALL_DIR | grep -v _CMAKE_INSTALL_DIR:INTERNAL | sed -e s/\"//g -e s/_CMAKE_INSTALL_DIR//g`
    CMAKE_BUILD_DIR=`echo $CMAKE_BUILD_DIR`
    CMAKE_INSTALL="$CMAKE_BUILD_DIR/bin"
    CMAKE_ROOT=`"$CMAKE_COMMAND" --system-information 2>&1 | grep CMAKE_ROOT | grep -v CMAKE_ROOT:INTERNAL | sed -e s/\"//g -e s/CMAKE_ROOT//g` 
    CMAKE_ROOT=`echo "$CMAKE_ROOT"`
    CMAKE_ROOT=`echo $CMAKE_ROOT`

    echo "version: $CMAKE_VERSION build: $CMAKE_BUILD_DIR bin: $CMAKE_INSTALL root: $CMAKE_ROOT"
}

function bv_cmake_system_cmake
{
    echo "using system cmake"

    TEST=`which cmake`
    [ $? != 0 ] && error "System CMake not found"
    
    bv_cmake_enable
    
    USE_SYSTEM_CMAKE="yes"
    
    CMAKE_COMMAND="cmake"
    CMAKE_FILE=""
    cmake_set_vars_helper #set vars..
}

function bv_cmake_alt_cmake_dir
{
    CMAKE_ALT_DIR="$1"
    echo "Using cmake from alternative directory $1"

    [ ! -e "$CMAKE_ALT_DIR/bin/cmake" ] && error "cmake was not found in directory: $1/bin"

    bv_cmake_enable
    USE_SYSTEM_CMAKE="yes"

    CMAKE_COMMAND="$CMAKE_ALT_DIR/bin/cmake"
    CMAKE_FILE=""
    cmake_set_vars_helper #set vars..
}

function bv_cmake_bin_cmake_dir
{
    CMAKE_BIN_DIR="$1"
    echo "Using cmake from bin directory $1"

    [ ! -e "$CMAKE_BIN_DIR/cmake" ] && error "cmake was not found in directory: $1/"

    bv_cmake_enable
    USE_SYSTEM_CMAKE="yes"

    CMAKE_COMMAND="$CMAKE_BIN_DIR/cmake"
    CMAKE_FILE=""
    cmake_set_vars_helper #set vars..
}


function bv_cmake_info
{
    export CMAKE_URL=${CMAKE_URL:-"https://cmake.org/files/v3.24/"}
    export CMAKE_VERSION=${CMAKE_VERSION:-"3.24.3"}
    export CMAKE_FILE=${CMAKE_FILE:-"cmake-${CMAKE_VERSION}.tar.gz"}
    export CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:-"cmake-${CMAKE_VERSION}"}
    export CMAKE_SHA256_CHECKSUM="b53aa10fa82bff84ccdb59065927b72d3bee49f4d86261249fc0984b3b367291"
}

function bv_cmake_print
{
    printf "%s%s\n" "CMAKE_FILE=" "${CMAKE_FILE}"
    printf "%s%s\n" "CMAKE_VERSION=" "${CMAKE_VERSION}"
    printf "%s%s\n" "CMAKE_BUILD_DIR=" "${CMAKE_BUILD_DIR}"
}

function bv_cmake_print_usage
{
    printf "%-20s %s\n" "--cmake" "Build CMake"
    printf "%-20s %s [%s]\n" "--system-cmake"  "Use the system installed CMake"
    printf "%-20s %s [%s]\n" "--alt-cmake-dir" "Use CMake from an alternative directory"
    printf "%-20s %s [%s]\n" "--bin-cmake-dir" "Use CMake from an alternative binary directory"
}

function bv_cmake_host_profile
{
    #nothing to be done for cmake in cmake host profile..
    echo "##" >> $HOSTCONF
}

function bv_cmake_initialize_vars
{
    if [[ "$USE_SYSTEM_CMAKE" != "yes" ]]; then 
        if [[ "$DO_CMAKE" == "yes" || "$DO_VTK" == "yes" ]] ; then
            #initialize variables where cmake should exist..
            CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/${VISITARCH}/bin"}
            CMAKE_ROOT=${CMAKE_ROOT:-"$VISITDIR/cmake/${CMAKE_VERSION}/${VISITARCH}/share/cmake-${CMAKE_VERSION%.*}"}
            CMAKE_COMMAND="${CMAKE_INSTALL}/cmake"
        fi
    fi

}

function bv_cmake_ensure
{
    if [[ "$USE_SYSTEM_CMAKE" != "yes" ]]; then 
        if [[ "$DO_CMAKE" == "yes" || "$DO_VTK" == "yes" ]] ; then
            ensure_built_or_ready "cmake"  $CMAKE_VERSION  $CMAKE_BUILD_DIR  $CMAKE_FILE $CMAKE_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

# *************************************************************************** #
#                          Function 5, build_cmake                            #
# *************************************************************************** #

function build_cmake
{
    #
    # Prepare cmake build directory
    #
    prepare_build_dir $CMAKE_BUILD_DIR $CMAKE_FILE
    untarred_cmake=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_cmake == -1 ]] ; then
        warn "Unable to prepare CMake build directory. Giving Up!"
        return 1
    fi

    #
    # CMake's logic to test for C++11 constructs like unique_ptr is broken. It fails anytime 'warning'
    # appears on stderr. But, 'warning' can appear on stderr for all sorts of reasons some of which
    # are totally unrelated to the test. One example is file system clock skew...which was happening
    # on LANL's Crossroads. I don't see any benefit in confirming these C++11 constructs now that it
    # is 2024. We just entirely disable these tests by setting these bootstrap flags.
    #
    CMAKE_BOOTSTRAP_FLAGS="-- -DCMake_HAVE_CXX_MAKE_UNIQUE=1 -DCMake_HAVE_CXX_UNIQUE_PTR=1 CMake_HAVE_CXX_UNIQUE_PTR=1"

    #
    # Issue "bootstrap", which takes the place of configure for CMake.
    #
    info "Bootstrapping CMake . . ."
    cd $CMAKE_BUILD_DIR || error "Can't cd to CMake build dir."
    if [[ "$OPSYS" == "AIX" ]]; then
        env CXX=xlC CC=xlc CXXFLAGS="" CFLAGS="" ./bootstrap --prefix="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH"
    elif [[ "$OPSYS" == "Linux" && "$C_COMPILER" == "xlc" ]]; then
        env CXX=xlC CC=xlc CXXFLAGS="" CFLAGS="" ./bootstrap --prefix="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH"
    else
        env CC=${C_COMPILER} CXX=${CXX_COMPILER} CXXFLAGS="" CFLAGS="" ./bootstrap --prefix="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH --parallel=8 ${CMAKE_BOOTSTRAP_FLAGS}"
    fi
    if [[ $? != 0 ]] ; then
        warn "Bootstrap for cmake failed, giving up."
        return 1
    fi

    #
    # Build the CMake program.
    #
    info "Building CMake . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Cannot build cmake, giving up."
        return 1
    fi

    info "Installing CMake . . ."
    $MAKE install
    info "Successfully built CMake"
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/cmake"
        chgrp -R ${GROUP} "$VISITDIR/cmake"
    fi
    cd "$START_DIR"
    info "Done with CMake"
}

function bv_cmake_is_enabled
{
    if [[ $DO_CMAKE == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cmake_is_installed
{
    if [[ "$USE_SYSTEM_CMAKE" == "yes" ]]; then
        return 1
    fi

    check_if_installed "cmake" $CMAKE_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cmake_build
{
    #
    # Build CMake
    #
    cd "$START_DIR"
    if [[ "$DO_CMAKE" == "yes" && "$USE_SYSTEM_CMAKE" == "no" ]]; then
        check_if_installed "cmake" $CMAKE_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CMake build.  CMake is already installed."
        else
            info "Building CMake (~5 minutes)"
            build_cmake
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CMake.  Bailing out."
            fi
            info "Done building CMake"
        fi
    fi
}
