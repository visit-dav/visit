function bv_hdf5_initialize
{
    export DO_HDF5="no"
    export USE_SYSTEM_HDF5="no"
    add_extra_commandline_args "hdf5" "alt-hdf5-dir" 1 "Use alternative directory for hdf5"
}

function bv_hdf5_enable
{
    DO_HDF5="yes"
}

function bv_hdf5_disable
{
    DO_HDF5="no"
}

function bv_hdf5_alt_hdf5_dir
{
    bv_hdf5_enable
    USE_SYSTEM_HDF5="yes"
    HDF5_INSTALL_DIR="$1"
}

function bv_hdf5_depends_on
{
    if [[ "$USE_SYSTEM_HDF5" == "yes" ]]; then
        echo ""
    else
        local depends_on=""

        if [[ "$DO_ZLIB" == "yes" ]] ; then
            depends_on="$depends_on zlib"
        fi

        if [[ "$DO_SZIP" == "yes" ]] ; then
            depends_on="$depends_on szip"
        fi

        if [[ -n "$PAR_COMPILER" && "$DO_MOAB" == "yes"  && "$DO_MPICH" == "yes" ]]; then
            depends_on="$depends_on mpich"
        fi

        echo $depends_on
    fi
}

function bv_hdf5_initialize_vars
{
    if [[ "$USE_SYSTEM_HDF5" == "no" ]]; then
        HDF5_INSTALL_DIR="${VISITDIR}/hdf5/$HDF5_VERSION/${VISITARCH}"
        if [[ -n "$PAR_COMPILER" && "$DO_MOAB" == "yes" ]]; then
            HDF5_MPI_INSTALL_DIR="${VISITDIR}/hdf5_mpi/$HDF5_VERSION/${VISITARCH}"
        fi
    fi
}

function bv_hdf5_info
{
    export HDF5_VERSION=${HDF5_VERSION:-"1.8.14"}
    export HDF5_FILE=${HDF5_FILE:-"hdf5-${HDF5_VERSION}.tar.gz"}
    export HDF5_COMPATIBILITY_VERSION=${HDF5_COMPATIBILITY_VERSION:-"1.8"}
    export HDF5_BUILD_DIR=${HDF5_BUILD_DIR:-"hdf5-${HDF5_VERSION}"}
    # Note: Versions of HDF5 1.6.5 and earlier DO NOT have last path component
    export HDF5_URL=${HDF5_URL:-"http://www.hdfgroup.org/ftp/HDF5/prev-releases/hdf5-${HDF5_VERSION}/src"}
    export HDF5_SHA256_CHECKSUM="1dbefeeef7f591897c632b2b090db96bb8d35ad035beaa36bc39cb2bc67e0639"
}

function bv_hdf5_print
{
    printf "%s%s\n" "HDF5_FILE=" "${HDF5_FILE}"
    printf "%s%s\n" "HDF5_VERSION=" "${HDF5_VERSION}"
    printf "%s%s\n" "HDF5_COMPATIBILITY_VERSION=" "${HDF5_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "HDF5_BUILD_DIR=" "${HDF5_BUILD_DIR}"
}

function bv_hdf5_print_usage
{
    printf "%-20s %s [%s]\n" "--hdf5" "Build HDF5" "${DO_HDF5}"
    printf "%-20s %s [%s]\n" "--alt-hdf5-dir" "Use HDF5 from an alternative directory"
}

function bv_hdf5_host_profile
{
    if [[ "$DO_HDF5" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## HDF5" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_HDF5" == "yes" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR $HDF5_INSTALL_DIR)" \
                >> $HOSTCONF 
        else
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_DIR \${VISITHOME}/hdf5/$HDF5_VERSION/\${VISITARCH})" \
                >> $HOSTCONF 

            if [[ -n "$HDF5_MPI_INSTALL_DIR" ]]; then
                echo \
                    "VISIT_OPTION_DEFAULT(VISIT_HDF5_MPI_DIR \${VISITHOME}/hdf5_mpi/$HDF5_VERSION/\${VISITARCH})" \
                    >> $HOSTCONF 
            fi

            ZLIB_LIBDEP=""
            if [[ "$DO_ZLIB" == "yes" ]] ; then
                ZLIB_LIBDEP="\${VISITHOME}/zlib/\${ZLIB_VERSION}/\${VISITARCH}/lib z"
            fi
            SZIP_LIBDEP=""
            if [[ "$DO_SZIP" == "yes" ]] ; then
                SZIP_LIBDEP="\${VISITHOME}/szip/$SZIP_VERSION/\${VISITARCH}/lib sz"
            fi
            
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP $SZIP_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                    >> $HOSTCONF
            if [[ -n "$HDF5_MPI_INSTALL_DIR" ]]; then
                echo \
                    "VISIT_OPTION_DEFAULT(VISIT_HDF5_MPI_LIBDEP $SZIP_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                        >> $HOSTCONF
            fi
        fi
    fi
}

function bv_hdf5_ensure
{
    if [[ "$DO_HDF5" == "yes" && "$USE_SYSTEM_HDF5" == "no" ]] ; then
        ensure_built_or_ready "hdf5" $HDF5_VERSION $HDF5_BUILD_DIR $HDF5_FILE $HDF5_URL 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_HDF5="no"
            error "Unable to build HDF5.  ${HDF5_FILE} not found."
        fi
    fi
}

function apply_hdf5_1814_static_patch
{
    info "Patching hdf5 1.8.14 for static build"
    patch -p0 << EOF
*** src/H5PL.c.orig    2015-10-23 11:51:35.000000000 -0700
--- src/H5PL.c  2015-10-23 11:56:48.000000000 -0700
***************
*** 159,165 ****
      char        *preload_path;
  
      FUNC_ENTER_STATIC_NOERR
! 
      /* Retrieve pathnames from HDF5_PLUGIN_PRELOAD if the user sets it
       * to tell the library to load plugin libraries without search.
       */
--- 159,165 ----
      char        *preload_path;
  
      FUNC_ENTER_STATIC_NOERR
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* Retrieve pathnames from HDF5_PLUGIN_PRELOAD if the user sets it
       * to tell the library to load plugin libraries without search.
       */
***************
*** 168,174 ****
          if(!HDstrcmp(preload_path, H5PL_NO_PLUGIN))
              H5PL_no_plugin_g = TRUE;
      } /* end if */
! 
      FUNC_LEAVE_NOAPI(SUCCEED)
  } /* end H5PL__init_interface() */
  
--- 168,174 ----
          if(!HDstrcmp(preload_path, H5PL_NO_PLUGIN))
              H5PL_no_plugin_g = TRUE;
      } /* end if */
! #endif
      FUNC_LEAVE_NOAPI(SUCCEED)
  } /* end H5PL__init_interface() */
  
***************
*** 193,201 ****
      htri_t ret_value;
  
      FUNC_ENTER_NOAPI(FAIL)
! 
      ret_value = (htri_t)H5PL_no_plugin_g;
! 
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL_no_plugin() */
--- 193,201 ----
      htri_t ret_value;
  
      FUNC_ENTER_NOAPI(FAIL)
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      ret_value = (htri_t)H5PL_no_plugin_g;
! #endif
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL_no_plugin() */
***************
*** 224,230 ****
      int  i = 0;
      
      FUNC_ENTER_NOAPI_NOINIT_NOERR
! 
      if(H5_interface_initialize_g) {
          size_t u;       /* Local index variable */
  
--- 224,230 ----
      int  i = 0;
      
      FUNC_ENTER_NOAPI_NOINIT_NOERR
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      if(H5_interface_initialize_g) {
          size_t u;       /* Local index variable */
  
***************
*** 246,252 ****
        H5_interface_initialize_g = 0;
          i = 1;
      } /* end if */
! 
      FUNC_LEAVE_NOAPI(i)
  } /* end H5PL_term_interface() */
  
--- 246,252 ----
        H5_interface_initialize_g = 0;
          i = 1;
      } /* end if */
! #endif
      FUNC_LEAVE_NOAPI(i)
  } /* end H5PL_term_interface() */
  
***************
*** 273,279 ****
      const void  *ret_value = NULL;
  
      FUNC_ENTER_NOAPI(NULL)
! 
      /* Check for "no plugins" indicated" */
      if(H5PL_no_plugin_g)
          HGOTO_ERROR(H5E_PLUGIN, H5E_CANTLOAD, NULL, "required dynamically loaded plugin filter '%d' is not available", id)
--- 273,279 ----
      const void  *ret_value = NULL;
  
      FUNC_ENTER_NOAPI(NULL)
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* Check for "no plugins" indicated" */
      if(H5PL_no_plugin_g)
          HGOTO_ERROR(H5E_PLUGIN, H5E_CANTLOAD, NULL, "required dynamically loaded plugin filter '%d' is not available", id)
***************
*** 308,314 ****
      /* Check if we found the plugin */
      if(found)
          ret_value = plugin_info;
! 
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL_load() */
--- 308,314 ----
      /* Check if we found the plugin */
      if(found)
          ret_value = plugin_info;
! #endif
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL_load() */
***************
*** 335,341 ****
      herr_t      ret_value = SUCCEED;    /* Return value */
  
      FUNC_ENTER_STATIC
! 
      /* Retrieve paths from HDF5_PLUGIN_PATH if the user sets it
       * or from the default paths if it isn't set.
       */
--- 335,341 ----
      herr_t      ret_value = SUCCEED;    /* Return value */
  
      FUNC_ENTER_STATIC
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* Retrieve paths from HDF5_PLUGIN_PATH if the user sets it
       * or from the default paths if it isn't set.
       */
***************
*** 360,366 ****
      } /* end while */
  
      H5PL_path_found_g = TRUE;
! 
  done:
      if(dl_path)
          dl_path = (char *)H5MM_xfree(dl_path);
--- 360,366 ----
      } /* end while */
  
      H5PL_path_found_g = TRUE;
! #endif
  done:
      if(dl_path)
          dl_path = (char *)H5MM_xfree(dl_path);
***************
*** 396,402 ****
      htri_t         ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! 
      /* Open the directory */  
      if(!(dirp = HDopendir(dir)))
          HGOTO_ERROR(H5E_PLUGIN, H5E_OPENERROR, FAIL, "can't open directory")
--- 396,402 ----
      htri_t         ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* Open the directory */  
      if(!(dirp = HDopendir(dir)))
          HGOTO_ERROR(H5E_PLUGIN, H5E_OPENERROR, FAIL, "can't open directory")
***************
*** 438,444 ****
                  pathname = (char *)H5MM_xfree(pathname);
          } /* end if */
      } /* end while */
! 
  done:
      if(dirp) 
          if(HDclosedir(dirp) < 0)
--- 438,444 ----
                  pathname = (char *)H5MM_xfree(pathname);
          } /* end if */
      } /* end while */
! #endif
  done:
      if(dirp) 
          if(HDclosedir(dirp) < 0)
***************
*** 459,465 ****
      htri_t          ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! 
      /* Specify a file mask. *.* = We want everything! */
      sprintf(service, "%s\\*.dll", dir);
      if((hFind = FindFirstFile(service, &fdFile)) == INVALID_HANDLE_VALUE)
--- 459,465 ----
      htri_t          ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* Specify a file mask. *.* = We want everything! */
      sprintf(service, "%s\\*.dll", dir);
      if((hFind = FindFirstFile(service, &fdFile)) == INVALID_HANDLE_VALUE)
***************
*** 494,500 ****
                  pathname = (char *)H5MM_xfree(pathname);
          } /* end if */
      } while(FindNextFile(hFind, &fdFile)); /* Find the next file. */
! 
  done:
      if(hFind) 
          FindClose(hFind);
--- 494,500 ----
                  pathname = (char *)H5MM_xfree(pathname);
          } /* end if */
      } while(FindNextFile(hFind, &fdFile)); /* Find the next file. */
! #endif
  done:
      if(hFind) 
          FindClose(hFind);
***************
*** 529,535 ****
      htri_t         ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! 
      /* There are different reasons why a library can't be open, e.g. wrong architecture.
       * simply continue if we can't open it.
       */
--- 529,535 ----
      htri_t         ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* There are different reasons why a library can't be open, e.g. wrong architecture.
       * simply continue if we can't open it.
       */
***************
*** 588,594 ****
                      HGOTO_ERROR(H5E_PLUGIN, H5E_CLOSEERROR, FAIL, "can't close dynamic library")
          } /* end if */
      } /* end else */
! 
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL__open() */
--- 588,594 ----
                      HGOTO_ERROR(H5E_PLUGIN, H5E_CLOSEERROR, FAIL, "can't close dynamic library")
          } /* end if */
      } /* end else */
! #endif
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL__open() */
***************
*** 615,621 ****
      htri_t         ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! 
      /* Search in the table of already opened dynamic libraries */
      if(H5PL_table_used_g > 0) {
          size_t         i;
--- 615,621 ----
      htri_t         ret_value = FALSE;
  
      FUNC_ENTER_STATIC
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      /* Search in the table of already opened dynamic libraries */
      if(H5PL_table_used_g > 0) {
          size_t         i;
***************
*** 636,642 ****
              } /* end if */
          } /* end for */
      } /* end if */
! 
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL__search_table() */
--- 636,642 ----
              } /* end if */
          } /* end for */
      } /* end if */
! #endif
  done:
      FUNC_LEAVE_NOAPI(ret_value)
  } /* end H5PL__search_table() */
***************
*** 658,666 ****
  H5PL__close(H5PL_HANDLE handle)
  {
      FUNC_ENTER_STATIC_NOERR
! 
      H5PL_CLOSE_LIB(handle);
!    
      FUNC_LEAVE_NOAPI(SUCCEED)
  } /* end H5PL__close() */
  #endif /*H5_VMS*/
--- 658,666 ----
  H5PL__close(H5PL_HANDLE handle)
  {
      FUNC_ENTER_STATIC_NOERR
! #ifdef H5_SUPPORT_DYNAMIC_LOADING
      H5PL_CLOSE_LIB(handle);
! #endif
      FUNC_LEAVE_NOAPI(SUCCEED)
  } /* end H5PL__close() */
  #endif /*H5_VMS*/
EOF
    if [[ $? != 0 ]] ; then
        warn "HDF5 1.8.14 static patch failed."
        return 1
    fi

    return 0;
}


function apply_hdf5_1814_isatty_patch
{
    info "Patching hdf5 1.8.14 for isatty"
    patch -p0 << EOF
--- hl/src/H5LTanalyze.c.orig	2014-11-07 04:53:42.000000000 -0800
+++ hl/src/H5LTanalyze.c	2021-02-01 13:40:36.000000000 -0800
@@ -40,6 +40,7 @@
 #include <string.h>
 #include <errno.h>
 #include <stdlib.h>
+#include <unistd.h>
 
 /* end standard C headers. */
EOF
    if [[ $? != 0 ]] ; then
        warn "HDF5 1.8.14 isatty patch failed."
        return 1
    fi

    return 0;
}

function apply_hdf5_patch
{
    # Apply a patch for static if we build statically.
    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        apply_hdf5_1814_static_patch
        if [[ $? != 0 ]]; then
            return 1
        fi
    fi

    apply_hdf5_1814_isatty_patch
    if [[ $? != 0 ]]; then
        return 1
    fi

    return 0
}

# *************************************************************************** #
#                          Function 8.1, build_hdf5                           #
# *************************************************************************** #

function build_hdf5
{
    #
    # Prepare build dir
    #
    prepare_build_dir $HDF5_BUILD_DIR $HDF5_FILE
    untarred_hdf5=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_hdf5 == -1 ]] ; then
        warn "Unable to prepare HDF5 Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    cd $HDF5_BUILD_DIR || error "Can't cd to HDF5 build dir."
    apply_hdf5_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_hdf5 == 1 ]] ; then
            warn "Giving up on HDF5 build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure HDF5
    #
    info "Configuring HDF5 . . ."
    if [[ "$OPSYS" == "Darwin" ]]; then
        export DYLD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
    else
        export LD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
    fi
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cf_build_type="--disable-shared --enable-static"
    else
        cf_build_type="--enable-shared --disable-static"
    fi
    cf_szip=""
    if test "x${DO_SZIP}" = "xyes"; then
        info "SZip requested.  Configuring HDF5 with SZip support."
        sz_dir="${VISITDIR}/szip/${SZIP_VERSION}/${VISITARCH}"
        cf_szip="--with-szlib=${sz_dir}"
    fi
    cf_zlib=""
    if [[ "$DO_ZLIB" == "yes" ]]; then
        info "Configuring HDF5 with ZLib support."
        cf_zlib="--with-zlib=\"${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}\""
    fi

    cf_extra_flags=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "$(uname -m)" == "arm64" ]]; then
            cf_extra_flags="-Wno-error=implicit-function-declaration"
        fi
    fi

    # Disable Fortran on Darwin since it causes HDF5 builds to fail.
    if [[ "$OPSYS" == "Darwin" ]]; then
        cf_fortranargs=""
    elif [[ "$FC_COMPILER" == "no" ]] ; then
        cf_fortranargs=""
    else
        cf_fortranargs="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    cf_build_thread=""
    if [[ "$DO_THREAD_BUILD" == "yes" ]]; then
        cf_build_thread="--enable-threadsafe --with-pthread"
    fi

    build_mode=""
    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        build_mode="--disable-production"
    fi

    par_build_types="serial"
    if [[ -n "$PAR_COMPILER" && "$DO_MOAB" == "yes" ]]; then
        par_build_types="$par_build_types parallel"
    fi

    extra_ac_flags=""
    # detect coral and NVIDIA Grace CPU (ARM) systems, which older versions of 
    # autoconf don't detect
    if [[ "$(uname -m)" == "ppc64le" ]] ; then
         extra_ac_flags="ac_cv_build=powerpc64le-unknown-linux-gnu"
    elif [[ "$(uname -m)" == "aarch64" ]] ; then
         extra_ac_flags="ac_cv_build=aarch64-unknown-linux-gnu"
    fi 
    
    for bt in $par_build_types; do

        rm -rf build_$bt
        mkdir build_$bt
        pushd build_$bt

        cf_build_parallel=""
        cf_par_suffix=""
        if [[ "$bt" == "serial" ]]; then
            cf_build_parallel="--disable-parallel"
            cf_c_compiler="$C_COMPILER"
        elif [[ "$bt" == "parallel" ]]; then
            # these commands ruin the untar'd source code for 'normal' builds
            sed -e 's/libhdf5/libhdf5_mpi/g' -i.orig ../configure
            find .. -name Makefile.in -exec sed -e 's/libhdf5/libhdf5_mpi/g' -i.orig {} \;
            sed -e 's/libhdf5\.settings/libhdf5_mpi.settings/g' -i.orig ../src/H5make_libsettings.c
            pushd ../src; ln -s libhdf5.settings.in libhdf5_mpi.settings.in; popd
            cf_build_parallel="--enable-parallel"
            cf_par_suffix="_mpi"
            cf_c_compiler="$PAR_COMPILER"
        fi

        # In order to ensure $cf_fortranargs is expanded to build the arguments to
        # configure, we wrap the invokation in 'sh -c "..."' syntax
        info "Invoking command to configure $bt HDF5"
        set -x
        sh -c "../configure CC=\"$cf_c_compiler\" \
            CFLAGS=\"$CFLAGS $C_OPT_FLAGS $cf_extra_flags\" $cf_fortranargs \
            --prefix=\"$VISITDIR/hdf5${cf_par_suffix}/$HDF5_VERSION/$VISITARCH\" \
            ${cf_szip} ${cf_zlib} ${cf_build_type} ${cf_build_thread} \
            ${cf_build_parallel} ${extra_ac_flags} $build_mode"
        set +x
        if [[ $? != 0 ]] ; then
            warn "$bt HDF5 configure failed.  Giving up"
            return 1
        fi

        #
        # Build HDF5
        #
        info "Making $bt HDF5 . . ."
        set -x
        $MAKE $MAKE_OPT_FLAGS lib
        set +x
        if [[ $? != 0 ]] ; then
            warn "$bt HDF5 build failed.  Giving up"
            return 1
        fi
        #
        # Install into the VisIt third party location.
        #
        # Install all targets until we can figure out
        # how to avoid installing just the tests.
        #
        info "Installing $bt HDF5 . . ."
        $MAKE install

        if [[ $? != 0 ]] ; then
            warn "$bt HDF5 install failed.  Giving up"
            return 1
        fi

        if [[ "$DO_GROUP" == "yes" ]] ; then
            chmod -R ug+w,a+rX "$VISITDIR/hdf5"
            chgrp -R ${GROUP} "$VISITDIR/hdf5"
        fi

        popd
    done

    cd "$START_DIR"
    info "Done with HDF5"
    return 0
}

function bv_hdf5_is_enabled
{
    if [[ $DO_HDF5 == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_hdf5_is_installed
{

    if [[ "$USE_SYSTEM_HDF5" == "yes" ]]; then
        return 1
    fi

    check_if_installed "hdf5" $HDF5_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_hdf5_build
{
    cd "$START_DIR"
    
    if [[ "$DO_HDF5" == "yes" && "$USE_SYSTEM_HDF5" == "no" ]] ; then
        check_if_installed "hdf5" $HDF5_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping HDF5 build.  HDF5 is already installed."
        else
            info "Building HDF5 (~15 minutes)"
            build_hdf5
            if [[ $? != 0 ]] ; then
                error "Unable to build or install HDF5.  Bailing out."
            fi
            info "Done building HDF5"
        fi
    fi
}
