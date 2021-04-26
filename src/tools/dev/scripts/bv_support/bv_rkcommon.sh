function bv_rkcommon_initialize
{
    export DO_RKCOMMON="no"
    export USE_SYSTEM_RKCOMMON="no"
    add_extra_commandline_args "rkcommon" "alt-rkcommon-dir" 1 "Use alternative directory for rkcommon"
}

function bv_rkcommon_enable
{
    DO_RKCOMMON="yes"
}

function bv_rkcommon_disable
{
    DO_RKCOMMON="no"
}

function bv_rkcommon_alt_rkcommon_dir
{
    echo "Using alternate rkcommon directory"
    bv_rkcommon_enable
    USE_SYSTEM_RKCOMMON="yes"
    RKCOMMON_INSTALL_DIR="$1"
}

function bv_rkcommon_depends_on
{
    if [[ "$USE_SYSTEM_NEKTARPP" == "yes" ]]; then
        echo ""
    else
        echo "cmake"
    fi
}

function bv_rkcommon_initialize_vars
{
    info "Initializing rkcommon vars"

    if [[ "$USE_SYSTEM_RKCOMMON" == "no" ]]; then
        RKCOMMON_INSTALL_DIR=$VISITDIR/rkcommon/$RKCOMMON_VERSION/$VISITARCH
    fi
}

function bv_rkcommon_info
{
    export RKCOMMON_VERSION=${RKCOMMON_VERSION:-"1.6.0"}
    export RKCOMMON_FILE=${RKCOMMON_FILE:-"rkcommon-${RKCOMMON_VERSION}.tar.gz"}
    export RKCOMMON_COMPATIBILITY_VERSION=${RKCOMMON_COMPATIBILITY_VERSION:-"1.6"}
    export RKCOMMON_SRC_DIR=${RKCOMMON_SRC_DIR:-"rkcommon-${RKCOMMON_VERSION}"}
    export RKCOMMON_BUILD_DIR=${RKCOMMON_BUILD_DIR:-"${RKCOMMON_SRC_DIR}-build"}

    export RKCOMMON_MD5_CHECKSUM="9eaff694a0ae530e9dd8d89fda99316a"
    export RKCOMMON_SHA256_CHECKSUM="24d0c9c58a4d2f22075850df170ec5732cfaa0a16f22f90dbd6538232be009b0"
}

function bv_rkcommon_print
{
    printf "%s%s\n" "RKCOMMON_FILE=" "${RKCOMMON_FILE}"
    printf "%s%s\n" "RKCOMMON_VERSION=" "${RKCOMMON_VERSION}"
    printf "%s%s\n" "RKCOMMON_COMPATIBILITY_VERSION=" "${RKCOMMON_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "RKCOMMON_SRC_DIR=" "${RKCOMMON_SRC_DIR}"
    printf "%s%s\n" "RKCOMMON_BUILD_DIR=" "${RKCOMMON_BUILD_DIR}"
}

function bv_rkcommon_print_usage
{
    printf "%-20s %s [%s]\n" "--rkcommon"   "Build Rkcommon" "$DO_RKCOMMON"
}

function bv_rkcommon_host_profile
{
    if [[ "$DO_RKCOMMON" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Rkcommon" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_RKCOMMON" == "no" ]]; then
            echo "SETUP_APP_VERSION(RKCOMMON ${RKCOMMON_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(rkcommon_DIR \${VISITHOME}/rkcommon/\${RKCOMMON_VERSION}/\${VISITARCH}/lib/cmake/rkcommon-${RKCOMMON_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_RKCOMMON_DIR \${VISITHOME}/rkcommon/\${RKCOMMON_VERSION}/\${VISITARCH})" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_RKCOMMON_DIR ${RKCOMMON_INSTALL_DIR})" >> $HOSTCONF
        fi
    fi
}

function bv_rkcommon_ensure
{
    if [[ "$DO_RKCOMMON" == "yes" ]] ; then
        check_installed_or_have_src "rkcommon" $RKCOMMON_VERSION $RKCOMMON_BUILD_DIR $RKCOMMON_FILE $RKCOMMON_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_RKCOMMON="no"
            error "Unable to build Rkcommon. ${RKCOMMON_FILE} not found."
        fi
    fi
}

function bv_rkcommon_dry_run
{
    if [[ "$DO_RKCOMMON" == "yes" ]] ; then
        echo "Dry run option not set for Rkcommon."
    fi
}

function bv_rkcommon_is_enabled
{
    if [[ $DO_RKCOMMON == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_rkcommon_is_installed
{
    if [[ "$USE_SYSTEM_RKCOMMON" == "yes" ]]; then
        return 1
    fi

    check_if_installed "rkcommon" $RKCOMMON_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_rkcommon_build
{
    cd "$START_DIR"

    if [[ "$DO_RKCOMMON" == "yes" ]] ; then
        check_if_installed "rkcommon" $RKCOMMON_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Rkcommon build. Rkcommon is already installed."
        else
            info "Building Rkcommon (~5 minutes)"
            build_rkcommon
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Rkcommon. Bailing out."
            fi
            info "Done building Rkcommon"
        fi
    fi
}

# *************************************************************************** #
# build_rkcommon
# *************************************************************************** #

function build_rkcommon
{
    #
    # Uncompress the source file
    #
    uncompress_src_file $RKCOMMON_SRC_DIR $RKCOMMON_FILE
    untarred_rkcommon=$?
    if [[ $untarred_rkcommon == -1 ]] ; then
        warn "Unable to uncompress Rkcommon source file. Giving Up!"
        return 1
    fi

    #
    # Make a build directory for an out-of-source build.
    #
    cd "$START_DIR"
    if [[ ! -d $RKCOMMON_BUILD_DIR ]] ; then
        echo "Making build directory $RKCOMMON_BUILD_DIR"
        mkdir $RKCOMMON_BUILD_DIR
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf ${RKCOMMON_BUILD_DIR}/CMakeCache.txt ${RKCOMMON_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd ${RKCOMMON_BUILD_DIR}

    #
    # Call configure
    #
    info "Configuring Rkcommon . . ."

    cfg_opts=""
    # normal stuff
    cfg_opts="${cfg_opts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    cfg_opts="${cfg_opts} -DCMAKE_INSTALL_PREFIX:PATH=${RKCOMMON_INSTALL_DIR}"

    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    cfg_opts="${cfg_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    cfg_opts="${cfg_opts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    cfg_opts="${cfg_opts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS} ${PAR_LINKER_FLAGS}\""
    cfg_opts="${cfg_opts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS} ${PAR_LINKER_FLAGS}\""

    if test "${OPSYS}" = "Darwin" ; then
        cfg_opts="${cfg_opts} -DCMAKE_INSTALL_NAME_DIR:PATH=${RKCOMMON_INSTALL_DIR}/lib"
        if test "${MACOSX_DEPLOYMENT_TARGET}" = "10.10"; then
            # If building on 10.10 (Yosemite) check if we are building
            # with Xcode 7 ...
            XCODE_VER=$(xcodebuild -version | head -n 1 | awk '{print $2}')
            if test ${XCODE_VER%.*} == 7; then
                # Workaround for Xcode 7 not having a 10.10 SDK:
                # Prevent CMake from linking to 10.11 SDK by using
                # Frameworks installed in root directory.
                echo "Xcode 7 on MacOS 10.10 detected: Enabling CMake workaround"
                cfg_opts="${cfg_opts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=\"\" -DCMAKE_OSX_SYSROOT:STRING=/"
            fi
        fi
    fi

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command". This was first discovered on
    # BGQ and then showed up in random cases for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - we write a simple
    # script that we invoke with bash which calls cmake with all of the properly
    # arguments. We are now using this strategy for all platforms.
    #
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${cfg_opts} ../${RKCOMMON_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "Rkcommon configure failed. Giving up"
        return 1
    fi

    #
    # Build Rkcommon
    #
    info "Building Rkcommon . . . (~5 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Rkcommon build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing Rkcommon . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Rkcommon install failed. Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/rkcommon"
        chgrp -R ${GROUP} "$VISITDIR/rkcommon"
    fi

    cd "$START_DIR"
    info "Done with Rkcommon"
    return 0
}
