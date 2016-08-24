function bv_moab_initialize
{
    export DO_MOAB="no"
    export ON_MOAB="off"
}

function bv_moab_enable
{
    DO_MOAB="yes"
    ON_MOAB="on"
}

function bv_moab_disable
{
    DO_MOAB="no"
    ON_MOAB="off"
}

function bv_moab_depends_on
{
    local depends_on="hdf5"

    if [[ "$DO_SZIP" == "yes" ]] ; then
        depends_on="$depends_on szip"
    fi

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        depends_on="$depends_on zlib"
    fi

    echo $depends_on
}

function bv_moab_info
{
    export MOAB_VERSION=${MOAB_VERSION:-"4.9.1"}
    export MOAB_FILE=${MOAB_FILE:-"moab-${MOAB_VERSION}.tar.gz"}
    export MOAB_BUILD_DIR=${MOAB_BUILD_DIR:-"moab-${MOAB_VERSION}"}
    export MOAB_URL=${MOAB_URL:-"ftp://ftp.mcs.anl.gov/pub/fathom"}
}

function bv_moab_print
{
    printf "%s%s\n" "MOAB_FILE=" "${MOAB_FILE}"
    printf "%s%s\n" "MOAB_VERSION=" "${MOAB_VERSION}"
    printf "%s%s\n" "MOAB_BUILD_DIR=" "${MOAB_BUILD_DIR}"
}

function bv_moab_print_usage
{
    printf "%-15s %s [%s]\n" "--moab" "Build moab support" "$DO_MOAB"
}

function bv_moab_graphical
{
    local graphical_out="moab     $MOAB_VERSION($MOAB_FILE)      $ON_MOAB"
    echo "$graphical_out"
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

function bv_moab_dry_run
{
    if [[ "$DO_MOAB" == "yes" ]] ; then
        echo "Dry run option not set for moab."
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
    rm -f src/moab/MOABConfig.h

    if [[ "DO_STATIC_BUILD" == "yes" && "$parallel" == "yes" ]]; then
        par_build_types="serial parallel"
    else
        par_build_types="default"
    fi

    for bt in $par_build_types; do 

        mkdir build_$bt
        pushd build_$bt

        par_prefix=""
        if [[ "$bt" == "parallel" ]]; then
            mpi_arg="--with-mpi"
            par_prefix="mpipar/"
        elif [[ "$bt" == "default" ]]; then
            if [[ "$parallel" == "yes" ]]; then
                mpi_arg="--with-mpi"
            fi
        fi

        prefix_arg="--prefix=$VISITDIR/moab/$MOAB_VERSION/${par_prefix}$VISITARCH"
        common_args="--with-pic --disable-fortran"

        if [[ "DO_STATIC_BUILD" == "yes" ]]; then
            static_args="--enable-static --disable-shared"
        else
            static_args="--disable-static --enable-shared"
        fi

        hdf5_ldflags_arg=""
        szip_arg=""
        zlib_arg=""
        hdf5_arg="--with-hdf5=$VISITDIR/hdf5/$HDF5_VERSION/${par_prefix}$VISITARCH"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            szip_arg="--with-szip=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
            hdf5_ldflags_arg="-lsz"
        fi
        if [[ "$DO_ZLIB" == "yes" ]] ; then
            zlib_arg="--with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"
            hdf5_ldflags_arg="$hdf5_ldflags_arg -lz"
        fi
        if [[ -n "$hdf5_ldflags_arg" ]]; then
            hdf5_ldflags_arg="--with-hdf5-ldflags=\"$hdf5_ldflags_arg\""
        fi

        info "Configuring $bt moab . . ."
        info ../configure CXX=\"$CXX_COMPILER\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
            CC=\"$C_COMPILER\" CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
            ${prefix_arg} ${mpi_arg} ${common_args} ${static_args} \
            ${hdf5_arg} ${hdf5_ldflags_arg} \
            ${szip_arg} ${zlib_arg}

        sh -c "../configure \
            CXX=\"$CXX_COMPILER\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
            CC=\"$C_COMPILER\" CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
            ${prefix_arg} ${mpi_arg} ${common_args} ${static_args} \
            ${hdf5_arg} ${hdf5_ldflags_arg} \
            ${szip_arg} ${zlib_arg}"

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
            chmod -R ug+w,a+rX "$VISITDIR/moab"
            chgrp -R ${GROUP} "$VISITDIR/moab"
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
