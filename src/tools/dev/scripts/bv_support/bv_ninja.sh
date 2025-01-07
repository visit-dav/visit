function bv_ninja_initialize
{
    export DO_NINJA="no"
    export USE_SYSTEM_NINJA="no"
    add_extra_commandline_args "ninja" "alt-ninja-dir" 1 "Use alternative directory for ninja"
}

function bv_ninja_enable
{
    DO_NINJA="yes"
}

function bv_ninja_disable
{
    DO_NINJA="no"
}

function bv_ninja_alt_ninja_dir
{
    bv_ninja_enable
    USE_SYSTEM_NINJA="yes"
    NINJA_INSTALL_DIR="$1"
    info "Using Alternate ninja: $NINJA_INSTALL_DIR"
}

function bv_ninja_depends_on
{
    depends_on="cmake"

    echo ${depends_on}
}

function bv_ninja_initialize_vars
{
    if [[ "$USE_SYSTEM_NINJA" == "no" ]]; then
        NINJA_INSTALL_DIR="\${VISITDIR}/ninja/$NINJA_VERSION/\${VISITARCH}"
    fi
}

function bv_ninja_info
{
    export NINJA_VERSION=${NINJA_VERSION:-"1.12.1"}
    export NINJA_FILE=${NINJA_FILE:-"ninja-${NINJA_VERSION}.tar.gz"}
    export NINJA_BUILD_DIR=${NINJA_BUILD_DIR:-"ninja-${NINJA_VERSION}"}
    export NINJA_SHA256_CHECKSUM="821bdff48a3f683bc4bb3b6f0b5fe7b2d647cf65d52aeb63328c91a6c6df285a"
}

function bv_ninja_print
{
    printf "%s%s\n" "NINJA_FILE=" "${NINJA_FILE}"
    printf "%s%s\n" "NINJA_VERSION=" "${NINJA_VERSION}"
    printf "%s%s\n" "NINJA_BUILD_DIR=" "${NINJA_BUILD_DIR}"
}

function bv_ninja_print_usage
{
    printf "%-20s %s [%s]\n" "--ninja" "Build ninja support" "$DO_NINJA"
    printf "%-20s %s [%s]\n" "--alt-ninja-dir" "Use ninja from an alternative directory"
}

function bv_ninja_host_profile
{
    # Nothing added to the host profile since ninja is only used for
    # building third party libraries.
    return 0
}

function bv_ninja_ensure
{
    if [[ "$DO_NINJA" == "yes" && "$USE_SYSTEM_NINJA" == "no" ]] ; then
        ensure_built_or_ready "ninja" $NINJA_VERSION $NINJA_BUILD_DIR $NINJA_FILE $NINJA_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_NINJA="no"
            error "Unable to build ninja. ${NINJA_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_ninja
#
#
# *************************************************************************** #
function build_ninja
{
    #
    # Extract the sources
    #
    if [[ -d $NINJA_BUILD_DIR ]] ; then
        if [[ ! -f $NINJA_FILE ]] ; then
            warn "The directory NINJA exists, deleting before uncompressing"
            rm -Rf $NINJA_BUILD_DIR
            ensure_built_or_ready $NINJA_INSTALL_DIR $NINJA_VERSION $NINJA_BUILD_DIR $NINJA_FILE
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $NINJA_BUILD_DIR $NINJA_FILE
    untarred_ninja=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ninja == -1 ]] ; then
        warn "Unable to prepare ninja build directory. Giving Up!"
        return 1
    fi

    #
    # Configure NINJA
    #
    info "Configuring ninja . . ."

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    cd $NINJA_BUILD_DIR || error "Can't cd to ninja build dir."

    vopts=""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/ninja/${NINJA_VERSION}/${VISITARCH}"
    vopts="${vopts} -DBUILD_TESTING:BOOL=OFF"
    vopts="${vopts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"

    #
    # Several platforms have had problems with the VTK cmake configure
    # command issued simply via "issue_command".  This was first discovered
    # on BGQ and then showed up in random cases for both OSX and Linux
    # machines. Brad resolved this on BGQ  with a simple work around - we
    # write a simple script that we invoke with bash which calls cmake with
    # all of the properly arguments. We are now using this strategy for all
    # platforms.
    #
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${vopts} . > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "ninja configuration failed."

    #
    # Build ninja
    #
    info "Building ninja . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS || error "Ninja did not build correctly. Giving up."

    info "Installing Ninja . . . (~2 minutes)"
    $MAKE install || error "Ninja did not install correctly."

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/ninja"
        chgrp -R ${GROUP} "$VISITDIR/ninja"
    fi
    cd "$START_DIR"
    info "Done with ninja"
    return 0
}

function bv_ninja_is_enabled
{
    if [[ $DO_NINJA == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_ninja_is_installed
{
    if [[ "$USE_SYSTEM_NINJA" == "yes" ]]; then
        return 1
    fi

    check_if_installed "ninja" $NINJA_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_ninja_build
{
    cd "$START_DIR"
    if [[ "$DO_NINJA" == "yes" && "$USE_SYSTEM_NINJA" == "no" ]] ; then
        check_if_installed "ninja" $NINJA_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping ninja build. Ninja is already installed."
        else
            info "Building ninja (~2 minutes)"
            build_ninja
            if [[ $? != 0 ]] ; then
                error "Unable to build or install ninja.  Bailing out."
            fi
            info "Done building ninja"
        fi
    fi
}
