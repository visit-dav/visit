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

    if [[ "$DO_SZIP" == "yes" ]] ; then
        depends_on="$depends_on szip"
    fi

    echo $depends_on
}

function bv_moab_info
{
    export MOAB_VERSION=${MOAB_VERSION:-"5.4.0"}
    export MOAB_FILE=${MOAB_FILE:-"moab-${MOAB_VERSION}.tar.gz"}
    export MOAB_URL=${MOAB_URL:-"ftp://ftp.mcs.anl.gov/pub/fathom"}
    export MOAB_BUILD_DIR=${MOAB_BUILD_DIR:-"moab-5.4.0"}
    export MOAB_MD5_CHECKSUM="b3857a791130569701b8fca788c2ed7c"
    export MOAB_SHA256_CHECKSUM="a30d2a1911fbf214ae0175b0856e0475c0077dc51ea5914c850d631155a72952"
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

    par_build_types="serial"
    if [[ -n "$PAR_COMPILER_CXX" ]]; then
        par_build_types="$par_build_types parallel"
    fi

    for bt in $par_build_types; do 

        mkdir build_$bt
        pushd build_$bt

        cf_mpi_arg=""
        cf_par_suffix=""
        if [[ "$bt" == "serial" ]]; then
            cf_c_compiler="$C_COMPILER"
            cf_cxx_compiler="$CXX_COMPILER"
        elif [[ "$bt" == "parallel" ]]; then
            # these commands ruin the untar'd source code for normal builds
            sed -i.orig -e 's/libhdf5/libhdf5_mpi/g' ../configure
            sed -i.orig -e 's/libMOAB/libMOAB_mpi/g' ../configure
            sed -i.orig -e 's/=hdf5/=hdf5_mpi/' ../configure
            sed -i.orig -e 's/^LIBS = @LIBS@/LIBS = @HDF5_LIBS@ @LIBS@/' ../tools/Makefile.in
            find .. -name Makefile.in -exec sed -e 's/libMOAB/libMOAB_mpi/g' -i.orig {} \;
            cf_mpi_arg="--with-mpi"
            cf_par_suffix="_mpi"
            cf_c_compiler="$PAR_COMPILER"
            cf_cxx_compiler="$PAR_COMPILER_CXX"
        fi

        cf_prefix_arg="--prefix=$VISITDIR/moab${cf_par_suffix}/$MOAB_VERSION/$VISITARCH"
        cf_common_args="--with-pic --disable-fortran --disable-imesh --disable-cgns"

        if [[ "DO_STATIC_BUILD" == "yes" ]]; then
            cf_static_args="--enable-static --disable-shared"
        else
            cf_static_args="--disable-static --enable-shared"
        fi

        cf_hdf5_ldflags_arg=""
        cf_szip_arg=""
        cf_zlib_arg=""
        cf_hdf5_arg="--with-hdf5=$VISITDIR/hdf5${cf_par_suffix}/$HDF5_VERSION/$VISITARCH"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            cf_szip_arg="--with-szip=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
            cf_hdf5_ldflags_arg="-lsz"
        fi
        cf_zlib_arg="--with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"
        cf_hdf5_ldflags_arg="$cf_hdf5_ldflags_arg -lz"
        if [[ -n "$cf_hdf5_ldflags_arg" ]]; then
            cf_hdf5_ldflags_arg="--with-hdf5-ldflags=\"$cf_hdf5_ldflags_arg\""
        fi

        info "Configuring $bt moab . . ."
        set -x
        sh -c "../configure \
            CXX=\"$cf_cxx_compiler\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
            CC=\"$cf_c_compiler\" CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
            ${cf_prefix_arg} ${cf_mpi_arg} ${cf_common_args} ${cf_static_args} \
            ${cf_hdf5_arg} ${cf_hdf5_ldflags_arg} \
            ${cf_szip_arg} ${cf_zlib_arg}"
        set +x
        if [[ $? != 0 ]] ; then
            warn "$bt MOAB configure failed.  Giving up"
            return 1
        fi

        #
        # Build moab
        #

        info "Building $bt moab . . . (~2 minutes)"
        $MAKE $MAKE_OPT_FLAGS
        if [[ $? != 0 ]] ; then
            warn "$bt moab build failed.  Giving up"
            return 1
        fi

        #
        # Install into the VisIt third party location.
        #
        info "Installing $bt moab"
        $MAKE install

        if [[ "$DO_GROUP" == "yes" ]] ; then
            chmod -R ug+w,a+rX "$VISITDIR/moab${cf_par_suffix}"
            chgrp -R ${GROUP} "$VISITDIR/moab${cf_par_suffix}"
        fi

        #
        # Change name of installed lib to libXXX_mpi.whatever
        #
        if [[ "$bt" == "parallel" ]]; then
            pushd $VISITDIR/moab${cf_par_suffix}/$MOAB_VERSION/$VISITARCH/lib
            if [[ "$OPSYS" == "Darwin" ]]; then
                install_name_tool -id $VISITDIR/moab${cf_par_suffix}/$MOAB_VERSION/$VISITARCH/lib/libMOAB_mpi.dylib libMOAB_mpi.dylib
            fi
            popd
        fi

        popd
    done

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
