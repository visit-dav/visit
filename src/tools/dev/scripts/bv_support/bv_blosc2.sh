# by default, turn blosc2 off
function bv_blosc2_initialize
{
    export DO_BLOSC2="no"
}

# Enables the module
function bv_blosc2_enable
{
    DO_BLOSC2="yes"
}

# Disables the module
function bv_blosc2_disable
{
    DO_BLOSC2="no"
}

# Where to get the module, the version, etc...
function bv_blosc2_info
{
    BLOSC2_DIR="${VISITDIR}/blosc2/${BLOSC2_VERSION}/${VISITARCH}"
    # note this is c-blosc2, NOT c-blosc
    export BLOSC2_VERSION=${BLOSC2_VERSION:-"2.11.3"}
    export BLOSC2_FILE=${BLOSC2_FILE:-"c-blosc2-${BLOSC2_VERSION}.tar.gz"}
    # the URL is commented out because the filename is different in the blosc2 release
    # there, the filename is v2.11.3.tar.gz.
    # export BLOSC2_URL=${BLOSC2_URL:-"https://github.com/Blosc/c-blosc2/releases/tag"}
    # to use that download you'd need to rename the file once downloaded.
    export BLOSC2_BUILD_DIR=${BLOSC2_BUILD_DIR:-"c-blosc2-${BLOSC2_VERSION}"}
    export BLOSC2_SHA256_CHECKSUM="7273ec3ab42adc247425ab34b0601db86a6e2a6aa1a97a11e29df02e078f5037"
}

# Ensure the module has been downloaded and extracted properly.
function bv_blosc2_ensure
{
    if [[ "$DO_BLOSC2" == "yes" ]] ; then
        ensure_built_or_ready "blosc2" $BLOSC2_VERSION $BLOSC2_BUILD_DIR $BLOSC2_FILE $BLOSC2_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_BLOSC2="no"
            error "Unable to build c-blosc2.  ${BLOSC2_FILE} not found."
        fi
    fi
}

# What other modules does c-blosc2 depend on.
function bv_blosc2_depends_on
{
    echo "cmake"
}

function build_blosc2
{
    #
    # Blosc2 uses CMake  -- make sure we have it built.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "Blosc2: CMake found"
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
    prepare_build_dir $BLOSC2_BUILD_DIR $BLOSC2_FILE
    untarred_blosc2=$?
    if [[ $untarred_blosc2 == -1 ]] ; then
        warn "Unable to prepare Blosc2 build directory. Giving Up!"
        return 1
    fi

    cd $BLOSC2_BUILD_DIR || error "Can't cd to BLOSC2 source dir."

    cfg_opts="-DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/blosc2/${BLOSC2_VERSION}/${VISITARCH}"

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${cfg_opts} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "Blosc2 configure failed.  Giving up"
        return 1
    fi

    #
    # Build Blosc2
    #
    info "Building Blosc2 . . . (~5 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Blosc2 build failed.  Giving up"
        return 1
    fi
    
    #
    # Install into the VisIt third party location.
    #
    info "Installing Blosc2"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Blosc2 install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/blosc2"
        chgrp -R ${GROUP} "$VISITDIR/blosc2"
    fi

    cd "$START_DIR"
    info "Done with Blosc2"
    return 0
}

# build the module
function bv_blosc2_build
{
    cd "$START_DIR"
    if [[ "$DO_BLOSC2" == "yes" ]] ; then
        check_if_installed "blosc2" $BLOSC2_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Blosc2 build.  Blosc2 is already installed."
        else
            info "Building Blosc2 (~5 minutes)"

            build_blosc2
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Blosc2.  Bailing out."
            fi
            info "Done building Blosc2"
        fi
    fi
}

function bv_blosc2_print
{
    printf "%s%s\n" "BLOSC2_FILE=" "${BLOSC2_FILE}"
    printf "%s%s\n" "BLOSC2_VERSION=" "${BLOSC2_VERSION}"
    printf "%s%s\n" "BLOSC2_BUILD_DIR=" "${BLOSC2_BUILD_DIR}"
}

function bv_blosc2_print_usage
{
    printf "%-20s %s [%s]\n" "--blosc2" "Build Blosc2" "DO_BLOSC2"
}

function bv_blosc2_host_profile
{
    if [[ "$DO_BLOSC2" == "yes" ]] ; then
        echo "##" >> $HOSTCONF
        echo "## BLOSC2" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_BLOSC2_DIR \${VISITHOME}/blosc2/$BLOSC2_VERSION/\${VISITARCH})" \
             >> $HOSTCONF
    fi
}

function bv_blosc2_dry_run
{
    if [[ "$DO_BLOSC2" == "yes" ]] ; then
        echo "Dry run option not set for blosc2."
    fi
}

function bv_blosc2_is_installed
{
    check_if_installed "blosc2" $BLOSC2_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_blosc2_is_enabled
{
    if [[ $DO_BLOSC2 == "yes" ]]; then
        return 1
    fi
    return 0
}
