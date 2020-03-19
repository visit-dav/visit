function bv_openexr_initialize
{
    export DO_OPENEXR="no"
}

function bv_openexr_enable
{
    DO_OPENEXR="yes"
}

function bv_openexr_disable
{
    DO_OPENEXR="no"
}

function bv_openexr_depends_on
{
    echo ""
}

function bv_openexr_info
{
    export OPENEXR_FILE=${OPENEXR_FILE:-"openexr-2.2.0.tar.gz"}
    export OPENEXR_VERSION=${OPENEXR_VERSION:-"2.2.0"}
    export OPENEXR_COMPATIBILITY_VERSION=${OPENEXR_COMPATIBILITY_VERSION:-"2.2.0"}
    export OPENEXR_BUILD_DIR=${OPENEXR_BUILD_DIR:-"openexr-2.2.0"}
    export OPENEXR_MD5_CHECKSUM="b64e931c82aa3790329c21418373db4e"
    export OPENEXR_SHA256_CHECKSUM="36a012f6c43213f840ce29a8b182700f6cf6b214bea0d5735594136b44914231"

    export ILMBASE_FILE=${ILMBASE_FILE:-"ilmbase-2.2.0.tar.gz"}
    export ILMBASE_VERSION=${ILMBASE_VERSION:-"2.2.0"}
    export ILMBASE_COMPATIBILITY_VERSION=${OPENEXR_COMPATIBILITY_VERSION:-"2.2.0"}
    export ILMBASE_BUILD_DIR=${ILMBASE_BUILD_DIR:-"ilmbase-2.2.0"}
    export ILMBASE_MD5_CHECKSUM="b540db502c5fa42078249f43d18a4652"
    export ILMBASE_SHA256_CHECKSUM="ecf815b60695555c1fbc73679e84c7c9902f4e8faa6e8000d2f905b8b86cedc7"
}

function bv_openexr_print
{
    printf "%s%s\n" "OPENEXR_FILE=" "${OPENEXR_FILE}"
    printf "%s%s\n" "OPENEXR_VERSION=" "${OPENEXR_VERSION}"
    printf "%s%s\n" "OPENEXR_COMPATIBILITY_VERSION=" "${OPENEXR_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "OPENEXR_BUILD_DIR=" "${OPENEXR_BUILD_DIR}"

    printf "%s%s\n" "ILMBASE_FILE=" "${ILMBASE_FILE}"
    printf "%s%s\n" "ILMBASE_VERSION=" "${ILMBASE_VERSION}"
    printf "%s%s\n" "ILMBASE_COMPATIBILITY_VERSION=" "${ILMBASE_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ILMBASE_BUILD_DIR=" "${ILMBASE_BUILD_DIR}"
}

function bv_openexr_host_profile
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OpenEXR" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_OPENEXR_DIR \${VISITHOME}/openexr/$OPENEXR_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_openexr_print_usage
{
    #openexr does not have an option, it is only dependent on openexr.
    printf "%-20s %s [%s]\n" "--openexr" "Build OpenEXR" "$DO_OPENEXR"
}

function bv_openexr_ensure
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        ensure_built_or_ready "openexr" $OPENEXR_VERSION $OPENEXR_BUILD_DIR $OPENEXR_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENEXR="no"
            error "Unable to build OpenEXR.  ${OPENEXR_FILE} not found."
        fi
        ensure_built_or_ready "openexr (ILMBase) " $ILMBASE_VERSION $ILMBASE_BUILD_DIR $ILMBASE_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENEXR="no"
            error "Unable to build OpenEXR (ILMBase).  ${ILMBASE_FILE} not found."
        fi
    fi
}

function bv_openexr_dry_run
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        echo "Dry run option not set for OpenEXR."
    fi
}

# ***************************************************************************
# build_ilmbase
#
# Modifications:
#
# ***************************************************************************

function build_ilmbase
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ILMBASE_BUILD_DIR $ILMBASE_FILE
    untarred_ilmbase=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ilmbase == -1 ]] ; then
        warn "Unable to prepare ILMBase Build Directory. Giving Up"
        return 1
    fi
    
    #
    # Configure ILMBase
    #
    cd $ILMBASE_BUILD_DIR || error "Can't cd to ILMBase build dir."
    if [[ "$DO_STATIC_BUILD" == "yes" || "$OPSYS" == "Linux"  ]]; then
        DISABLE_BUILDTYPE="--disable-shared"
    else
        DISABLE_BUILDTYPE="--disable-static"
    fi
    info "Configuring ILMBase . . ."
    ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
                CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
                --prefix="$VISITDIR/openexr/$ILMBASE_VERSION/$VISITARCH" $DISABLE_BUILDTYPE
    if [[ $? != 0 ]] ; then
        warn "ILMBase configure failed.  Giving up"
        return 1
    fi

    #
    # Build ILMBase
    #
    info "Building ILMBase . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "ILMBase build failed.  Giving up"
        return 1
    fi
    info "Installing ILMBase . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "ILMBase build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/openexr"
        chgrp -R ${GROUP} "$VISITDIR/openexr"
    fi
    cd "$START_DIR"
    info "Done with ILMBase"
    return 0
}

# ***************************************************************************
# build_openexr
#
# Modifications:
#
# ***************************************************************************

function build_openexr
{
    #
    # Prepare build dir
    #
    prepare_build_dir $OPENEXR_BUILD_DIR $OPENEXR_FILE
    untarred_openexr=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_openexr == -1 ]] ; then
        warn "Unable to prepare OpenEXR Build Directory. Giving Up"
        return 1
    fi
    
    #
    # Configure OpenEXR
    #
    cd $OPENEXR_BUILD_DIR || error "Can't cd to OpenEXR build dir."
    if [[ "$DO_STATIC_BUILD" == "yes" || "$OPSYS" == "Linux" ]]; then
        DISABLE_BUILDTYPE="--disable-shared"
    else
        DISABLE_BUILDTYPE="--disable-static"
    fi
    info "Configuring OpenEXR . . ."
    ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
                CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
                --prefix="$VISITDIR/openexr/$OPENEXR_VERSION/$VISITARCH" \
                --with-ilmbase-prefix="$VISITDIR/openexr/$OPENEXR_VERSION/$VISITARCH" \
                --with-pic --enable-imfexamples $DISABLE_BUILDTYPE
    if [[ $? != 0 ]] ; then
        warn "openexr configure failed.  Giving up"
        return 1
    fi

    #
    # Build OpenEXR
    #
    info "Building OpenEXR . . . (~5 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "OpenEXR build failed.  Giving up"
        return 1
    fi
    info "Installing OpenEXR . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "OpenEXR build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/openexr"
        chgrp -R ${GROUP} "$VISITDIR/openexr"
    fi
    cd "$START_DIR"
    info "Done with OpenEXR"
    return 0
}

function bv_openexr_is_enabled
{
    if [[ $DO_OPENEXR == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_openexr_is_installed
{
    check_if_installed "openexr" $OPENEXR_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_openexr_build
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        check_if_installed "openexr" $OPENEXR_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of OpenEXR"
        else
            build_ilmbase
            if [[ $? != 0 ]] ; then
                error "Unable to build or install ILMBase for OpenEXR.  Bailing out."
            fi
            info "Done building ILMBase"
            build_openexr
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OpenEXR.  Bailing out."
            fi
            info "Done building OpenEXR"
        fi
    fi
}

