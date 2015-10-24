function bv_hdf5_initialize
{
export DO_HDF5="no"
export ON_HDF5="off"
export USE_SYSTEM_HDF5="no"
add_extra_commandline_args "hdf5" "alt-hdf5-dir" 1 "Use alternative directory for hdf5"
}

function bv_hdf5_enable
{
DO_HDF5="yes"
ON_HDF5="on"
}

function bv_hdf5_disable
{
DO_HDF5="no"
ON_HDF5="off"
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
        local depends_on="szip"

        if [[ "$DO_ZLIB" == "yes" ]] ; then
           depends_on="$depends_on zlib"    
        fi

        echo $depends_on
    fi
}

function bv_hdf5_initialize_vars
{
    if [[ "$USE_SYSTEM_HDF5" == "no" ]]; then
        HDF5_INSTALL_DIR="${VISITDIR}/hdf5/$HDF5_VERSION/${VISITARCH}"
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
export HDF5_MD5_CHECKSUM="a482686e733514a51cde12d6fe5c5d95"
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
printf "%-15s %s [%s]\n" "--hdf5" "Build HDF5" "${DO_HDF5}"
}

function bv_hdf5_graphical
{
local graphical_out="HDF5     $HDF5_VERSION($HDF5_FILE)      $ON_HDF5"
echo $graphical_out
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

            if [[ "$DO_ZLIB" == "yes" ]] ; then
               ZLIB_LIBDEP="\${VISITHOME}/zlib/$ZLIB_VERSION/\${VISITARCH}/lib z"
            else
               ZLIB_LIBDEP="/usr/lib z"
               #moving global patch to have limited effect
               if [[ -d /usr/lib/x86_64-linux-gnu ]]; then
                ZLIB_LIBDEP="/usr/lib/x86_64-linux-gnu z"
               fi
            fi

            if [[ "$DO_SZIP" == "yes" ]] ; then
                echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP \${VISITHOME}/szip/$SZIP_VERSION/\${VISITARCH}/lib sz $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
            else
                echo \
                "VISIT_OPTION_DEFAULT(VISIT_HDF5_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
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

function bv_hdf5_dry_run
{
  if [[ "$DO_HDF5" == "yes" ]] ; then
    echo "Dry run option not set for hdf5."
  fi
}

function apply_hdf5_187_188_patch
{
    info "Patching hdf5"
    patch -p0 << \EOF
diff -c tools/lib/h5diff.c.orig tools/lib/h5diff.c
*** tools/lib/h5diff.c.orig     2013-11-13 08:14:48.924716921 -0800
--- tools/lib/h5diff.c  2013-11-13 08:15:28.066716686 -0800
***************
*** 635,641 ****
      char         filenames[2][MAX_FILENAME];
      hsize_t      nfound = 0;
      int i;
!     //int i1, i2;
      int l_ret;
      const char * obj1fullname = NULL;
      const char * obj2fullname = NULL;
--- 635,641 ----
      char         filenames[2][MAX_FILENAME];
      hsize_t      nfound = 0;
      int i;
!     /* int i1, i2; */
      int l_ret;
      const char * obj1fullname = NULL;
      const char * obj2fullname = NULL;
EOF
    if [[ $? != 0 ]] ; then
      warn "HDF5 patch failed."
      return 1
    fi

    return 0
}

function apply_hdf5_187_thread_patch
{
    info "Patching thread hdf5"
    patch -p0 << \EOF
diff -c src/H5private.h.orig src/H5private.h
*** src/H5private.h.orig    2014-07-28 10:46:54.821807839 -0700
--- src/H5private.h 2014-07-08 13:00:12.562002468 -0700
***************
*** 30,40 ****
  
  /* include the pthread header */
  #ifdef H5_HAVE_THREADSAFE
- #ifdef H5_HAVE_PTHREAD_H
  #include <pthread.h>
- #else /* H5_HAVE_PTHREAD_H */
- #define H5_HAVE_WIN_THREADS
- #endif /* H5_HAVE_PTHREAD_H */
  #endif /* H5_HAVE_THREADSAFE */
  
  /*
--- 30,36 ----
EOF
    if [[ $? != 0 ]] ; then
      warn "HDF5 thread patch failed."
      return 1
    fi

    return 0;
}

function apply_hdf5_1814_static_patch
{
    info "Patching hdf5 for static build"
    patch -p0 << \EOF
*** src/H5PL.c.orig	2015-10-23 11:51:35.000000000 -0700
--- src/H5PL.c	2015-10-23 11:56:48.000000000 -0700
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
      warn "HDF5 static patch failed."
      return 1
    fi

    return 0;
}

function apply_hdf5_patch
{
    if [[ "${HDF5_VERSION}" == 1.8.7 ]] ; then
        apply_hdf5_187_188_patch
        if [[ $? != 0 ]]; then
            return 1
        fi
        if [[ "$DO_THREAD_BUILD" == "yes" ]]; then
            apply_hdf5_187_thread_patch
            if [[ $? != 0 ]]; then
                return 1
            fi
        fi
    else
        if [[ "${HDF5_VERSION}" == 1.8.8 ]] ; then
            apply_hdf5_187_188_patch
            if [[ $? != 0 ]]; then
                return 1
            fi
        else
            # Latest HDF5.

            # Apply a patch for static if we build statically.
            if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
                apply_hdf5_1814_static_patch
                if [[ $? != 0 ]]; then
                    return 1
                fi
            fi
        fi
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
    if [[ $untarred_hdf5 == -1 ]] ; then
       warn "Unable to prepare HDF5 Build Directory. Giving Up"
       return 1
    fi

    #
    cd $HDF5_BUILD_DIR || error "Can't cd to HDF5 build dir."
    apply_hdf5_patch
    if [[ $? != 0 ]]; then
        warn "Patch failed, but continuing."
    fi
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

    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS=""
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    cf_build_thread=""
    if [[ "$DO_THREAD_BUILD" == "yes" ]]; then
        cf_build_thread="--enable-threadsafe --with-pthread"
    fi

    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    info "Invoking command to configure HDF5"
    info "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH\" \
        ${cf_szip} ${cf_build_type} ${cf_build_thread}"
    sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH\" \
        ${cf_szip} ${cf_build_type} ${cf_build_thread}"
    if [[ $? != 0 ]] ; then
       warn "HDF5 configure failed.  Giving up"
       return 1
    fi

    #
    # Build HDF5
    #
    info "Making HDF5 . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "HDF5 build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing HDF5 . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "HDF5 install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for HDF5 . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/hdf5"
       chgrp -R ${GROUP} "$VISITDIR/hdf5"
    fi
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
