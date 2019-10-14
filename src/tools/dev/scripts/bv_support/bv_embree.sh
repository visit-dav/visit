function bv_embree_initialize
{
    export DO_EMBREE="no"
    export USE_SYSTEM_EMBREE="no"
    add_extra_commandline_args "embree" "alt-embree-dir" 1 "Use alternative directory for embree"
}

function bv_embree_enable
{
    DO_EMBREE="yes"
}

function bv_embree_disable
{
    DO_EMBREE="no"
}

function bv_embree_alt_embree_dir
{
    echo "Using alternate embree directory"
    bv_embree_enable
    USE_SYSTEM_EMBREE="yes"
    EMBREE_INSTALL_DIR="$1"
}

function bv_embree_depends_on
{
    echo "tbb"
}

function bv_embree_initialize_vars
{
    info "initializing embree vars"
    if [[ "$DO_EMBREE" == "yes" ]] ; then
        if [[ "$USE_SYSTEM_EMBREE" == "no" ]]; then
            EMBREE_INSTALL_DIR=$VISITDIR/embree/$EMBREE_VERSION/$VISITARCH
        fi
    fi
}

function bv_embree_info
{
    export EMBREE_VERSION=${EMBREE_VERSION:-"3.2.0"}
    if [[ "$OPSYS" == "Darwin" ]] ; then
        export EMBREE_FILE=${EMBREE_FILE:-"embree-${EMBREE_VERSION}.x86_64.macosx.tar.gz"}
        export EMBREE_INSTALL_DIR_NAME=embree-$EMBREE_VERSION.x86_64.macosx
        # these are binary builds, not source tarballs so the mdf5s and shas differ 
        # between platforms 
        export EMBREE_MD5_CHECKSUM="8a3874975f1883d8df1714b3ba3eacba"
        export EMBREE_SHA256_CHECKSUM="31cbbe96c6f19bb9c5463e181070bd667d3dbb93e702671e8406ce26be259109"
    else
        export EMBREE_FILE=${EMBREE_FILE:-"embree-${EMBREE_VERSION}.x86_64.linux.tar.gz"}
        export EMBREE_INSTALL_DIR_NAME=embree-$EMBREE_VERSION.x86_64.linux
        # these are binary builds, not source tarballs so the mdf5s and shas differ 
        # between platforms 
        export EMBREE_MD5_CHECKSUM="7a1c3d12e8732cfee7d389f81d008798"
        export EMBREE_SHA256_CHECKSUM="7671cc37c4dc4e3da00b2b299b906b35816f058efea92701e7b89574b15e652d"
    fi
    export EMBREE_COMPATIBILITY_VERSION=${EMBREE_COMPATIBILITY_VERSION:-"${EMBREE_VERSION}"}
    export EMBREE_URL=${EMBREE_URL:-"https://github.com/embree/embree/releases/download/v${EMBREE_VERSION}/"}
    export EMBREE_BUILD_DIR=${EMBREE_BUILD_DIR:-"${EMBREE_VERSION}"}
}

function bv_embree_print
{
    printf "%s%s\n" "EMBREE_FILE=" "${EMBREE_FILE}"
    printf "%s%s\n" "EMBREE_VERSION=" "${EMBREE_VERSION}"
    printf "%s%s\n" "EMBREE_COMPATIBILITY_VERSION=" "${EMBREE_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "EMBREE_BUILD_DIR=" "${EMBREE_BUILD_DIR}"
}

function bv_embree_host_profile
{
    if [[ "$DO_EMBREE" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## EMBREE" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_EMBREE" == "no" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_EMBREE_DIR \${VISITHOME}/embree/$EMBREE_VERSION/\${VISITARCH})" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_EMBREE_DIR ${EMBREE_INSTALL_DIR})" >> $HOSTCONF
        fi
    fi
}

function bv_embree_print_usage
{
    #embree does not have an option, it is only dependent on embree.
    printf "%-20s %s [%s]\n" "--embree" "Build embree" "$DO_EMBREE"
}

function bv_embree_ensure
{
    if [[ "$DO_EMBREE" == "yes" && "$USE_SYSTEM_EMBREE" == "no" ]] ; then
        ensure_built_or_ready "embree" $EMBREE_VERSION $EMBREE_BUILD_DIR $EMBREE_FILE $EMBREE_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_EMBREE="no"
            error "Unable to build embree.  ${EMBREE_FILE} not found."
        fi
    elif [[ "$USE_SYSTEM_EMBREE" == "yes" ]] ; then
        if [[ ! -d $EMBREE_INSTALL_DIR/include/embree3 ]]; then
            error "Unable to find embree v3.+ in the alternative path, perhaps a wrong embree version is provided."
        fi
    fi
}

function bv_embree_dry_run
{
    if [[ "$DO_EMBREE" == "yes" ]] ; then
        echo "Dry run option not set for embree."
    fi
}

# ***************************************************************************
# build_embree
#
# Modifications:
#
# ***************************************************************************

function build_embree
{
    # Unzip the EMBREE tarball and copy it to the VisIt installation.
    info "Installing prebuilt embree"    
    tar zxvf $EMBREE_FILE
    rm $EMBREE_INSTALL_DIR_NAME/lib/libtbb*
    mkdir -p $VISITDIR/embree/$EMBREE_VERSION/$VISITARCH || error "Cannot create embree install directory"
    cp -R $EMBREE_INSTALL_DIR_NAME/* $VISITDIR/embree/$EMBREE_VERSION/$VISITARCH || error "Cannot copy to embree install directory"
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/embree/$EMBREE_VERSION/$VISITARCH"
        chgrp -R ${GROUP} "$VISITDIR/embree/$EMBREE_VERSION/$VISITARCH"
    fi
    cd "$START_DIR"
    info "Done with embree"
    return 0
}

function bv_embree_is_enabled
{
    if [[ $DO_EMBREE == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_embree_is_installed
{
    if [[ "$USE_SYSTEM_EMBREE" == "yes" ]]; then   
        return 1
    fi

    check_if_installed "embree" $EMBREE_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_embree_build
{
    if [[ "$DO_EMBREE" == "yes" && "$USE_SYSTEM_EMBREE" == "no" ]] ; then
        check_if_installed "embree" $EMBREE_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of embree"
        else
            build_embree
            if [[ $? != 0 ]] ; then
                error "Unable to build or install embree.  Bailing out."
            fi
            info "Done building embree"
        fi
    fi
}

