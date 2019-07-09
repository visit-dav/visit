function bv_tbb_initialize
{
    export DO_TBB="no"
    export USE_SYSTEM_TBB="no"
    add_extra_commandline_args "tbb" "alt-tbb-dir" 1 "Use alternative directory for tbb"
}

function bv_tbb_enable
{
    DO_TBB="yes"
}

function bv_tbb_disable
{
    DO_TBB="no"
}

function bv_tbb_alt_tbb_dir
{
    echo "Using alternate TBB directory"
    bv_tbb_enable
    USE_SYSTEM_TBB="yes"
    TBB_INSTALL_DIR="$1"
}

function bv_tbb_depends_on
{
    if [[ "$USE_SYSTEM_TBB" == "yes" ]] ; then
        echo ""
    else
        echo ""
    fi
}

function bv_tbb_initialize_vars
{
    info "initializing TBB vars"
    if [[ "$DO_TBB" == "yes" ]] ; then
        if [[ "$USE_SYSTEM_TBB" == "no" ]]; then
            TBB_INSTALL_DIR=$VISITDIR/tbb/$TBB_VERSION/$VISITARCH
        fi
    fi
    export TBB_ROOT="${TBB_INSTALL_DIR}"
}

function bv_tbb_info
{
    export TBB_VERSION=${TBB_VERSION:-"tbb2018_20171205oss"}
    if [[ "$OPSYS" == "Darwin" ]] ; then
        export TBB_FILE=${TBB_FILE:-"${TBB_VERSION}_mac.tgz"}
        export TBB_MD5_CHECKSUM="ff7a02f58fee4e2e637db6da19a21806"
        export TBB_SHA256_CHECKSUM="00955b15609298c13104c0b2a500757ea57a5e0ae9fe80a71ce20f29d76629ed"
    else
        export TBB_FILE=${TBB_FILE:-"${TBB_VERSION}_lin.tgz"}
        export TBB_MD5_CHECKSUM="d637d29f59ee31fe5830a0366e2e973a"
        export TBB_SHA256_CHECKSUM="7c2ec94f6f1c2b95293fc0fc7652e0fdeefabfc10cdb66f7176fbf6d99431fb6"
    fi
    export TBB_COMPATIBILITY_VERSION=${TBB_COMPATIBILITY_VERSION:-"${TBB_VERSION}"}
    export TBB_BUILD_DIR=${TBB_BUILD_DIR:-"${TBB_VERSION}"}
}

function bv_tbb_print
{
    printf "%s%s\n" "TBB_FILE=" "${TBB_FILE}"
    printf "%s%s\n" "TBB_VERSION=" "${TBB_VERSION}"
    printf "%s%s\n" "TBB_COMPATIBILITY_VERSION=" "${TBB_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "TBB_BUILD_DIR=" "${TBB_BUILD_DIR}"
}

function bv_tbb_host_profile
{
    if [[ "$DO_TBB" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## TBB" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        # Note by Qi
        #   bv_tbb.sh is not completely written by me. TBB_ROOT cmake variable is commonly used in many
        #   TBB related projects. I want to keep it set here since I am afraid of breaking other packages
        #   other than ospray
        if [[ "$USE_SYSTEM_TBB" == "no" ]]; then
            echo "VISIT_OPTION_DEFAULT(TBB_ROOT \${VISITHOME}/tbb/$TBB_VERSION/\${VISITARCH})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_TBB_DIR \${VISITHOME}/tbb/$TBB_VERSION/\${VISITARCH})" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(TBB_ROOT ${TBB_INSTALL_DIR})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_TBB_DIR ${TBB_INSTALL_DIR})" >> $HOSTCONF
        fi
    fi
}

function bv_tbb_print_usage
{
    #tbb does not have an option, it is only dependent on tbb.
    printf "%-20s %s [%s]\n" "--tbb" "Build TBB" "$DO_TBB"
}

function bv_tbb_ensure
{
    if [[ "$DO_TBB" == "yes" && "$USE_SYSTEM_TBB" == "no" ]] ; then
        ensure_built_or_ready "tbb" $TBB_VERSION $TBB_BUILD_DIR $TBB_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_TBB="no"
            error "Unable to build TBB.  ${TBB_FILE} not found."
        fi
    fi
}

function bv_tbb_dry_run
{
    if [[ "$DO_TBB" == "yes" ]] ; then
        echo "Dry run option not set for TBB."
    fi
}

# ***************************************************************************
# build_tbb
#
# Modifications:
#
# ***************************************************************************

function build_tbb
{
    # Unzip the TBB tarball and copy it to the VisIt installation.
    info "Installing prebuilt TBB"
    tar zxvf $TBB_FILE
    mkdir -p $VISITDIR/tbb/$TBB_VERSION/$VISITARCH || error "Cannot make tbb install directory"
    cp -R $TBB_VERSION/* $VISITDIR/tbb/$TBB_VERSION/$VISITARCH || error "Cannot copy to tbb install directory"

    # others
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/tbb/$TBB_VERSION/$VISITARCH"
        chgrp -R ${GROUP} "$VISITDIR/tbb/$TBB_VERSION/$VISITARCH"
    fi
    cd "$START_DIR"
    info "Done with TBB"
    return 0
}

function bv_tbb_is_enabled
{
    if [[ $DO_TBB == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_tbb_is_installed
{
    if [[ "$USE_SYSTEM_TBB" == "yes" ]]; then   
        return 1
    fi

    check_if_installed "tbb" $TBB_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_tbb_build
{
    if [[ "$DO_TBB" == "yes" && "$USE_SYSTEM_TBB" == "no" ]] ; then
        check_if_installed "tbb" $TBB_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of TBB"
        else
            build_tbb
            if [[ $? != 0 ]] ; then
                error "Unable to build or install TBB.  Bailing out."
            fi
            info "Done building TBB"
        fi
    fi
}

