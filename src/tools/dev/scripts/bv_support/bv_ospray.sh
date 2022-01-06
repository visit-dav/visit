function bv_ospray_initialize
{
    export DO_OSPRAY="no"
    export USE_SYSTEM_OSPRAY="no"
    export OSPRAY_CONFIG_DIR=""
    add_extra_commandline_args "ospray" "system-ospray" 0 "Using system OSPRay (exp)"
    add_extra_commandline_args "ospray" "alt-ospray-dir" 1 "Use alternate OSPRay (exp)"
}

function bv_ospray_enable
{
    DO_OSPRAY="yes"
}

function bv_ospray_disable
{
    DO_OSPRAY="no"
}

function bv_ospray_system_ospray
{
    TEST=`which ospray-config`
    [ $? != 0 ] && error "System ospray-config not found, cannot configure ospray"

    bv_ospray_enable
    USE_SYSTEM_OSPRAY="yes"
    OSPRAY_INSTALL_DIR="$1"
    info "Using System OSPRAY: $OSPRAY_INSTALL_DIR"
}

function bv_ospray_alt_ospray_dir
{
    bv_ospray_enable
    USE_SYSTEM_OSPRAY="yes"
    OSPRAY_INSTALL_DIR="$1"
    info "Using Alternate OSPRAY: $OSPRAY_INSTALL_DIR"
}

function bv_ospray_depends_on
{
    depends_on="cmake"

    echo ${depends_on}
}

function bv_ospray_initialize_vars
{
    if [[ "$USE_SYSTEM_OSPRAY" == "no" ]]; then
        OSPRAY_INSTALL_DIR="${VISITDIR}/ospray/${OSPRAY_VERSION}/${VISITARCH}"
    fi
}

function bv_ospray_info
{
    # versions
    export OSPRAY_VERSION=${OSPRAY_VERSION:-"2.8.0"}
    export OSPRAY_FILE=${OSPRAY_FILE:-"ospray-${OSPRAY_VERSION}.tar.gz"}
    export OSPRAY_SRC_DIR=${OSPRAY_SRC_DIR:-"${OSPRAY_FILE%.tar*}"}
    export OSPRAY_BUILD_DIR=${OSPRAY_BUILD_DIR:-"${OSPRAY_SRC_DIR}-build"}
    export OSPRAY_URL=${OSPRAY_URL:-"https://github.com/ospray/OSPRay/archive/v${OSPRAY_VERSION}"}
#    export OSPRAY_MD5_CHECKSUM="1654f0582de2443db0b717986d82fbbe"
#    export OSPRAY_SHA256_CHECKSUM="074bfd83b5a554daf8da8d9b778b6ef1061e54a1688eac13e0bdccf95593883d"
}

function bv_ospray_print
{
    print "%s%s\n" "OSPRAY_FILE=" "${OSPRAY_FILE}"
    print "%s%s\n" "OSPRAY_VERSION=" "${OSPRAY_VERSION}"
    print "%s%s\n" "OSPRAY_COMPATIBILITY_VERSION=" "${OSPRAY_COMPATIBILITY_VERSION}"
    print "%s%s\n" "OSPRAY_SRC_DIR=" "${OSPRAY_SRC_DIR}"
    print "%s%s\n" "OSPRAY_BUILD_DIR=" "${OSPRAY_BUILD_DIR}"
}

function bv_ospray_print_usage
{
    printf "%-20s %s\n" "--ospray" "Build OSPRAY"
    printf "%-20s %s [%s]\n" "--system-ospray" "Use the system installed OSPRAY"
    printf "%-20s %s [%s]\n" "--alt-ospray-dir" "Use OSPRAY from an alternative directory"
}

function bv_ospray_host_profile
{
    if [[ "$DO_OSPRAY" == "yes" ]]; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OSPRay" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY ON TYPE BOOL)" >> $HOSTCONF
        if [[ "$USE_SYSTEM_OSPRAY" == "no" ]]; then
            echo "SETUP_APP_VERSION(OSPRAY ${OSPRAY_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR \${VISITHOME}/ospray/\${OSPRAY_VERSION}/\${VISITARCH})" >> $HOSTCONF
        else
            local _tmp_=$(basename ${OSPRAY_CONFIG_DIR})
            echo "SETUP_APP_VERSION(OSPRAY ${_tmp_:7})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR ${OSPRAY_INSTALL_DIR})" >> $HOSTCONF
        fi
    fi
}

function bv_ospray_ensure
{
    if [[ "$DO_OSPRAY" == "yes" && "$USE_SYSTEM_OSPRAY" == "no" ]] ; then
        ensure_built_or_ready "ospray" $OSPRAY_VERSION $OSPRAY_BUILD_DIR $OSPRAY_FILE $OSPRAY_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OSPRAY="no"
            error "Unable to build ospray. ${OSPRAY_FILE} not found."
        fi
    fi
}

function bv_ospray_dry_run
{
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        echo "Dry run option not set for ospray."
    fi
}

function bv_ospray_is_enabled
{
    if [[ $DO_OSPRAY == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_ospray_is_installed
{
    if [[ "$USE_SYSTEM_OSPRAY" == "yes" ]]; then
        return 1
    fi

    check_if_installed "ospray" $OSPRAY_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_ospray_build
{
    cd "$START_DIR"

    if [[ "$DO_OSPRAY" == "yes" && "$USE_SYSTEM_OSPRAY" == "no" ]] ; then
        check_if_installed "ospray" $OSPRAY_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OSPRAY build. OSPRAY is already installed."
        else
            info "Building OSPRAY (~10 minutes)"
            build_ospray
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OSPRAY. Bailing out."
            fi
            info "Done building OSPRAY"
        fi
    fi
}

# *************************************************************************** #
#              Function 8.1, build_ospray                                     #
# *************************************************************************** #
function build_ospray
{
    #
    # Uncompress the source file
    #
    prepare_build_dir $OSPRAY_SRC_DIR $OSPRAY_FILE
    untarred_ospray=$?
    if [[ $untarred_ospray == -1 ]] ; then
        warn "Unable to uncompress OSPRay source file. Giving Up!"
        return 1
    fi

    #
    # Make a build directory for an out-of-source build.
    #
    cd "$START_DIR"
    if [[ ! -d $OSPRAY_BUILD_DIR ]] ; then
        echo "Making build directory $OSPRAY_BUILD_DIR"
        mkdir $OSPRAY_BUILD_DIR
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf ${OSPRAY_BUILD_DIR}/CMakeCache.txt ${OSPRAY_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd ${OSPRAY_BUILD_DIR}

    #
    # Configure OSPRAY
    #
    info "Configuring OSPRAY . . ."

    # set compiler if the user hasn't explicitly set CC and CXX
    if [ -z $CC ]; then
        echo "***NOTE: using compiler $C_COMPILER/$CXX_COMPILER!"
        export CC=$C_COMPILER
        export CXX=$CXX_COMPILER
    fi

    CMAKE_VARS=""
    CMAKE_VARS="${CMAKE_VARS} -DCMAKE_INSTALL_PREFIX:PATH=${OSPRAY_INSTALL_DIR}"

    #
    # Several platforms have had problems with the cmake configure
    # command issued simply via "issue_command". This was first
    # discovered on BGQ and then showed up in random cases for both
    # OSX and Linux machines.  Brad resolved this on BGQ with a simple
    # work around - we write a simple script that we invoke with bash
    # which calls cmake with all of the properly arguments. We are now
    # using this strategy for all platforms.
    #
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${CMAKE_VARS} ../${OSPRAY_SRC_DIR}/scripts/superbuild > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "OSPRAY configure failed. Giving up"
        return 1
    fi

    #
    # Now build OSPRAY.
    #
    info "Building OSPRAY . . . (~5 minutes)"
    #    $MAKE $MAKE_OPT_FLAGS
    ${CMAKE_BIN} --build .

    if [[ $? != 0 ]] ; then
        warn "OSPRAY build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #

    # No need to install as the cmake build does that.

#    info "Installing OSPRAY . . . "
#    $MAKE install
#    if [[ $? != 0 ]] ; then
#        warn "OSPRAY install failed. Giving up"
#        return 1
#    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/ospray"
        chgrp -R ${GROUP} "$VISITDIR/ospray"
    fi

    cd "$START_DIR"
    info "Done with OSPRAY"
    return 0
}
