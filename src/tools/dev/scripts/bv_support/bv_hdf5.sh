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

        if [[ -n "$PAR_COMPILER" && "$DO_MPICH" == "yes" ]]; then
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
    export HDF5_VERSION=${HDF5_VERSION:-"2.0.0"}
    export HDF5_FILE=${HDF5_FILE:-"hdf5-${HDF5_VERSION}.tar.xz"}
    export HDF5_COMPATIBILITY_VERSION=${HDF5_COMPATIBILITY_VERSION:-"2.0"}
    export HDF5_BUILD_DIR=${HDF5_BUILD_DIR:-"hdf5-${HDF5_VERSION}"}
    export HDF5_SHA256_CHECKSUM="7649f45b4b3cd3350e0b13b790204d377c89e4ce3b346d9ecffb29552737dc71"
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

function build_hdf5
{
    #
    # Prepare build dir
    #
    prepare_build_dir $HDF5_BUILD_DIR $HDF5_FILE
    untarred_hdf5=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_hdf5 == -1 ]] ; then
        warn "Unable to prepare HDF5 Build Directory. Giving Up"
        return 1
    fi

    # Turn most things off except tools
    #-DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=ON
    cmk_opts=" \
        -DBUILD_TESTING:BOOL=OFF \
        -DHDF5_USE_FILE_LOCKING:BOOL=OFF \
        -DHDF5_IGNORE_DISABLED_FILE_LOCKS:BOOL=ON \
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
           -DHDF5_ONLY_SHARED_LIBS:BOOL=OFF"
    else
        cmk_opts="${cmk_opts} \
           -DBUILD_STATIC_LIBS:BOOL=OFF \
           -DBUILD_SHARED_LIBS:BOOL=ON \
           -DHDF5_ONLY_SHARED_LIBS:BOOL=ON"
    fi

    if [[ "$DO_MOAB" == "yes" || "$DO_MFEM" == "yes" || "$DO_NETCDF" == "yes" ]]; then
        cmk_opts="${cmk_opts} -DHDF5_BUILD_HL_LIB:BOOL=ON"
    else
        cmk_opts="${cmk_opts} -DHDF5_BUILD_HL_LIB:BOOL=OFF"
    fi   

    if [[ "$DO_ZLIB" == "yes" ]]; then
        info "Configuring HDF5 with zlib support."
        cmk_opts="${cmk_opts} \
            -DHDF5_ENABLE_ZLIB_SUPPORT:BOOL=ON \
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
            -DMPI_HOME:STRING=${PAR_HOME}"
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

    # ensure hdf5-shared target specifies it needs to link with zlib
    #if [[ "$DO_ZLIB" == "yes" && "$DO_STATIC_BUILD" == "no" ]]; then
    #    targetsfile=${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}/cmake/hdf5-targets.cmake
    #    info "adding ZLIB::ZLIB to $targetsfile"
    #    echo "if(TARGET ZLIB::ZLIB)" >> $targetsfile
    #    echo "    set_property(TARGET hdf5-shared APPEND PROPERTY INTERFACE_LINK_LIBRARIES ZLIB::ZLIB)" >> $targetsfile
    #    echo "endif()" >> $targetsfile
    #fi

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
