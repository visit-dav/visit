function bv_visus_initialize
{
    export VISUS_OS=`uname` # Used later to determine which ViSUS binary to download
    export DO_VISUS="no"
    export USE_SYSTEM_VISUS="no"
    add_extra_commandline_args "visus" "alt-visus-dir" 1 "Use alternative directory for ViSUS"
}

function bv_visus_alt_visus_dir
{
    bv_visus_enable
    USE_SYSTEM_VISUS="yes"
    VISUS_INSTALL_DIR="$1"
    VISUS_INSTALL_DIR_cmake="$1"
}

function bv_visus_enable
{
    DO_VISUS="yes"
    if [[ -z "$(echo $VISUS_OS | grep Linux\\\|Darwin\\\|Win64\\\|Win32)" ]]; then
        warn "ViSUS not available on this [$VISUS_OS] architecture. Disabling it."
        DO_VISUS="no"
    fi
}

function bv_visus_disable
{
    DO_VISUS="no"
}

function bv_visus_depends_on
{
    if [[ "$USE_SYSTEM_VISUS" == "yes" ]]; then
        echo ""
    fi
}

function bv_visus_initialize_vars
{
    if [[ "$USE_SYSTEM_VISUS" == "no" ]]; then
        VISUS_INSTALL_DIR="${VISITDIR}/visus/$VISUS_VERSION/${VISITARCH}"
    fi
}

function bv_visus_info
{
    #todo: change build_dir to install_dir or something
    export VISUS_VERSION=${VISUS_VERSION:-"5f5fd6c"}
    export VISUS_EXTENSION=${VISUS_EXTENSION:-"tgz"}
    if [[ "$VISUS_OS" == "Linux" ]]; then
        export VISUS_MD5_CHECKSUM="79ed0196efe22f30b6368a92bd2df481"
        export VISUS_SHA256_CHECKSUM="0b05301aee13ec70f55606a89d8fa27ceb970925f600eb4d52b5378d82c930e6"
    elif [[ "$VISUS_OS" == "Darwin" ]]; then
        export VISUS_MD5_CHECKSUM="b335ca6c4d4f366cf53857e4cc933d60"
        export VISUS_SHA256_CHECKSUM="7de8b2349a84269424e1b7ee8a835bc2ba25653db84ebfdf6b6a97f05531890b"
    elif [[ "$VISUS_OS" == "Win64" ]]; then
        export VISUS_MD5_CHECKSUM="7d7c68d28d9dc45d1dfbddadf774b9c8"
        export VISUS_SHA256_CHECKSUM="2da2995795bed3bc9b58f962aac9c451759b05645c100b3ee0160c9d66eca70b"
        export VISUS_EXTENSION="zip"
    elif [[ "$VISUS_OS" == "Win32" ]]; then
        export VISUS_MD5_CHECKSUM="9affb263318a2f87a270e8d78d1f01fe"
        export VISUS_SHA256_CHECKSUM="fe8af6e27b929f0d1dc3849bacbc5f3e5f437e87dbfa711a18a27563f763b7ba"
        export VISUS_EXTENSION="zip"
    fi
    export VISUS_FILE=${VISUS_FILE:-"ViSUS-${VISUS_VERSION}-${VISUS_OS}.${VISUS_EXTENSION}"}
    export VISUS_URL=${VISUS_URL:-"http://atlantis.sci.utah.edu/builds/visit-plugin"}
    export VISUS_BUILD_DIR=${VISUS_BUILD_DIR:-"ViSUS"}
}

function bv_visus_print
{
    printf "%s%s\n" "VISUS_FILE=" "${VISUS_FILE}"
    printf "%s%s\n" "VISUS_VERSION=" "${VISUS_VERSION}"
    printf "%s%s\n" "VISUS_BUILD_DIR=" "${VISUS_BUILD_DIR}"
}

function bv_visus_print_usage
{
    printf "%-20s %s [%s]\n" "--visus" "Build ViSUS support" "$DO_VISUS"
    printf "%-20s %s [%s]\n" "--alt-visus-dir" "Use ViSUS from an alternative directory"
}

function bv_visus_host_profile
{
    if [[ "$DO_VISUS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VISUS " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_VISUS" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR ${VISUS_INSTALL_DIR_cmake})" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR \${VISITHOME}/visus/$VISUS_VERSION/\${VISITARCH})" >> $HOSTCONF
        fi
    fi
}

function bv_visus_ensure
{
    if [[ "$DO_VISUS" == "yes" && "$USE_SYSTEM_VISUS" == "no" ]] ; then
        ensure_built_or_ready "visus" $VISUS_VERSION $VISUS_BUILD_DIR $VISUS_FILE $VISUS_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_VISUS="no"
            error "Unable to build ViSUS.  ${VISUS_FILE} not found."
        fi
    fi
}

function bv_visus_dry_run
{
    if [[ "$DO_VISUS" == "yes" ]] ; then
        echo "Dry run option not set for ViSUS."
    fi
}

function install_visus
{
    if [[ "$DO_VISUS" == "no" ]] ; then
        return
    fi

    #
    # Prepare install dir
    #
    prepare_build_dir $VISUS_BUILD_DIR $VISUS_FILE
    untarred_visus=$?
    if [[ $untarred_visus == -1 ]] ; then
        warn "Unable to prepare visus install directory. Giving Up!"
        return 1
    fi
    
    #
    # install
    #
    info "Installing ViSUS . . . (a few seconds)"
    mkdir -p $VISUS_INSTALL_DIR
    mv $VISUS_BUILD_DIR/* $VISUS_INSTALL_DIR
    rmdir $VISUS_BUILD_DIR
    copied_visus=$?
    if [[ $copied_visus == -1 ]] ; then
        warn "Unable to install ViSUS. Giving Up!"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/visus"
        chgrp -R ${GROUP} "$VISITDIR/visus"
    fi

    cd "$START_DIR"
    info "Done installing ViSUS"
    return 0
}

function bv_visus_is_enabled
{
    if [[ $DO_VISUS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_visus_is_installed
{
    if [[ "$USE_SYSTEM_VISUS" == "yes" ]]; then
        return 1
    fi

    check_if_installed "visus" $VISUS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_visus_build
{
    cd "$START_DIR"
    if [[ "$DO_VISUS" == "yes" && "$USE_SYSTEM_VISUS" == "no" ]] ; then
        check_if_installed "visus" $VISUS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping ViSUS install. ViSUS is already installed."
        else
            install_visus
            if [[ $? != 0 ]] ; then
                error "Unable to install ViSUS.  Bailing out."
            fi
        fi
    fi
}
