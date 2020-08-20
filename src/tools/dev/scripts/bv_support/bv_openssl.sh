function bv_openssl_initialize
{
    export DO_OPENSSL="yes"
}

function bv_openssl_enable
{
    DO_OPENSSL="yes"
}

function bv_openssl_disable
{
    DO_OPENSSL="no"
}

function bv_openssl_depends_on
{
    echo "zlib"

}

function bv_openssl_info
{
    export OPENSSL_VERSION=${OPENSSL_VERSION:-"1.0.2j"}
    export OPENSSL_FILE=${OPENSSL_FILE:-"openssl-${OPENSSL_VERSION}.tar.gz"}
    export OPENSSL_URL=${OPENSSL_URL:-"https://www.openssl.org/source/"}
    export OPENSSL_BUILD_DIR=${OPENSSL_BUILD_DIR:-"openssl-${OPENSSL_VERSION}"}
    export OPENSSL_MD5_CHECKSUM="96322138f0b69e61b7212bc53d5e912b"
    export OPENSSL_SHA256_CHECKSUM="e7aff292be21c259c6af26469c7a9b3ba26e9abaaffd325e3dccc9785256c431"
}

function bv_openssl_print
{
    printf "%s%s\n" "OPENSSL_FILE=" "${OPENSSL_FILE}"
    printf "%s%s\n" "OPENSSL_VERSION=" "${OPENSSL_VERSION}"
    printf "%s%s\n" "OPENSSL_BUILD_DIR=" "${OPENSSL_BUILD_DIR}"
}

function bv_openssl_print_usage
{
    printf "%-20s %s [%s]\n" "--openssl" "Build openssl support" "$DO_OPENSSL"
}

function bv_openssl_host_profile
{
    if [[ "$DO_OPENSSL" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OPENSSL " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_OPENSSL_DIR \${VISITHOME}/openssl/$OPENSSL_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_openssl_ensure
{
    if [[ "$DO_OPENSSL" == "yes" ]] ; then
        ensure_built_or_ready "openssl" $OPENSSL_VERSION $OPENSSL_BUILD_DIR $OPENSSL_FILE $OPENSSL_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENSSL="no"
            error "Unable to build openssl.  ${OPENSSL_FILE} not found."
        fi
    fi
}

function bv_openssl_dry_run
{
    if [[ "$DO_OPENSSL" == "yes" ]] ; then
        echo "Dry run option not set for openssl."
    fi
}

# *************************************************************************** #
#                            Function 8, build_openssl
# *************************************************************************** #
function build_openssl
{
    #
    # Prepare build dir
    #
    prepare_build_dir $OPENSSL_BUILD_DIR $OPENSSL_FILE
    untarred_openssl=$?
    if [[ $untarred_openssl == -1 ]] ; then
        warn "Unable to prepare openssl build directory. Giving Up!"
        return 1
    fi

    cd $OPENSSL_BUILD_DIR || error "Can't cd to openssl build dir."

    #
    # Call configure
    #
    info "Configuring openssl . . ."

    WITH_ZLIB_LIB="--with-zlib-lib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib"
    WITH_ZLIB_INCLUDE="--with-zlib-include=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/include"

    info env CC="$C_COMPILER $CFLAGS $C_OPT_FLAGS" KERNEL_BITS=64 ./config \
            $WITH_ZLIB_LIB $WITH_ZLIB_INCLUDE \
            --prefix="$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/" \
            --openssldir="$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/etc/openssl" 
    env CC="$C_COMPILER $CFLAGS $C_OPT_FLAGS" KERNEL_BITS=64 ./config \
            $WITH_ZLIB_LIB $WITH_ZLIB_INCLUDE \
            --prefix="$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/" \
            --openssldir="$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/etc/openssl" 

    #
    # Build openssl
    #
    info "Building openssl . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "openssl build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing openssl"
    $MAKE install 

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/openssl"
        chgrp -R ${GROUP} "$VISITDIR/openssl"
    fi
    cd "$START_DIR"
    info "Done with openssl"
    return 0
}


function bv_openssl_is_enabled
{
    if [[ $DO_OPENSSL == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_openssl_is_installed
{
    check_if_installed "openssl" $OPENSSL_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_openssl_build
{
    cd "$START_DIR"
    if [[ "$DO_OPENSSL" == "yes" ]] ; then
        check_if_installed "openssl" $OPENSSL_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping openssl build.  openssl is already installed."
        else
            info "Building openssl (~2 minutes)"
            build_openssl
            if [[ $? != 0 ]] ; then
                error "Unable to build or install openssl.  Bailing out."
            fi
            info "Done building openssl"
        fi
    fi
}
