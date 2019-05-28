function bv_zlib_initialize
{
    export DO_ZLIB="yes"
}

function bv_zlib_enable
{
    DO_ZLIB="yes"
}

function bv_zlib_disable
{
    DO_ZLIB="no"
}

function bv_zlib_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_zlib_info
{
    export ZLIB_VERSION=${ZLIB_VERSION:-"1.2.8"}
    export ZLIB_FILE=${ZLIB_FILE:-"zlib-${ZLIB_VERSION}.tar.gz"}
    export ZLIB_COMPATIBILITY_VERSION=${ZLIB_COMPATIBILITY_VERSION:-"1.2"}
    export ZLIB_URL=${ZLIB_URL:-https://wci.llnl.gov/codes/zlib/zlib-${ZLIB_VERSION}}
    export ZLIB_BUILD_DIR=${ZLIB_BUILD_DIR:-"zlib-${ZLIB_VERSION}"}
    export ZLIB_MD5_CHECKSUM="44d667c142d7cda120332623eab69f40"
    export ZLIB_SHA256_CHECKSUM="36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d"
}

function bv_zlib_print
{
    printf "%s%s\n" "ZLIB_FILE=" "${ZLIB_FILE}"
    printf "%s%s\n" "ZLIB_VERSION=" "${ZLIB_VERSION}"
    printf "%s%s\n" "ZLIB_COMPATIBILITY_VERSION=" "${ZLIB_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ZLIB_BUILD_DIR=" "${ZLIB_BUILD_DIR}"
}

function bv_zlib_print_usage
{
    printf "%-20s %s [%s]\n" "--zlib" "Build ZLIB support" "$DO_ZLIB"
}

function bv_zlib_host_profile
{
    echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## ZLIB" >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo \
        "VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR \${VISITHOME}/zlib/$ZLIB_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
}

function bv_zlib_initialize_vars
{
    export VISIT_ZLIB_DIR=${VISIT_ZLIB_DIR:-"$VISITDIR/zlib/${ZLIB_VERSION}/${VISITARCH}"}
    export ZLIB_LIBRARY_DIR="${VISIT_ZLIB_DIR}/lib"
    export ZLIB_INCLUDE_DIR="${VISIT_ZLIB_DIR}/include"
    export ZLIB_LIBRARY="${ZLIB_LIBRARY_DIR}/libz.${SO_EXT}"
}

function bv_zlib_ensure
{
    ensure_built_or_ready "zlib" $ZLIB_VERSION $ZLIB_BUILD_DIR $ZLIB_FILE $ZLIB_URL
    if [[ $? != 0 ]] ; then
        ANY_ERRORS="yes"
        error "Unable to build ZLIB.  ${ZLIB_FILE} not found."
    fi
}

function bv_zlib_dry_run
{
    echo "Dry run option not set for zlib."
}

# *************************************************************************** #
#                            Function 8, build_zlib
#
# Modfications:
#
# *************************************************************************** #

function build_zlib
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ZLIB_BUILD_DIR $ZLIB_FILE
    untarred_zlib=$?
    if [[ $untarred_zlib == -1 ]] ; then
        warn "Unable to prepare ZLIB build directory. Giving Up!"
        return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring ZLIB . . ."
    cd $ZLIB_BUILD_DIR || error "Can't cd to ZLIB build dir."
    info "Invoking command to configure ZLIB"

    STATICARGS="--static"
    if [[ "$DO_STATIC_BUILD" == "no" ]] ; then
        STATICARGS=""
    fi

    info "env CXX=$CXX_COMPILER CC=$C_COMPILER ./configure \
        --prefix=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH $STATICARGS"

    # Call configure
    env CXX=$CXX_COMPILER CC=$C_COMPILER ./configure \
        --prefix=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH $STATICARGS

    if [[ $? != 0 ]] ; then
        warn "ZLIB configure failed.  Giving up"
        return 1
    fi

    #
    # Build ZLIB
    #
    info "Building ZLIB . . . (~1 minute)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "ZLIB build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing ZLIB"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "ZLIB install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/zlib"
        chgrp -R ${GROUP} "$VISITDIR/zlib"
    fi
    cd "$START_DIR"
    info "Done with ZLIB"
    return 0
}

function bv_zlib_is_enabled
{
    if [[ $DO_ZLIB == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_zlib_is_installed
{
    check_if_installed "zlib" $ZLIB_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_zlib_build
{
    cd "$START_DIR"
    check_if_installed "zlib" $ZLIB_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping ZLIB build.  ZLIB is already installed."
    else
        info "Building ZLIB (~1 minute)"
        build_zlib
        if [[ $? != 0 ]] ; then
            error "Unable to build or install ZLIB.  Bailing out."
        fi
        info "Done building ZLIB"
    fi
}
