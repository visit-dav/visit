function bv_mpich_initialize
{
    export DO_MPICH="no"
}

function bv_mpich_enable
{
    DO_MPICH="yes"
}

function bv_mpich_disable
{
    DO_MPICH="no"
}

function bv_mpich_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_mpich_info
{
    export MPICH_VERSION=${MPICH_VERSION:-"3.4.1"}
    export MPICH_FILE=${MPICH_FILE:-"mpich-${MPICH_VERSION}.tar.gz"}
    export MPICH_COMPATIBILITY_VERSION=${MPICH_COMPATIBILITY_VERSION:-"3.4"}
    export MPICH_SRC_DIR=${MPICH_SRC_DIR:-"mpich-${MPICH_VERSION}"}
    export MPICH_URL=${MPICH_URL:-http://www.mpich.org/static/tarballs/${MPICH_VERSION}}
    export MPICH_MD5_CHECKSUM="10e43fd61fd1a13c89cfbed3eb30b97a"
    export MPICH_SHA256_CHECKSUM="8836939804ef6d492bcee7d54abafd6477d2beca247157d92688654d13779727"
}

function bv_mpich_print
{
    printf "%s%s\n" "MPICH_FILE=" "${MPICH_FILE}"
    printf "%s%s\n" "MPICH_VERSION=" "${MPICH_VERSION}"
    printf "%s%s\n" "MPICH_COMPATIBILITY_VERSION=" "${MPICH_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "MPICH_SRC_DIR=" "${MPICH_SRC_DIR}"
}

function bv_mpich_print_usage
{
    printf "%-20s %s [%s]\n" "--mpich" "Build MPICH support" "$DO_MPICH"
}

function bv_mpich_host_profile
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MPICH" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(MPICH $MPICH_VERSION)" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MPICH_DIR \${VISITHOME}/mpich/\${MPICH_VERSION}/\${VISITARCH})" \
            >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MPICH_INSTALL ON TYPE BOOL)" >> $HOSTCONF
        echo "" >> $HOSTCONF
        echo "# Tell VisIt the parallel compiler so it can deduce parallel flags" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER \${VISIT_MPICH_DIR}/bin/mpicc TYPE FILEPATH)"  >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)" >> $HOSTCONF
    fi
}

function bv_mpich_ensure
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        check_installed_or_have_src "mpich" $MPICH_VERSION $MPICH_SRC_DIR $MPICH_FILE $MPICH_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MPICH="no"
            error "Unable to build MPICH.  ${MPICH_FILE} not found."
        fi
    fi
}

function bv_mpich_dry_run
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        echo "Dry run option not set for mpich."
    fi
}

function bv_mpich_is_enabled
{
    if [[ $DO_MPICH == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mpich_is_installed
{
    check_if_installed "mpich" $MPICH_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mpich_build
{
    cd "$START_DIR"
    if [[ "$DO_MPICH" == "yes" ]] ; then
        check_if_installed "mpich" $MPICH_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping MPICH build.  MPICH is already installed."
        else
            info "Building MPICH (~2 minutes)"
            build_mpich
            if [[ $? != 0 ]] ; then
                error "Unable to build or install MPICH.  Bailing out."
            fi
            info "Done building MPICH"
        fi
    fi
}

# *************************************************************************** #
# build_mpich
# *************************************************************************** #

function build_mpich
{
    #
    # Uncompress the source file
    #
    uncompress_src_file $MPICH_SRC_DIR $MPICH_FILE
    untarred_mpich=$?
    if [[ $untarred_mpich == -1 ]] ; then
        warn "Unable to uncompress MPICH source file. Giving Up!"
        return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring MPICH . . ."
    cd $MPICH_SRC_DIR || error "Can't cd to MPICH source dir."
    info "Invoking command to configure MPICH"

    #
    # Turn on shared version of the libs
    #
    mpich_opts="--enable-shared"
    if [[ "$OPSYS" == "Darwin" ]]; then
        mpich_opts="${mpich_opts} --enable-two-level-namespace --enable-threads=single"
	# For macOS (Catalina and Big Sur) IPv6 needs to be disabled
	# for ch4:ofi sockets to work. That can be done by setting the
	# environment variable `MPIR_CVAR_OFI_SKIP_IPV6=1` Otherwsie
	# set the '--with-device to Ch3 as it works.
        mpich_opts="${mpich_opts} --with-device=ch3"
    fi

    #
    # MPICH will fail to build if we disable common blocks '-fno-common'
    # Screen the flags vars to make sure we don't use this option for MPICH
    #
    MPICH_CFLAGS=`echo $CFLAGS | sed -e 's/-fno-common//g'`
    MPICH_C_OPT_FLAGS=`echo $C_OPT_FLAGS | sed -e 's/-fno-common//g'`
    MPICH_CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-fno-common//g'`
    MPICH_CXX_OPT_FLAGS=`echo $CXX_OPT_FLAGS | sed -e 's/-fno-common//g'`
    MPICH_FCFLAGS=`echo $FCFLAGS | sed -e 's/-fno-common//g'`

    #
    # Enable/disable fortran as needed.
    #
    if [[ "$FC_COMPILER" == "no" ]] ; then
        mpich_opts="${mpich_opts} --enable-fortran=no"
    else
        mpich_opts="${mpich_opts} --enable-fortran=all"	
    fi

    issue_command env CXX="$CXX_COMPILER" \
                  CC="$C_COMPILER" \
                  CFLAGS="$MPICH_CFLAGS $MPICH_C_OPT_FLAGS" \
                  CXXFLAGS="$MPICH_CXXFLAGS $MPICH_CXX_OPT_FLAGS"\
                  FFLAGS="$MPICH_FCFLAGS -fallow-argument-mismatch"\
                  ./configure ${mpich_opts} \
                  --prefix="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH"

    if [[ $? != 0 ]] ; then
        warn "MPICH configure failed.  Giving up"
        return 1
    fi

    #
    # Build MPICH
    #
    info "Building MPICH . . . (~5 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "MPICH build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing MPICH"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "MPICH install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/mpich"
        chgrp -R ${GROUP} "$VISITDIR/mpich"
    fi
    cd "$START_DIR"
    info "Done with MPICH"
    return 0
}
