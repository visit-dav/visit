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

function apply_mpich_331_libtool_darwin_patch
{
   # Patch confdb/libtool.m4 file for newer darwin linker flags for undef'd symbols.
   # Note: There are a ton of libtool.m4 files in the untard source code. This one
   # I believe is used to generate libtool everywhere or, at the very least, influences
   # the final links.
   # Note: this patch command uses the -l option to ignore whitespace.
   patch -l -p0 << \EOF
--- confdb/libtool.m4.old
+++ confdb/libtool.m4
@@ -1067,16 +1067,11 @@
       _lt_dar_allow_undefined='$wl-undefined ${wl}suppress' ;;
     darwin1.*)
       _lt_dar_allow_undefined='$wl-flat_namespace $wl-undefined ${wl}suppress' ;;
-    darwin*) # darwin 5.x on
-      # if running on 10.5 or later, the deployment target defaults
-      # to the OS version, if on x86, and 10.4, the deployment
-      # target defaults to 10.4. Don't you love it?
-      case ${MACOSX_DEPLOYMENT_TARGET-10.0},$host in
-       10.0,*86*-darwin8*|10.0,*-darwin[[91]]*)
-         _lt_dar_allow_undefined='$wl-undefined ${wl}dynamic_lookup' ;;
-       10.[[012]][[,.]]*)
+    darwin*)
+      case ${MACOSX_DEPLOYMENT_TARGET},$host in
+       10.[[012]],*|,*powerpc*)
          _lt_dar_allow_undefined='$wl-flat_namespace $wl-undefined ${wl}suppress' ;;
-       10.*)
+       *)
          _lt_dar_allow_undefined='$wl-undefined ${wl}dynamic_lookup' ;;
       esac
     ;;
EOF 

    if [[ $? != 0 ]] ; then
        warn "mpich 3.3.1 patch for confdb/libtool.m4 failed."
        return 1
    fi
    return 0;
}

function apply_mpich_patch
{   
    if [[ "$OPSYS" == "Darwin" ]]; then
        apply_mpich_331_libtool_darwin_patch
        if [[ $? != 0 ]] ; then 
            return 1
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
    # Apply patches
    #
    info "Patching MPICH . . ."
    apply_mpich_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_mpich == 1 ]] ; then
            warn "Giving up on MPICH build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Call configure
    #
    info "Configuring MPICH . . ."
    info "Invoking command to configure MPICH"

    #
    # Turn on shared version of the libs
    #
    mpich_opts="--enable-shared"
    if [[ "$OPSYS" == "Darwin" ]]; then
        mpich_opts="${mpich_opts} --enable-two-level-namespace --enable-threads=single CPPFLAGS=-DMPICH_MPI_FROM_PMPI"
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
    issue_command env PATH=`pwd`:${PATH} CXX="$CXX_COMPILER" \
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
    env PATH=`pwd`:${PATH} $MAKE $MAKE_OPT_FLAGS
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
