function bv_fastquery_initialize
{
    export DO_FASTQUERY="no"
}

function bv_fastquery_enable
{
    DO_FASTQUERY="yes"
}

function bv_fastquery_disable
{
    DO_FASTQUERY="no"
}

function bv_fastquery_depends_on
{
    depends_on="fastbit"

    if [[ "$DO_ADIOS" == "yes" ]] ; then
        depends_on="$depends_on adios"
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="$depends_on hdf5"
    fi

    if [[ "$DO_NETCDF" == "yes" ]] ; then
        depends_on="$depends_on netcdf"
    fi

    if [[ "$depends_on" == "fastquery" ]] ; then
        error "FastQuery must be built with either ADIOS, HDF5, or NetCDF."
    fi
}

function bv_fastquery_info
{
    export FASTQUERY_VERSION=${FASTQUERY_VERSION:-"0.8.4.3"}
    export FASTQUERY_FILE=${FASTQUERY_FILE:-"fastquery-${FASTQUERY_VERSION}.tar.gz"}
    # Note: last 3-digit field in URL changes with version.
    export FASTQUERY_URL=${FASTQUERY_URL:-"https://code.lbl.gov/frs/download.php/file/428"}
    export FASTQUERY_BUILD_DIR=${FASTQUERY_BUILD_DIR:-"fastquery-${FASTQUERY_VERSION}"}
    export FASTQUERY_MD5_CHECKSUM="de6b6ed6a17e861e23b0349254a13ea9"
    export FASTQUERY_SHA256_CHECKSUM="452c778eedd9956410393acbc44aae49505b48b821cc575b168bd05501578f16"
}

function bv_fastquery_print
{
    printf "%s%s\n" "FASTQUERY_FILE=" "${FASTQUERY_FILE}"
    printf "%s%s\n" "FASTQUERY_VERSION=" "${FASTQUERY_VERSION}"
    printf "%s%s\n" "FASTQUERY_BUILD_DIR=" "${FASTQUERY_BUILD_DIR}"
}

function bv_fastquery_print_usage
{
    printf "%-20s %s [%s]\n" "--fastquery" "Build FastQuery" "$DO_FASTQUERY"
    printf "%-20s %s\n" "" "NOTE: FastQuery not available for download from web" 
}

function bv_fastquery_host_profile
{
    if [[ "$DO_FASTQUERY" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## FastQuery" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(FASTQUERY $FASTQUERY_VERSION)" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_FASTQUERY_DIR \${VISITHOME}/fastquery/\${FASTQUERY_VERSION}/\${VISITARCH})" \
            >> $HOSTCONF
    fi

}

function bv_fastquery_ensure
{
    if [[ "$DO_FASTQUERY" == "yes" ]] ; then
        ensure_built_or_ready "fastquery" $FASTQUERY_VERSION $FASTQUERY_BUILD_DIR $FASTQUERY_FILE $FASTQUERY_URL
        if [[ $? != 0 ]] ; then
            warn "Unable to build FastQuery.  ${FASTQUERY_FILE} not found."
            warn "FastQuery is not available for download from the VisIt build site"
            ANY_ERRORS="yes"
            DO_FASTQUERY="no"
            error "Try going to https://codeforge.lbl.gov/frs/?group_id=44"
        fi
    fi
}

function bv_fastquery_dry_run
{
    if [[ "$DO_FASTQUERY" == "yes" ]] ; then
        echo "Dry run option not set for fastquery."
    fi
}

function apply_fastquery_patch
{
#    if [[ ${FASTQUERY_VERSION} == 2.0.3 ]] ; then
#        apply_fastquery_2_0_3_patch
#        if [[ $? != 0 ]] ; then
#            return 1
#        fi
#    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.14, build_fastquery                        #
# *************************************************************************** #

function build_fastquery
{
    #
    # Prepare build dir
    #
    prepare_build_dir $FASTQUERY_BUILD_DIR $FASTQUERY_FILE
    untarred_fastquery=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_fastquery == -1 ]] ; then
        warn "Unable to prepare FastQuery Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_fastquery_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_fastquery == 1 ]] ; then
            warn "Giving up on FastQuery build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    # MPI support
    if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
        FASTQUERY_MPI_OPTS="MPICC=\"$VISIT_MPI_COMPILER\" MPICXX=\"$VISIT_MPI_COMPILER_CXX\" LDFLAGS=\"$PAR_LINKER_FLAGS\""
        FASTQUERY_MPI_INC="$PAR_INCLUDE"
    else
        FASTQUERY_MPI_OPTS="--disable-parallel"
        FASTQUERY_MPI_INC=""
    fi

    FASTQUERY_MPI_OPTS="--disable-parallel"
    FASTQUERY_MPI_INC=""
    
    # FastBit support
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        info "FastBit requested.  Configuring FastQuery with FastBit support."
        export FASTBITROOT="$VISITDIR/fastbit/$FASTBIT_VERSION/$VISITARCH"
        WITH_FASTBIT_ARG="--with-fastbit=$FASTBITROOT"
    else
        WITH_FASTBIT_ARG=""
    fi

    # ADIOS support
    if [[ "$DO_ADIOS" == "yes" ]] ; then
        info "ADIOS requested.  Configuring FastQuery with ADIOS support."
        export ADIOSROOT="$VISITDIR/adios/$ADIOS_VERSION/$VISITARCH"
        WITH_ADIOS_ARG="--with-adios=$ADIOSROOT"
    else
        WITH_ADIOS_ARG="--without-adios"
    fi

    # HDF5 support
    if [[ "$DO_HDF5" == "yes" ]] ; then
        info "HDF5 requested.  Configuring FastQuery with HDF5 support."
        export HDF5ROOT="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        export SZIPROOT="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        WITH_HDF5_ARG="--with-hdf5=$HDF5ROOT"
        #HDF5_DYLIB="-L$HDF5ROOT/lib -L$SZIPROOT/lib -lhdf5 -lsz -lz"
    else
        WITH_HDF5_ARG="--without-hdf5"
        #HDF5_DYLIB=""
    fi

    # NETCDF support
    if [[ "$DO_NETCDF" == "yes" ]] ; then
        info "NetCDF requested.  Configuring FastQuery with NetCDF support."
        export NETCDFROOT="$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH"
        WITH_NETCDF_ARG="--with-netcdf=$NETCDFROOT"
    else
        WITH_NETCDF_ARG="--without-netcdf"
    fi

    #
    # Apply configure
    #
    info "Configuring FastQuery . . ."
    cd $FASTQUERY_BUILD_DIR || error "Can't cd to FastQuery build dir."
    
    info "Invoking command to configure FastQuery"

    info ./configure \
	CC="$C_COMPILER" CXX="$CXX_COMPILER" \
	CFLAGS=\"$CFLAGS $C_OPT_FLAGS $FASTQUERY_MPI_INC\" \
        CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS $FASTQUERY_MPI_INC\" \
	$FASTQUERY_MPI_OPTS $WITH_FASTBIT_ARG \
	$WITH_ADIOS_ARG $WITH_HDF5_ARG $WITH_NETCDF_ARG \
        --prefix="$VISITDIR/fastquery/$FASTQUERY_VERSION/$VISITARCH"

    ./configure \
	CC="$C_COMPILER" CXX="$CXX_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS $FASTQUERY_MPI_INC" \
        CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS $FASTQUERY_MPI_INC" \
        CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS $FASTQUERY_MPI_INC" \
	$FASTQUERY_MPI_OPTS $WITH_FASTBIT_ARG \
	$WITH_ADIOS_ARG $WITH_HDF5_ARG $WITH_NETCDF_ARG \
        --prefix="$VISITDIR/fastquery/$FASTQUERY_VERSION/$VISITARCH"

    if [[ $? != 0 ]] ; then
        echo "FastQuery configure failed.  Giving up"
        return 1
    fi

    #
    # Build FastQuery
    #
    info "Building FastQuery . . . (~7 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "FastQuery build failed.  Giving up"
        return 1
    fi
    
    info "Installing FastQuery . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "FastQuery build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/fastquery"
        chgrp -R ${GROUP} "$VISITDIR/fastquery"
    fi

    cd "$START_DIR"
    info "Done with FastQuery"
    return 0
}

function bv_fastquery_is_enabled
{
    if [[ $DO_FASTQUERY == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_fastquery_is_installed
{
    check_if_installed "fastquery" $FASTQUERY_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_fastquery_build
{
    cd "$START_DIR"
    if [[ "$DO_FASTQUERY" == "yes" ]] ; then
        check_if_installed "fastquery" $FASTQUERY_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping FastQuery build.  FastQuery is already installed."
        else
            info "Building FastQuery (~7 minutes)"
            build_fastquery
            if [[ $? != 0 ]] ; then
                error "Unable to build or install FastQuery.  Bailing out."
            fi
            info "Done building FastQuery"
        fi
    fi
}
