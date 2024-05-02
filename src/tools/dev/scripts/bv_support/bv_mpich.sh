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
    export MPICH_VERSION=${MPICH_VERSION:-"3.3.1"}
    export MPICH_FILE=${MPICH_FILE:-"mpich-${MPICH_VERSION}.tar.gz"}
    export MPICH_COMPATIBILITY_VERSION=${MPICH_COMPATIBILITY_VERSION:-"3.3"}
    export MPICH_BUILD_DIR=${MPICH_BUILD_DIR:-"mpich-${MPICH_VERSION}"}
    export MPICH_URL=${MPICH_URL:-http://www.mpich.org/static/tarballs/${MPICH_VERSION}}
    export MPICH_SHA256_CHECKSUM="fe551ef29c8eea8978f679484441ed8bb1d943f6ad25b63c235d4b9243d551e5"
}

function bv_mpich_print
{
    printf "%s%s\n" "MPICH_FILE=" "${MPICH_FILE}"
    printf "%s%s\n" "MPICH_VERSION=" "${MPICH_VERSION}"
    printf "%s%s\n" "MPICH_COMPATIBILITY_VERSION=" "${MPICH_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "MPICH_BUILD_DIR=" "${MPICH_BUILD_DIR}"
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
        ensure_built_or_ready "mpich" $MPICH_VERSION $MPICH_BUILD_DIR $MPICH_FILE $MPICH_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MPICH="no"
            error "Unable to build MPICH.  ${MPICH_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_mpich
#
# Modfications:
#
# *************************************************************************** #

function build_mpich
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MPICH_BUILD_DIR $MPICH_FILE
    untarred_mpich=$?
    if [[ $untarred_mpich == -1 ]] ; then
        warn "Unable to prepare MPICH build directory. Giving Up!"
        return 1
    fi
    
    cd $MPICH_BUILD_DIR || error "Can't cd to MPICH build dir."

    #
    # Call configure
    #
    info "Configuring MPICH . . ."
    info "Invoking command to configure MPICH"

    #
    # Turning off maintainer mode avoids need for any local autotools tools.
    # We should not ever need them because we are not developing mpich.
    # I guess we need shared libs.
    #
    mpich_opts="--disable-maintainer-mode --enable-shared"
    if [[ "$OPSYS" == "Darwin" ]]; then
        mpich_opts="${mpich_opts} --enable-threads=single"
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

    set -x
    issue_command env CXX="$CXX_COMPILER" \
                  CC="$C_COMPILER" \
                  CFLAGS="$MPICH_CFLAGS $MPICH_C_OPT_FLAGS" \
                  CXXFLAGS="$MPICH_CXXFLAGS $MPICH_CXX_OPT_FLAGS"\
                  FFLAGS="$MPICH_FCFLAGS"\
                  ./configure ${mpich_opts} \
                  --prefix="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH"
    set +x
    if [[ $? != 0 ]] ; then
        warn "MPICH configure failed.  Giving up"
        return 1
    fi

    #
    # Build MPICH
    #
    info "Building MPICH . . . (~5 minutes)"
    env $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            warn "MPICH build failed but maybe due to LDFLAGS.\n" \
                 "Retrying MPICH build with LDFLAGS set."
            env $MAKE $MAKE_OPT_FLAGS LDFLAGS="-Wl,-flat_namespace -Wl,-undefined -Wl,suppress"
            if [[ $? != 0 ]] ; then
                warn "MPICH build failed.  Giving up"
                return 1
            fi
        else
            warn "MPICH build failed.  Giving up"
            return 1
        fi
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
