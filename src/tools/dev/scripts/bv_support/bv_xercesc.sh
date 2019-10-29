function bv_xercesc_initialize
{
    export DO_XERCESC="no"
}

function bv_xercesc_enable
{
    DO_XERCESC="yes"
}

function bv_xercesc_disable
{
    DO_XERCESC="no"
}

function bv_xercesc_depends_on
{
    echo ""
}

function bv_xercesc_info
{
    export XERCESC_FILE=${XERCESC_FILE:-"xerces-c-3.1.2.tar.gz"}
    export XERCESC_VERSION=${XERCESC_VERSION:-"3.1.2"}
    export XERCESC_COMPATIBILITY_VERSION=${XERCESC_COMPATIBILITY_VERSION:-"3.1"}
    export XERCESC_BUILD_DIR=${XERCESC_BUILD_DIR:-"xerces-c-${XERCESC_VERSION}"}
    export XERCESC_URL=${XERCESC_URL:-"http://archive.apache.org/dist/xerces/c/3/sources"}
    export XERCESC_MD5_CHECKSUM="9eb1048939e88d6a7232c67569b23985"
    export XERCESC_SHA256_CHECKSUM="743bd0a029bf8de56a587c270d97031e0099fe2b7142cef03e0da16e282655a0"
}

function bv_xercesc_print
{
    printf "%s%s\n" "XERCESC_FILE=" "${XERCESC_FILE}"
    printf "%s%s\n" "XERCESC_VERSION=" "${XERCESC_VERSION}"
    printf "%s%s\n" "XERCESC_COMPATIBILITY_VERSION=" "${XERCESC_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "XERCESC_BUILD_DIR=" "${XERCESC_BUILD_DIR}"
}

function bv_xercesc_print_usage
{
    printf "%-20s %s [%s]\n" "--xercesc"   "Build XERCESC" "$DO_XERCESC"
}

function bv_xercesc_host_profile
{
    if [[ "$DO_XERCESC" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## XERCESC" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_XERCESC_DIR \${VISITHOME}/xerces-c/$XERCESC_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_xercesc_ensure
{
    if [[ "$DO_XERCESC" == "yes" ]] ; then
        ensure_built_or_ready "xerces-c" $XERCESC_VERSION $XERCESC_BUILD_DIR $XERCESC_FILE $XERCESC_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XERCESC="no"
            error "Unable to build XERCESC.  ${XERCESC_FILE} not found."
        fi
    fi
}

function bv_xercesc_dry_run
{
    if [[ "$DO_XERCESC" == "yes" ]] ; then
        echo "Dry run option not set for Xerces-C"
    fi
}

function build_xercesc
{

    #
    # Prepare build dir
    #
    prepare_build_dir $XERCESC_BUILD_DIR $XERCESC_FILE
    untarred_xc=$?
    if [[ $untarred_xc == -1 ]] ; then
        warn "Unable to prepare Xerces-C build directory. Giving Up!"
        return 1
    fi

    #
    # Call configure
    #
    info "Configuring Xerces-C . . ."
    cd $XERCESC_BUILD_DIR || error "Can't cd to Xerces-C build dir."

    info "env CXX=$CXX_COMPILER CC=$C_COMPILER ./configure \
    --prefix=$VISITDIR/xerces-c/$XERCESC_VERSION/$VISITARCH \
    --disable-threads --disable-network --disable-shared \
    --enable-transcoder-iconv"
    
    env CXX=$CXX_COMPILER CC=$C_COMPILER ./configure \
        --prefix=$VISITDIR/xerces-c/$XERCESC_VERSION/$VISITARCH \
        --disable-threads --disable-network --disable-shared \
        --enable-transcoder-iconv

    if [[ $? != 0 ]] ; then
        warn "Xerces-C configuration failed. Giving up"
        return 1
    fi

    #
    # Build Xerces-C
    #
    info "Building Xerces-C . . . (~10 minute)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Xerces-C build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location
    #
    info "Installing Xerces-C"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Xerces-C install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/xerces-c"
        chgrp -R ${GROUP} "$VISITDIR/xerces-c"
    fi
    cd "$START_DIR"
    return 0
}

function bv_xercesc_is_enabled
{
    if [[ $DO_XERCESC == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_xercesc_is_installed
{
    check_if_installed "xerces-c" $XERCESC_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_xercesc_build
{

    if [[ "$DO_XERCESC" == "yes" ]] ; then
        check_if_installed "xerces-c" $XERCESC_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Xerces-C build.  Xerces-C is already installed."
        else
            build_xercesc
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Xerces-C.  Bailing out."
            fi
            info "Done building Xerces-C"
        fi
    fi
}
