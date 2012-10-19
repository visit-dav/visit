function bv_netcdf_initialize
{
export DO_NETCDF="no"
export ON_NETCDF="off"
export USE_SYSTEM_NETCDF="no"
add_extra_commandline_args "netcdf" "alt-netcdf-dir" 1 "Use alternative directory for netcdf"
}

function bv_netcdf_enable
{
DO_NETCDF="yes"
ON_NETCDF="on"
}

function bv_netcdf_disable
{
DO_NETCDF="no"
ON_NETCDF="off"
}

function bv_netcdf_alt_netcdf_dir
{
    bv_netcdf_enable
    USE_SYSTEM_NETCDF="yes"
    NETCDF_INSTALL_DIR="$1"
}

function bv_netcdf_depends_on
{
    if [[ "$USE_SYSTEM_NETCDF" == "yes" ]]; then
        echo ""
    else
        local depends_on=""
        if [[ "$DO_HDF5" == "yes" ]] ; then
            depends_on="hdf5"        
            if [[ "$DO_SZIP" == "yes" ]] ; then
                depends_on="${depends_on} szip"        
            fi
        fi
        echo ${depends_on}
    fi
}

function bv_netcdf_initialize_vars
{
    if [[ "$USE_SYSTEM_NETCDF" == "no" ]]; then
            NETCDF_INSTALL_DIR="${VISITDIR}/netcdf/$NETCDF_VERSION/${VISITARCH})"
    fi
}

function bv_netcdf_info
{
export NETCDF_VERSION=${NETCDF_VERSION-"4.1.1"}
export NETCDF_FILE=${NETCDF_FILE-"netcdf-${NETCDF_VERSION}.tar.gz"}
export NETCDF_COMPATIBILITY_VERSION=${NETCDF_COMPATIBILITY_VERSION-"4.1"}
export NETCDF_BUILD_DIR=${NETCDF_BUILD_DIR-"netcdf-4.1.1"}
export NETCDF_MD5_CHECKSUM="79c5ff14c80d5e18dd8f1fceeae1c8e1"
export NETCDF_SHA256_CHECKSUM=""
}

function bv_netcdf_print
{
  printf "%s%s\n" "NETCDF_FILE=" "${NETCDF_FILE}"
  printf "%s%s\n" "NETCDF_VERSION=" "${NETCDF_VERSION}"
  printf "%s%s\n" "NETCDF_COMPATIBILITY_VERSION=" "${NETCDF_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "NETCDF_BUILD_DIR=" "${NETCDF_BUILD_DIR}"
}

function bv_netcdf_print_usage
{
printf "\t\t%15s\n" "NOTE: not available for download from web"
printf "%-15s %s [%s]\n" "--netcdf" "Build NetCDF" "${DO_NETCDF}"
}

function bv_netcdf_graphical
{
local graphical_out="NetCDF   $NETCDF_VERSION($NETCDF_FILE)    $ON_NETCDF"
echo "$graphical_out"
}

function bv_netcdf_host_profile
{
    if [[ "$DO_NETCDF" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## NetCDF" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_NETCDF" == "yes" ]]; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR $NETCDF_INSTALL_DIR)" \
            >> $HOSTCONF
        else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR \${VISITHOME}/netcdf/$NETCDF_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
            if [[ "$DO_HDF5" == "yes" ]] ; then
                echo \
                "VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_LIBRARY_DIR hdf5_hl HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
                >> $HOSTCONF
            fi
        fi
    fi
}

function bv_netcdf_ensure
{
    if [[ "$DO_NETCDF" == "yes" && "$USE_SYSTEM_NETCDF" == "no" ]] ; then
        ensure_built_or_ready "netcdf" $NETCDF_VERSION $NETCDF_BUILD_DIR \
          $NETCDF_FILE \
          http://www.unidata.ucar.edu/downloads/netcdf/ftp/
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_NETCDF="no"
            error "Unable to build NetCDF.  ${NETCDF_FILE} not found."
        fi
    fi
}

function bv_netcdf_dry_run
{
  if [[ "$DO_NETCDF" == "yes" ]] ; then
    echo "Dry run option not set for netcdf."
  fi
}

# *************************************************************************** #
#                         Function 8.4, build_netcdf                          #
#                                                                             #
# Mark C. Miller, Wed Oct 27 19:25:09 PDT 2010                                #
# Added patch for exodusII. This way, a single netcdf installation should     #
# work for 'normal' netcdf operations as well as for ExodusII.                #
# *************************************************************************** #
function apply_netcdf_patch_for_exodusii
{
    local retval=0
    pushd $NETCDF_BUILD_DIR 1>/dev/null 2>&1
    patch -p0 << \EOF
*** libsrc/netcdf.h	Wed Oct 27 11:50:22 2010
--- libsrc/netcdf.h.ex	Wed Oct 27 11:50:31 2010
***************
*** 141,151 ****
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS	1024	 /* max dimensions per file */
! #define NC_MAX_ATTRS	8192	 /* max global or per variable attributes */
! #define NC_MAX_VARS	8192	 /* max variables per file */
! #define NC_MAX_NAME	256	 /* max length of a name */
! #define NC_MAX_VAR_DIMS	NC_MAX_DIMS /* max per variable dimensions */
  
  /*
   * The netcdf version 3 functions all return integer error status.
--- 141,152 ----
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS	65536	 /* max dimensions per file */
! #define NC_MAX_ATTRS	8192	 /* max global or per variable attributes */
! #define NC_MAX_VARS	524288	 /* max variables per file */
! #define NC_MAX_NAME	256	 /* max length of a name */
! #define NC_MAX_VAR_DIMS 8      /* max per variable dimensions */
! 
  
  /*
   * The netcdf version 3 functions all return integer error status.
EOF
    retval1=$?
    patch -p0 << \EOF
*** libsrc4/netcdf.h	2010-04-12 11:48:02.000000000 -0700
--- libsrc4/netcdf.h.ex	2011-01-03 15:51:46.000000000 -0800
***************
*** 199,209 ****
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS	1024	 /* max dimensions per file */
  #define NC_MAX_ATTRS	8192	 /* max global or per variable attributes */
! #define NC_MAX_VARS	8192	 /* max variables per file */
  #define NC_MAX_NAME	256	 /* max length of a name */
! #define NC_MAX_VAR_DIMS	NC_MAX_DIMS /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
--- 199,209 ----
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS	65536	 /* max dimensions per file */
  #define NC_MAX_ATTRS	8192	 /* max global or per variable attributes */
! #define NC_MAX_VARS	524288	 /* max variables per file */
  #define NC_MAX_NAME	256	 /* max length of a name */
! #define NC_MAX_VAR_DIMS	8        /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
EOF
    retval2=$?
    patch -p0 << \EOF
*** libsrc4/netcdf_base.h	2010-01-21 08:00:18.000000000 -0800
--- libsrc4/netcdf_base.h.ex	2011-01-03 16:03:36.000000000 -0800
***************
*** 192,202 ****
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS	1024	 /* max dimensions per file */
  #define NC_MAX_ATTRS	8192	 /* max global or per variable attributes */
! #define NC_MAX_VARS	8192	 /* max variables per file */
  #define NC_MAX_NAME	256	 /* max length of a name */
! #define NC_MAX_VAR_DIMS	NC_MAX_DIMS /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
--- 192,202 ----
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS	65536	 /* max dimensions per file */
  #define NC_MAX_ATTRS	8192	 /* max global or per variable attributes */
! #define NC_MAX_VARS	524288	 /* max variables per file */
  #define NC_MAX_NAME	256	 /* max length of a name */
! #define NC_MAX_VAR_DIMS	8        /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
EOF
    retval3=$?
    popd 1>/dev/null 2>&1
    if [[ $retval1 -eq 0 && $retval2 -eq 0 && $retval3 -eq 0 ]]; then
        return 0
    fi
    return 1
}

function apply_netcdf_patch
{
   apply_netcdf_patch_for_exodusii
   return $?
}

function build_netcdf
{
    # Prepare build dir
    #
    prepare_build_dir $NETCDF_BUILD_DIR $NETCDF_FILE
    untarred_netcdf=$?
    if [[ $untarred_netcdf == -1 ]] ; then
       warn "Unable to prepare NetCDF Build Directory. Giving Up"
       return 1
    fi

    #
    # Apply patches
    #
    info "Patching NetCDF . . ."
    apply_netcdf_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_netcdf == 1 ]] ; then
          warn "Giving up on NetCDF build because the patch failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script\n" \
          warn "tried to apply a patch to an existing directory which had\n" \
          warn "already been patched ... that is, that the patch is\n" \
          warn "failing harmlessly on a second application."
       fi
    fi

    info "Configuring NetCDF . . ."
    cd $NETCDF_BUILD_DIR || error "Can't cd to netcdf build dir."
    info "Invoking command to configure NetCDF"
    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "$DO_STATIC_BUILD" == "no" ]]; then
            EXTRA_FLAGS="--enable-largefile --enable-shared --disable-static"
        else
            EXTRA_FLAGS="--enable-largefile"
        fi
    else
        EXTRA_FLAGS=""
    fi
    H5ARGS=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        H5ARGS="--enable-netcdf4"
        H5ARGS="$H5ARGS --with-hdf5=$HDF5_INSTALL_DIR"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            H5ARGS="$H5ARGS --with-szlib=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        fi
    fi

    info "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$C_OPT_FLAGS\" CXXFLAGS=\"$CXX_OPT_FLAGS\" \
        FC=\"\" $EXTRA_FLAGS --enable-cxx-4 $H5ARGS \
        --disable-dap \
        --prefix=\"$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH\""

    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        FC="" $EXTRA_FLAGS --enable-cxx-4 $H5ARGS \
        --disable-dap \
        --prefix="$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH"

    if [[ $? != 0 ]] ; then
        warn "NetCDF configure failed.  Giving up"
        return 1
    fi

    #
    # Build NetCDF
    #
    info "Building NetCDF . . . (~2 minutes)"
    $MAKE
    if [[ $? != 0 ]] ; then
        warn "NetCDF build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing NetCDF . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "NetCDF install failed.  Giving up"
        return 1
    fi

    #
    # Patch up the library names on Darwin.
    #
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        info "Creating dynamic libraries for NetCDF . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/netcdf"
       chgrp -R ${GROUP} "$VISITDIR/netcdf"
    fi
    cd "$START_DIR"
    info "Done with NetCDF"
    return 0
}

function bv_netcdf_is_enabled
{
    if [[ $DO_NETCDF == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_netcdf_is_installed
{
    if [[ "$USE_SYSTEM_NETCDF" == "yes" ]]; then
        return 1
    fi

    check_if_installed "netcdf" $NETCDF_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_netcdf_build
{
cd "$START_DIR"
if [[ "$DO_NETCDF" == "yes" && "$USE_SYSTEM_NETCDF" == "no" ]] ; then
    check_if_installed "netcdf" $NETCDF_VERSION
   if [[ $? == 0 ]] ; then
        info "Skipping NetCDF build.  NetCDF is already installed."
   else
        info "Building NetCDF (~5 minutes)"
        build_netcdf
        if [[ $? != 0 ]] ; then
            error "Unable to build or install NetCDF.  Bailing out."
        fi
        info "Done building NetCDF"
   fi
fi
}
