function bv_hdf4_initialize
{
    export DO_HDF4="no"
}

function bv_hdf4_enable
{
    DO_HDF4="yes"
    DO_SZIP="yes"
}

function bv_hdf4_disable
{
    DO_HDF4="no"
}

function bv_hdf4_depends_on
{
    echo "szip zlib"
}

function bv_hdf4_info
{
    export HDF4_FILE=${HDF4_FILE:-"hdf-4.2.5.tar.gz"}
    export HDF4_VERSION=${HDF4_VERSION:-"4.2.5"}
    export HDF4_COMPATIBILITY_VERSION=${HDF4_COMPATIBILITY_VERSION:-"4.2"}
    export HDF4_BUILD_DIR=${HDF4_BUILD_DIR:-"hdf-4.2.5"}
    export HDF4_URL=${HDF4_URL:-"http://www.hdfgroup.org/ftp/HDF/HDF_Current/src"}
    export HDF4_MD5_CHECKSUM="7241a34b722d29d8561da0947c06069f"
    export HDF4_SHA256_CHECKSUM="73b0021210bae8c779f9f1435a393ded0f344cfb01f7ee8b8794ec9d41dcd427"
}

function bv_hdf4_initialize_vars
{
    info "testing hdf4 requirements"
    local lexv=`which lex`
    local yaccv=`which yacc`

    if [[ "$lexv" == "" || "$yaccv" == "" ]]; then
        error "HDF4 is enabled, but lex and yacc have not been found in system path."
    fi 
}

function bv_hdf4_print
{
    printf "%s%s\n" "HDF4_FILE=" "${HDF4_FILE}"
    printf "%s%s\n" "HDF4_VERSION=" "${HDF4_VERSION}"
    printf "%s%s\n" "HDF4_COMPATIBILITY_VERSION=" "${HDF4_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "HDF4_BUILD_DIR=" "${HDF4_BUILD_DIR}"
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
        if [[ "$DO_SZIP" == "yes" ]] ; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP \${VISITHOME}/szip/$SZIP_VERSION/\${VISITARCH}/lib sz TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_hdf4_ensure
{
    if [[ "$DO_HDF4" == "yes" ]] ; then
        ensure_built_or_ready "hdf4" $HDF4_VERSION $HDF4_BUILD_DIR $HDF4_FILE $HDF4_URL
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

function build_hdf4
{
    #
    # Prepare build dir
    #
    prepare_build_dir $HDF4_BUILD_DIR $HDF4_FILE
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

    #
    # Set  Fortran compiler
    #
    # Disable Fortran on Darwin since it causes HDF4 builds to fail.
    if [[ "$OPSYS" == "Darwin" ]]; then
        FORTRANARGS="--disable-fortran"
    elif [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS="--disable-fortran"
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    hdf4_build_mode=""
    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        hdf4_build_mode="--disable-production"
    fi

    #
    # Configure HDF4
    #
    info "Configuring HDF4 . . ."
    cd $HDF4_BUILD_DIR || error "Can't cd to hdf4 build dir."
    info "Invoking command to configure HDF4"
    MAKEOPS=""
    if [[ "$OPSYS" == "Darwin" || "$OPSYS" == "AIX" ]]; then
        export DYLD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
        # In order to ensure $FORTRANARGS is expanded to build the arguments to
        # configure, we wrap the invokation in 'sh -c "..."' syntax
        sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
        $FORTRANARGS $hdf4_build_mode \
        --prefix=\"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH\" \
        --with-szlib=\"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH\" \
        --with-zlib=\"$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH\" \
        --disable-dependency-tracking --disable-netcdf"
        if [[ $? != 0 ]] ; then
            warn "HDF4 configure failed.  Giving up"\
                 "You can see the details of the build failure at $HDF4_BUILD_DIR/config.log\n"
            return 1
        fi
        MAKEOPS="-i"
    else
        export LD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
        # In order to ensure $FORTRANARGS is expanded to build the arguments to
        # configure, we wrap the invokation in 'sh -c "..."' syntax
        issue_command sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" LIBS=\"-lm\" \
        $FORTRANARGS $hdf4_build_mode \
        --prefix=\"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH\" \
        --with-szlib=\"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH\" \
        --with-zlib=\"$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH\" --disable-netcdf"
        if [[ $? != 0 ]] ; then
            warn "HDF4 configure failed.  Giving up.\n"\
                 "You can see the details of the build failure at $HDF4_BUILD_DIR/config.log\n"
            return 1
        fi
    fi

    #
    # Build HDF4
    #
    info "Building HDF4 . . . (~2 minutes)"

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
                      -Wl,-current_version,$HDF4_VERSION \
                      -L"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib" \
                      -lvtkjpeg-${VTK_SHORT_VERSION} -lsz -lz
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
                      -L"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib" \
                      -L"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib" \
                      -lvtkjpeg-${VTK_SHORT_VERSION} -ldf -lsz -lz
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
            info "Building HDF4 (~2 minutes)"
            build_hdf4
            if [[ $? != 0 ]] ; then
                error "Unable to build or install HDF4.  Bailing out."
            fi
            info "Done building HDF4"
        fi
    fi
}
