function bv_cfitsio_initialize
{
    export DO_CFITSIO="no"
}

function bv_cfitsio_enable
{
    DO_CFITSIO="yes"
}

function bv_cfitsio_disable
{
    DO_CFITSIO="no"
}

function bv_cfitsio_depends_on
{
    echo ""
}

function bv_cfitsio_info
{
    export CFITSIO_FILE=${CFITSIO_FILE:-"cfitsio-4.6.3.tar.gz"}
    export CFITSIO_VERSION=${CFITSIO_VERSION:-"4.6.3"}
    export CFITSIO_COMPATIBILITY_VERSION=${CFITSIO_COMPATIBILITY_VERSION:-"4.0"}
    export CFITSIO_BUILD_DIR=${CFITSIO_BUILD_DIR:-"cfitsio-${CFITSIO_VERSION}"}
    export CFITSIO_SHA256_CHECKSUM="fad44fff274fdda5ffcc0c0fff3bc3c596362722b9292fc8944db91187813600"
}

function bv_cfitsio_print
{
    printf "%s%s\n" "CFITSIO_FILE=" "${CFITSIO_FILE}"
    printf "%s%s\n" "CFITSIO_VERSION=" "${CFITSIO_VERSION}"
    printf "%s%s\n" "CFITSIO_COMPATIBILITY_VERSION=" "${CFITSIO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CFITSIO_BUILD_DIR=" "${CFITSIO_BUILD_DIR}"
}

function bv_cfitsio_print_usage
{
    printf "%-20s %s [%s]\n" "--cfitsio" "Build CFITSIO" "$DO_CFITSIO"
}

function bv_cfitsio_host_profile
{
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CFITSIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR \${VISITHOME}/cfitsio/$CFITSIO_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi

}
function bv_cfitsio_ensure
{
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        ensure_built_or_ready "cfitsio" $CFITSIO_VERSION $CFITSIO_BUILD_DIR $CFITSIO_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CFITSIO="no"
            error "Unable to build CFITSIO.  ${CFITSIO_FILE} not found."
        fi
    fi
}
# *************************************************************************** #
#                         Function 8.9, build_cfitsio                         #
# *************************************************************************** #

function build_cfitsio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CFITSIO_BUILD_DIR $CFITSIO_FILE
    untarred_cfitsio=$?
    if [[ $untarred_cfitsio == -1 ]] ; then
        warn "Unable to prepare CFITSIO Build Directory. Giving Up"
        return 1
    fi

    info "Configuring CFITSIO . . ."
    cd $CFITSIO_BUILD_DIR || error "Can't cd to cfits IO build dir."

    set -x
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        ./configure \
        --prefix="$VISITDIR/cfitsio/$CFITSIO_VERSION/$VISITARCH"
    set +x
    if [[ $? != 0 ]] ; then
        warn "CFITSIO configure failed.  Giving up"
        return 1
    fi

    #
    # Build CFITSIO
    #
    info "Building CFITSIO . . . (~2 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
        warn "CFITSIO build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing CFITSIO . . ."

    mkdir "$VISITDIR/cfitsio"
    mkdir "$VISITDIR/cfitsio/$CFITSIO_VERSION"
    mkdir "$VISITDIR/cfitsio/$CFITSIO_VERSION/$VISITARCH"

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "CFITSIO install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/cfitsio"
        chgrp -R ${GROUP} "$VISITDIR/cfitsio"
    fi
    cd "$START_DIR"
    info "Done with CFITSIO"
    return 0
}

function bv_cfitsio_is_enabled
{
    if [[ $DO_CFITSIO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cfitsio_is_installed
{
    check_if_installed "cfitsio" $CFITSIO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cfitsio_build
{
    cd "$START_DIR"
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        check_if_installed "cfitsio" $CFITSIO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CFITSIO build.  CFITSIO is already installed."
        else
            info "Building CFITSIO (~2 minutes)"
            build_cfitsio
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CFITSIO.  Bailing out."
            fi
            info "Done building CFITSIO"
        fi
    fi
}
