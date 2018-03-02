function bv_openswr_initialize
{
    export DO_OPENSWR="no"
}

function bv_openswr_enable
{
    DO_OPENSWR="yes"
}

function bv_openswr_disable
{
    DO_OPENSWR="no"
}

function bv_openswr_depends_on
{
    depends_on="llvm"

    echo ${depends_on}
}

function bv_openswr_info
{
    export OPENSWR_FILE=${OPENSWR_FILE:-"mesa-17.0.6.tar.gz"}
    export OPENSWR_VERSION=${OPENSWR_VERSION:-"17.0.6"}
    export OPENSWR_BUILD_DIR=${OPENSWR_BUILD_DIR:-"mesa-17.0.6"}
    export OPENSWR_MD5_CHECKSUM="05a111fad5297025e5e47c1d6f8ab79f"
    export OPENSWR_SHA256_CHECKSUM=""
}

function bv_openswr_print
{
    printf "%s%s\n" "OPENSWR_FILE=" "${OPENSWR_FILE}"
    printf "%s%s\n" "OPENSWR_VERSION=" "${OPENSWR_VERSION}"
    printf "%s%s\n" "OPENSWR_TARGET=" "${OPENSWR_TARGET}"
    printf "%s%s\n" "OPENSWR_BUILD_DIR=" "${OPENSWR_BUILD_DIR}"
}

function bv_openswr_print_usage
{
    printf "%-20s %s [%s]\n" "--openswr" "Build OpenSWR" "$DO_OPENSWR"
}

function bv_openswr_host_profile
{
    # If we are using openswr as the GL for VTK in a static build, we'll tell VisIt
    # about openswr using a different mechanism.
    addhp="yes"
    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
            addhp="no"
        fi
    fi

    if [[ "$DO_OPENSWR" == "yes" && "$addhp" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OpenSWR" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OPENSWR_DIR \${VISITHOME}/openswr/$OPENSWR_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_openswr_selected
{
    args=$@
    if [[ $args == "--openswr" ]]; then
        DO_OPENSWR="yes"
        return 1
    fi

    return 0
}

function bv_openswr_initialize_vars
{
    info "initalizing openswr vars"
    if [[ "$DO_OPENSWR" == "yes" ]]; then
        OPENSWR_INSTALL_DIR="${VISITDIR}/openswr/${OPENSWR_VERSION}/${VISITARCH}"
        OPENSWR_INCLUDE_DIR="${OPENSWR_INSTALL_DIR}/include"
        OPENSWR_LIB_DIR="${OPENSWR_INSTALL_DIR}/lib"
        if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            OPENSWR_LIB="${OPENSWR_LIB_DIR}/libOSMesa.a"
        else
            OPENSWR_LIB="${OPENSWR_LIB_DIR}/libOSMesa.${SO_EXT}"
        fi
    fi
}

function bv_openswr_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_OPENSWR" == "yes" ]] ; then
            ensure_built_or_ready "openswr"   $OPENSWR_VERSION   $OPENSWR_BUILD_DIR   $OPENSWR_FILE
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function bv_openswr_dry_run
{
    if [[ "$DO_OPENSWR" == "yes" ]] ; then
        echo "Dry run option not set for openswr."
    fi
}

function build_openswr
{
    #
    # prepare build dir
    #
    prepare_build_dir $OPENSWR_BUILD_DIR $OPENSWR_FILE
    untarred_openswr=$?
    if [[ $untarred_openswr == -1 ]] ; then
        warn "Unable to prepare OpenSWR build directory. Giving Up!"
        return 1
    fi

    #
    # Build OPENSWR.
    #
    cd $OPENSWR_BUILD_DIR || error "Couldn't cd to openswr build dir."

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        OPENSWR_STATIC_DYNAMIC="--disable-shared --disable-shared-glapi --enable-static --enable-static-glapi"
    fi

    info "Configuring OpenSWR . . ."
    LLVM_INSTALL=${VISIT_LLVM_DIR}/bin
    export PATH=${LLVM_INSTALL}:${PATH}
    echo env LDFLAGS="-Wl,-rpath=${VISIT_LLVM_DIR}/lib" \
        CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        CXX=${CXX_COMPILER} \
        CFLAGS="${CFLAGS} ${C_OPT_FLAGS}" \
        CC=${C_COMPILER} \
        ./autogen.sh \
        --prefix=${VISITDIR}/openswr/${OPENSWR_VERSION}/${VISITARCH} \
        --disable-dri \
        --disable-egl \
        --disable-gbm \
        --disable-gles1 \
        --disable-gles2 \
        --disable-xvmc \
        --disable-vdpau \
        --disable-omx \
        --disable-va \
        --disable-glx \
        --with-gallium-drivers=swrast,swr \
        --enable-gallium-osmesa $OPENSWR_STATIC_DYNAMIC
    env LDFLAGS="-Wl,-rpath=${VISIT_LLVM_DIR}/lib" \
        CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        CXX=${CXX_COMPILER} \
        CFLAGS="${CFLAGS} ${C_OPT_FLAGS}" \
        CC=${C_COMPILER} \
        ./autogen.sh \
        --prefix=${VISITDIR}/openswr/${OPENSWR_VERSION}/${VISITARCH} \
        --disable-dri \
        --disable-egl \
        --disable-gbm \
        --disable-gles1 \
        --disable-gles2 \
        --disable-xvmc \
        --disable-vdpau \
        --disable-omx \
        --disable-va \
        --disable-glx \
        --with-gallium-drivers=swrast,swr \
        --enable-gallium-osmesa $OPENSWR_STATIC_DYNAMIC

    if [[ $? != 0 ]] ; then
        warn "OpenSWR configure failed.  Giving up"
        return 1
    fi

    info "Building OpenSWR . . ."
    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "OpenSWR build failed.  Giving up"
        return 1
    fi

    info "Installing OpenSWR ..."
    ${MAKE} ${MAKE_OPT_FLAGS} install
    if [[ $? != 0 ]] ; then
        warn "OpenSWR install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/openswr"
        chgrp -R ${GROUP} "$VISITDIR/openswr"
    fi
    cd "$START_DIR"
    info "Done with OpenSWR"
    return 0
}

function bv_openswr_is_enabled
{
    if [[ $DO_OPENSWR == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_openswr_is_installed
{
    check_if_installed "openswr" $OPENSWR_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_openswr_build
{
    #
    # Build OpenSWR
    #
    cd "$START_DIR"
    if [[ "$DO_OPENSWR" == "yes" ]] ; then
        check_if_installed "openswr" $OPENSWR_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OpenSWR build.  OpenSWR is already installed."
        else
            info "Building OpenSWR (~20 minutes)"
            build_openswr
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OpenSWR.  Bailing out."
            fi
            info "Done building OpenSWR"
        fi
    fi
}
