function bv_xkbcommon_initialize
{
    export DO_XKBCOMMON="no"
}

function bv_xkbcommon_enable
{
    DO_XKBCOMMON="yes"
}

function bv_xkbcommon_disable
{
    DO_XKBCOMMON="no"
}

function bv_xkbcommon_depends_on
{
    depends_on="meson ninja"

    echo ${depends_on}
}

function bv_xkbcommon_initialize_vars
{
    XKBCOMMON_INSTALL_DIR="${VISITDIR}/xkbcommon/$XKBCOMMON_VERSION/${VISITARCH}"
}

function bv_xkbcommon_info
{
    export XKBCOMMON_VERSION=${XKBCOMMON_VERSION:-"1.7.0"}
    export XKBCOMMON_FILE=${XKBCOMMON_FILE:-"libxkbcommon-${XKBCOMMON_VERSION}.tar.xz"}
    export XKBCOMMON_BUILD_DIR=${XKBCOMMON_BUILD_DIR:-"libxkbcommon-${XKBCOMMON_VERSION}"}
    export XKBCOMMON_SHA256_CHECKSUM=""65782f0a10a4b455af9c6baab7040e2f537520caa2ec2092805cdfd36863b247
}

function bv_xkbcommon_print
{
    printf "%s%s\n" "XKBCOMMON_FILE=" "${XKBCOMMON_FILE}"
    printf "%s%s\n" "XKBCOMMON_VERSION=" "${XKBCOMMON_VERSION}"
    printf "%s%s\n" "XKBCOMMON_BUILD_DIR=" "${XKBCOMMON_BUILD_DIR}"
}

function bv_xkbcommon_print_usage
{
    printf "%-20s %s [%s]\n" "--xkbcommon" "Build xkbcommon support" "$DO_XKBCOMMON"
}

function bv_xkbcommon_host_profile
{
    # Nothing added to the host profile since xkbcommon is only used for
    # building third party libraries.
    return 0
}

function bv_xkbcommon_ensure
{
    if [[ "$DO_XKBCOMMON" == "yes" ]] ; then
        ensure_built_or_ready "xkbcommon" $XKBCOMMON_VERSION $XKBCOMMON_BUILD_DIR $XKBCOMMON_FILE $XKBCOMMON_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XKBCOMMON="no"
            error "Unable to build xkbcommon. ${XKBCOMMON_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_xkbcommon
#
#
# *************************************************************************** #
function build_xkbcommon
{
    #
    # Extract the sources
    #
    if [[ -d $XKBCOMMON_BUILD_DIR ]] ; then
        if [[ ! -f $XKBCOMMON_FILE ]] ; then
            warn "The directory XKBCOMMON exists, deleting before uncompressing"
            rm -Rf $XKBCOMMON_BUILD_DIR
            ensure_built_or_ready $XKBCOMMON_INSTALL_DIR $XKBCOMMON_VERSION $XKBCOMMON_BUILD_DIR $XKBCOMMON_FILE
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $XKBCOMMON_BUILD_DIR $XKBCOMMON_FILE
    untarred_xkbcommon=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xkbcommon == -1 ]] ; then
        warn "Unable to prepare xkbcommon build directory. Giving Up!"
        return 1
    fi

    #
    # Configure XKBCOMMON
    #
    info "Configuring xkbcommon . . ."

    cd $XKBCOMMON_BUILD_DIR || error "Can't cd to xkbcommon build dir."

    export PATH=$MESON_INSTALL_DIR/bin:$NINJA_INSTALL_DIR/bin:$PATH

    meson setup build || error "Xkbcommon did not configure correctly. Giving up."

    #
    # Build xkbcommon
    #
    info "Building xkbcommon . . . (~1 minutes)"
    meson compile -C build || error "Xkbcommon did not build correctly. Giving up."

    info "Installing Xkbcommon . . . (~1 minutes)"

    # The libraries
    if [[ ! -d ${XKBCOMMON_INSTALL_DIR}/lib ]] ; then
        mkdir -p ${XKBCOMMON_INSTALL_DIR}/lib
    fi
    cp build/libxkbcommon.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib
    ln -s libxkbcommon.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib/libxkbcommon.so
    ln -s libxkbcommon.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib/libxkbcommon.so.0
    cp build/libxkbcommon-x11.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib
    ln -s libxkbcommon-x11.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib/libxkbcommon-x11.so
    ln -s libxkbcommon-x11.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib/libxkbcommon-x11.so.0
    cp build/libxkbregistry.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib
    ln -s libxkbregistry.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib/libxkbregistry.so
    ln -s libxkbregistry.so.0.0.0 ${XKBCOMMON_INSTALL_DIR}/lib/libxkbregistry.so.0

    # The header files
    if [[ ! -d ${XKBCOMMON_INSTALL_DIR}/include/xkbcommon ]] ; then
        mkdir -p ${XKBCOMMON_INSTALL_DIR}/include/xkbcommon
    fi
    cp include/xkbcommon/* ${XKBCOMMON_INSTALL_DIR}/include/xkbcommon

    # The pkg-config files
    if [[ ! -d ${XKBCOMMON_INSTALL_DIR}/pkgconfig ]] ; then
        mkdir -p ${XKBCOMMON_INSTALL_DIR}/pkgconfig
    fi

    PC_FILE=${XKBCOMMON_INSTALL_DIR}/pkgconfig/xkbcommon.pc
    if [[ -f $PC_FILE ]] ; then
        rm $PC_FILE
    fi
    echo "prefix=${XKBCOMMON_INSTALL_DIR}" > $PC_FILE
    echo "libdir=\${prefix}/lib" >> $PC_FILE
    echo "includedir=\${prefix}/include" >> $PC_FILE
    echo "" >> $PC_FILE
    echo "Name: xkbcommon" >> $PC_FILE
    echo "Description: XKB API common to servers and clients" >> $PC_FILE
    echo "Version: 1.7.0" >> $PC_FILE
    echo "Libs: -L\${libdir} -lxkbcommon" >> $PC_FILE
    echo "Cflags: -I\${includedir}" >> $PC_FILE

    PC_FILE=${XKBCOMMON_INSTALL_DIR}/pkgconfig/xkbcommon-x11.pc
    if [[ -f $PC_FILE ]] ; then
        rm $PC_FILE
    fi
    echo "prefix=${XKBCOMMON_INSTALL_DIR}" > $PC_FILE
    echo "libdir=\${prefix}/lib" >> $PC_FILE
    echo "includedir=\${prefix}/include" >> $PC_FILE
    echo "" >> $PC_FILE
    echo "Name: xkbcommon-x11" >> $PC_FILE
    echo "Description: XKB API common to servers and clients - X11 support" >> $PC_FILE
    echo "Version: 1.7.0" >> $PC_FILE
    echo "Requires: xkbcommon" >> $PC_FILE
    echo "Requires.private: xcb >= 1.10, xcb-xkb >= 1.10" >> $PC_FILE
    echo "Libs: -L\${libdir} -lxkbcommon" >> $PC_FILE
    echo "Cflags: -I\${includedir}" >> $PC_FILE

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/xkbcommon"
        chgrp -R ${GROUP} "$VISITDIR/xkbcommon"
    fi
    cd "$START_DIR"
    info "Done with xkbcommon"
    return 0
}

function bv_xkbcommon_is_enabled
{
    if [[ $DO_XKBCOMMON == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_xkbcommon_is_installed
{
    check_if_installed "xkbcommon" $XKBCOMMON_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_xkbcommon_build
{
    cd "$START_DIR"
    if [[ "$DO_XKBCOMMON" == "yes" ]] ; then
        check_if_installed "xkbcommon" $XKBCOMMON_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping xkbcommon build. Xkbcommon is already installed."
        else
            info "Building xkbcommon (~2 minutes)"
            build_xkbcommon
            if [[ $? != 0 ]] ; then
                error "Unable to build or install xkbcommon.  Bailing out."
            fi
            info "Done building xkbcommon"
        fi
    fi
}
