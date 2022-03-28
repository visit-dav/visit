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
    export BLOSC_VERSION=${BLOSC_VERSION:-"2.0.4"}
    export BLOSC_FILE=${BLOSC_FILE:-"c-blosc2-${BLOSC_VERSION}.tar.gz"}
    # the URL is commented out because the filename is different in the blosc release
    # there, the filename is v2.0.4.tar.gz.
    # export BLOSC_URL=${BLOSC_URL:-"https://github.com/Blosc/c-blosc2/archive/refs/tags"}
    export BLOSC_BUILD_DIR=${BLOSC_BUILD_DIR:-"c-blosc2-${BLOSC_VERSION}"}
    export BLOSC_MD5_CHECKSUM="d427fcf68543f971dd7dd22974a47c74"
    export BLOSC_SHA256_CHECKSUM="90c78edcc262759dd16d243141513310624bb4fda3d98ac34dcfb78255e151c1"
}

# Ensure the module has been downloaded and extracted properly. Set and check variables here..
function bv_blosc_ensure
{
    if [[ "$DO_BLOSC" == "yes" ]] ; then
        ensure_built_or_ready "blosc" $BLOSC_VERSION $BLOSC_BUILD_DIR $BLOSC_FILE $BLOSC_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_BLOSC="no"
            error "Unable to build CBlosc2.  ${BLOSC_FILE} not found."
        fi
    fi
}

# What other modules does this module depend on. Example "adios" returns string "mxml"
function bv_blosc_depends_on
{
    echo "cmake"
}

function build_blosc
{
    # TODO there is a chance it might need zlib? need to investigate further
    
    #
    # Blosc uses CMake  -- make sure we have it built.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "Blosc: CMake found"
    else
        build_cmake
        if [[ $? != 0 ]] ; then
            warn "Unable to build cmake.  Giving up"
            return 1
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $BLOSC_BUILD_DIR $BLOSC_FILE
    untarred_blosc=$?
    if [[ $untarred_blosc == -1 ]] ; then
        warn "Unable to prepare Blosc build directory. Giving Up!"
        return 1
    fi

    cd $BLOSC_BUILD_DIR || error "Can't cd to BLOSC source dir."

    cfg_opts="-DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/blosc/${BLOSC_VERSION}/${VISITARCH}"

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${cfg_opts} ../${BLOSC_BUILD_DIR}/src > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "Blosc configure failed.  Giving up"
        return 1
    fi

    #
    # Build Blosc
    #
    info "Building Blosc . . . (~5 minutes)"
    echo "hooray you made it this far woo"
}

# build the module
function bv_blosc_build
{
    cd "$START_DIR"
    if [[ "$DO_BLOSC" == "yes" ]] ; then
        check_if_installed "blosc" $BLOSC_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Blosc build.  Blosc is already installed."
        else
            info "Building Blosc (~5 minutes)"

            build_blosc
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Blosc.  Bailing out."
            fi
            info "Done building Blosc"
        fi
    fi
}

function bv_blosc_print
{
    printf "%s%s\n" "BLOSC_FILE=" "${BLOSC_FILE}"
    printf "%s%s\n" "BLOSC_VERSION=" "${BLOSC_VERSION}"
    printf "%s%s\n" "BLOSC_BUILD_DIR=" "${BLOSC_BUILD_DIR}"
}

function bv_blosc_print_usage
{
    printf "%-20s %s [%s]\n" "--blosc" "Build Blosc" "DO_BLOSC"
}

function bv_blosc_host_profile
{
    if [[ "$DO_BLOSC" == "yes" ]] ; then
        echo "##" >> $HOSTCONF
        echo "## BLOSC" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_BLOSC_DIR \${VISITHOME}/blosc/$BLOSC_VERSION/\${VISITARCH})" \
             >> $HOSTCONF
    fi
}

function bv_blosc_dry_run
{
    if [[ "$DO_BLOSC" == "yes" ]] ; then
        echo "Dry run option not set for blosc."
    fi
}

function bv_blosc_is_installed
{
    check_if_installed "blosc" $BLOSC_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_blosc_is_enabled
{
    if [[ $DO_BLOSC == "yes" ]]; then
        return 1
    fi
    return 0
}
