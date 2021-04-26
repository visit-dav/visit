# Module automatically read in from construct_build_visit_module
# Insert header and comments

function bv_conduit_initialize
{
    export DO_CONDUIT="no"
    export USE_SYSTEM_CONDUIT="no"
    add_extra_commandline_args "conduit" "system-conduit" 0 "Using system conduit"
    add_extra_commandline_args "conduit" "alt-conduit-dir" 1 "Use alternate conduit"
}

function bv_conduit_enable
{
    DO_CONDUIT="yes"
}

function bv_conduit_disable
{
    DO_CONDUIT="no"
}

function bv_conduit_system_conduit
{
#    TEST=`which conduit-config`
#    [ $? != 0 ] && error "System conduit-config not found, cannot configure conduit"

    bv_conduit_enable
    USE_SYSTEM_CONDUIT="yes"
    CONDUIT_INSTALL_DIR="$1"
    info "Using System conduit: ${CONDUIT_INSTALL_DIR}"
}

function bv_conduit_alt_conduit_dir
{
    bv_conduit_enable
    USE_SYSTEM_CONDUIT="yes"
    CONDUIT_INSTALL_DIR="$1"
    info "Using Alternate conduit: ${CONDUIT_INSTALL_DIR}"
}

function bv_conduit_depends_on
{
    local depends_on="cmake"

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="$depends_on hdf5"
    fi

    if [[ "$DO_PYTHON" == "yes" ]] ; then
        depends_on="$depends_on python"
    fi

    if [[ "$DO_MPICH" == "yes" ]] ; then
        depends_on="$depends_on mpich"
    fi

    echo ${depends_on}
}

function bv_conduit_initialize_vars
{
    if [[ "${USE_SYSTEM_CONDUIT}" == "no" ]]; then
        CONDUIT_INSTALL_DIR="${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}"
    fi
}

function bv_conduit_info
{
    export CONDUIT_VERSION=${CONDUIT_VERSION:-"v0.7.1"}
    export CONDUIT_FILE=${CONDUIT_FILE:-"conduit-${CONDUIT_VERSION}-src-with-blt.tar.gz"}
    export CONDUIT_COMPATIBILITY_VERSION=${CONDUIT_COMPATIBILITY_VERSION:-"v0.7.1"}
    export CONDUIT_SRC_DIR=${CONDUIT_SRC_DIR:-"conduit-${CONDUIT_VERSION}"}
    export CONDUIT_BUILD_DIR=${CONDUIT_BUILD_DIR:-"${CONDUIT_SRC_DIR}-build"}

    export CONDUIT_MD5_CHECKSUM="7f9bb79ef0a6bf42fa9bc05af3829e5b"
    export CONDUIT_SHA256_CHECKSUM="460a480cf08fedbf5b38f707f94f20828798327adadb077f80dbab048fd0a07d"
}

function bv_conduit_print
{
    printf "%s%s\n" "CONDUIT_FILE=" "${CONDUIT_FILE}"
    printf "%s%s\n" "CONDUIT_VERSION=" "${CONDUIT_VERSION}"
    printf "%s%s\n" "CONDUIT_COMPATIBILITY_VERSION=" "${CONDUIT_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CONDUIT_SRC_DIR=" "${CONDUIT_SRC_DIR}"
    printf "%s%s\n" "CONDUIT_BUILD_DIR=" "${CONDUIT_BUILD_DIR}"
}

function bv_conduit_print_usage
{
    printf "%-20s %s [%s]\n" "--conduit" "Build conduit support" "${DO_CONDUIT}"
    printf "%-20s %s [%s]\n" "--no-conduit" "Prevent conduit from being built"
    printf "%-20s %s [%s]\n" "--system-conduit" "Use the system installed conduit"
    printf "%-20s %s [%s]\n" "--alt-conduit-dir" "Use conduit from an alternative directory"
}

function bv_conduit_host_profile
{
    if [[ "${DO_CONDUIT}" == "yes" ]]; then
        echo >> ${HOSTCONF}
        echo "##" >> ${HOSTCONF}
        echo "## conduit" >> ${HOSTCONF}
        echo "##" >> ${HOSTCONF}

        if [[ "${USE_SYSTEM_CONDUIT}" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_CONDUIT_DIR ${CONDUIT_INSTALL_DIR})" >> ${HOSTCONF}
        else
            echo "SETUP_APP_VERSION(conduit ${CONDUIT_VERSION})" >> ${HOSTCONF}
            echo "VISIT_OPTION_DEFAULT(VISIT_CONDUIT_DIR \${VISITHOME}/conduit/\${CONDUIT_VERSION}/\${VISITARCH})" >> ${HOSTCONF}
            if [[ "$DO_HDF5" == "yes" ]] ; then
                echo "VISIT_OPTION_DEFAULT(VISIT_CONDUIT_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" >> $HOSTCONF
            fi
        fi
    fi
}

function bv_conduit_ensure
{
    if [[ "${DO_CONDUIT}" == "yes" && "${USE_SYSTEM_CONDUIT}" == "no" ]] ; then
        check_installed_or_have_src "conduit" ${CONDUIT_VERSION} ${CONDUIT_BUILD_DIR} ${CONDUIT_FILE} ${CONDUIT_URL}
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CONDUIT="no"
            error "Unable to build conduit. ${CONDUIT_FILE} not found."
        fi
    fi
}

function bv_conduit_dry_run
{
    if [[ "${DO_CONDUIT}" == "yes" ]] ; then
        echo "Dry run option not set for conduit"
    fi
}

function bv_conduit_is_enabled
{
    if [[ "${DO_CONDUIT}" == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_conduit_is_installed
{
    if [[ "${USE_SYSTEM_CONDUIT}" == "yes" ]]; then
        return 1
    fi

    check_if_installed "conduit" ${CONDUIT_VERSION}
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_conduit_build
{
    cd "${START_DIR}"

    if [[ "${DO_CONDUIT}" == "yes" && "${USE_SYSTEM_CONDUIT}" == "no" ]] ; then
        check_if_installed "conduit" ${CONDUIT_VERSION}
        if [[ $? == 0 ]] ; then
            info "Skipping conduit build. conduit is already installed."
        else
            info "Building conduit (~20 minutes)"
            build_conduit
            if [[ $? != 0 ]] ; then
                error "Unable to build or install conduit. Bailing out."
            fi
            info "Done building conduit"
        fi
    fi
}

function apply_conduit_patch
{
    cd "${CONDUIT_SRC_DIR}" || error "Can't cd to conduit source dir."

#    info "Patching conduit . . ."

#    apply_xyz_patch
#    if [[ $? != 0 ]] ; then
#        return 1
#    fi

    cd "${START_DIR}"

    return 0
}

function build_conduit
{
    #
    # Uncompress the source file
    #
    uncompress_src_file ${CONDUIT_SRC_DIR} ${CONDUIT_FILE}
    untarred_conduit=$?
    if [[ ${untarred_conduit} == -1 ]] ; then
        warn "Unable to uncompress conduit source file. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    apply_conduit_patch
    if [[ $? != 0 ]] ; then
        if [[ ${untarred_conduit} == 1 ]] ; then
            warn "Giving up on conduit build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing. It is believed that this\n" \
                 "script tried to apply a patch to an existing directory\n" \
                 "that had already been patched. That is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Make a build directory for an out-of-source build.
    #
    cd "${START_DIR}"
    if [[ ! -d ${CONDUIT_BUILD_DIR} ]] ; then
        echo "Making build directory ${CONDUIT_BUILD_DIR}"
        mkdir ${CONDUIT_BUILD_DIR}
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf ${CONDUIT_BUILD_DIR}/CMakeCache.txt ${CONDUIT_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd "${CONDUIT_BUILD_DIR}"

    #
    # Configure conduit
    #
    info "Configuring conduit . . ."

    cfg_opts=""

    # Normal stuff
    cfg_opts="${cfg_opts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    cfg_opts="${cfg_opts} -DCMAKE_INSTALL_PREFIX:PATH=${CONDUIT_INSTALL_DIR}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    cfg_opts="${cfg_opts} -DENABLE_TESTS:BOOL=false"
    cfg_opts="${cfg_opts} -DENABLE_DOCS:BOOL=false"
    cfg_opts="${cfg_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    cfg_opts="${cfg_opts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    cfg_opts="${cfg_opts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS} ${PAR_LINKER_FLAGS}\""
    cfg_opts="${cfg_opts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS} ${PAR_LINKER_FLAGS}\""
    if test "${OPSYS}" = "Darwin" ; then
        cfg_opts="${cfg_opts} -DCMAKE_INSTALL_NAME_DIR:PATH=${CONDUIT_INSTALL_DIR}/lib"
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

    if [[ "$DO_HDF5" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DHDF5_DIR:STRING=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/"
    fi

    if [[ "$DO_PYTHON" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DPYTHON_EXECUTABLE:STRING=$PYTHON_COMMAND"
        cfg_opts="${cfg_opts} -DENABLE_PYTHON:STRING=TRUE"
    fi

    if [[ "$FC_COMPILER" != "no" ]] ; then
        cfg_opts="${cfg_opts} -DENABLE_FORTRAN:BOOL=ON"
        cfg_opts="${cfg_opts} -DCMAKE_Fortran_COMPILER:STRING=${FC_COMPILER}"
    fi

    #
    # Conduit Relay MPI Support
    #

    if [[ "$PAR_COMPILER" != "" ]] ; then
        cfg_opts="${cfg_opts} -DENABLE_MPI:BOOL=ON"
        cfg_opts="${cfg_opts} -DMPI_C_COMPILER:STRING=${PAR_COMPILER}"
        cfg_opts="${cfg_opts} -DMPI_CXX_COMPILER:STRING=${PAR_COMPILER}"
    fi

    if [[ "$PAR_INCLUDE" != "" ]] ; then
        cfg_opts="${cfg_opts} -DMPI_C_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
        cfg_opts="${cfg_opts} -DMPI_CXX_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
    fi

    if [[ "$PAR_LIBS" != "" ]] ; then
        cfg_opts="${cfg_opts} -DMPI_C_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cfg_opts="${cfg_opts} -DMPI_C_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
        cfg_opts="${cfg_opts} -DMPI_CXX_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cfg_opts="${cfg_opts} -DMPI_CXX_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
    fi

    #
    # Several platforms have had problems with the cmake configure command
    # issued simply via "issue_command". This issue was first discovered
    # on BGQ and then occurred randomly for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - write a simple
    # script that is invoked with bash which calls cmake with all of the
    # properly arguments. This strategy is being used for all platforms.
    #
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${cfg_opts} ../${CONDUIT_SRC_DIR}/src > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "conduit configure failed. Giving up"
        return 1
    fi

    #
    # Now build conduit.
    #
    info "Building conduit . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "conduit build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing conduit . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "conduit install failed. Giving up"
        return 1
    fi

    if [[ "${DO_GROUP}" == "yes" ]] ; then
        chmod -R ug+w,a+rX "${VISITDIR}/conduit"
        chgrp -R ${GROUP} "${VISITDIR}/conduit"
    fi

    cd "$START_DIR"
    info "Done with conduit"
    return 0
}
