function bv_xcb_initialize
{
    export DO_XCB="no"
}

function bv_xcb_enable
{
    DO_XCB="yes"
}

function bv_xcb_disable
{
    DO_XCB="no"
}

function bv_xcb_depends_on
{
    depends_on="meson ninja"

    echo ${depends_on}
}

function bv_xcb_initialize_vars
{
    XCB_INSTALL_DIR="${VISITDIR}/xcb/$XCB_VERSION/${VISITARCH}"
}

function bv_xcb_info
{
    export XCB_IMAGE_VERSION=${XCB_IMAGE_VERSION:-"0.4.1"}
    export XCB_IMAGE_FILE=${XCB_IMAGE_FILE:-"libxcb-image-xcb-util-image-${XCB_IMAGE_VERSION}.tar.gz"}
    export XCB_IMAGE_BUILD_DIR=${XCB_IMAGE_BUILD_DIR:-"libxcb-image-xcb-util-image-${XCB_IMAGE_VERSION}"}
    export XCB_IMAGE_SHA256_CHECKSUM="f8aea5230dcf736aa86a005ba486f58b689f183006c26ecc44b91ed6b11598d4"
    export XCB_KEYSYMS_VERSION=${XCB_KEYSYMS_VERSION:-"0.4.1"}
    export XCB_KEYSYMS_FILE=${XCB_KEYSYMS_FILE:-"libxcb-keysyms-xcb-util-keysyms-${XCB_KEYSYMS_VERSION}.tar.gz"}
    export XCB_KEYSYMS_BUILD_DIR=${XCB_KEYSYMS_BUILD_DIR:-"libxcb-keysyms-xcb-util-keysyms-${XCB_KEYSYMS_VERSION}"}
    export XCB_KEYSYMS_SHA256_CHECKSUM="2780db069685a95f132a48d637e7f66ffd0a2483e960157536f58f4671d93f5c"
    export XCB_M4_VERSION=${XCB_M4_VERSION:-"0.4.1"}
    export XCB_M4_FILE=${XCB_M4_FILE:-"libxcb-m4-xcb-util-m4-${XCB_M4_VERSION}.tar.gz"}
    export XCB_M4_BUILD_DIR=${XCB_M4_BUILD_DIR:-"libxcb-m4-xcb-util-m4-${XCB_M4_VERSION}"}
    export XCB_M4_SHA256_CHECKSUM="f60369c3bad234798867b768fc01183395a20f74c521a48e5e996f938df2d45a"
    export XCB_RENDERUTIL_VERSION=${XCB_RENDERUTIL_VERSION:-"0.3.10"}
    export XCB_RENDERUTIL_FILE=${XCB_RENDERUTIL_FILE:-"libxcb-render-util-xcb-util-renderutil-${XCB_RENDERUTIL_VERSION}.tar.gz"}
    export XCB_RENDERUTIL_BUILD_DIR=${XCB_RENDERUTIL_BUILD_DIR:-"libxcb-render-util-xcb-util-renderutil-${XCB_RENDERUTIL_VERSION}"}
    export XCB_RENDERUTIL_SHA256_CHECKSUM="160017e3e8e61acb8ddfbce885f294623a46f92f20c5f5066ee6d1b720971548"
    export XCB_UTIL_VERSION=${XCB_UTIL_VERSION:-"0.4.1"}
    export XCB_UTIL_FILE=${XCB_UTIL_FILE:-"libxcb-util-xcb-util-${XCB_UTIL_VERSION}.tar.gz"}
    export XCB_UTIL_BUILD_DIR=${XCB_UTIL_BUILD_DIR:-"libxcb-util-xcb-util-${XCB_UTIL_VERSION}"}
    export XCB_UTIL_SHA256_CHECKSUM="7b56592b339d47809cbefb9f46721705c662de1a001bc773d335975cd2eba34f"
    export XCB_WM_VERSION=${XCB_WM_VERSION:-"0.4.2"}
    export XCB_WM_FILE=${XCB_WM_FILE:-"libxcb-wm-xcb-util-wm-${XCB_WM_VERSION}.tar.gz"}
    export XCB_WM_BUILD_DIR=${XCB_WM_BUILD_DIR:-"libxcb-wm-xcb-util-wm-${XCB_WM_VERSION}"}
    export XCB_WM_SHA256_CHECKSUM="c1b792306874c36b535413a33edc71a0ac46e78adcf6ddb1a34090a07393d717"
}

function bv_xcb_print
{
    printf "%s%s\n" "XCB_IMAGE_FILE=" "${XCB_IMAGE_FILE}"
    printf "%s%s\n" "XCB_IMAGE_VERSION=" "${XCB_IMAGE_VERSION}"
    printf "%s%s\n" "XCB_IMAGE_BUILD_DIR=" "${XCB_IMAGE_BUILD_DIR}"
    printf "%s%s\n" "XCB_KEYSYMS_FILE=" "${XCB_KEYSYMS_FILE}"
    printf "%s%s\n" "XCB_KEYSYMS_VERSION=" "${XCB_KEYSYMS_VERSION}"
    printf "%s%s\n" "XCB_KEYSYMS_BUILD_DIR=" "${XCB_KEYSYMS_BUILD_DIR}"
    printf "%s%s\n" "XCB_M4_FILE=" "${XCB_M4_FILE}"
    printf "%s%s\n" "XCB_M4_VERSION=" "${XCB_M4_VERSION}"
    printf "%s%s\n" "XCB_M4_BUILD_DIR=" "${XCB_M4_BUILD_DIR}"
    printf "%s%s\n" "XCB_RENDERUTIL_FILE=" "${XCB_RENDERUTIL_FILE}"
    printf "%s%s\n" "XCB_RENDERUTIL_VERSION=" "${XCB_RENDERUTIL_VERSION}"
    printf "%s%s\n" "XCB_RENDERUTIL_BUILD_DIR=" "${XCB_RENDERUTIL_BUILD_DIR}"
    printf "%s%s\n" "XCB_UTIL_FILE=" "${XCB_UTIL_FILE}"
    printf "%s%s\n" "XCB_UTIL_VERSION=" "${XCB_UTIL_VERSION}"
    printf "%s%s\n" "XCB_UTIL_BUILD_DIR=" "${XCB_UTIL_BUILD_DIR}"
    printf "%s%s\n" "XCB_WM_FILE=" "${XCB_WM_FILE}"
    printf "%s%s\n" "XCB_WM_VERSION=" "${XCB_WM_VERSION}"
    printf "%s%s\n" "XCB_WM_BUILD_DIR=" "${XCB_WM_BUILD_DIR}"
}

function bv_xcb_print_usage
{
    printf "%-20s %s [%s]\n" "--xcb" "Build xcb support" "$DO_XCB"
}

function bv_xcb_host_profile
{
    # Nothing added to the host profile since xcb is only used for
    # building third party libraries.
    return 0
}

function bv_xcb_ensure
{
    if [[ "$DO_XCB" == "yes" ]] ; then
        ensure_built_or_ready "xcb-image" $XCB_IMAGE_VERSION $XCB_IMAGE_BUILD_DIR $XCB_IMAGE_FILE $XCB_IMAGE_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XCB="no"
            error "Unable to build xcb image. ${XCB_IMAGE_FILE} not found."
        fi

        ensure_built_or_ready "xcb-keysyms" $XCB_KEYSYMS_VERSION $XCB_KEYSYMS_BUILD_DIR $XCB_KEYSYMS_FILE $XCB_KEYSYMS_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XCB="no"
            error "Unable to build xcb keysyms. ${XCB_KEYSYMS_FILE} not found."
        fi

        ensure_built_or_ready "xcb-m4" $XCB_M4_VERSION $XCB_M4_BUILD_DIR $XCB_M4_FILE $XCB_M4_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XCB="no"
            error "Unable to build xcb m4. ${XCB_M4_FILE} not found."
        fi

        ensure_built_or_ready "xcb-renderutil" $XCB_RENDERUTIL_VERSION $XCB_RENDERUTIL_BUILD_DIR $XCB_RENDERUTIL_FILE $XCB_RENDERUTIL_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XCB="no"
            error "Unable to build xcb renderutil. ${XCB_RENDERUTIL_FILE} not found."
        fi

        ensure_built_or_ready "xcb-util" $XCB_UTIL_VERSION $XCB_UTIL_BUILD_DIR $XCB_UTIL_FILE $XCB_UTIL_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XCB="no"
            error "Unable to build xcb util. ${XCB_UTIL_FILE} not found."
        fi

        ensure_built_or_ready "xcb-wm" $XCB_WM_VERSION $XCB_WM_BUILD_DIR $XCB_WM_FILE $XCB_WM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XCB="no"
            error "Unable to build xcb wm. ${XCB_WM_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_xcb
#
# Modifications:
#
# *************************************************************************** #
function build_xcb
{
    # QT6 requires the following XCB modules
    # The modules marked with an asterisk are being built by build_visit.
    # The remaining ones come from XCB proper and are usually installed on
    # the system. If we run across a system that doesn't have any XCB
    # stuff installed we can add building XCB. It can be found at
    # https://gitlab.freedesktop.org/xorg/lib/libxcb.
    #
    # XCB
    # ICCCM       *
    # SHM
    # IMAGE       *
    # KEYSYMS     *
    # RENDER
    # RENDERUTIL  *
    # RANDR
    # SHAPE
    # SYNC
    # XFIXES
    # XKB
    # GLX
    # XINPUT

    # XCB M4
    # https://gitlab.freedesktop.org/xorg/util/xcb-util-m4
    # Provides m4 macros needed as part of configure process by other packages.
    #
    # Prepare build dir
    #
    prepare_build_dir $XCB_IMAGE_BUILD_DIR $XCB_IMAGE_FILE
    untarred_xcb=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xcb == -1 ]] ; then
        warn "Unable to prepare xcb image build directory. Giving Up!"
        return 1
    fi

    #
    # Nothing else to do since all we do is untar the file.
    #

    # XCB UTIL
    # https://gitlab.freedesktop.org/xorg/lib/libxcb-util
    # Provides aux, atom, event
    #
    # Prepare build dir
    #
    prepare_build_dir $XCB_UTIL_BUILD_DIR $XCB_UTIL_FILE
    untarred_xcb=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xcb == -1 ]] ; then
        warn "Unable to prepare xcb util build directory. Giving Up!"
        return 1
    fi

    #
    # Configure and install
    #
    info "Configuring and installing xcb util . . . (~1 minute)"
    cd $XCB_UTIL_BUILD_DIR || error "Can't cd to xcb util build dir."
    cd m4
    cp ../../libxcb-m4-xcb-util-m4-0.4.1/* .
    cd ..

    ./autogen.sh --prefix=${XCB_INSTALL_DIR}

    make install
    cd ..

    # Add the pkgconfig directory to the PKG_CONFIG_PATH so that pkg-config
    # can find xcb util needed by the rest of the xcb packages and qt6.
    export PKG_CONFIG_PATH=/usr/WS1/brugger/visit_spack_xcb_build/bv_test/third_party/xcb/0.4.1/linux-ppc64le_gcc-8.3/lib/pkgconfig:$PKG_CONFIG_PATH

    # XCB IMAGE
    # https://gitlab.freedesktop.org/xorg/lib/libxcb-image
    # Provides image
    #
    # Prepare build dir
    #
    prepare_build_dir $XCB_IMAGE_BUILD_DIR $XCB_IMAGE_FILE
    untarred_xcb=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xcb == -1 ]] ; then
        warn "Unable to prepare xcb image build directory. Giving Up!"
        return 1
    fi

    #
    # Configure and install
    #
    info "Configuring and installing xcb image . . . (~1 minute)"
    cd $XCB_IMAGE_BUILD_DIR || error "Can't cd to xcb image build dir."
    cd m4
    cp ../../libxcb-m4-xcb-util-m4-0.4.1/* .
    cd ..

    ./autogen.sh --prefix=${XCB_INSTALL_DIR}
    make install
    cd ..

    # XCB KEYSYMS
    # https://gitlab.freedesktop.org/xorg/lib/libxcb-keysyms
    # Provides keysyms
    #
    # Prepare build dir
    #
    prepare_build_dir $XCB_KEYSYMS_BUILD_DIR $XCB_KEYSYMS_FILE
    untarred_xcb=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xcb == -1 ]] ; then
        warn "Unable to prepare xcb keysyms build directory. Giving Up!"
        return 1
    fi

    #
    # Configure and install
    #
    info "Configuring and installing xcb keysyms . . . (~1 minute)"
    cd $XCB_KEYSYMS_BUILD_DIR || error "Can't cd to xcb keysyms build dir."
    cd m4
    cp ../../libxcb-m4-xcb-util-m4-0.4.1/* .
    cd ..

    ./autogen.sh --prefix=${XCB_INSTALL_DIR}
    make install
    cd ..

    # XCB WM
    # https://gitlab.freedesktop.org/xorg/lib/libxcb-wm
    # Provides ewmh, icccm
    #
    # Prepare build dir
    #
    prepare_build_dir $XCB_WM_BUILD_DIR $XCB_WM_FILE
    untarred_xcb=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xcb == -1 ]] ; then
        warn "Unable to prepare xcb wm build directory. Giving Up!"
        return 1
    fi

    #
    # Configure and install
    #
    info "Configuring and installing xcb wm . . . (~1 minute)"
    cd $XCB_WM_BUILD_DIR || error "Can't cd to xcb wm build dir."
    cd m4
    cp ../../libxcb-m4-xcb-util-m4-0.4.1/* .
    cd ..

    ./autogen.sh --prefix=${XCB_INSTALL_DIR}
    make install
    cd ..

    # XCB RENDERUTIL
    # https://gitlab.freedesktop.org/xorg/lib/libxcb-render-util
    # Provides renderutil
    #
    # Prepare build dir
    #
    prepare_build_dir $XCB_RENDERUTIL_BUILD_DIR $XCB_RENDERUTIL_FILE
    untarred_xcb=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_xcb == -1 ]] ; then
        warn "Unable to prepare xcb renderutil build directory. Giving Up!"
        return 1
    fi

    #
    # Configure and install
    #
    info "Configuring and installing xcb renderutil . . . (~1 minute)"
    cd $XCB_RENDERUTIL_BUILD_DIR || error "Can't cd to xcb renderutil build dir."
    cd m4
    cp ../../libxcb-m4-xcb-util-m4-0.4.1/* .
    cd ..

    ./autogen.sh --prefix=${XCB_INSTALL_DIR}
    make install
    cd ..

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/xcb"
        chgrp -R ${GROUP} "$VISITDIR/xcb"
    fi
    cd "$START_DIR"
    info "Done with xcb"
    return 0
}

function bv_xcb_is_enabled
{
    if [[ $DO_XCB == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_xcb_is_installed
{
    check_if_installed "xcb" $XCB_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_xcb_build
{
    cd "$START_DIR"
    if [[ "$DO_XCB" == "yes" ]] ; then
        check_if_installed "xcb" $XCB_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping xcb build. Xcb is already installed."
        else
            info "Building xcb (~5 minutes)"
            build_xcb
            if [[ $? != 0 ]] ; then
                error "Unable to build or install xcb.  Bailing out."
            fi
            info "Done building xcb"
        fi
    fi
}
