function bv_netcdf_initialize
{
    export DO_NETCDF="no"
    export USE_SYSTEM_NETCDF="no"
    add_extra_commandline_args "netcdf" "alt-netcdf-dir" 1 "Use alternative directory for netcdf"
}

function bv_netcdf_enable
{
    DO_NETCDF="yes"
}

function bv_netcdf_disable
{
    DO_NETCDF="no"
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
        local depends_on="zlib"
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
        NETCDF_INSTALL_DIR="${VISITDIR}/netcdf/$NETCDF_VERSION/${VISITARCH}"
    fi
}

function bv_netcdf_info
{
    export NETCDF_VERSION=${NETCDF_VERSION-"4.1.1"}
    export NETCDF_FILE=${NETCDF_FILE-"netcdf-${NETCDF_VERSION}.tar.gz"}
    export NETCDF_COMPATIBILITY_VERSION=${NETCDF_COMPATIBILITY_VERSION-"4.1"}
    export NETCDF_BUILD_DIR=${NETCDF_BUILD_DIR-"netcdf-4.1.1"}
    export NETCDF_MD5_CHECKSUM="79c5ff14c80d5e18dd8f1fceeae1c8e1"
    export NETCDF_SHA256_CHECKSUM="7933d69d378c57f038375bae4dd78c52442a06e2647fce4b75c13a225e342fb0"
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
    printf "%-20s %s [%s]\n" "--netcdf" "Build NetCDF" "${DO_NETCDF}"
    printf "%-20s %s [%s]\n" "--alt-netcdf-dir" "Use NetCDF from an alternative directory"
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

function apply_netcdf_411_macOS_patch
{
    patch -p0 << \EOF
diff -c netcdf-4.1.1/ncgen3/orig/genlib.h netcdf-4.1.1/ncgen3/genlib.h 
*** netcdf-4.1.1/ncgen3/orig/genlib.h	Thu Aug 23 21:46:38 2018
--- netcdf-4.1.1/ncgen3/genlib.h	Thu Aug 23 21:07:33 2018
***************
*** 5,10 ****
--- 5,11 ----
   *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
   *   $Header: /upc/share/CVS/netcdf-3/ncgen3/genlib.h,v 1.15 2009/12/29 18:42:35 dmh Exp $
   *********************************************************************/
+ #include <config.h>
  #include <stdlib.h>
  #include <limits.h>
  
EOF
    if [[ $? != 0 ]] ; then
        warn "netcdf 4.1.1 OSX 10.13 patch failed."
        return 1
    fi

    return 0;
}

function apply_netcdf_411_darwin_patch
{
    patch -p0 << \EOF
diff -c netcdf-4.1.1/ncgen3/genlib.h.orig netcdf-4.1.1/ncgen3/genlib.h
*** netcdf-4.1.1/ncgen3/genlib.h.orig   2014-11-13 17:16:23.000000000 -0800
--- netcdf-4.1.1/ncgen3/genlib.h        2014-11-13 16:27:08.000000000 -0800
***************
*** 81,87 ****
  
  /* In case we are missing strlcat */
  #ifndef HAVE_STRLCAT
! extern size_t strlcat(char *dst, const char *src, size_t siz);
  #endif
  
  #ifdef __cplusplus
--- 81,87 ----
  
  /* In case we are missing strlcat */
  #ifndef HAVE_STRLCAT
! /* extern size_t strlcat(char *dst, const char *src, size_t siz); */
  #endif
  
  #ifdef __cplusplus
EOF

    if [[ $? == 0 ]] ; then
        return 0;
    fi

    return 1;
}

function apply_netcdf_patch_for_exodusii
{
    local retval=0
    pushd $NETCDF_BUILD_DIR 1>/dev/null 2>&1
    patch -p0 << \EOF
*** libsrc/netcdf.h     Wed Oct 27 11:50:22 2010
--- libsrc/netcdf.h.ex  Wed Oct 27 11:50:31 2010
***************
*** 141,151 ****
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS   1024     /* max dimensions per file */
! #define NC_MAX_ATTRS  8192     /* max global or per variable attributes */
! #define NC_MAX_VARS   8192     /* max variables per file */
! #define NC_MAX_NAME   256      /* max length of a name */
! #define NC_MAX_VAR_DIMS       NC_MAX_DIMS /* max per variable dimensions */
  
  /*
   * The netcdf version 3 functions all return integer error status.
--- 141,152 ----
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS   65536    /* max dimensions per file */
! #define NC_MAX_ATTRS  8192     /* max global or per variable attributes */
! #define NC_MAX_VARS   524288   /* max variables per file */
! #define NC_MAX_NAME   256      /* max length of a name */
! #define NC_MAX_VAR_DIMS 8      /* max per variable dimensions */
! 
  
  /*
   * The netcdf version 3 functions all return integer error status.
EOF
    retval1=$?
    patch -p0 << \EOF
*** libsrc4/netcdf.h    2010-04-12 11:48:02.000000000 -0700
--- libsrc4/netcdf.h.ex 2011-01-03 15:51:46.000000000 -0800
***************
*** 199,209 ****
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS   1024     /* max dimensions per file */
  #define NC_MAX_ATTRS  8192     /* max global or per variable attributes */
! #define NC_MAX_VARS   8192     /* max variables per file */
  #define NC_MAX_NAME   256      /* max length of a name */
! #define NC_MAX_VAR_DIMS       NC_MAX_DIMS /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
--- 199,209 ----
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS   65536    /* max dimensions per file */
  #define NC_MAX_ATTRS  8192     /* max global or per variable attributes */
! #define NC_MAX_VARS   524288   /* max variables per file */
  #define NC_MAX_NAME   256      /* max length of a name */
! #define NC_MAX_VAR_DIMS       8        /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
EOF
    retval2=$?
    patch -p0 << \EOF
*** libsrc4/netcdf_base.h       2010-01-21 08:00:18.000000000 -0800
--- libsrc4/netcdf_base.h.ex    2011-01-03 16:03:36.000000000 -0800
***************
*** 192,202 ****
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS   1024     /* max dimensions per file */
  #define NC_MAX_ATTRS  8192     /* max global or per variable attributes */
! #define NC_MAX_VARS   8192     /* max variables per file */
  #define NC_MAX_NAME   256      /* max length of a name */
! #define NC_MAX_VAR_DIMS       NC_MAX_DIMS /* max per variable dimensions */
  
  /* In HDF5 files you can set the endianness of variables with
   * nc_def_var_endian(). These defines are used there. */   
--- 192,202 ----
   * applications and utilities.  However, nothing is statically allocated to
   * these sizes internally.
   */
! #define NC_MAX_DIMS   65536    /* max dimensions per file */
  #define NC_MAX_ATTRS  8192     /* max global or per variable attributes */
! #define NC_MAX_VARS   524288   /* max variables per file */
  #define NC_MAX_NAME   256      /* max length of a name */
! #define NC_MAX_VAR_DIMS       8        /* max per variable dimensions */
  
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

function apply_netcdf_strlcat_patch
{
    info "Patching netcdf for strlcat"
    patch -p0 << \EOF
*** ncgen3/genlib.h.orig	2023-04-14 11:00:05.000000000 -0700
--- ncgen3/genlib.h	2023-04-07 17:04:37.000000000 -0700
*************** extern void nc_fill ( nc_type  type, siz
*** 81,89 ****
--- 81,91 ----
  extern void clearout(void);
  
  /* In case we are missing strlcat */
+ #if 0
  #ifndef HAVE_STRLCAT
  extern size_t strlcat(char *dst, const char *src, size_t siz);
  #endif
+ #endif
  
  #ifdef __cplusplus
  }
EOF

    if [[ $? != 0 ]] ; then
      warn "netcdf patch for strlcat failed."
      return 1
    fi
    return 0;
}

function apply_netcdf_patch
{
    apply_netcdf_patch_for_exodusii

    if [[ ${NETCDF_VERSION} == 4.1.1 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]] ; then
            productVersion=`sw_vers -productVersion`
            if [[ $productVersion == 10.9.[0-9]* ||
                  $productVersion == 10.10.[0-9]* ||
                  $productVersion == 10.11.[0-9]* ||
                  $productVersion == 10.12.[0-9]* ]] ; then
                info "Applying OS X 10.9 and up patch . . ."
                apply_netcdf_411_darwin_patch
            fi
            
            if [[ $productVersion == 10.13.[0-9]* ||
                  $productVersion == 10.14.[0-9]* || 
                  $productVersion == 10.15.[0-9]* ]] ; then
                info "Applying macOS 10.13 and up patch . . ."
                apply_netcdf_411_macOS_patch
            fi
        fi
    fi
    
    if [[ "$OPSYS" == "Darwin" ]] ; then
        apply_netcdf_strlcat_patch
        if [[ $? != 0 ]] ; then
           return 1
        fi
    fi

    return $?
}

# *************************************************************************** #
#                         Function 8.4, build_netcdf                          #
#                                                                             #
# Mark C. Miller, Wed Oct 27 19:25:09 PDT 2010                                #
# Added patch for exodusII. This way, a single netcdf installation should     #
# work for 'normal' netcdf operations as well as for ExodusII.                #
#                                                                             #
# Kevin Griffin, Mon Nov 17 11:31:52 PST 2014                                 #
# Added patch for OS X 10.9 Mavericks. HAVE_STRLCAT is not getting defined    #
# in this version so its trying to add a duplicate strlcat definition. This   #
# patch comments out the duplicate strlcat definition.                        #
# *************************************************************************** #
function build_netcdf
{
    # Prepare build dir
    #
    prepare_build_dir $NETCDF_BUILD_DIR $NETCDF_FILE
    untarred_netcdf=$?
    # 0, already exists, 1 untarred src, 2 error

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
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure NetCDF
    #
    info "Configuring NetCDF . . ."
    cd $NETCDF_BUILD_DIR || error "Can't cd to netcdf build dir."
    info "Invoking command to configure NetCDF"
    EXTRA_FLAGS=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "$DO_STATIC_BUILD" == "no" ]]; then
            EXTRA_FLAGS="--enable-largefile --enable-shared --disable-static"
        else
            EXTRA_FLAGS="--enable-largefile"
        fi
    fi
    EXTRA_AC_FLAGS=""
    # detect coral systems, which older versions of autoconf don't detect
    if [[ "$(uname -m)" == "ppc64le" ]] ; then
         EXTRA_AC_FLAGS="ac_cv_build=powerpc64le-unknown-linux-gnu"
    fi
    H5ARGS=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        H5ARGS="--enable-netcdf4"
        H5ARGS="$H5ARGS --with-hdf5=$HDF5_INSTALL_DIR"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            H5ARGS="$H5ARGS --with-szlib=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        fi
    fi
    ZLIBARGS="--with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"

    # probably always best to do this now with newer compilers
    CFLAGS="$CFLAGS -Wno-error=implicit-function-declaration"

    set -x
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
                CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
                FC="" $EXTRA_AC_FLAGS $EXTRA_FLAGS --enable-cxx-4 $H5ARGS $ZLIBARGS\
                --disable-dap \
                --prefix="$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH"
    set +x


    if [[ $? != 0 ]] ; then
        warn "NetCDF configure failed.  Giving up"
        return 1
    fi

    # there is an include file on newer macOS #include <version> which case-clashes
    # with any file living in a dir that is -I included on the compilation line
    if [[ "$OPSYS" == "Darwin" ]]; then
        mv -f VERSION VERSION.orig
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
