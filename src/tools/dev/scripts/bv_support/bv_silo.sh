function bv_silo_initialize
{
    export DO_SILO="no"
    export DO_SILEX="no"
    add_extra_commandline_args "silo" "silex" 0 "Enable silex when building Silo"
}

function bv_silo_enable
{
    DO_SILO="yes"
}

function bv_silo_disable
{
    DO_SILO="no"
}

function bv_silo_silex
{
    info "Enabling silex in Silo build"
    DO_SILEX="yes"
    bv_silo_enable
}

function bv_silo_depends_on
{
    local depends_on="zlib"

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="hdf5"
    fi
    
    if [[ "$DO_SZIP" == "yes" ]] ; then
        depends_on="$depends_on szip"
    fi


    echo $depends_on
}

function bv_silo_info
{
    export SILO_VERSION=${SILO_VERSION:-"4.10.2"}
    export SILO_FILE=${SILO_FILE:-"silo-${SILO_VERSION}.tar.gz"}
    export SILO_COMPATIBILITY_VERSION=${SILO_COMPATIBILITY_VERSION:-"4.10.2"}
    export SILO_URL=${SILO_URL:-https://wci.llnl.gov/codes/silo/silo-${SILO_VERSION}}
    export SILO_BUILD_DIR=${SILO_BUILD_DIR:-"silo-${SILO_VERSION}"}
    export SILO_MD5_CHECKSUM="9ceac777a2f2469ac8cef40f4fab49c8"
    export SILO_SHA256_CHECKSUM="3af87e5f0608a69849c00eb7c73b11f8422fa36903dd14610584506e7f68e638"
}

function bv_silo_print
{
    printf "%s%s\n" "SILO_FILE=" "${SILO_FILE}"
    printf "%s%s\n" "SILO_VERSION=" "${SILO_VERSION}"
    printf "%s%s\n" "SILO_COMPATIBILITY_VERSION=" "${SILO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "SILO_BUILD_DIR=" "${SILO_BUILD_DIR}"
}

function bv_silo_print_usage
{
    printf "%-20s %s [%s]\n" "--silo" "Build Silo support" "$DO_SILO"
    printf "%-20s %s [%s]\n" "--silex" "Enable silex when building Silo" "$DO_SILEX"
}

function bv_silo_host_profile
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Silo" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_SILO_DIR \${VISITHOME}/silo/$SILO_VERSION/\${VISITARCH})" \
            >> $HOSTCONF

        libdep=""
        if [[ "$DO_HDF5" == "yes" ]] ; then
            libdep="HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP}"
        fi
        libdep="$libdep ZLIB_LIBRARY_DIR z"
        if [[ -n "$libdep" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP $libdep TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_silo_ensure
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        ensure_built_or_ready "silo" $SILO_VERSION $SILO_BUILD_DIR $SILO_FILE $SILO_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_SILO="no"
            error "Unable to build Silo.  ${SILO_FILE} not found."
        fi
    fi
}

function bv_silo_dry_run
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        echo "Dry run option not set for silo."
    fi
}

# *************************************************************************** #
#                            Function 8, build_silo
#
# Modfications:
#   Mark C. Miller, Wed Feb 18 22:57:25 PST 2009
#   Added logic to build silex and copy bins on Mac. Removed disablement of
#   browser.
#
#   Mark C. Miller Mon Jan  7 10:31:46 PST 2013
#   PDB/SCORE lite headers are now handled in Silo and require additional
#   configure option to ensure they are installed.
#
#   Brad Whitlock, Tue Apr  9 12:20:22 PDT 2013
#   Add support for custom zlib.
#
#   Kathleen Biagas, Tue Jun 10 08:21:33 MST 2014
#   Disable silex for static builds.
#
# *************************************************************************** #

function build_silo
{
    #
    # Prepare build dir
    #
    prepare_build_dir $SILO_BUILD_DIR $SILO_FILE
    untarred_silo=$?
    if [[ $untarred_silo == -1 ]] ; then
        warn "Unable to prepare Silo build directory. Giving Up!"
        return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring Silo . . ."
    cd $SILO_BUILD_DIR || error "Can't cd to Silo build dir."
    info "Invoking command to configure Silo"
    SILO_LINK_OPT=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        export HDF5INCLUDE="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/include"
        export HDF5LIB="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib"
        WITHHDF5ARG="--with-hdf5=$HDF5INCLUDE,$HDF5LIB"
        SILO_LINK_OPT="-L$HDF5LIB -lhdf5"
    else
        WITHHDF5ARG="--without-hdf5"
    fi
    SILO_LINK_OPT="$SILO_LINK_OPT -lz"
    if [[ "$DO_SZIP" == "yes" ]] ; then
        export SZIPDIR="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        WITHSZIPARG="--with-szlib=$SZIPDIR"
        SILO_LINK_OPT="$SILO_LINK_OPT -L$SZIPDIR/lib -lsz"
    else
        WITHSZIPARG="--without-szlib"
    fi
    WITHSHAREDARG="--enable-shared"
    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        WITHSHAREDARG="--disable-shared"
    fi
    #if [[ "$DO_SILEX" == "no" || "$DO_QT" != "yes" || "$DO_STATIC_BUILD" == "yes" ]] ; then
        WITHSILOQTARG='--disable-silex'
    #else
    #    export SILOQTDIR="$QT_INSTALL_DIR" #"${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}"
    #    if [[ "$OPSYS" == "Darwin" ]] ; then
    #        WITHSILOQTARG='--enable-silex --with-Qt-dir=$SILOQTDIR --with-Qt-lib="m -F${SILOQTDIR}/lib -framework QtGui -framework QtCore"'
    #    else
    #        WITHSILOQTARG='--enable-silex --with-Qt-dir=$SILOQTDIR --with-Qt-lib="QtGui -lQtCore"'
    #    fi
    #fi

    ZLIBARGS="--with-zlib=${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/include,${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib"

    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS="--disable-fortran"
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\""
    fi

    extra_ac_flags=""
    # detect coral systems, which older versions of autoconf don't detect
    if [[ "$(uname -m)" == "ppc64le" ]] ; then
         extra_ac_flags="ac_cv_build=powerpc64le-unknown-linux-gnu"
    fi 

    info "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/silo/$SILO_VERSION/$VISITARCH\" \
        $WITHHDF5ARG $WITHSZIPARG $WITHSILOQTARG $WITHSHAREDARG \
        --enable-install-lite-headers --without-readline \
        $ZLIBARGS $SILO_EXTRA_OPTIONS ${extra_ac_flags}"

    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/silo/$SILO_VERSION/$VISITARCH\" \
        $WITHHDF5ARG $WITHSZIPARG $WITHSILOQTARG $WITHSHAREDARG \
        --enable-install-lite-headers --without-readline \
        $ZLIBARGS $SILO_EXTRA_OPTIONS ${extra_ac_flags}"

    if [[ $? != 0 ]] ; then
        warn "Silo configure failed.  Giving up"
        return 1
    fi

    #
    # Build Silo
    #
    info "Building Silo . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Silo build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing Silo"

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Silo install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/silo"
        chgrp -R ${GROUP} "$VISITDIR/silo"
    fi
    cd "$START_DIR"
    info "Done with Silo"
    return 0
}

function bv_silo_is_enabled
{
    if [[ $DO_SILO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_silo_is_installed
{
    check_if_installed "silo" $SILO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_silo_build
{
    cd "$START_DIR"
    if [[ "$DO_SILO" == "yes" ]] ; then
        check_if_installed "silo" $SILO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Silo build.  Silo is already installed."
        else
            info "Building Silo (~2 minutes)"
            build_silo
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Silo.  Bailing out."
            fi
            info "Done building Silo"
        fi
    fi
}
