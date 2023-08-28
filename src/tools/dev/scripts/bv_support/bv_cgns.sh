function bv_cgns_initialize
{
    export DO_CGNS="no"
}

function bv_cgns_enable
{
    DO_CGNS="yes"
}

function bv_cgns_disable
{
    DO_CGNS="no"
}

function bv_cgns_depends_on
{
    local depends=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends="hdf5"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            depends="szip hdf5"
        fi
    fi
    
    echo $depends
}

function bv_cgns_info
{
    export CGNS_FILE=${CGNS_FILE:-"CGNS-4.1.0.tar.gz"}
    export CGNS_VERSION=${CGNS_VERSION:-"4.1.0"}
    export CGNS_COMPATIBILITY_VERSION=${CGNS_COMPATIBILITY_VERSION:-"4.1"}
    export CGNS_BUILD_DIR=${CGNS_BUILD_DIR:-"CGNS-4.1.0/src"}
    export CGNS_MD5_CHECKSUM="f90b85ae10693d4db0825c7ce61c6f73"
    export CGNS_SHA256_CHECKSUM="b4584e4d0fa52c737a0fb4738157a88581df251c8c5886175ee287e1777e99fd"
}

function bv_cgns_print
{
    printf "%s%s\n" "CGNS_FILE=" "${CGNS_FILE}"
    printf "%s%s\n" "CGNS_VERSION=" "${CGNS_VERSION}"
    printf "%s%s\n" "CGNS_COMPATIBILITY_VERSION=" "${CGNS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CGNS_BUILD_DIR=" "${CGNS_BUILD_DIR}"
}

function bv_cgns_print_usage
{
    printf "%-20s %s [%s]\n" "--cgns"    "Build CGNS" "$DO_CGNS" 
}

function bv_cgns_host_profile
{
    if [[ "$DO_CGNS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CGNS" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR \${VISITHOME}/cgns/$CGNS_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        if [[ "$DO_HDF5" == "yes" ]] ; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi

}

function bv_cgns_ensure
{
    if [[ "$DO_CGNS" == "yes" ]] ; then
        ensure_built_or_ready "cgns" $CGNS_VERSION $CGNS_BUILD_DIR $CGNS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CGNS="no"
            error "Unable to build CGNS.  ${CGNS_FILE} not found."
        fi
    fi
}

function apply_cgns_410_patch
{
    info "Patching CGNS 4.1.0"
    patch -p0 << \EOF
diff -c CGNS-4.1.0/src/configure.orig CGNS-4.1.0/src/configure
*** CGNS-4.1.0/src/configure.orig	Thu Feb 11 17:51:22 2021
--- CGNS-4.1.0/src/configure	Fri Feb 12 07:55:02 2021
***************
*** 5939,5945 ****
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-lz  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
--- 5939,5945 ----
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="$ZLIBLIB -lz  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
***************
*** 5974,5980 ****
  #define HAVE_LIBZ 1
  _ACEOF
  
!   LIBS="-lz $LIBS"
  
  else
    unset HAVE_ZLIB
--- 5974,5980 ----
  #define HAVE_LIBZ 1
  _ACEOF
  
!   LIBS="$ZLIBLIB -lz $LIBS"
  
  else
    unset HAVE_ZLIB
***************
*** 6031,6037 ****
  
  
      if test $shared = yes; then
!       ZLIBLIB="-L$zlib_lib"
      else
        if test -n "$zlib_lib"; then
          for a in $exts ; do
--- 6031,6037 ----
  
  
      if test $shared = yes; then
!       ZLIBLIB="-L$zlib_lib -lz"
      else
        if test -n "$zlib_lib"; then
          for a in $exts ; do
***************
*** 6050,6056 ****
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-lz  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
--- 6050,6056 ----
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="$ZLIBLIB $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
***************
*** 6085,6091 ****
  #define HAVE_LIBZ 1
  _ACEOF
  
!   LIBS="-lz $LIBS"
  
  else
    unset HAVE_ZLIB
--- 6085,6091 ----
  #define HAVE_LIBZ 1
  _ACEOF
  
!   LIBS="$ZLIBLIB $LIBS"
  
  else
    unset HAVE_ZLIB
***************
*** 6147,6153 ****
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-lsz  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
--- 6147,6153 ----
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="$SZIPLIB -lsz  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
***************
*** 6182,6188 ****
  #define HAVE_LIBSZ 1
  _ACEOF
  
!   LIBS="-lsz $LIBS"
  
  else
    unset HAVE_SZIP
--- 6182,6188 ----
  #define HAVE_LIBSZ 1
  _ACEOF
  
!   LIBS="$SZIPLIB -lsz $LIBS"
  
  else
    unset HAVE_SZIP
***************
*** 6239,6245 ****
  
  
      if test $shared = yes; then
!       SZIPLIB="-L$szip_lib"
      else
        if test -n "$szip_lib"; then
          for a in $exts ; do
--- 6239,6245 ----
  
  
      if test $shared = yes; then
!       SZIPLIB="-L$szip_lib -lsz"
      else
        if test -n "$szip_lib"; then
          for a in $exts ; do
***************
*** 6258,6264 ****
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-lsz  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
--- 6258,6264 ----
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="$SZIPLIB $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */
  
***************
*** 6293,6299 ****
  #define HAVE_LIBSZ 1
  _ACEOF
  
!   LIBS="-lsz $LIBS"
  
  else
    unset HAVE_SZIP
--- 6293,6299 ----
  #define HAVE_LIBSZ 1
  _ACEOF
  
!   LIBS="$SZIPLIB $LIBS"
  
  else
    unset HAVE_SZIP
EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi

    patch -p0 << \EOF
diff -u CGNS-4.1.0/src/Makefile.in.orig CGNS-4.1.0/src/Makefile.in
--- CGNS-4.1.0/src/Makefile.in.orig	2021-01-29 09:15:50.000000000 -0800
+++ CGNS-4.1.0/src/Makefile.in	2021-05-05 08:52:32.000000000 -0700
@@ -53,7 +53,7 @@
 
 $(CGNSLIB) : $(OBJDIR) $(CGNSOBJS) $(FGNSOBJS) $(ADFOBJS) $(F2COBJS)
 	-@$(RM) $@
-	@AR_LIB@ $@ $(CGNSOBJS) $(FGNSOBJS) $(ADFOBJS) $(F2COBJS)
+	@AR_LIB@ $@ $(LDFLAGS) $(CGNSOBJS) $(FGNSOBJS) $(ADFOBJS) $(F2COBJS) $(CLIBS)
 	@RAN_LIB@ $@
 
 $(OBJDIR) :
EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}

function apply_cgns_patch
{
    if [[ ${CGNS_VERSION} == "4.1.0" ]] ; then
        apply_cgns_410_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.5, build_cgns                            #
#                                                                             #
# Kevin Griffin, Tue Dec 30 11:39:02 PST 2014                                 #
# Added a patch for the configure script to correctly locate and use the      #
# the dylib for hdf5 and szip. Added the correct linker options to the        #
# dynamic library creation.                                                   #
#                                                                             #
# Kevin Griffin, Fri Jan 16 10:28:21 PST 2015                                 #
# Fixed the --with-szip and --with-zlib to specify the full path to the       #
# library for both OSX and linux                                              #
#                                                                             #
# Kevin Griffin, Mon Apr 27 15:20:43 PDT 2015                                 #
# Patched the configure file to use the zlib library specified in the         #
# --with-zlib option.                                                         #
#                                                                             #
# *************************************************************************** #

function build_cgns
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CGNS_BUILD_DIR $CGNS_FILE
    untarred_cgns=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_cgns == -1 ]] ; then
        warn "Unable to prepare CGNS Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_cgns_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_cgns == 1 ]] ; then
            warn "Giving up on CGNS build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure CGNS
    #
    info "Configuring CGNS . . ."
    cd $CGNS_BUILD_DIR || error "Can't cd to CGNS build dir."
    info "Invoking command to configure CGNS"
    LIBEXT=""
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cf_build_type=""
        LIBEXT="a"
    else
        cf_build_type="--enable-shared=all"
        if [[ "$OPSYS" == "Darwin" ]] ; then
            LIBEXT="dylib"
        else
            LIBEXT="so"
        fi
    fi

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cf_build_type="$cf_build_type --enable-debug"
    fi

    # optionally add HDF5 and szip to the configure.
    LIBS_ENV=""
    LDFLAGS_ENV=""
    H5ARGS=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        LIBS_ENV="-lhdf5"
        LDFLAGS_ENV="-L$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib"
        H5ARGS="--with-hdf5=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            LIBS_ENV="$LIBS_ENV -lsz"
            LDFLAGS_ENV="$LDFLAGS_ENV -L$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib"
            H5ARGS="$H5ARGS --with-szip=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        fi
        LIBS_ENV="$LIBS_ENV -lz"
        LDFLAGS_ENV="$LDFLAGS_ENV -L$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib"
        H5ARGS="$H5ARGS --with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"
    fi

    # Disable fortran
    FORTRANARGS="--with-fortran=no"

    set -x
    if [[ "$OPSYS" == "Darwin" ]] ; then
        env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
            CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
            LDFLAGS="$LDFLAGS_ENV" LIBS="$LIBS_ENV" \
            ./configure --enable-64bit --enable-cgnstools=no ${cf_build_type} $H5ARGS $FORTRANARGS --prefix="$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH"
    else
        env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
            CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
            ./configure --enable-64bit --enable-cgnstools=no ${cf_build_type} $H5ARGS $FORTRANARGS --prefix="$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH"
    fi
    set +x

    if [[ $? != 0 ]] ; then
        warn "CGNS configure failed.  Giving up"
        return 1
    fi

    #
    # Build CGNS
    #
    info "Building CGNS . . . (~2 minutes)"

    $MAKE cgns
    if [[ $? != 0 ]] ; then
        warn "CGNS build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing CGNS . . ."

    $MAKE install-cgns
    if [[ $? != 0 ]] ; then
        warn "CGNS install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for CGNS . . ."

        INSTALLNAMEPATH="$VISITDIR/cgns/${CGNS_VERSION}/$VISITARCH/lib"

        $C_COMPILER -dynamiclib -o libcgns.${SO_EXT} lib/*.o \
                    -Wl,-headerpad_max_install_names \
                    -Wl,-twolevel_namespace,-undefined,dynamic_lookup \
                    -Wl,-install_name,$INSTALLNAMEPATH/libcgns.${SO_EXT} \
                    -Wl,-compatibility_version,$CGNS_COMPATIBILITY_VERSION \
                    -Wl,-current_version,$CGNS_VERSION -lSystem 
        if [[ $? != 0 ]] ; then
            warn "CGNS dynamic library creation failed.  Giving up"
            return 1
        fi
        rm -f "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/lib/libcgns.${SO_EXT}"
        cp libcgns.${SO_EXT} "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/lib"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/cgns"
        chgrp -R ${GROUP} "$VISITDIR/cgns"
    fi
    cd "$START_DIR"
    info "Done with CGNS"
    return 0
}

function bv_cgns_is_enabled
{
    if [[ $DO_CGNS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cgns_is_installed
{
    check_if_installed "cgns" $CGNS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cgns_build
{
    cd "$START_DIR"
    if [[ "$DO_CGNS" == "yes" ]] ; then
        check_if_installed "cgns" $CGNS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CGNS build.  CGNS is already installed."
        else
            info "Building CGNS (~2 minutes)"
            build_cgns
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CGNS.  Bailing out."
            fi
            info "Done building CGNS"
        fi
    fi
}
