function bv_moab_initialize
{
    export DO_MOAB="no"
}

function bv_moab_enable
{
    DO_MOAB="yes"
}

function bv_moab_disable
{
    DO_MOAB="no"
}

function bv_moab_depends_on
{
    local depends_on="hdf5"

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        depends_on="$depends_on zlib"
    fi 

    echo $depends_on
}

function bv_moab_info
{
    export MOAB_VERSION=${MOAB_VERSION:-"5.5.0"}
    export MOAB_FILE=${MOAB_FILE:-"moab-${MOAB_VERSION}.tar.gz"}
    export MOAB_URL=${MOAB_URL:-"https://web.cels.anl.gov/projects/sigma/downloads/moab"}
    export MOAB_BUILD_DIR=${MOAB_BUILD_DIR:-"moab-5.5.0"}
    export MOAB_SHA256_CHECKSUM="58969f8a1b209ec9036c08c53a6b7078b368eb3bf99d0368a4de5a2f2a8db678"
}

function bv_moab_print
{
    printf "%s%s\n" "MOAB_FILE=" "${MOAB_FILE}"
    printf "%s%s\n" "MOAB_VERSION=" "${MOAB_VERSION}"
    printf "%s%s\n" "MOAB_BUILD_DIR=" "${MOAB_BUILD_DIR}"
}

function bv_moab_print_usage
{
    printf "%-20s %s [%s]\n" "--moab" "Build moab support" "$DO_MOAB"
}

function bv_moab_host_profile
{
    if [[ "$DO_MOAB" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MOAB " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MOAB_DIR \${VISITHOME}/moab/$MOAB_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MOAB_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
            >> $HOSTCONF
    fi
}

function bv_moab_ensure
{
    if [[ "$DO_MOAB" == "yes" ]] ; then
        ensure_built_or_ready "moab" $MOAB_VERSION $MOAB_BUILD_DIR $MOAB_FILE $MOAB_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MOAB="no"
            error "Unable to build moab.  ${MOAB_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_moab
# *************************************************************************** #
function build_moab
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MOAB_BUILD_DIR $MOAB_FILE
    untarred_moab=$?
    if [[ $untarred_moab == -1 ]] ; then
        warn "Unable to prepare moab build directory. Giving Up!"
        return 1
    fi

    # -DCMAKE_SHARED_LINKER_FLAGS:STRING=\"-Wl,--no-undefined\" \
    cmk_opts="\
        -DENABLE_TESTING:BOOL=OFF \
        -DENABLE_BLASLAPACK:BOOL=OFF \
        -DENABLE_FORTRAN:BOOL=OFF \
        -DENABLE_NETCDF=OFF \
        -DCMAKE_INSTALL_PREFIX:PATH=$VISITDIR/moab/$MOAB_VERSION/$VISITARCH"

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cmk_opts="${cmk_opts} -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
    else
        cmk_opts="${cmk_opts} -DCMAKE_BUILD_TYPE:STRING=Release"
    fi

    if [[ "DO_STATIC_BUILD" == "yes" ]]; then
        cmk_opts="${cmk_opts} -DBUILD_SHARED_LIBS=OFF"
    else
        cmk_opts="${cmk_opts} -DBUILD_SHARED_LIBS=ON"
    fi

    if [[ "$DO_ZLIB" == "yes" ]]; then
        cmk_opts="${cmk_opts} \
            -DENABLE_ZLIB=ON \
            -DZLIB_ROOT=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"
    else
        cmk_opts="${cmk_opts} -DENABLE_ZLIB=OFF"
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        cmk_opts="${cmk_opts} \
            -DENABLE_HDF5=ON \
            -DHDF5_ROOT=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
    else
        cmk_opts="${cmk_opts} -DENABLE_HDF5=OFF"
    fi

    if [[ "$PAR_COMPILER" != "" ]] ; then
        cmk_opts="${cmk_opts} \
            -DENABLE_MPI:BOOL=ON \
            -DMPI_C_COMPILER:PATH=\"${PAR_COMPILER}\" \
            -DMPI_CXX_COMPILER:PATH=\"${PAR_COMPILER_CXX}\""
    fi

    # work around a potential issue in MOAB tarball by removing this file
    rm -f ${MOAB_SRC_DIR}/src/moab/MOABConfig.h

    # Make a build directory for an out-of-source build.. Change the
    # VISIT_BUILD_DIR variable to represent the out-of-source build directory.
    MOAB_SRC_DIR=${MOAB_BUILD_DIR}
    MOAB_BUILD_DIR="${MOAB_SRC_DIR}-build"
    if [[ ! -d $MOAB_BUILD_DIR ]] ; then
        echo "Making build directory $MOAB_BUILD_DIR"
        mkdir $MOAB_BUILD_DIR
    fi

    pushd $MOAB_BUILD_DIR > /dev/null || error "Can't cd to MOAB build dir."

    info "CMaking MOAB. . . (~5 minutes)"
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${cmk_opts} ../${MOAB_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh
    if [[ $? != 0 ]] ; then
        warn "CMaking MOAB failed. Giving up"
        return 1
    fi

    #
    # Build MOAB
    #
    info "Building MOAB. . . (~5 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "MOAB build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing MOAB"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "MOAB install failed. Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/moab"
        chgrp -R ${GROUP} "$VISITDIR/moab"
    fi

    popd > /dev/null 
    info "Done with moab"
    return 0
}


function bv_moab_is_enabled
{
    if [[ $DO_MOAB == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_moab_is_installed
{
    check_if_installed "moab" $MOAB_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_moab_build
{
    cd "$START_DIR"
    if [[ "$DO_MOAB" == "yes" ]] ; then
        check_if_installed "moab" $MOAB_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping moab build.  moab is already installed."
        else
            info "Building moab (~2 minutes)"
            build_moab
            if [[ $? != 0 ]] ; then
                error "Unable to build or install moab.  Bailing out."
            fi
            info "Done building moab"
        fi
    fi
}
