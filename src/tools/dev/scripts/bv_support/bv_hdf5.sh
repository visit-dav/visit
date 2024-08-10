function bv_hdf5_initialize
{
    export DO_HDF5="no"
    export USE_SYSTEM_HDF5="no"
    add_extra_commandline_args "hdf5" "alt-hdf5-dir" 1 "Use alternative directory for hdf5"
}

function bv_hdf5_enable
{
    DO_HDF5="yes"
}

function bv_hdf5_disable
{
    DO_HDF5="no"
}

function bv_hdf5_alt_hdf5_dir
{
    bv_hdf5_enable
    USE_SYSTEM_HDF5="yes"
    HDF5_INSTALL_DIR="$1"
}

function bv_hdf5_depends_on
{
    if [[ "$USE_SYSTEM_HDF5" == "yes" ]]; then
        echo ""
    else
        local depends_on=""

        if [[ "$DO_ZLIB" == "yes" ]] ; then
            depends_on="$depends_on zlib"
        fi

        if [[ -n "$PAR_COMPILER" && "$DO_MOAB" == "yes"  && "$DO_MPICH" == "yes" ]]; then
            depends_on="$depends_on mpich"
        fi

        echo $depends_on
    fi
}

function bv_hdf5_initialize_vars
{
    if [[ "$USE_SYSTEM_HDF5" == "no" ]]; then
        HDF5_INSTALL_DIR="${VISITDIR}/hdf5/$HDF5_VERSION/${VISITARCH}"
    fi
}

function bv_hdf5_info
{
    export HDF5_VERSION=${HDF5_VERSION:-"1.14.4-3"}
    export HDF5_FILE=${HDF5_FILE:-"hdf5-${HDF5_VERSION}.tar.gz"}
    export HDF5_COMPATIBILITY_VERSION=${HDF5_COMPATIBILITY_VERSION:-"1.14"}
    export HDF5_BUILD_DIR=${HDF5_BUILD_DIR:-"hdf5-${HDF5_VERSION}"}
    export HDF5_URL=${HDF5_URL:-"https://github.com/HDFGroup/hdf5/releases/download/hdf5_1.14.4.3/${HDF5_FILE}"}
    export HDF5_SHA256_CHECKSUM="019ac451d9e1cf89c0482ba2a06f07a46166caf23f60fea5ef3c37724a318e03"
}

function bv_hdf5_print
{
    printf "%s%s\n" "HDF5_FILE=" "${HDF5_FILE}"
    printf "%s%s\n" "HDF5_VERSION=" "${HDF5_VERSION}"
    printf "%s%s\n" "HDF5_COMPATIBILITY_VERSION=" "${HDF5_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "HDF5_BUILD_DIR=" "${HDF5_BUILD_DIR}"
}

function bv_hdf5_print_usage
{
    printf "%-20s %s [%s]\n" "--hdf5" "Build HDF5" "${DO_HDF5}"
    printf "%-20s %s [%s]\n" "--alt-hdf5-dir" "Use HDF5 from an alternative directory"
}

function bv_hdf5_host_profile
{
    if [[ "$DO_HDF5" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## HDF5" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_HDF5" == "yes" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR $HDF5_INSTALL_DIR)" \
                >> $HOSTCONF 
        else
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR \${VISITHOME}/hdf5/$HDF5_VERSION/\${VISITARCH})" \
                >> $HOSTCONF 

            ZLIB_LIBDEP=""
            if [[ "$DO_ZLIB" == "yes" ]] ; then
                ZLIB_LIBDEP="\${VISITHOME}/zlib/\${ZLIB_VERSION}/\${VISITARCH}/lib z"
            fi
            
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                    >> $HOSTCONF
        fi
    fi
}

function bv_hdf5_ensure
{
    if [[ "$DO_HDF5" == "yes" && "$USE_SYSTEM_HDF5" == "no" ]] ; then
        ensure_built_or_ready "hdf5" $HDF5_VERSION $HDF5_BUILD_DIR $HDF5_FILE $HDF5_URL 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_HDF5="no"
            error "Unable to build HDF5.  ${HDF5_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                          Function 8.1, build_hdf5                           #
# *************************************************************************** #

function build_hdf5
{
    #
    # Prepare build dir
    #
    prepare_build_dir $HDF5_BUILD_DIR $HDF5_FILE
    untarred_hdf5=$?

    if [[ $untarred_hdf5 == -1 ]] ; then
        warn "Unable to prepare HDF5 Build Directory. Giving Up"
        return 1
    fi

    # Turn most things off except tools
    #-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=ON
    cmk_opts=" \
        -DBUILD_TESTING:BOOL=OFF \
        -DHDF5_BUILD_CPP_LIB:BOOL=OFF \
        -DHDF5_BUILD_JAVA:BOOL=OFF \
        -DHDF5_BUILD_EXAMPLES:BOOL=OFF \
        -DHDF5_BUILD_TOOLS:BOOL=ON \
        -DHDF5_ENABLE_SUBFILING_VFD:BOOL=OFF \
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER} \
        -DCMAKE_C_FLAGS:STRING=\"-Wno-error=implicit-function-declaration ${C_OPT_FLAGS} ${PAR_LINKER_FLAGS}\" \
        -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS} ${PAR_LINKER_FLAGS}\" \
        -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}"

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cmk_opts="${cmk_opts} -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
    else
        cmk_opts="${cmk_opts} -DCMAKE_BUILD_TYPE:STRING=Release"
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cmk_opts="${cmk_opts} \
           -DBUILD_STATIC_LIBS:BOOL=ON \
           -DBUILD_SHARED_LIBS:BOOL=OFF \
           -DONLY_SHARED_LIBS:BOOL=OFF"
    else
        cmk_opts="${cmk_opts} \
           -DBUILD_STATIC_LIBS:BOOL=OFF \
           -DBUILD_SHARED_LIBS:BOOL=ON \
           -DONLY_SHARED_LIBS:BOOL=ON"
    fi

    if [[ "$DO_MOAB" == "yes" ]]; then
        cmk_opts="${cmk_opts} -DHDF5_BUILD_HL_LIB:BOOL=ON"
    else
        cmk_opts="${cmk_opts} -DHDF5_BUILD_HL_LIB:BOOL=OFF"
    fi   

    if [[ "$DO_ZLIB" == "yes" ]]; then
        info "Configuring HDF5 with ZLib support."
        cmk_opts="${cmk_opts} \
            -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON \
            -DZLIB_INCLUDE_DIR:PATH=\"${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/include\" \
            -DZLIB_LIBRARY_RELEASE:FILEPATH=\"${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib/libz.${SO_EXT}\""
    fi

    if [[ "$FC_COMPILER" == "no" ]] ; then
        cmk_opts="${cmk_opts} -DHDF5_BUILD_FORTRAN:BOOL=OFF"
    else
        cmk_opts="${cmk_opts} \
            -DHDF5_BUILD_FORTRAN:BOOL=ON \
            -DCMAKE_Fortran_COMPILER:STRING=\"${FC_COMPILER}\""
    fi

    if [[ "$PAR_COMPILER" != "" ]] ; then
        cmk_opts="${cmk_opts} \
            -DHDF5_ENABLE_PARALLEL:BOOL=ON \
            -DMPI_C_COMPILER:PATH=\"${PAR_COMPILER}\" \
            -DMPI_CXX_COMPILER:PATH=\"${PAR_COMPILER_CXX}\""
    fi

    if [[ "$PAR_INCLUDE" != "" ]] ; then
        cmk_opts="${cmk_opts} \
            -DMPI_C_COMPILER_INCLUDE_DIRS:STRING=\"${PAR_INCLUDE_PATH}\" \
            -DMPI_C_HEADER_DIR:PATH=\"${PAR_INCLUDE_PATH}\""
    fi

    if [[ "$PAR_LIBS" != "" ]] ; then
        cmk_opts="${cmk_opts} -DMPI_C_LINK_FLAGS:STRING=\"${PAR_LINKER_FLAGS}\""
    fi

    # Make a build directory for an out-of-source build.. Change the
    # VISIT_BUILD_DIR variable to represent the out-of-source build directory.
    HDF5_SRC_DIR=${HDF5_BUILD_DIR}
    HDF5_BUILD_DIR="${HDF5_SRC_DIR}-build"
    if [[ ! -d $HDF5_BUILD_DIR ]] ; then
        echo "Making build directory $HDF5_BUILD_DIR"
        mkdir $HDF5_BUILD_DIR
    fi

    pushd $HDF5_BUILD_DIR > /dev/null || error "Can't cd to HDF5 build dir."

    info "CMaking HDF5. . . (~5 minutes)"
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${cmk_opts} ../${HDF5_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh
    if [[ $? != 0 ]] ; then
        warn "CMaking HDF5 failed. Giving up"
        return 1
    fi

    #
    # Build HDF5
    #
    info "Building HDF5. . . (~5 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "HDF5 build failed. Giving up"
        return 1
    fi
   
    #
    # Install into the VisIt third party location.
    #
    info "Installing HDF5"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "HDF5 install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/hdf5"
        chgrp -R ${GROUP} "$VISITDIR/hdf5"
    fi
    popd > /dev/null
    info "Done with HDF5"
    return 0
}

function bv_hdf5_is_enabled
{
    if [[ $DO_HDF5 == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_hdf5_is_installed
{

    if [[ "$USE_SYSTEM_HDF5" == "yes" ]]; then
        return 1
    fi

    check_if_installed "hdf5" $HDF5_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_hdf5_build
{
    cd "$START_DIR"
    
    if [[ "$DO_HDF5" == "yes" && "$USE_SYSTEM_HDF5" == "no" ]] ; then
        check_if_installed "hdf5" $HDF5_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping HDF5 build.  HDF5 is already installed."
        else
            info "Building HDF5 (~5 minutes)"
            build_hdf5
            if [[ $? != 0 ]] ; then
                error "Unable to build or install HDF5.  Bailing out."
            fi
            info "Done building HDF5"
        fi
    fi
}
