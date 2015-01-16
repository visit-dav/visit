function bv_cgns_initialize
{
export DO_CGNS="no"
export ON_CGNS="off"
}

function bv_cgns_enable
{
DO_CGNS="yes"
ON_CGNS="on"
}

function bv_cgns_disable
{
DO_CGNS="no"
ON_CGNS="off"
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
export CGNS_FILE=${CGNS_FILE:-"cgnslib_3.2.1.tar.gz"}
export CGNS_VERSION=${CGNS_VERSION:-"3.2.1"}
export CGNS_COMPATIBILITY_VERSION=${CGNS_COMPATIBILITY_VERSION:-"3.2"}
export CGNS_BUILD_DIR=${CGNS_BUILD_DIR:-"cgnslib_3.2.1/src"}
export CGNS_MD5_CHECKSUM="2d26f88b2058dcd0ee5ce58f483bfccb"
export CGNS_SHA256_CHECKSUM="34306316f04dbf6484343a4bc611b3bf912ac7dbc3c13b581defdaebbf6c1fc3"

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
printf "%-15s %s [%s]\n" "--cgns"    "Build CGNS" "$DO_CGNS" 
}

function bv_cgns_graphical
{
local graphical_out="CGNS     $CGNS_VERSION($CGNS_FILE)      $ON_CGNS"
echo $graphical_out
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

function bv_cgns_dry_run
{
  if [[ "$DO_CGNS" == "yes" ]] ; then
    echo "Dry run option not set for cgns."
  fi
}

function apply_cgns_321_darwin_patch
{
   patch -p0 << \EOF
diff -c cgnslib_3.2.1/src/configure.orig cgnslib_3.2.1/src/configure
*** cgnslib_3.2.1/src/configure.orig	2014-12-17 15:31:56.000000000 -0800
--- cgnslib_3.2.1/src/configure	2014-12-17 15:33:22.000000000 -0800
***************
*** 2324,2333 ****
  echo "$ac_t""$shared" 1>&6
  
  if test $shared = all; then
!   exts="so sl a"
    shared=yes
  else
!   exts="a so sl"
  fi
  if test $shared = yes; then
    cgnsdir=`pwd`
--- 2324,2333 ----
  echo "$ac_t""$shared" 1>&6
  
  if test $shared = all; then
!   exts="dylib so sl a"
    shared=yes
  else
!   exts="dylib a so sl"
  fi
  if test $shared = yes; then
    cgnsdir=`pwd`
***************
*** 2352,2363 ****
        shared=no
      else
        CFGFLAGS="-fPIC $CFGFLAGS"
!       AR_LIB="\$(CC) -shared $SYSCFLAGS -Wl,-rpath,$LIBDIR:$cgnsdir/$BUILDDIR -o"
!       EXT_LIB=so
      fi
    fi
    if test $shared = yes; then
!     RAN_LIB="\$(STRIP)"
    fi
  fi
  
--- 2352,2363 ----
        shared=no
      else
        CFGFLAGS="-fPIC $CFGFLAGS"
!       AR_LIB="\$(CC) -shared $SYSCFLAGS -Wl,-L$with_hdf5/lib -Wl,-lhdf5 -o"
!       EXT_LIB=dylib
      fi
    fi
    if test $shared = yes; then
!     RAN_LIB="\$(STRIP) -x"
    fi
  fi
  
EOF
   if [[ $? != 0 ]] ; then
      return 1
   fi

   return 0
}

function apply_cgns_321_patch
{
   if [[ "$OPSYS" == "Darwin" ]] ; then
       info "Applying OS X patch . . ."
       apply_cgns_321_darwin_patch
   fi

   return $?
}

function apply_cgns_patch
{
   if [[ ${CGNS_VERSION} == 3.2.1 ]] ; then
      apply_cgns_321_patch
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
# *************************************************************************** #

function build_cgns
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CGNS_BUILD_DIR $CGNS_FILE
    untarred_cgns=$?
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
          warn "tried to apply a patch to an existing directory which had\n" \
          warn "already been patched ... that is, that the patch is\n" \
          warn "failing harmlessly on a second application."
       fi
    fi

    info "Configuring CGNS . . ."
    cd $CGNS_BUILD_DIR || error "Can't cd to CGNS build dir."
    info "Invoking command to configure CGNS"
#    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
#            cf_build_type=""
#        else
#            cf_build_type="--enable-shared=all"
#    fi
    # optionally add HDF5 and szip to the configure.
    H5ARGS=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        H5ARGS="--with-hdf5=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            if [[ "$OPSYS" == "Darwin" ]] ; then
               H5ARGS="$H5ARGS --with-szip=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib/libsz.dylib"
            else
               H5ARGS="$H5ARGS --with-szip=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib/libsz.so"
            fi
        fi
        if [[ "$DO_ZLIB" == "yes" ]] ; then
            if [[ "$OPSYS" == "Darwin" ]] ; then
               H5ARGS="$H5ARGS --with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib/libz.dylib"
            else
               H5ARGS="$H5ARGS --with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib/libz.so"
            fi
        fi
    fi
    if [[ "$OPSYS" == "Darwin" ]] ; then
       info "    env CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
       CFLAGS=\"$C_OPT_FLAGS\" CXXFLAGS=\"$CXX_OPT_FLAGS\" \
       ./configure --enable-64bit --enable-shared=all $H5ARGS --prefix=\"$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH\""

       env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       ./configure --enable-64bit --enable-shared=all $H5ARGS --prefix=\"$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH\"
    else
       info "    env CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
       CFLAGS=\"$C_OPT_FLAGS\" CXXFLAGS=\"$CXX_OPT_FLAGS\" \
       ./configure --enable-64bit ${cf_build_type} $H5ARGS --prefix=\"$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH\""

       env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       ./configure --enable-64bit ${cf_build_type} $H5ARGS --prefix=\"$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH\"
    fi

    if [[ $? != 0 ]] ; then
       warn "CGNS configure failed.  Giving up"
       return 1
    fi

    #
    # Build CGNS
    #
    info "Building CGNS . . . (~2 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
       warn "CGNS build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing CGNS . . ."

    mkdir "$VISITDIR/cgns"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/include"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/lib"
    $MAKE install
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

