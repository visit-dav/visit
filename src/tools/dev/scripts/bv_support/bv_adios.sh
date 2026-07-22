function bv_adios_initialize
{
    export DO_ADIOS="no"
    export USE_SYSTEM_ADIOS="no"
    add_extra_commandline_args "adios" "alt-adios-dir" 1 "Use alternative directory for adios"

}

function bv_adios_enable
{
    DO_ADIOS="yes"
}

function bv_adios_disable
{
    DO_ADIOS="no"
}

function bv_adios_alt_adios_dir
{
    echo "Using alternate Adios directory"

    # Check to make sure the directory or a particular include file exists.
    #    [ ! -e "$1" ] && error "Adios not found in $1"

    bv_adios_enable
    USE_SYSTEM_ADIOS="yes"
    ADIOS_INSTALL_DIR="$1"
}

function bv_adios_depends_on
{
    if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
        echo ""
    else
        depends_on=""

        if [[ "$DO_MPICH" == "yes" ]] ; then
            depends_on="$depends_on mpich"
        fi

        if [[ "$DO_HDF5" == "yes" ]] ; then
            depends_on="$depends_on hdf5"
        fi

        if [[ "$DO_BLOSC" == "yes" ]] ; then
            depends_on="$depends_on blosc"
        fi

        echo $depends_on
    fi
}

function bv_adios_initialize_vars
{
    if [[ "$parallel" == "no" ]]; then
        bv_adios_disable
        warn "Adios requested by default but the parallel flag has not been set. Adios will not be built."
        return
    fi

    if [[ "$USE_SYSTEM_ADIOS" == "no" ]]; then
        ADIOS_INSTALL_DIR="${VISITDIR}/adios/$ADIOS_VERSION/$VISITARCH"
    fi
}

function bv_adios_info
{
    export ADIOS_VERSION=${ADIOS_VERSION:-"1.13.1"}
    export ADIOS_FILE=${ADIOS_FILE:-"adios-${ADIOS_VERSION}.tar.gz"}
    export ADIOS_COMPATIBILITY_VERSION=${ADIOS_COMPATIBILITY_VERSION:-"${ADIOS_VERSION}"}
    export ADIOS_BUILD_DIR=${ADIOS_BUILD_DIR:-"adios-${ADIOS_VERSION}"}
    export ADIOS_SHA256_CHECKSUM="684096cd7e5a7f6b8859601d4daeb1dfaa416dfc2d9d529158a62df6c5bcd7a0"
}

function bv_adios_print
{
    printf "%s%s\n" "ADIOS_FILE=" "${ADIOS_FILE}"
    printf "%s%s\n" "ADIOS_VERSION=" "${ADIOS_VERSION}"
    printf "%s%s\n" "ADIOS_COMPATIBILITY_VERSION=" "${ADIOS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ADIOS_BUILD_DIR=" "${ADIOS_BUILD_DIR}"
}

function bv_adios_print_usage
{
    printf "%-20s %s [%s]\n" "--adios" "Build ADIOS" "$DO_ADIOS"
    printf "%-20s %s [%s]\n" "--alt-adios-dir" "Use ADIOS from an alternative directory"
}

function bv_adios_host_profile
{
    if [[ "$DO_ADIOS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## ADIOS" >> $HOSTCONF
        if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
            echo "## (configured w/ mpi compiler wrapper)" >> $HOSTCONF
        fi
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
            warn "Assuming version 1.11.0 for Adios"
            echo "SETUP_APP_VERSION(ADIOS 1.11.0)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR $ADIOS_INSTALL_DIR)" >> $HOSTCONF
        else
            echo "SETUP_APP_VERSION(ADIOS $ADIOS_VERSION)" >> $HOSTCONF
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR \${VISITHOME}/adios/\${ADIOS_VERSION}/\${VISITARCH})" \
                >> $HOSTCONF
        fi
    fi
}

function bv_adios_ensure
{
    if [[ "$DO_ADIOS" == "yes" && "$USE_SYSTEM_ADIOS" == "no" ]] ; then
        ensure_built_or_ready "adios" $ADIOS_VERSION $ADIOS_BUILD_DIR $ADIOS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ADIOS="no"
            error "Unable to build ADIOS.  ${ADIOS_FILE} not found."
        fi
    fi
}

# ***************************************************************************
#                         Function 8.22, build_ADIOS
#
# Modifications:
#
# ***************************************************************************


function apply_adios_patch
{
    info "No patches for adios"
    return 0
}

function build_adios
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADIOS_BUILD_DIR $ADIOS_FILE sha256 $ADIOS_SHA256_CHECKSUM
    untarred_ADIOS=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ADIOS == -1 ]] ; then
        warn "Unable to prepare ADIOS Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_adios_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_ADIOS == 1 ]] ; then
            warn "Giving up on ADIOS build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Apply configure
    #
    info "Configuring ADIOS . . ."
    cd $ADIOS_BUILD_DIR || error "Can't cd to ADIOS build dir."

    info "Invoking command to configure ADIOS"

    # MPI support
    if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
        WITH_MPI_ARGS="MPICC=\"$VISIT_MPI_COMPILER\" MPICXX=\"$VISIT_MPI_COMPILER_CXX\" LDFLAGS=\"-lpthread $PAR_LINKER_FLAGS\""
        WITH_MPI_INC="$PAR_INCLUDE"
        if [[ "$PAR_COMPILER" != "" ]] ; then
            C_OPT_FLAGS="-I${PAR_HOME}/include"
            MAKE_OPT2_FLAGS="CPPFLAGS=-U_NOMPI"
        fi
    else
        WITH_MPI_ARGS="--without-mpi"
    fi

    # HDF5 support
    if [[ "$DO_HDF5" == "yes" ]] ; then
        export HDF5ROOT="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        WITH_HDF5_ARGS="--with-hdf5=$HDF5ROOT"
    else
        WITH_HDF5_ARGS="--without-hdf5"
        #HDF5_DYLIB=""
    fi

    if [[ "$DO_ZLIB" == "yes" && "$DO_HDF5" == "yes" ]] ; then
        WITH_ZLIB_ARGS="--with-zlib=${VISIT_ZLIB_DIR}"
    else
        WITH_ZLIB_ARGS="--without-zlib"
    fi

    # blosc support
    if [[ "$DO_BLOSC" == "yes" ]]; then
        WITH_BLOSC_ARGS="--with-blosc=$BLOSCDIR"
    else
        WITH_BLOSC_ARGS="--without-blosc"
    fi

    # Fix compilation error on newer Darwin
    if [[ "$OPSYS" == "Darwin" && $(uname -r | cut -d'.' -f1) -ge 23 ]]; then
        sed -i '' 's/^libparse_test_query_xml_a_LIBADD/#libparse_test_query_xml_a_LIBADD/' tests/C/query/common/Makefile.in
    fi

    pyenv=""
    if [[ "$DO_PYTHON" == "yes" ]]; then
        pyenv="env PYTHON=$PYTHON_COMMAND"
    fi

    set -x
    ${pyenv} sh -c "./configure ${OPTIONAL} CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
           CFLAGS=\"$CFLAGS $C_OPT_FLAGS $WITH_MPI_INC\" \
           CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS $WITH_MPI_INC\" \
           $WITH_MPI_ARGS $WITH_HDF5_ARGS $WITH_BLOSC_ARGS $WITH_ZLIB_ARGS \
           --disable-fortran \
           --without-netcdf --without-nc4par --without-phdf5 --without-mxml \
           --prefix=\"$VISITDIR/adios/$ADIOS_VERSION/$VISITARCH\""

    set +x
    if [[ $? != 0 ]] ; then
        warn "ADIOS configure failed.  Giving up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_adios_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_adios == 1 ]] ; then
            warn "Giving up on Adios build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build ADIOS
    #
    info "Building ADIOS . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "ADIOS build failed. Attempting install anyways..."
    fi

    info "Installing ADIOS . . ."
    $MAKE $MAKE_OPT2_FLAGS install
    if [[ $? != 0 ]] ; then
        warn "ADIOS build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/ADIOS"
        chgrp -R ${GROUP} "$VISITDIR/ADIOS"
    fi

    cd "$START_DIR"
    info "Done with ADIOS"
    return 0
}

function bv_adios_is_enabled
{
    if [[ $DO_ADIOS == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_adios_is_installed
{
    if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
        return 1
    fi

    check_if_installed "adios" $ADIOS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_adios_build
{
    cd "$START_DIR"

    if [[ "$DO_ADIOS" == "yes" && "$USE_SYSTEM_ADIOS" == "no" ]] ; then
        check_if_installed "adios" $ADIOS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping ADIOS build.  ADIOS is already installed."
        else
            info "Building ADIOS (~1 minutes)"
            build_adios
            if [[ $? != 0 ]] ; then
                error "Unable to build or install ADIOS.  Bailing out."
            fi
            info "Done building ADIOS"
        fi
    fi
}
