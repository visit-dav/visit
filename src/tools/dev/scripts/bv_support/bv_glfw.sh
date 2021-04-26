function bv_glfw_initialize
{
    export DO_GLFW="no"
    export USE_SYSTEM_GLFW="no"
    add_extra_commandline_args "glfw" "alt-glfw-dir" 1 "Use alternative directory for GLFW"
}

function bv_glfw_enable
{
    DO_GLFW="yes"
}

function bv_glfw_disable
{
    DO_GLFW="no"
}

function bv_glfw_alt_glfw_dir
{
    echo "Using alternate GLFW directory"
    bv_glfw_enable
    USE_SYSTEM_GLFW="yes"
    GLFW_INSTALL_DIR="$1"
}

function bv_glfw_depends_on
{
    echo "tbb"
}

function bv_glfw_initialize_vars
{
    info "initializing GLFW vars"
    if [[ "$DO_GLFW" == "yes" ]] ; then
        if [[ "$USE_SYSTEM_GLFW" == "no" ]]; then
            GLFW_INSTALL_DIR=$VISITDIR/glfw/$GLFW_VERSION/$VISITARCH
        fi
    fi
}

function bv_glfw_info
{
    export GLFW_VERSION=${GLFW_VERSION:-"3.3.3"}
    if [[ "$OPSYS" == "Darwin" ]] ; then
        export GLFW_FILE=${GLFW_FILE:-"glfw-${GLFW_VERSION}.bin.MACOS.zip"}
        export GLFW_BINARY_DIR=${GLFW_BINARY_DIR:-glfw-"$GLFW_VERSION.bin.MACOS"}
        # these are binary builds, not source tarballs so the mdf5s
        # and shas differ between platforms
#        export GLFW_MD5_CHECKSUM="8a3874975f1883d8df1714b3ba3eacba"
#        export GLFW_SHA256_CHECKSUM="31cbbe96c6f19bb9c5463e181070bd667d3dbb93e702671e8406ce26be259109"
    else
        export GLFW_FILE=${GLFW_FILE:-"glfw-${GLFW_VERSION}.x86_64.linux.tar.gz"}
        export GLFW_BINARY_DIR=${GLFW_BINARY_DIR:-glfw-"glfw-$GLFW_VERSION.x86_64.linux"}
        # these are binary builds, not source tarballs so the mdf5s
        # and shas differ between platforms
#        export GLFW_MD5_CHECKSUM="7a1c3d12e8732cfee7d389f81d008798"
#        export GLFW_SHA256_CHECKSUM="7671cc37c4dc4e3da00b2b299b906b35816f058efea92701e7b89574b15e652d"
    fi

    export GLFW_COMPATIBILITY_VERSION=${GLFW_COMPATIBILITY_VERSION:-"${GLFW_VERSION}"}
    export GLFW_URL=${GLFW_URL:-"https://github.com/glfw/glfw/releases/download/v${GLFW_VERSION}/"}
}

function bv_glfw_print
{
    printf "%s%s\n" "GLFW_FILE=" "${GLFW_FILE}"
    printf "%s%s\n" "GLFW_VERSION=" "${GLFW_VERSION}"
    printf "%s%s\n" "GLFW_COMPATIBILITY_VERSION=" "${GLFW_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "GLFW_BINARY_DIR=" "${GLFW_BINARY_DIR}"
}

function bv_glfw_host_profile
{
    if [[ "$DO_GLFW" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## GLFW" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_GLFW" == "no" ]]; then
            echo "SETUP_APP_VERSION(GLFW ${GLFW_VERSION})" >> $HOSTCONF
        fi

        echo "VISIT_OPTION_DEFAULT(VISIT_GLFW_DIR ${GLFW_INSTALL_DIR})" >> $HOSTCONF
    fi
}

function bv_glfw_print_usage
{
    #glfw does not have an option, it is only dependent on GLFW.
    printf "%-20s %s [%s]\n" "--glfw" "Build GLFW" "$DO_GLFW"
}

function bv_glfw_ensure
{
    if [[ "$DO_GLFW" == "yes" && "$USE_SYSTEM_GLFW" == "no" ]] ; then
        check_installed_or_have_src "glfw" $GLFW_VERSION $GLFW_BINARY_DIR $GLFW_FILE $GLFW_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_GLFW="no"
            error "Unable to build GLFW.  ${GLFW_FILE} not found."
        fi
    elif [[ "$USE_SYSTEM_GLFW" == "yes" ]] ; then
        if [[ ! -d $GLFW_INSTALL_DIR/include/glfw3 ]]; then
            error "Unable to find GLFW v3.+ in the alternative path, perhaps a wrong GLFW version is provided."
        fi
    fi
}

function bv_glfw_dry_run
{
    if [[ "$DO_GLFW" == "yes" ]] ; then
        echo "Dry run option not set for GLFW."
    fi
}

function bv_glfw_is_enabled
{
    if [[ $DO_GLFW == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_glfw_is_installed
{
    if [[ "$USE_SYSTEM_GLFW" == "yes" ]]; then   
        return 1
    fi

    check_if_installed "glfw" $GLFW_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_glfw_build
{
    if [[ "$DO_GLFW" == "yes" && "$USE_SYSTEM_GLFW" == "no" ]] ; then
        check_if_installed "glfw" $GLFW_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of GLFW"
        else
            build_glfw
            if [[ $? != 0 ]] ; then
                error "Unable to build or install GLFW.  Bailing out."
            fi
            info "Done building GLFW"
        fi
    fi
}

# ***************************************************************************
# build_glfw
# ***************************************************************************
function build_glfw
{
    #
    # Uncompress the source file
    #
    uncompress_src_file $GLFW_BINARY_DIR $GLFW_FILE
    untarred_glfw=$?
    if [[ $untarred_glfw == -1 ]] ; then
        warn "Unable to uncompress GLFW source file. Giving Up!"
        return 1
    fi

    # Remove the tbb library as it should installed via TBB.
    rm $GLFW_BINARY_DIR/lib/libtbb*

    #
    # Install into the VisIt third party location.
    #
    info "Installing GLFW . . ."
    mkdir -p ${GLFW_INSTALL_DIR}
    if [[ $? != 0 ]] ; then
        warn "Cannot create GLFW install directory"
        return 1
    fi

    cp -R $GLFW_BINARY_DIR/* ${GLFW_INSTALL_DIR}
    if [[ $? != 0 ]] ; then
        warn "GLFW install failed. Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/glfw"
        chgrp -R ${GROUP} "$VISITDIR/glfw"
    fi

    cd "$START_DIR"
    info "Done with GLFW"
    return 0
}
