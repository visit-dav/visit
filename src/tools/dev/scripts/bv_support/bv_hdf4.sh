function bv_hdf4_initialize
{
    export DO_HDF4="no"
}

function bv_hdf4_enable
{
    DO_HDF4="yes"
}

function bv_hdf4_disable
{
    DO_HDF4="no"
}

function bv_hdf4_depends_on
{
    echo ""
}

function bv_hdf4_info
{
    export HDF4_FILE=${HDF4_FILE:-"CMake-hdf-4.2.15.tar.gz"}
    export HDF4_VERSION=${HDF4_VERSION:-"4.2.15"}
    export HDF4_COMPATIBILITY_VERSION=${HDF4_COMPATIBILITY_VERSION:-"4.2"}
    export HDF4_BUILD_DIR=${HDF4_BUILD_DIR:-"CMake-hdf-4.2.15-build"}
    export HDF4_ROOT_DIR=${HDF4_ROOT_DIR:-"CMake-hdf-4.2.15"}
    export HDF4_SRC_DIR=${HDF4_SRC_DIR:-"CMake-hdf-4.2.15/hdf-4.2.15"}
    export HDF4_URL=${HDF4_URL:-"https://support.hdfgroup.org/ftp/HDF/releases/HDF4.2.15/src/CMake-hdf-4.2.15.tar.gz"}
    #export HDF4_MD5_CHECKSUM=""
    #export HDF4_SHA256_CHECKSUM=""
}

function bv_hdf4_initialize_vars
{
  echo ""
}

function bv_hdf4_print
{
    printf "%s%s\n" "HDF4_FILE=" "${HDF4_FILE}"
    printf "%s%s\n" "HDF4_VERSION=" "${HDF4_VERSION}"
    printf "%s%s\n" "HDF4_COMPATIBILITY_VERSION=" "${HDF4_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "HDF4_BUILD_DIR=" "${HDF4_BUILD_DIR}"
    printf "%s%s\n" "HDF4_SRC_DIR=" "${HDF4_SRC_DIR}"
}

function bv_hdf4_print_usage
{
    printf "%-20s %s [%s]\n" "--hdf4" "Build HDF4" "${DO_HDF4}"
}

function bv_hdf4_host_profile
{
    if [[ "$DO_HDF4" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## HDF4" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR \${VISITHOME}/hdf4/$HDF4_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_hdf4_ensure
{
    if [[ "$DO_HDF4" == "yes" ]] ; then
        ensure_built_or_ready "hdf4" $HDF4_VERSION $HDF4_ROOT_DIR $HDF4_FILE $HDF4_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_HDF4="no"
            error "Unable to build HDF4.  ${HDF4_FILE} not found."
        fi
    fi
}

function bv_hdf4_dry_run
{
    if [[ "$DO_HDF4" == "yes" ]] ; then
        echo "Dry run option not set for hdf4."
    fi
}


# *************************************************************************** #
#                          Function 8.3, build_hdf4                           #
# *************************************************************************** #

function apply_hdf4_patch
{
    return 0
}

function build_hdf4
{
    #
    # Prepare build dir
    #
    prepare_build_dir $HDF4_ROOT_DIR $HDF4_FILE
    untarred_hdf4=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_hdf4 == -1 ]] ; then
        warn "Unable to prepare HDF4 Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    #info "Patching HDF . . ." 
    #apply_hdf4_patch
    #if [[ $? != 0 ]] ; then
    #    if [[ $untarred_hdf4 == 1 ]] ; then
    #        warn "Giving up on HDF4 build because the patch failed."
    #        return 1
    #    else
    #        warn "Patch failed, but continuing.  I believe that this script\n" \
    #             "tried to apply a patch to an existing directory that had\n" \
    #             "already been patched ... that is, the patch is\n" \
    #             "failing harmlessly on a second application."
    #    fi
    #fi

    info "Configuring HDF4 . . . "
    cd "$START_DIR"
    # Make the build directory for an out-of-source build.
    if [[ ! -d $HDF4_BUILD_DIR ]] ; then
        echo "Making build directory $HDF4_BUILD_DIR"
        mkdir $HDF4_BUILD_DIR
    fi

    hopts="-C $START_DIR/${HDF4_SRC_DIR}/config/cmake/cacheinit.cmake "
    hopts="${hopts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    hopts="${hopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/hdf4/${HDF4_VERSION}/${VISITARCH}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        hopts="${hopts} -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_STATIC_LIBS:BOOL=ON"
    else
        hopts="${hopts} -DBUILD_SHARED_LIBS:BOOL=ON -DBUILD_STATIC_LIBS=OFF -DONLY_SHARED_LIBS:BOOL=ON"
    fi

    #
    # Set  Fortran compiler
    #
    # Disable Fortran on Darwin since it causes HDF4 builds to fail.
    if [[ "$OPSYS" == "Darwin" ]]; then
        hopts="${hopts} -DHDF4_BUILD_FORTRAN:BOOL=OFF"
    elif [[ "$FC_COMPILER" == "no" ]] ; then
        hopts="${hopts} -DHDF4_BUILD_FORTRAN:BOOL=OFF"
    else
        # Is this correct? 
        hopts="${hopts} -DHDF4_BUILD_FORTRAN:BOOL=ON"
        hopts="${hopts} -DCMAKE_Fortran_COMPILER=$FF_COMPILER"
        hopts="${hopts} -DCMAKE_Fortran_FLAGS=$FF_FLAGS"
    fi
 
    hopts="${hopts} -DBUILD_TESTING:BOOL=OFF"
    hopts="${hopts} -DHDF4_BUILD_JAVA:BOOL=OFF"
    hopts="${hopts} -DHDF4_ALLOW_EXTERNAL_SUPPORT:STRING=TGZ"
    hopts="${hopts} -DHDF4_PACKAGE_EXTLIBS:BOOL=ON" 
    hopts="${hopts} -DHDF4_BUILD_EXAMPLES:BOOL=OFF"
    hopts="${hopts} -DHDF4_BUILD_GENERATORS:BOOL=OFF"
    hopts="${hopts} -DHDF4_ENABLE_NETCDF:BOOL=OFF"
    hopts="${hopts} -DHDF4_BUILD_TOOLS:BOOL=OFF"
    hopts="${hopts} -DHDF4_BUILD_UTILS:BOOL=OFF"
    hopts="${hopts} -DTGZPATH:PATH=$START_DIR/${HDF4_ROOT_DIR}"

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    cd ${HDF4_BUILD_DIR}

    #
    # Write a simple script that we invoke with bash which calls cmake with all of the
    # arguments. 
    #

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${hopts} ../${HDF4_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "HDF4 configuration failed."

    #
    # Build HDF4
    #
    info "Building HDF4 . . ."

    $MAKE $MAKEOPS
    if [[ $? != 0 ]] ; then
        warn "HDF4 build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing HDF4 . . ."
    $MAKE $MAKEOPS install
    if [[ $? != 0 ]] ; then
        warn "HDF4 install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for HDF4 . . ."
        # Relink libdf.
        INSTALLNAMEPATH="$VISITDIR/hdf4/${HDF4_VERSION}/$VISITARCH/lib"

        ${C_COMPILER} -dynamiclib -o libdf.${SO_EXT} hdf/src/*.o \
                      -Wl,-headerpad_max_install_names \
                      -Wl,-install_name,$INSTALLNAMEPATH/libdf.${SO_EXT} \
                      -Wl,-compatibility_version,$HDF4_COMPATIBILITY_VERSION \
                      -Wl,-current_version,$HDF4_VERSION 
        if [[ $? != 0 ]] ; then
            warn \
                "HDF4 dynamic library build failed for libdf.${SO_EXT}.  Giving up"
            return 1
        fi
        cp libdf.${SO_EXT} "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib"

        # Relink libmfhdf.
        ${C_COMPILER} -dynamiclib -o libmfhdf.${SO_EXT} mfhdf/libsrc/*.o \
                      -Wl,-headerpad_max_install_names \
                      -Wl,-install_name,$INSTALLNAMEPATH/libmfhdf.${SO_EXT} \
                      -Wl,-compatibility_version,$HDF4_COMPATIBILITY_VERSION \
                      -Wl,-current_version,$HDF4_VERSION \
                      -L"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib" 
        if [[ $? != 0 ]] ; then
            warn \
                "HDF4 dynamic library build failed for libmfhdf.${SO_EXT}.  Giving up"
            return 1
        fi
        cp libmfhdf.${SO_EXT} "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib"

        # relocate the .a's we don't want them.
        mkdir "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib/static"
        mv "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib/libdf.a" "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib/static"
        mv "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib/libmfhdf.a" "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib/static"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/hdf4"
        chgrp -R ${GROUP} "$VISITDIR/hdf4"
    fi
    cd "$START_DIR"
    info "Done with HDF4"
    return 0
}

function bv_hdf4_is_enabled
{
    if [[ $DO_HDF4 == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_hdf4_is_installed
{
    check_if_installed "hdf4" $HDF4_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_hdf4_build
{
    cd "$START_DIR"
    if [[ "$DO_HDF4" == "yes" ]] ; then
        check_if_installed "hdf4" $HDF4_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping HDF4 build.  HDF4 is already installed."
        else
            info "Building HDF4 (~10 minutes)"
            build_hdf4
            if [[ $? != 0 ]] ; then
                error "Unable to build or install HDF4.  Bailing out."
            fi
            info "Done building HDF4"
        fi
    fi
}
