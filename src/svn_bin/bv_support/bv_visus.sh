
function bv_visus_initialize
{
    export DO_VISUS="no"
    export ON_VISUS="off"
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
    ON_VISUS="on"
}

function bv_visus_disable
{
    DOVISUS_="no"
    ON_VISUS="off"
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
    #todo: add query system info to be used here to determine which file to download, and change build_dir to install_dir or something
    export VISUS_OS=`uname`
    export VISUS_VERSION=${VISUS_VERSION:-"ad09cb8"}
    export VISUS_FILE=${VISUS_FILE:-"ViSUS-${VISUS_VERSION}-${VISUS_OS}.tgz"}
    export VISUS_BUILD_DIR=${VISUS_BUILD_DIR:-"ViSUS"}
    export VISUS_URL=${VISUS_URL:-"http://atlantis.sci.utah.edu/builds/visit-plugin"}
}

function bv_visus_print
{
    printf "%s%s\n" "VISUS_FILE=" "${VISUS_FILE}"
    printf "%s%s\n" "VISUS_VERSION=" "${VISUS_VERSION}"
    printf "%s%s\n" "VISUS_BUILD_DIR=" "${VISUS_BUILD_DIR}"
}

function bv_visus_print_usage
{
    printf "%-15s %s [%s]\n" "--visus" "Build ViSUS support" "$DO_VISUS"
    printf "%-15s %s [%s]\n" "--alt-visus-dir"  "Use ViSUS" "Use ViSUS from alternative directory"
}

function bv_visus_graphical
{
    local graphical_out="visus     $VISUS_VERSION($VISUS_FILE)      $ON_VISUS"
    echo "$graphical_out"
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

