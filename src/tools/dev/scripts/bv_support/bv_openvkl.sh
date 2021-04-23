function bv_openvkl_initialize
{
    export DO_OPENVKL="no"
    export USE_SYSTEM_OPENVKL="no"
    add_extra_commandline_args "openvkl" "alt-openvkl-dir" 1 "Use alternative directory for openvkl"
}

function bv_openvkl_enable
{
    DO_OPENVKL="yes"
}

function bv_openvkl_disable
{
    DO_OPENVKL="no"
}

function bv_openvkl_alt_openvkl_dir
{
    echo "Using alternate openvkl directory"
    bv_openvkl_enable
    USE_SYSTEM_OPENVKL="yes"
    OPENVKL_INSTALL_DIR="$1"
}

function bv_openvkl_depends_on
{
    echo "tbb"
}

function bv_openvkl_initialize_vars
{
    info "initializing openvkl vars"
    if [[ "$DO_OPENVKL" == "yes" ]] ; then
        if [[ "$USE_SYSTEM_OPENVKL" == "no" ]]; then
            OPENVKL_INSTALL_DIR=$VISITDIR/openvkl/$OPENVKL_VERSION/$VISITARCH
        fi
    fi
}

function bv_openvkl_info
{
    export OPENVKL_VERSION=${OPENVKL_VERSION:-"0.12.0"}
    if [[ "$OPSYS" == "Darwin" ]] ; then
        export OPENVKL_FILE=${OPENVKL_FILE:-"openvkl-${OPENVKL_VERSION}.x86_64.macos.zip"}
        export OPENVKL_BINARY_DIR=${OPENVKL_BINARY_DIR:-openvkl-"$OPENVKL_VERSION.x86_64.macos"}
        # these are binary builds, not source tarballs so the mdf5s
        # and shas differ between platforms
#        export OPENVKL_MD5_CHECKSUM="8a3874975f1883d8df1714b3ba3eacba"
#        export OPENVKL_SHA256_CHECKSUM="31cbbe96c6f19bb9c5463e181070bd667d3dbb93e702671e8406ce26be259109"
    else
        export OPENVKL_FILE=${OPENVKL_FILE:-"openvkl-${OPENVKL_VERSION}.x86_64.linux.tar.gz"}
        export OPENVKL_BINARY_DIR=${OPENVKL_BINARY_DIR:-openvkl-"openvkl-$OPENVKL_VERSION.x86_64.linux"}
        # these are binary builds, not source tarballs so the mdf5s
        # and shas differ between platforms
#        export OPENVKL_MD5_CHECKSUM="7a1c3d12e8732cfee7d389f81d008798"
#        export OPENVKL_SHA256_CHECKSUM="7671cc37c4dc4e3da00b2b299b906b35816f058efea92701e7b89574b15e652d"
    fi

    export OPENVKL_COMPATIBILITY_VERSION=${OPENVKL_COMPATIBILITY_VERSION:-"${OPENVKL_VERSION}"}
    export OPENVKL_URL=${OPENVKL_URL:-"https://github.com/openvkl/openvkl/releases/download/v${OPENVKL_VERSION}/"}
}

function bv_openvkl_print
{
    printf "%s%s\n" "OPENVKL_FILE=" "${OPENVKL_FILE}"
    printf "%s%s\n" "OPENVKL_VERSION=" "${OPENVKL_VERSION}"
    printf "%s%s\n" "OPENVKL_COMPATIBILITY_VERSION=" "${OPENVKL_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "OPENVKL_BINARY_DIR=" "${OPENVKL_BINARY_DIR}"
}

function bv_openvkl_host_profile
{
    if [[ "$DO_OPENVKL" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OPENVKL" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_OPENVKL" == "no" ]]; then
            echo "SETUP_APP_VERSION(OPENVKL ${OPENVKL_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(openvkl_DIR \${VISITHOME}/openvkl/${OPENVKL_VERSION}/\${VISITARCH}/lib/cmake/openvkl-${OPENVKL_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OPENVKL_DIR \${VISITHOME}/openvkl/${OPENVKL_VERSION}/\${VISITARCH})" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_OPENVKL_DIR ${OPENVKL_INSTALL_DIR})" >> $HOSTCONF
        fi
    fi
}

function bv_openvkl_print_usage
{
    #openvkl does not have an option, it is only dependent on openvkl.
    printf "%-20s %s [%s]\n" "--openvkl" "Build openvkl" "$DO_OPENVKL"
}

function bv_openvkl_ensure
{
    if [[ "$DO_OPENVKL" == "yes" && "$USE_SYSTEM_OPENVKL" == "no" ]] ; then
        check_installed_or_have_src "openvkl" $OPENVKL_VERSION $OPENVKL_BINARY_DIR $OPENVKL_FILE $OPENVKL_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENVKL="no"
            error "Unable to build openvkl. ${OPENVKL_FILE} not found."
        fi
    elif [[ "$USE_SYSTEM_OPENVKL" == "yes" ]] ; then
        if [[ ! -d $OPENVKL_INSTALL_DIR/include/openvkl3 ]]; then
            error "Unable to find openvkl v3.+ in the alternative path, perhaps a wrong openvkl version is provided."
        fi
    fi
}

function bv_openvkl_dry_run
{
    if [[ "$DO_OPENVKL" == "yes" ]] ; then
        echo "Dry run option not set for openvkl."
    fi
}

# ***************************************************************************
# build_openvkl
#
# Modifications:
#
# ***************************************************************************

function build_openvkl
{
    # Unzip the OPENVKL tarball and copy it to the VisIt installation.
    info "Installing prebuilt openvkl"
    tar zxvf $OPENVKL_FILE
    rm $OPENVKL_BINARY_DIR/lib/libtbb*
    mkdir -p ${OPENVKL_INSTALL_DIR} || error "Cannot create openvkl install directory"
    cp -R $OPENVKL_BINARY_DIR/* ${OPENVKL_INSTALL_DIR} || error "Cannot copy to openvkl install directory"
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "${OPENVKL_INSTALL_DIR}"
        chgrp -R ${GROUP} "${OPENVKL_INSTALL_DIR}"
    fi
    cd "$START_DIR"
    info "Done with openvkl"
    return 0
}

function bv_openvkl_is_enabled
{
    if [[ $DO_OPENVKL == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_openvkl_is_installed
{
    if [[ "$USE_SYSTEM_OPENVKL" == "yes" ]]; then
        return 1
    fi

    check_if_installed "openvkl" $OPENVKL_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_openvkl_build
{
    if [[ "$DO_OPENVKL" == "yes" && "$USE_SYSTEM_OPENVKL" == "no" ]] ; then
        check_if_installed "openvkl" $OPENVKL_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of OpenVKL. OpenVKL is already installed."
        else
            build_openvkl
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OpenVKL. Bailing out."
            fi
            info "Done building OpenVKL"
        fi
    fi
}
