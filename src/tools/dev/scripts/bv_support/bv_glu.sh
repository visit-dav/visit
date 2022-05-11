function bv_glu_initialize
{
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        export DO_GLU="yes"
    else 
        export DO_GLU="no"
    fi
}

function bv_glu_enable
{
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        DO_GLU="yes"
    fi
}

function bv_glu_disable
{
    DO_GLU="no"
}

function bv_glu_depends_on
{
    # We install into the mesagl directory so it needs to be on.

    if [[ "$DO_MESAGL" == "yes" ]] ; then
        echo "mesagl"
    fi
}

function bv_glu_info
{
    export GLU_FILE=${GLU_FILE:-"glu-9.0.0.tar.gz"}
    export GLU_VERSION=${GLU_VERSION:-"9.0.0"}
    export GLU_BUILD_DIR=${GLU_BUILD_DIR:-"glu-9.0.0"}
    export GLU_MD5_CHECKSUM="bbc57d4fe3bd3fb095bdbef6fcb977c4"
    export GLU_SHA256_CHECKSUM="4387476a1933f36fec1531178ea204057bbeb04cc2d8396c9ea32720a1f7e264"
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
        if [[ "$DO_MESAGL" == "yes" ]] ; then
            GLU_INSTALL_DIR="${MESAGL_INSTALL_DIR}"
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

function apply_glu_ppc64le_config_patch
{
  # patch glu's config.guess to allow it to recognize ppc64le
  patch -p0 << \EOF
*** ./glu-9.0.0/config.guess.orig 2018-03-22 11:22:30.000000000 
--- ./glu-9.0.0/config.guess 2018-03-22 11:23:23.000000000 
***************
*** 984,995 ****
--- 984,998 ----
  	  *)    echo hppa-unknown-linux-${LIBC} ;;
  	esac
  	exit ;;
      ppc64:Linux:*:*)
  	echo powerpc64-unknown-linux-${LIBC}
  	exit ;;
+     ppc64le:Linux:*:*)
+ 	echo powerpc64-unknown-linux-${LIBC}
+ 	exit ;;
      ppc:Linux:*:*)
  	echo powerpc-unknown-linux-${LIBC}
  	exit ;;
      s390:Linux:*:* | s390x:Linux:*:*)
  	echo ${UNAME_MACHINE}-ibm-linux
  	exit ;;

EOF

    if [[ $? != 0 ]] ; then
      warn "glu patch for config.guess failed."
      return 1
    fi
    return 0;
}

function apply_glu_patch
{
    apply_glu_ppc64le_config_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

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

    # NOTE: we install the library into the MesaGL directories.
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        set -x
        issue_command env GL_LIBS="-L${MESAGL_INSTALL_DIR}/lib" GL_CFLAGS="-I${MESAGL_INSTALL_DIR}/include" \
            CC=${C_COMPILER} CFLAGS="${C_OPT_FLAGS}" \
            CXX=${CXX_COMPILER} CXXFLAGS="${CXX_OPT_FLAGS}" \
           ./configure --prefix=${MESAGL_INSTALL_DIR} ${GLU_STATIC_DYNAMIC}
        set +x
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
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        if [[ -e $VISITDIR/mesagl/$MESAGL_VERSION/$VISITARCH/lib/libGLU.${EXT} ]] ; then
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
