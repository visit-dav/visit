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
            error "Unable to build P7ZIP.  ${P7ZIP_FILE} not found."
        fi
    fi
}

function bv_p7zip_dry_run
{
    if [[ "$DO_P7ZIP" == "yes" ]] ; then
        echo "Dry run option not set for p7zip."
    fi
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
        warn "Unable to prepare P7ZIP build directory. Giving Up!"
        return 1
    fi
    
    cd $P7ZIP_BUILD_DIR || error "Can't cd to P7ZIP build dir."
    if [[ "$OPSYS" == "Darwin" ]] ; then
        DTDIGITS=$(echo ${MACOSX_DEPLOYMENT_TARGET} | tr -d'.')
        if [[ $DTDIGITS -le 109 ]]; then #
            cp makefile.macosx_gcc_32bits makefile.machine
        else
            cp makefile.macosx_llvm_64bits makefile.machine
        fi
    fi

    #
    # Build P7ZIP
    #
    info "Building P7ZIP . . . (~1 minute)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "P7ZIP build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing P7ZIP"
    grep -v '^DEST_HOME=' install.sh > install2.sh
    env DEST_HOME="$VISITDIR/p7zip/$P7ZIP_VERSION/$VISITARCH" sh install2.sh
    if [[ $? != 0 ]] ; then
        warn "P7ZIP install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/p7zip"
        chgrp -R ${GROUP} "$VISITDIR/p7zip"
    fi
    cd "$START_DIR"
    info "Done with P7ZIP"
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
            info "Skipping P7ZIP build.  P7ZIP is already installed."
        else
            info "Building P7ZIP (~1 minute)"
            build_p7zip
            if [[ $? != 0 ]] ; then
                error "Unable to build or install P7ZIP.  Bailing out."
            fi
            info "Done building P7ZIP"
        fi
    fi
}
