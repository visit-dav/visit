function bv_p7zip_initialize
{
    export DO_P7ZIP="no"
}

function bv_p7zip_enable
{
    DO_P7ZIP="yes"
}

function bv_p7zip_disable
{
    DO_P7ZIP="no"
}

function bv_p7zip_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_p7zip_info
{
    export P7ZIP_VERSION=${P7ZIP_VERSION:-"16.02"}
    export P7ZIP_FILE=${P7ZIP_FILE:-"p7zip_${P7ZIP_VERSION}_src_all.tar.bz2"}
    export P7ZIP_COMPATIBILITY_VERSION=${P7ZIP_COMPATIBILITY_VERSION:-"16.0"}
    export P7ZIP_BUILD_DIR=${P7ZIP_BUILD_DIR:-"p7zip_${P7ZIP_VERSION}"}
    export P7ZIP_URL=${P7ZIP_URL:-https://sourceforge.net/projects/p7zip/files/p7zip/${P7ZIP_VERSION}}
    export P7ZIP_MD5_CHECKSUM="a0128d661cfe7cc8c121e73519c54fbf"
    export P7ZIP_SHA256_CHECKSUM="5eb20ac0e2944f6cb9c2d51dd6c4518941c185347d4089ea89087ffdd6e2341f"
}

function bv_p7zip_print
{
    printf "%s%s\n" "P7ZIP_FILE=" "${P7ZIP_FILE}"
    printf "%s%s\n" "P7ZIP_VERSION=" "${P7ZIP_VERSION}"
    printf "%s%s\n" "P7ZIP_COMPATIBILITY_VERSION=" "${P7ZIP_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "P7ZIP_BUILD_DIR=" "${P7ZIP_BUILD_DIR}"
}

function bv_p7zip_print_usage
{
    printf "%-20s %s [%s]\n" "--p7zip" "Build P7ZIP support" "$DO_P7ZIP"
}

function bv_p7zip_host_profile
{
    if [[ "$DO_P7ZIP" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## P7ZIP" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_SEVEN_ZIP_DIR \${VISITHOME}/p7zip/$P7ZIP_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_p7zip_ensure
{
    if [[ "$DO_P7ZIP" == "yes" ]] ; then
        ensure_built_or_ready "p7zip" $P7ZIP_VERSION $P7ZIP_BUILD_DIR $P7ZIP_FILE $P7ZIP_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_P7ZIP="no"
            error "Unable to build p7zip. ${P7ZIP_FILE} not found."
        fi
    fi
}

function bv_p7zip_dry_run
{
    if [[ "$DO_P7ZIP" == "yes" ]] ; then
        echo "Dry run option not set for p7zip."
    fi
}

function apply_p7zip_patch
{
    # Apply a patch to fix a narrowing error with gcc 10.3.
    patch -p0 << \EOF
diff -c CPP/Windows/ErrorMsg.cpp.orig CPP/Windows/ErrorMsg.cpp
*** CPP/Windows/ErrorMsg.cpp.orig	Wed Aug  4 15:41:48 2021
--- CPP/Windows/ErrorMsg.cpp	Wed Aug  4 15:42:01 2021
***************
*** 13,19 ****
    const char * txt = 0;
    AString msg;
  
!   switch(errorCode) {
      case ERROR_NO_MORE_FILES   : txt = "No more files"; break ;
      case E_NOTIMPL             : txt = "E_NOTIMPL"; break ;
      case E_NOINTERFACE         : txt = "E_NOINTERFACE"; break ;
--- 13,19 ----
    const char * txt = 0;
    AString msg;
  
!   switch((Int32)errorCode) {
      case ERROR_NO_MORE_FILES   : txt = "No more files"; break ;
      case E_NOTIMPL             : txt = "E_NOTIMPL"; break ;
      case E_NOINTERFACE         : txt = "E_NOINTERFACE"; break ;
EOF
    if [[ $? != 0 ]] ; then
        warn "p7zip patch failed."
        return 1
    fi

    return 0
}

# *************************************************************************** #
#                            Function 8, build_p7zip
#
# Modfications:
#
# *************************************************************************** #

function build_p7zip
{
    #
    # Prepare build dir
    #
    prepare_build_dir $P7ZIP_BUILD_DIR $P7ZIP_FILE
    untarred_p7zip=$?
    if [[ $untarred_p7zip == -1 ]] ; then
        warn "Unable to prepare p7zip build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    cd $P7ZIP_BUILD_DIR || error "Can't cd to p7zip build dir."
    info "Patching p7zip"
    apply_p7zip_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_p7zip == 1 ]] ; then
            warn "Giving up on p7zip build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing. I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build P7ZIP
    #
    if [[ "$OPSYS" == "Darwin" ]] ; then
        if [[ -z "${MACOSX_DEPLOYMENT_TARGET}" ]]; then
            cp makefile.macosx_llvm_64bits makefile.machine
        else
            DTDIGITS=$(echo ${MACOSX_DEPLOYMENT_TARGET} | tr -d '.')
            if [[ $DTDIGITS -le 109 ]]; then #
                cp makefile.macosx_gcc_32bits makefile.machine
            else
                cp makefile.macosx_llvm_64bits makefile.machine
            fi
        fi
    fi

    info "Building P7ZIP . . . (~1 minute)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "p7zip build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing p7zip"
    grep -v '^DEST_HOME=' install.sh > install2.sh
    env DEST_HOME="$VISITDIR/p7zip/$P7ZIP_VERSION/$VISITARCH" sh install2.sh
    if [[ $? != 0 ]] ; then
        warn "p7zip install failed. Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/p7zip"
        chgrp -R ${GROUP} "$VISITDIR/p7zip"
    fi
    cd "$START_DIR"
    info "Done with p7zip"
    return 0
}

function bv_p7zip_is_enabled
{
    if [[ $DO_P7ZIP == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_p7zip_is_installed
{
    check_if_installed "p7zip" $P7ZIP_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_p7zip_build
{
    cd "$START_DIR"
    if [[ "$DO_P7ZIP" == "yes" ]] ; then
        check_if_installed "p7zip" $P7ZIP_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping p7zip build. p7zip is already installed."
        else
            info "Building p7zip (~1 minute)"
            build_p7zip
            if [[ $? != 0 ]] ; then
                error "Unable to build or install p7zip. Bailing out."
            fi
            info "Done building p7zip"
        fi
    fi
}
