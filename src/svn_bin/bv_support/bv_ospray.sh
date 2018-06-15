function bv_ospray_initialize
{
    export DO_OSPRAY="no"
}

function bv_ospray_enable
{
    DO_OSPRAY="yes"
}

function bv_ospray_disable
{
    DO_OSPRAY="no"
}

function bv_ospray_depends_on
{
    depends_on=""

    echo ${depends_on}
}

function bv_ospray_info
{
    export OSPRAY_VERSION=${OSPRAY_VERSION:-"1.6.1"}
    export OSPRAY_FILE=${OSPRAY_FILE:-"ospray-${OSPRAY_VERSION}.x86_64.linux.tar.gz"}
    export OSPRAY_BUILD_DIR=${OSPRAY_BUILD_DIR:-"ospray-${OSPRAY_VERSION}.x86_64.linux"}
    export OSPRAY_URL=${OSPRAY_URL:-"https://github.com/ospray/ospray/releases/download/v${OSPRAY_VERSION}/"}
    export OSPRAY_MD5_CHECKSUM=""
    export OSPRAY_SHA256_CHECKSUM=""
}

function bv_ospray_print
{
    print "%s%s\n" "OSPRAY_FILE=" "${OSPRAY_FILE}"
    print "%s%s\n" "OSPRAY_VERSION=" "${OSPRAY_VERSION}"
    print "%s%s\n" "OSPRAY_TARGET=" "${OSPRAY_TARGET}"
    print "%s%s\n" "OSPRAY_BUILD_DIR=" "${OSPRAY_BUILD_DIR}"
}

function bv_ospray_print_usage
{
    printf "%-15s %s [%s]\n" "--ospray" "Build OSPRay rendering support" "$DO_OSPRAY"
}

function bv_ospray_host_profile
{
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OSPRay" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY ON TYPE BOOL)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR \${VISITHOME}/ospray/$OSPRAY_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_ospray_is_enabled
{
    if [[ $DO_OSPRAY == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_ospray_ensure
{
    if [[ "$DO_OSPRAY" == "yes" ]]; then
        ensure_built_or_ready "ospray" \
            $OSPRAY_VERSION \
            $OSPRAY_BUILD_DIR \
            $OSPRAY_FILE \
            $OSPRAY_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}

function bv_ospray_initialize_vars
{
    info "initializing ospray vars"
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        OSPRAY_INSTALL_DIR="${VISITDIR}/ospray/${OSPRAY_VERSION}/${VISITARCH}"
        OSPRAY_INCLUDE_DIR="${OSPRAY_INSTALL_DIR}/include"
        OSPRAY_LIB_DIR="${OSPRAY_INSTALL_DIR}/lib"
        OSPRAY_LIB="${OSPRAY_LIB_DIR}/libospray.so"

        VTK_USE_OSPRAY="yes"
    fi
}

function bv_ospray_dry_run
{
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        echo "Dry run option not set for ospray."
    fi
}

function bv_ospray_is_installed
{
    check_if_installed "ospray" $OSPRAY_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function build_ospray
{
    prepare_build_dir $OSPRAY_BUILD_DIR $OSPRAY_FILE
    untarred_ospray=$?
    if [[ $untarred_ospray == -1 ]] ; then
        warn "Unable to prepare OSPRay build directory. Giving up!"
        return 1
    fi

    cd $OSPRAY_BUILD_DIR || error "Couldn't cd to OSPRay build dir."
    info "Installing OSPRay . . ."
    echo mkdir -p ${OSPRAY_INSTALL_DIR}
    mkdir -p ${OSPRAY_INSTALL_DIR}
    echo cp -a include lib ${OSPRAY_INSTALL_DIR}
    cp -a include lib ${OSPRAY_INSTALL_DIR}

    if [[  $? != 0 ]] ; then
        warn "OSPRay install failed. Giving up"
        return 1
    fi
    
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/ospray"
        chgrp -R ${GROUP} "$VISITDIR/ospray"
    fi
    cd "$START_DIR"
    info "Done with OSPRay"
    return 0
}

function bv_ospray_build
{
    cd "$START_DIR"
    if [[ "$DO_OSPRAY" == "yes" ]]; then
        check_if_installed "ospray" $OSPRAY_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OSPRay build. OSPRay is already installed."
        else
            info "Building OSPRay (~1 minute)"
            build_ospray
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OSPRay. Bailing out."
            fi
            info "Done building OSPRay"
        fi
    fi
}
