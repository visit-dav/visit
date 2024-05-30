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

function bv_ospray_check_openmp
{
    _OPENMP=$(echo | cpp -fopenmp -dM | grep -i open)
    if [[ "$_OPENMP" == "#define _OPENMP"* ]]; then
        return 0
    fi
    return -1
}

function bv_ospray_depends_on
{
    depends_on="cmake"

    echo ${depends_on}
}

function bv_ospray_info
{
    export OSPRAY_VERSION=${OSPRAY_VERSION:-"3.0.0"}
    export OSPRAY_FILE=${OSPRAY_FILE:-"ospray-${OSPRAY_VERSION}.tar.gz"}
    export OSPRAY_SRC_DIR=${OSPRAY_SRC_DIR:-"${OSPRAY_FILE%.tar*}"}
    export OSPRAY_BUILD_DIR=${OSPRAY_BUILD_DIR:-"${OSPRAY_SRC_DIR}-build"}
    export OSPRAY_URL=${OSPRAY_URL:-"https://github.com/ospray/OSPRay/archive/v${OSPRAY_VERSION}"}
    export OSPRAY_SHA256_CHECKSUM="d8d8e632d77171c810c0f38f8d5c8387470ca19b75f5b80ad4d3d12007280288"
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
        if [[ "$USE_SYSTEM_OSPRAY" == "no" ]]; then
            echo "SETUP_APP_VERSION(OSPRAY ${OSPRAY_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR \${VISITHOME}/ospray/\${OSPRAY_VERSION}/\${VISITARCH}/ospray)" >> $HOSTCONF
        else
            local _tmp_=$(basename ${OSPRAY_CONFIG_DIR})
            echo "SETUP_APP_VERSION(OSPRAY ${_tmp_:7})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR ${OSPRAY_INSTALL_DIR})" >> $HOSTCONF
        fi
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
    if [[ "$DO_OSPRAY" == "yes" && "$USE_SYSTEM_OSPRAY" == "no" ]]; then
        ensure_built_or_ready "ospray" $OSPRAY_VERSION $OSPRAY_BUILD_DIR $OSPRAY_FILE $OSPRAY_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OSPRAY="no"
            error "Unable to build ospray. ${OSPRAY_FILE} not found."
        fi
    fi
}

function bv_ospray_initialize_vars
{
    if [[ "$USE_SYSTEM_OSPRAY" == "no" ]]; then
        OSPRAY_INSTALL_DIR="${VISITDIR}/ospray/${OSPRAY_VERSION}/${VISITARCH}"
    fi
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

function build_ospray_in_source
{
    # set compiler if the user hasn't explicitly set CC and CXX
    if [ -z $CC ]; then
        echo "***NOTE: using compiler $C_COMPILER/$CXX_COMPILER!"
        export CC=$C_COMPILER
        export CXX=$CXX_COMPILER
    fi

    #### Build OSPRay ####
    mkdir -p build
    cd build

    # Clean out build directory to be sure we are doing a fresh build
    rm -rf *

    # set release and RPM settings
    info "Configure OSPRay . . . "
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}

    CMAKE_VARS=""
    CMAKE_VARS=${CMAKE_VARS}" -D CMAKE_INSTALL_PREFIX=${OSPRAY_INSTALL_DIR} "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_BUILD_ISA=ALL "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_MODULE_VISIT=ON "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_MODULE_MPI=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_MODULE_MPI_APPS=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_APPS_EXAMPLEVIEWER=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_APPS_BENCHMARK=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_SG_CHOMBO=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_SG_OPENIMAGEIO=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_SG_VTK=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_ZIP_MODE=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D embree_DIR=${EMBREE_INSTALL_DIR} "
    CMAKE_VARS=${CMAKE_VARS}" -D ISPC_EXECUTABLE=${ISPC_INSTALL_DIR}/ispc "
    if [[ "${TBB_INSTALL_DIR}" == "" ]]; then
        bv_ospray_check_openmp
        if [[ $? == 0 ]]; then
            CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_TASKING_SYSTEM=OpenMP "
        else
            error "OSPRay cannot find neither TBB nor OpenMP."
        fi
    else
        CMAKE_VARS=${CMAKE_VARS}" -D TBB_ROOT=${TBB_INSTALL_DIR} "
    fi
    ${CMAKE_INSTALL}/cmake ${CMAKE_VARS} \
        .. || error "OSPRay did not configure correctly.  Giving up."

    #
    # Now build OSPRay
    #
    info "Building OSPRay (~10 minute)"
    env DYLD_LIBRARY_PATH=`pwd`/bin $MAKE $MAKE_OPT_FLAGS || \
        error "OSPRay did not build correctly.  Giving up."

    info "Installing OSPRay . . . "
    $MAKE install || error "OSPRay did not install correctly."
}

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
    CMAKE_VARS="${CMAKE_VARS} -DCMAKE_INSTALL_PREFIX:PATH=${OSPRAY_INSTALL_DIR} -DBUILD_OIDN:BOOL=OFF -DBUILD_OSPRAY_APPS:BOOL=OFF -DBUILD_GLFW:BOOL=OFF -DBUILD_BENCHMARK:BOOL=OFF"

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

    #
    # On Darwin, the build can fail in a cmake -E copy_directory due to
    # bad symlinks created fron unzipping a data file containing binary
    # tbb libs. So, we try to fix those and re-run the build a second time.
    #
    if [[ $? != 0 ]]; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            pushd embree/src/lib 1>/dev/null 2>&1
            rm -f libtbb.a libtbb.dylib libtbb.12.dylib libtbb.12.5.dylib
            ln -sf ../../../tbb/src/lib/libtbb.12.10.dylib .
            ln -sf ../../../tbb/src/lib/libtbb.12.dylib .
            ln -sf ../../../tbb/src/lib/libtbb.dylib .
            popd 1>/dev/null 2>&1
            ${CMAKE_BIN} --build .
            if [[ $? != 0 ]] ; then
                warn "OSPRAY build failed. Giving up"
                return 1
            fi
        else
            warn "OSPRAY build failed. Giving up"
            return 1
        fi
    fi

    #
    # Install into the VisIt third party location.
    #

    # No need to install as the cmake build does that.

    if [[ "$DO_GROUP" == "yes" ]]; then
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
    if [[ "$DO_OSPRAY" == "yes" && "$USE_SYSTEM_OSPRAY" == "no" ]]; then
        check_if_installed "ospray" $OSPRAY_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OSPRay build. OSPRay is already installed."
        else
            info "Building OSPRAY (~10 minutes)"
            build_ospray
            if [[ $? != 0 ]]; then
                error "Unable to build or install OSPRay. Bailing out."
            fi
            info "Done building OSPRAY"
        fi
    fi
}

