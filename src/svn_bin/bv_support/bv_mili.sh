function bv_mili_initialize
{
export DO_MILI="no"
export ON_MILI="off"
}

function bv_mili_enable
{
DO_MILI="yes"
ON_MILI="on"
}

function bv_mili_disable
{
DO_MILI="no"
ON_MILI="off"
}

function bv_mili_depends_on
{
echo ""
}

function bv_mili_info
{
export MILI_FILE=${MILI_FILE:-"mili_15_1.tar.gz"}
export MILI_VERSION=${MILI_VERSION:-"15.1"}
export MILI_COMPATIBILITY_VERSION=${MILI_COMPATIBILITY_VERSION:-"15.1"}
export MILI_BUILD_DIR=${MILI_BUILD_DIR:-"mili"}
export MILI_MD5_CHECKSUM="115c6aaf742f151aea6c644d719ee067"
export MILI_SHA256_CHECKSUM="564c50b003aded7ec33977f8192fbf6ec2f5dc22a41c9551a8fa492c484031bf"
}

function bv_mili_print
{
  printf "%s%s\n" "MILI_FILE=" "${MILI_FILE}"
  printf "%s%s\n" "MILI_VERSION=" "${MILI_VERSION}"
  printf "%s%s\n" "MILI_COMPATIBILITY_VERSION=" "${MILI_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "MILI_BUILD_DIR=" "${MILI_BUILD_DIR}"
}

function bv_mili_print_usage
{
printf "\t\t%15s\n" "NOTE: not available for download from web"
printf "%-15s %s [%s]\n" "--mili" "Build Mili" "$DO_MILI"
}

function bv_mili_graphical
{
local graphical_out="MILI    $MILI_VERSION($MILI_FILE)     $ON_MILI"
echo "$graphical_out"
}

function bv_mili_host_profile
{
    if [[ "$DO_MILI" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Mili" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_MILI_DIR \${VISITHOME}/mili/$MILI_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_mili_ensure
{
    if [[ "$DO_MILI" == "yes" ]] ; then
        ensure_built_or_ready "mili" $MILI_VERSION $MILI_BUILD_DIR $MILI_FILE
        if [[ $? != 0 ]] ; then
            warn "Unable to build Mili.  ${MILI_FILE} not found."
            ANY_ERRORS="yes"
            DO_MILI="no"
            if [[ "$DO_SVN" != "yes" ]] ; then
                warn "Note: You have requested to build the Mili library." 
                warn "Mili is not available for public download and" 
                warn "is only available through Subversion access." 
            fi
            error
        fi
    fi
}

function bv_mili_dry_run
{
  if [[ "$DO_MILI" == "yes" ]] ; then
    echo "Dry run option not set for mili."
  fi
}

# *************************************************************************** #
#                          Function 8.2, build_mili                           #
# *************************************************************************** #

function apply_mili_100_darwin_patch
{
    patch -p0 << \EOF
diff -c a/src/mili_internal.h mili/src/mili_internal.h
*** a/src/mili_internal.h
--- mili/src/mili_internal.h
***************
*** 54,59 ****
--- 54,60 ----
  #include <stdio.h>
  #include <stdlib.h>
  #include <dirent.h>
+ #include <sys/types.h>
  #include "list.h"
  #include "misc.h"
  #include "mili.h"
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply Darwin patch to Mili 1.10.0."
        return 1
    fi

    return 0
}

function apply_mili_100_patch
{
    if [[ "$OPSYS" == "Darwin" ]]; then
        apply_mili_100_darwin_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function apply_mili_111_patch_1
{
    patch -p0 << \EOF
diff -c a/src/mili.h mili/src/mili.h
*** a/src/mili.h
--- mili/src/mili.h
***************
*** 226,232 ****
  } ObjDef;

  /* Mili version */
! const char *mili_version;

  /*
                  * *                                      * *
--- 226,232 ----
  } ObjDef;

  /* Mili version */
! extern const char *mili_version;

  /*
                  * *                                      * *
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 1 to Mili 1.11.1."
        return 1
    fi

    return 0
}

function apply_mili_111_patch_2
{
    patch -p0 << \EOF
diff -c a/src/mili.c mili/src/mili.c
*** a/src/mili.c
--- mili/src/mili.c
***************
*** 94,99 ****
--- 94,102 ----
                           && ( f->ti_directory[f->ti_file_count - 1].commit_count == 0\
                                || f->non_state_ready ) )

+ /* Mili version */
+ const char *mili_version;
+
  static void set_path( char *in_path, char **out_path, int *out_path_len );
  static void map_old_header( char header[CHAR_HEADER_SIZE] );
  static Return_value create_family( Mili_family * );
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 2 to Mili 1.11.1."
        return 1
    fi

    return 0
}

function apply_mili_111_patch
{
    apply_mili_111_patch_1
    if [[ $? != 0 ]] ; then
        return 1
    fi
    apply_mili_111_patch_2
    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}

function apply_mili_151_darwin_patch1
{
    patch -p0 << \EOF
diff -c mili/src/mesh_u.c mili.patched/src/mesh_u.c
*** mili/src/mesh_u.c	2015-09-22 13:20:42.000000000 -0700
--- mili.patched/src/mesh_u.c	2015-10-19 12:44:52.000000000 -0700
***************
*** 14,20 ****
  
  #include <string.h>
  #ifndef _MSC_VER
! #include <values.h>
  #include <sys/time.h>
  #endif
  #include <time.h>
--- 14,20 ----
  
  #include <string.h>
  #ifndef _MSC_VER
! #include <limits.h>
  #include <sys/time.h>
  #endif
  #include <time.h>
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply Darwin patch 1 to Mili 15.1"
        return 1
    fi

    return 0
}

function apply_mili_151_darwin_patch2
{
    patch -p0 << \EOF
*** mili/Makefile.Library	2013-12-10 12:55:55.000000000 -0800
--- mili.patched/Makefile.Library	2015-10-20 13:37:27.000000000 -0700
***************
*** 386,393 ****
  	done
  
  uninstall:
- 
- ifneq ($(OS_NAME),Linux)
- include $(OBJS:.o=.d)
- endif
- 
--- 386,388 ----
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply Darwin patch 2 to Mili 15.1"
        return 1
    fi

    return 0
}

function apply_mili_151_darwin_patch3
{
    patch -p0 << \EOF
*** mili/src/mili_internal.h	2015-09-17 13:26:32.000000000 -0700
--- mili.patched/src/mili_internal.h	2015-10-20 16:57:21.000000000 -0700
***************
*** 534,542 ****
   * Library-private file family management routines and data.
   */
  
! int host_index;
! int internal_sizes[QTY_PD_ENTRY_TYPES + 1];
! int mili_verbose;
  Return_value validate_fam_id( Famid fam_id );
  Return_value parse_control_string( char *ctl_str, Mili_family *fam,
                                     Bool_type *p_create );
--- 534,542 ----
   * Library-private file family management routines and data.
   */
  
! extern int host_index;
! extern int internal_sizes[QTY_PD_ENTRY_TYPES + 1];
! extern int mili_verbose;
  Return_value validate_fam_id( Famid fam_id );
  Return_value parse_control_string( char *ctl_str, Mili_family *fam,
                                     Bool_type *p_create );
***************
*** 604,610 ****
  Return_value load_directories( Mili_family *fam );
  
  /* param.c - parameter management routines. */
! char *dtype_names[QTY_PD_ENTRY_TYPES + 1];
  Return_value read_scalar( Mili_family *fam, Param_ref *p_pr,  void *p_value );
  Return_value mili_read_string( Mili_family *fam, Param_ref *p_pr,
                                 char *p_value );
--- 604,610 ----
  Return_value load_directories( Mili_family *fam );
  
  /* param.c - parameter management routines. */
! extern char *dtype_names[QTY_PD_ENTRY_TYPES + 1];
  Return_value read_scalar( Mili_family *fam, Param_ref *p_pr,  void *p_value );
  Return_value mili_read_string( Mili_family *fam, Param_ref *p_pr,
                                 char *p_value );
***************
*** 647,653 ****
  /* dep.c - routines for handling architecture dependencies. */
  Return_value set_default_io_routines( Mili_family *fam );
  Return_value set_state_data_io_routines( Mili_family *fam );
! void (*write_funcs[QTY_PD_ENTRY_TYPES + 1])();
  
  /* svar.c - routines for managing state variables. */
  Bool_type valid_svar_data( Aggregate_type atype, char *name,
--- 647,653 ----
  /* dep.c - routines for handling architecture dependencies. */
  Return_value set_default_io_routines( Mili_family *fam );
  Return_value set_state_data_io_routines( Mili_family *fam );
! extern void (*write_funcs[QTY_PD_ENTRY_TYPES + 1])();
  
  /* svar.c - routines for managing state variables. */
  Bool_type valid_svar_data( Aggregate_type atype, char *name,
***************
*** 740,746 ****
  void mili_delete_mo_class_data( void *p_data );
  
  /* wrap_c.c - C-half of FORTRAN-to-C wrappers. */
! int fortran_api;
  /* write_db.c */
  Return_value
  write_state_data( int state_num, Mili_analysis *out_db );
--- 740,746 ----
  void mili_delete_mo_class_data( void *p_data );
  
  /* wrap_c.c - C-half of FORTRAN-to-C wrappers. */
! extern int fortran_api;
  /* write_db.c */
  Return_value
  write_state_data( int state_num, Mili_analysis *out_db );
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply Darwin patch 3 to Mili 15.1"
        return 1
    fi

    return 0
}

function apply_mili_patch
{
    if [[ ${MILI_VERSION} == 1.10.0 ]] ; then
        apply_mili_100_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    elif [[ ${MILI_VERSION} == 1.11.1 ]] ; then
        apply_mili_111_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    elif [[ ${MILI_VERSION} == 15.1 && "$OPSYS" == "Darwin" ]]; then
        apply_mili_151_darwin_patch1
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_151_darwin_patch2
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_151_darwin_patch3
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_mili
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MILI_BUILD_DIR $MILI_FILE
    untarred_mili=$?
    if [[ $untarred_mili == -1 ]] ; then
       warn "Unable to prepare Mili Build Directory. Giving Up"
       return 1
    fi

    #
    # Apply patches
    #
    info "Patching mili . . ."
    apply_mili_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_mili == 1 ]] ; then
          warn "Giving up on Mili build because the patches failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script " 
          warn "tried to apply a patch to an existing directory which had " 
          warn "already been patched ... that is, that the patch is "
          warn "failing harmlessly on a second application."
       fi
    fi

    info "Configuring Mili . . ."
    cd $MILI_BUILD_DIR || error "Can't cd to mili build dir."

    info "Invoking command to configure Mili"
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        ac_cv_prog_FOUND_GMAKE=make \
        --prefix="$VISITDIR/mili/$MILI_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "Mili configure failed.  Giving up"
       return 1
    fi

    #
    # Build Mili
    #
    info "Building Mili . . . (~2 minutes)"

    if [[ ${MILI_VERSION} == 1.10.0 ]] ; then
        cd MILI-$OPSYS-*
        cd src
        $C_COMPILER $CFLAGS $C_OPT_FLAGS -D_LARGEFILE64_SOURCE -c \
            mili.c direc.c param.c io.c util.c dep.c svar.c \
            srec.c mesh_u.c wrap_c.c io_mem.c eprtf.c \
            sarray.c gahl.c util.c partition.c ti.c tidirc.c
        if [[ $? != 0 ]] ; then
            warn "Mili build failed.  Giving up"
            return 1
        fi
    elif [[ ${MILI_VERSION} == 1.11.1 ]] ; then
        cd MILI-*-*
        cd src
        $C_COMPILER $CFLAGS $C_OPT_FLAGS -D_LARGEFILE64_SOURCE -c \
            mili.c dep.c direc.c eprtf.c gahl.c io_mem.c \
            mesh_u.c mr_funcs.c param.c partition.c read_db.c sarray.c \
            srec.c svar.c taurus_db.c taurus_mesh_u.c taurus_srec.c \
            taurus_svars.c taurus_util.c ti.c tidirc.c util.c wrap_c.c \
            write_db.c
        if [[ $? != 0 ]] ; then
            warn "Mili build failed.  Giving up"
            return 1
        fi
    elif [[ ${MILI_VERSION} == 13.1.1-patch || ${MILI_VERSION} == 15.1 ]] ; then
        cd MILI-*-*
        make opt fortran=false
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing Mili . . ." 

    mkdir "$VISITDIR/mili"
    mkdir "$VISITDIR/mili/$MILI_VERSION"
    mkdir "$VISITDIR/mili/$MILI_VERSION/$VISITARCH"
    mkdir "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
    mkdir "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/include"
    if [[ ${MILI_VERSION} == 1.10.0 ]] ; then
        cp mili.h mili_enum.h  "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/include"
    elif [[ ${MILI_VERSION} == 1.11.1 ]] ; then
        cp mili.h mili_enum.h misc.h  "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/include"
    elif [[ ${MILI_VERSION} == 13.1.1-patch || ${MILI_VERSION} == 15.1 ]] ; then
        cp src/{mili.h,mili_enum.h,misc.h}  "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/include"
    fi
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        INSTALLNAMEPATH="$VISITDIR/mili/${MILI_VERSION}/$VISITARCH/lib"

        if [[ ${MILI_VERSION} != 15.1 ]] ; then
            $C_COMPILER -dynamiclib -o libmili.$SO_EXT *.o \
              -Wl,-headerpad_max_install_names \
              -Wl,-install_name,$INSTALLNAMEPATH/libmili.${SO_EXT} \
              -Wl,-compatibility_version,$MILI_COMPATIBILITY_VERSION \
              -Wl,-current_version,$MILI_VERSION
        else
            $C_COMPILER -dynamiclib -o libmili.$SO_EXT objs_opt/*.o \
              -Wl,-headerpad_max_install_names \
              -Wl,-install_name,$INSTALLNAMEPATH/libmili.${SO_EXT} \
              -Wl,-compatibility_version,$MILI_COMPATIBILITY_VERSION \
              -Wl,-current_version,$MILI_VERSION
        fi
        if [[ $? != 0 ]] ; then
          warn "Mili dynamic library build failed.  Giving up"
          return 1
        fi
        cp libmili.$SO_EXT "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
    else
        if [[ ${MILI_VERSION} != 13.1.1-patch && ${MILI_VERSION} != 15.1 ]] ; then
            ar -rc libmili.a *.o 
            if [[ $? != 0 ]] ; then
              warn "Mili install failed.  Giving up"
              return 1
            fi
            cp libmili.a "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
        else
            cp lib_opt/libmili.a "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
        fi
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/mili"
       chgrp -R ${GROUP} "$VISITDIR/mili"
    fi
    cd "$START_DIR"
    info "Done with Mili"
    return 0
}

function bv_mili_is_enabled
{
    if [[ $DO_MILI == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mili_is_installed
{
    check_if_installed "mili" $MILI_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mili_build
{
cd "$START_DIR"
if [[ "$DO_MILI" == "yes" ]] ; then
    check_if_installed "mili" $MILI_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Mili build.  Mili is already installed."
    else
        info "Building Mili (~2 minutes)"
        build_mili
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Mili.  Bailing out."
        fi
        info "Done building Mili"
    fi
fi
}

