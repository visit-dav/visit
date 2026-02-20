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
    local depends_on="hdf5 zlib"

    echo $depends_on
}

function bv_moab_info
{
    export MOAB_VERSION=${MOAB_VERSION:-"5.5.0"}
    export MOAB_FILE=${MOAB_FILE:-"moab-${MOAB_VERSION}.tar.gz"}
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
        echo "## MOAB" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MOAB_DIR \${VISITHOME}/moab/$MOAB_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MOAB_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
            >> $HOSTCONF
        if [[ -n "$PAR_COMPILER" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_MOAB_MPI_DIR \${VISITHOME}/moab_mpi/$MOAB_VERSION/\${VISITARCH})" \
                >> $HOSTCONF
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_MOAB_MPI_LIBDEP HDF5_MPI_LIBRARY_DIR hdf5_mpi \${VISIT_HDF5_MPI_LIBDEP} TYPE STRING)" \
                >> $HOSTCONF
        fi
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

    cd $MOAB_BUILD_DIR || error "Can't cd to moab build dir."
    rm -f src/moab/MOABConfig.h # work around a potential issue in MOAB tarball

    cf_mpi_arg=""
    cf_par_suffix=""
    if [[ -n "$PAR_COMPILER" ]]; then
        cf_mpi_arg="--with-mpi=${PAR_HOME}"
        cf_c_compiler="$PAR_COMPILER"
        cf_cxx_compiler="$PAR_COMPILER_CXX"
    else
        cf_mpi_arg="--without-mpi"
        cf_c_compiler="$C_COMPILER"
        cf_cxx_compiler="$CXX_COMPILER"
    fi

    cf_prefix_arg="--prefix=$VISITDIR/moab/$MOAB_VERSION/$VISITARCH"
    cf_common_args="--with-pic --disable-fortran --disable-imesh --disable-cgns"

    if [[ "DO_STATIC_BUILD" == "yes" ]]; then
        cf_static_args="--enable-static --disable-shared"
    else
        cf_static_args="--disable-static --enable-shared"
    fi

    cf_hdf5_ldflags_arg=""
    cf_zlib_arg=""
    cf_hdf5_arg="--with-hdf5=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
    cf_zlib_arg="--with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"
    cf_hdf5_ldflags_arg="$cf_hdf5_ldflags_arg -lz"
    if [[ -n "$cf_hdf5_ldflags_arg" ]]; then
        cf_hdf5_ldflags_arg="--with-hdf5-ldflags=\"$cf_hdf5_ldflags_arg\""
    fi

    info "Configuring moab . . ."
    set -x
    sh -c "./configure \
        CXX=\"$cf_cxx_compiler\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS $hdf5_consumer_uses_mpi_flag\" \
        CC=\"$cf_c_compiler\" CFLAGS=\"$CFLAGS $C_OPT_FLAGS $hdf5_consumer_uses_mpi_flag\" \
        ${cf_prefix_arg} ${cf_mpi_arg} ${cf_common_args} ${cf_static_args} \
        ${cf_hdf5_arg} ${cf_hdf5_ldflags_arg} \
        ${cf_zlib_arg}"
    set +x
    if [[ $? != 0 ]] ; then
        warn "MOAB configure failed.  Giving up"
        return 1
    fi

    #
    # Build moab
    #
    info "Building moab . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "moab build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing moab"
    $MAKE install

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/moab"
        chgrp -R ${GROUP} "$VISITDIR/moab"
    fi

    cd "$START_DIR"
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
