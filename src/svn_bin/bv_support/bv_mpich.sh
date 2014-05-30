function bv_mpich_initialize
{
export DO_MPICH="no"
export ON_MPICH="off"
}

function bv_mpich_enable
{
DO_MPICH="yes"
ON_MPICH="on"
}

function bv_mpich_disable
{
DO_MPICH="no"
ON_MPICH="off"
}

function bv_mpich_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_mpich_info
{
export MPICH_VERSION=${MPICH_VERSION:-"3.0.4"}
export MPICH_FILE=${MPICH_FILE:-"mpich-${MPICH_VERSION}.tar.gz"}
export MPICH_COMPATIBILITY_VERSION=${MPICH_COMPATIBILITY_VERSION:-"3.0.0"}
export MPICH_BUILD_DIR=${MPICH_BUILD_DIR:-"mpich-${MPICH_VERSION}"}
export MPICH_URL=${MPICH_URL:-http://www.mpich.org/static/tarballs/3.0.4}
export MPICH_MD5_CHECKSUM="9c5d5d4fe1e17dd12153f40bc5b6dbc0"
export MPICH_SHA256_CHECKSUM="cf638c85660300af48b6f776e5ecd35b5378d5905ec5d34c3da7a27da0acf0b3"
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
printf "%-15s %s [%s]\n" "--mpich" "Build MPICH support" "$DO_MPICH"
}

function bv_mpich_graphical
{
local graphical_out="MPICH     $MPICH_VERSION($MPICH_FILE)      $ON_MPICH"
echo "$graphical_out"
}

function bv_mpich_host_profile
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MPICH" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "" >> $HOSTCONF
        echo "# Give VisIt information so it can install MPI into the binary distribution." >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_MPICH_DIR \${VISITHOME}/mpich/$MPICH_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MPICH_INSTALL ON)" >> $HOSTCONF
        echo "" >> $HOSTCONF
        echo "# Tell VisIt the parallel compiler so it can deduce parallel flags" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MPI_COMPILER \${VISIT_MPICH_DIR}/bin/mpicc)" \
        >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON)" >> $HOSTCONF
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

function bv_mpich_dry_run
{
  if [[ "$DO_MPICH" == "yes" ]] ; then
    echo "Dry run option not set for mpich."
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
    
    #
    # Call configure
    #
    info "Configuring MPICH . . ."
    cd $MPICH_BUILD_DIR || error "Can't cd to MPICH build dir."
    info "Invoking command to configure MPICH"

    mpich_opts=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        mpich_opts="${mpich_opts} --enable-two-level-namespace"
    fi

    #
    # mpich will fail to build if we disable common blocks '-fno-common'
    # Screen the flags vars to make sure we don't use this option for mpich
    #

    MPICH_CFLAGS=`echo $CFLAGS | sed -e 's/-fno-common//g'`
    MPICH_C_OPT_FLAGS=`echo $C_OPT_FLAGS | sed -e 's/-fno-common//g'`
    MPICH_CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-fno-common//g'`
    MPICH_CXX_OPT_FLAGS=`echo $CXX_OPT_FLAGS | sed -e 's/-fno-common//g'`
    MPICH_FCFLAGS=`echo $FCFLAGS | sed -e 's/-fno-common//g'`

    # disable fortran if we don't have a fortran compiler
        
    if [[ "$FC_COMPILER" == "no" ]] ; then
        mpich_opts="${mpich_opts} --disable-fc --disable-f77"
    fi

    issue_command env CXX="$CXX_COMPILER" \
                      CC="$C_COMPILER" \
                      CFLAGS="$MPICH_CFLAGS $MPICH_C_OPT_FLAGS" \
                      CXXFLAGS="$MPICH_CXXFLAGS $MPICH_CXX_OPT_FLAGS"\
                      FFLAGS="$MPICH_FCFLAGS"\
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

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        info "Relinking MPICH into Mac shared libraries"
        #
        # This code relinks the static C libraries for MPICH into dynamic
        # libraries that let a bundle-contained mpich run. Enabling dynamic
        # linking on the configure command line did not result in all 
        # libraries being dynamically linked.
        #   
        # We kind of hack things further in order to make the libraries by 
        # combining the real contents of libmpl, libopa, libpmpich, and 
        # libmpich into a single new libmpich. The other libraries contain 
        # a stub function that does not get used. This avoids some linking 
        # problems when creating the dynamic libraries.
        #
        lastdir=`pwd`
        MPICH_LIBDIR="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib"
        cd "$MPICH_LIBDIR"
        for f in libmpl libopa libpmpich libmpich libmpichcxx; do
            mkdir $f
            cd $f
            ar -x ../$f.a
            cd ..
        done

        rm -f *.dylib stub.c
        rm libpmpich/lib_libpmpich_la-comm_split_type.o
        echo "int STUB(void){return 0;}" > stub.c

        # Make fake stand-in libraries
        $C_COMPILER -dynamiclib -o libmpl.dylib -DSTUB=STUB_LIBMPL stub.c \
            -headerpad_max_install_names \
            -Wl,-install_name,"$MPICH_LIBDIR/libmpl.dylib" \
            -Wl,-compatibility_version,${MPICH_COMPATIBILITY_VERSION} \
            -Wl,-current_version,${MPICH_VERSION}

        $C_COMPILER -dynamiclib -o libopa.dylib -DSTUB=STUB_LIBOPA stub.c \
            -headerpad_max_install_names \
            -Wl,-install_name,"$MPICH_LIBDIR/libopa.dylib" \
            -Wl,-compatibility_version,${MPICH_COMPATIBILITY_VERSION} \
            -Wl,-current_version,${MPICH_VERSION}

        $C_COMPILER -dynamiclib -o libpmpich.dylib -DSTUB=STUB_LIBPMPICH stub.c \
            -headerpad_max_install_names \
            -Wl,-install_name,"$MPICH_LIBDIR/libpmpich.dylib" \
            -Wl,-compatibility_version,${MPICH_COMPATIBILITY_VERSION} \
            -Wl,-current_version,${MPICH_VERSION}

        # Combine the libaries into a new libmpich.
        $C_COMPILER -dynamiclib -o libmpich.dylib libmpl/*.o libopa/*.o libmpich/*.o libpmpich/lib_libpmpich*.o\
            -headerpad_max_install_names \
            -Wl,-install_name,"$MPICH_LIBDIR/libmpich.dylib" \
            -Wl,-compatibility_version,${MPICH_COMPATIBILITY_VERSION} \
            -Wl,-current_version,${MPICH_VERSION}

        # Clean up
        for f in libmpl libopa libpmpich libmpich libmpichcxx; do
            rm -rf $f
            rm -f $f.a $f.la
        done
        rm -rf stub.c pkgconfig
        cd "$lastdir"
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

