function bv_fms_initialize
{
    export DO_FMS="no"
}

function bv_fms_enable
{
    DO_FMS="yes"
}

function bv_fms_disable
{
    DO_FMS="no"
}

function bv_fms_depends_on
{
    local depends_on=""

    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        depends_on="$depends_on conduit"
    fi

    echo $depends_on
}

function bv_fms_info
{
    export FMS_VERSION=${FMS_VERSION:-"0.2"}
    export FMS_FILE=${FMS_FILE:-"FMS-${FMS_VERSION}.tar.gz"}
    export FMS_BUILD_DIR=${FMS_BUILD_DIR:-"FMS-${FMS_VERSION}"}
    export FMS_SHA256_CHECKSUM="872489a1325b247968dbb7265b8736660af94121a86c93f7938441ce7478183e"
}

function bv_fms_print
{
    printf "%s%s\n" "FMS_FILE=" "${FMS_FILE}"
    printf "%s%s\n" "FMS_VERSION=" "${FMS_VERSION}"
    printf "%s%s\n" "FMS_BUILD_DIR=" "${FMS_BUILD_DIR}"
}

function bv_fms_print_usage
{
    printf "%-20s %s [%s]\n" "--fms" "Build FMS support" "$DO_FMS"
}

function bv_fms_host_profile
{
    if [[ "$DO_FMS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## FMS " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_FMS_DIR \${VISITHOME}/fms/$FMS_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        if [[ "$DO_CONDUIT" == "yes" ]] ; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_FMS_LIBDEP CONDUIT_LIBRARY_DIR conduit CONDUIT_LIBRARY_DIR conduit_blueprint CONDUIT_LIBRARY_DIR conduit_relay \${VISIT_CONDUIT_LIBDEP} TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_fms_ensure
{
    if [[ "$DO_FMS" == "yes" ]] ; then
        ensure_built_or_ready "fms" $FMS_VERSION $FMS_BUILD_DIR $FMS_FILE $FMS_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_FMS="no"
            error "Unable to build FMS.  ${FMS_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_fms
# *************************************************************************** #
function build_fms
{
    #
    # Prepare build dir
    #
    prepare_build_dir $FMS_BUILD_DIR $FMS_FILE
    untarred_fms=$?
    if [[ $untarred_fms == -1 ]] ; then
        warn "Unable to prepare FMS build directory. Giving Up!"
        return 1
    fi

    cd $FMS_BUILD_DIR || error "Can't cd to FMS source dir."
    mkdir build
    cd build || error "Can't cd to FMS build dir."

    vopts="-DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/fms/${FMS_VERSION}/${VISITARCH}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        vopts="${vopts} -DCONDUIT_DIR=${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}"
    fi

    #
    # Call configure
    #
    info "Configuring FMS . . ."
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\" ${vopts} .." > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "FMS configuration failed."

    #
    # Build FMS
    #
    info "Building FMS . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "FMS build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing FMS"
    $MAKE install

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/fms"
        chgrp -R ${GROUP} "$VISITDIR/fms"
    fi
    cd "$START_DIR"    
    info "Done with FMS"
    return 0
}


function bv_fms_is_enabled
{
    if [[ $DO_FMS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_fms_is_installed
{
    check_if_installed "fms" $FMS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_fms_build
{
    cd "$START_DIR"
    if [[ "$DO_FMS" == "yes" ]] ; then
        check_if_installed "fms" $FMS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping FMS build.  FMS is already installed."
        else
            info "Building FMS (~2 minutes)"
            build_fms
            if [[ $? != 0 ]] ; then
                error "Unable to build or install FMS.  Bailing out."
            fi
            info "Done building FMS"
        fi
    fi
}
