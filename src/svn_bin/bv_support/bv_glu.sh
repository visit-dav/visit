function bv_glu_initialize
{
    export DO_GLU="no"
}

function bv_glu_enable
{
    if [[ "$DO_MESA" == "yes" || "$DO_OPENSWR" == "yes" ]] ; then
        DO_GLU="yes"
    fi
}

function bv_glu_disable
{
    DO_GLU="no"
}

function bv_glu_depends_on
{
    # We install into the openswr directory so it needs to be on.
    echo "openswr"
}

function bv_glu_info
{
    export GLU_FILE=${GLU_FILE:-"glu-9.0.0.tar.gz"}
    export GLU_VERSION=${GLU_VERSION:-"9.0.0"}
    export GLU_BUILD_DIR=${GLU_BUILD_DIR:-"glu-9.0.0"}
    export GLU_MD5_CHECKSUM=""
    export GLU_SHA256_CHECKSUM=""
    export GLU_URL=${GLU_URL:-"ftp://ftp.freedesktop.org/pub/mesa/glu"}
}

function bv_glu_print
{
    printf "%s%s\n" "GLU_FILE=" "${GLU_FILE}"
    printf "%s%s\n" "GLU_VERSION=" "${GLU_VERSION}"
    printf "%s%s\n" "GLU_TARGET=" "${GLU_TARGET}"
    printf "%s%s\n" "GLU_BUILD_DIR=" "${GLU_BUILD_DIR}"
}

function bv_glu_print_usage
{
    printf "%-20s %s [%s]\n" "--glu" "Build GLU" "$DO_GLU"
}

function bv_glu_host_profile
{
#    if [[ "$DO_GLU" == "yes" ]] ; then
#        echo >> $HOSTCONF
#        echo "##" >> $HOSTCONF
#        echo "## GLU" >> $HOSTCONF
#        echo "##" >> $HOSTCONF
#        echo "VISIT_OPTION_DEFAULT(VISIT_GLU_DIR \${VISITHOME}/glu/$GLU_VERSION/\${VISITARCH})" >> $HOSTCONF
#    fi
     return 0
}

function bv_glu_selected
{
    args=$@
    if [[ $args == "--glu" ]]; then
        DO_GLU="yes"
        return 1
    fi

    return 0
}

function bv_glu_initialize_vars
{
    info "initalizing glu vars"
    if [[ "$DO_GLU" == "yes" ]]; then
        if [[ "$DO_MESA" == "yes" ]] ; then
            GLU_INSTALL_DIR="${MESA_INSTALL_DIR}"
        elif [[ "$DO_OPENSWR" == "yes" ]] ; then
            GLU_INSTALL_DIR="${OPENSWR_INSTALL_DIR}"
        else
            GLU_INSTALL_DIR="${VISITDIR}/glu/${GLU_VERSION}/${VISITARCH}"
        fi
        GLU_INCLUDE_DIR="${GLU_INSTALL_DIR}/include"
        GLU_LIB_DIR="${GLU_INSTALL_DIR}/lib"
        if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            GLU_LIB="${GLU_LIB_DIR}/libGLU.a"
        else
            GLU_LIB="${GLU_LIB_DIR}/libGLU.${SO_EXT}"
        fi
    fi
}

function bv_glu_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_GLU" == "yes" ]] ; then
            ensure_built_or_ready "glu"   $GLU_VERSION   $GLU_BUILD_DIR   $GLU_FILE
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function bv_glu_dry_run
{
    if [[ "$DO_GLU" == "yes" ]] ; then
        echo "Dry run option not set for glu."
    fi
}

function apply_glu_patch
{
    return 0
}


function build_glu
{
    #
    # prepare build dir
    #
    prepare_build_dir $GLU_BUILD_DIR $GLU_FILE
    untarred_glu=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_glu == -1 ]] ; then
        warn "Unable to prepare GLU build directory. Giving Up!"
        return 1
    fi

    #
    # Patch glu
    #
    apply_glu_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_glu == 1 ]] ; then
            warn "Giving up on GLU build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build GLU.
    #
    info "Building GLU . . . (~2 minutes)"
    cd $GLU_BUILD_DIR || error "Couldn't cd to glu build dir."

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        GLU_STATIC_DYNAMIC="--disable-shared --enable-static"
    fi

    # NOTE: we install the library into the Mesa or OpenSWR directories.
    if [[ "$DO_MESA" == "yes" ]] ; then
        issue_command env PKG_CONFIG_LIBDIR=${MESA_INSTALL_DIR}/lib \
            CC=${C_COMPILER} CFLAGS="${C_OPT_FLAGS}" \
            CXX=${CXX_COMPILER} CXXFLAGS="${CXX_OPT_FLAGS}" \
           ./configure --prefix=${MESA_INSTALL_DIR} ${GLU_STATIC_DYNAMIC}
        if [[ $? != 0 ]] ; then
            warn "GLU: 'configure' failed.  Giving up"
            return 1
        fi
    elif [[ "$DO_OPENSWR" == "yes" ]] ; then
        issue_command env PKG_CONFIG_LIBDIR=${OPENSWR_INSTALL_DIR}/lib \
            CC=${C_COMPILER} CFLAGS="${C_OPT_FLAGS}" \
            CXX=${CXX_COMPILER} CXXFLAGS="${CXX_OPT_FLAGS}" \
           ./configure --prefix=${OPENSWR_INSTALL_DIR} ${GLU_STATIC_DYNAMIC}
        if [[ $? != 0 ]] ; then
            warn "GLU: 'configure' failed.  Giving up"
            return 1
        fi
    else
        warn "GLU: 'configure' failed.  Giving up"
        return 1
    fi

    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "GLU: 'make' failed.  Giving up"
        return 1
    fi
    info "Installing GLU ..."
    ${MAKE} install
    if [[ $? != 0 ]] ; then
        warn "GLU: 'make install' failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/glu"
        chgrp -R ${GROUP} "$VISITDIR/glu"
    fi
    cd "$START_DIR"
    info "Done with GLU"
    return 0
}

function bv_glu_is_enabled
{
    if [[ $DO_GLU == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_glu_is_installed
{
    EXT=${SO_EXT}
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        EXT="a"
    fi
    if [[ "$DO_MESA" == "yes" ]] ; then
        if [[ -e $VISITDIR/mesa/$MESA_VERSION/$VISITARCH/lib/libGLU.${EXT} ]] ; then
            return 1
        fi
    elif [[ "$DO_OPENSWR" == "yes" ]] ; then
        if [[ -e $VISITDIR/openswr/$OPENSWR_VERSION/$VISITARCH/lib/libGLU.${EXT} ]] ; then
            return 1
        fi
    fi
    return 0
}

function bv_glu_build
{
    #
    # Build GLU
    #
    cd "$START_DIR"
    if [[ "$DO_GLU" == "yes" ]] ; then
        bv_glu_is_installed
        if [[ $? == 1 ]] ; then
            info "Skipping GLU build.  GLU is already installed."
        else
            info "Building GLU (~2 minutes)"
            build_glu
            if [[ $? != 0 ]] ; then
                error "Unable to build or install GLU.  Bailing out."
            fi
            info "Done building GLU"
        fi
    fi
}
