function bv_mili_initialize
{
    export DO_MILI="no"
}

function bv_mili_enable
{
    DO_MILI="yes"
}

function bv_mili_disable
{
    DO_MILI="no"
}

function bv_mili_depends_on
{
    echo ""
}

function bv_mili_info
{
    export MILI_FILE=${MILI_FILE:-"mili-23.02.tar.gz"}
    export MILI_VERSION=${MILI_VERSION:-"23.02"}
    export MILI_COMPATIBILITY_VERSION=${MILI_COMPATIBILITY_VERSION:-"23.02"}
    export MILI_BUILD_DIR=${MILI_BUILD_DIR:-"mili-${MILI_VERSION}"}
    export MILI_SHA256_CHECKSUM="4973680e377f400a9fac12740b77c2297a4fcbcea7d6a4317d72b08dcffd4def"
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
    printf "%-20s %s [%s]\n" "--mili" "Build Mili" "$DO_MILI"
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
        fi
    fi
}

# *************************************************************************** #
#                          Function 8.2, build_mili                           #
# *************************************************************************** #

function apply_mili_221_path_length_patch
{
    info "Applying Mili 22.1 max path length patch."
    patch -p0 << \EOF
diff -c mili-22.1/src/mili.h mili-22.1/src/mili.h.pathlenpatch 
*** mili-22.1/src/mili.h    Wed Jun  1 15:21:14 2022
--- mili-22.1/src/mili.h.pathlenpatch   Mon Sep 12 13:39:59 2022
***************
*** 200,206 ****
  /*
   * Miscellaneous limits
   */
! #define M_MAX_NAME_LEN   (300) /* Longest name length */
  #define M_MAX_ARRAY_DIMS (6)   /* Maximum number of array dimensions */
  #define M_MAX_STRING_LEN (512) /* Maximum string length */
  
--- 200,206 ----
  /*
   * Miscellaneous limits
   */
! #define M_MAX_NAME_LEN   (4096) /* Longest name length */
  #define M_MAX_ARRAY_DIMS (6)   /* Maximum number of array dimensions */
  #define M_MAX_STRING_LEN (512) /* Maximum string length */
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply max path length patch to Mili 22.1"
        return 1
    fi

    return 0
}

function apply_mili_151_darwin_patch1
{
    info "Applying Mili 15.1 darwin patch 1."
    patch -p0 << \EOF
diff -c mili-22.1/src/mesh_u.c mili.patched/src/mesh_u.c
*** mili-22.1/src/mesh_u.c   2015-09-22 13:20:42.000000000 -0700
--- mili.patched/src/mesh_u.c   2015-10-19 12:44:52.000000000 -0700
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
    info "Applying Mili 15.1 darwin patch 2."
    patch -p0 << \EOF
*** mili-22.1/Makefile.Library       2013-12-10 12:55:55.000000000 -0800
--- mili.patched/Makefile.Library       2015-10-20 13:37:27.000000000 -0700
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
    return 0
    info "Applying Mili 15.1 darwin patch 3."
    patch -p0 << \EOF
*** mili-22.1/src/mili_internal.h    2015-09-17 13:26:32.000000000 -0700
--- mili.patched/src/mili_internal.h    2015-10-20 16:57:21.000000000 -0700
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

function apply_mili_221_cflags_patch
{
    info "Applying Mili 22.1 CFLAGS patch."
    patch -p0 << \EOF
diff -u mili-22.1/configure.orig mili-22.1/configure
--- mili-22.1/configure.orig    2022-06-01 15:21:14.000000000 -0700
+++ mili-22.1/configure         2022-11-04 09:17:02.890687000 -0700
@@ -4361,24 +4361,28 @@
 
     case $CC in
       *icc)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O3 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O3 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O3 $WORD_SIZE"
         ;;
       *xlc)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O4 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O4"
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O4 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O4"
         ;;
       *gcc)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O4 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O4 $WORD_SIZE"
-        ;;
-      *cc)
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O4 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O4 $WORD_SIZE"
+        ;;
+      *)
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O3 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O3 $WORD_SIZE"
         ;;
     esac
     case $F77 in
@@ -4395,12 +4399,16 @@
         FC_FLAGS_LD_OPT="-O3 $WORD_SIZE -WF,-DAIX"
         ;;
       *gfortran)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O3 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
+        FC_FLAGS_DEBUG="-g $WORD_SIZE "
+        FC_FLAGS_OPT="-O3 $WORD_SIZE "
+        FC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
+        FC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
         ;;
-      *cc)
+      *)
+        FC_FLAGS_DEBUG="-g $WORD_SIZE "
+        FC_FLAGS_OPT="-O3 $WORD_SIZE "
+        FC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
+        FC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
         ;;
     esac
     SHELL="/bin/sh"
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply CFLAGS patch to Mili 22.1"
        return 1
    fi

    return 0
}

function apply_mili_221_blueos_patch
{
    info "Applying Mili 22.1 blueos patch."
    patch -p0 << \EOF
diff -c mili-22.1/src/eprtf.c.orig mili-22.1/src/eprtf.c
*** mili-22.1/src/eprtf.c.orig	Wed Jun 15 16:38:26 2022
--- mili-22.1/src/eprtf.c	Wed Jun 15 16:38:51 2022
***************
*** 89,115 ****
  #include "win32-regex.h"
  #endif
  
! #include "mili_enum.h"
  #include "eprtf.h"
  
  static char destbuf[CMAX];
  static char *p_cur;
  static int cur_len;
  static va_list val;
- #ifdef HAVE_EPRINT
  static char *t_pattern = "%([0-9]+|[*])t";
  static regex_t all_re;
  static char *all_pattern =
     "%[0 -+#]*([0-9]*|[*])([.]([0-9]*|[*]))?[hlL]?[dioxXucsfeEgGpn%]";
  
- #endif
  static regex_t t_re;
  static regmatch_t t_match[1];
  
  
- #ifdef NOOPTERON
  static regmatch_t all_match[1];
- #endif
  
  
  /*****************************************************************
--- 89,111 ----
  #include "win32-regex.h"
  #endif
  
! #include "mili_internal.h"
  #include "eprtf.h"
  
  static char destbuf[CMAX];
  static char *p_cur;
  static int cur_len;
  static va_list val;
  static char *t_pattern = "%([0-9]+|[*])t";
  static regex_t all_re;
  static char *all_pattern =
     "%[0 -+#]*([0-9]*|[*])([.]([0-9]*|[*]))?[hlL]?[dioxXucsfeEgGpn%]";
  
  static regex_t t_re;
  static regmatch_t t_match[1];
  
  
  static regmatch_t all_match[1];
  
  
  /*****************************************************************
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply blueos patch to Mili 22.1"
        return 1
    fi

    return 0
}

function apply_mili_221_write_funcs_patch
{
    #
    # write_funcs is not needed and having it in the header leads to
    # multiple definitions, which gcc 10.2 on Debian 11 doesn't like.
    #
    patch -p0 << \EOF
diff -c mili-22.1/src/mili_internal.h.orig mili-22.1/src/mili_internal.h
*** mili-22.1/src/mili_internal.h.orig  Tue Jul 12 10:49:05 2022
--- mili-22.1/src/mili_internal.h       Tue Jul 12 10:49:29 2022
***************
*** 674,680 ****
  /* dep.c - routines for handling architecture dependencies. */
  Return_value set_default_io_routines( Mili_family *fam );
  Return_value set_state_data_io_routines( Mili_family *fam );
- void (*write_funcs[QTY_PD_ENTRY_TYPES + 1])();
  
  /* svar.c - routines for managing state variables. */
  Bool_type valid_svar_data( Aggregate_type atype, char *name,
--- 674,679 ----
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply write funcs patch to Mili 22.1"
        return 1
    fi

    return 0
}

function apply_mili_2302_cflags_patch
{
    info "Applying Mili 23.02 CFLAGS patch."
    patch -p0 << \EOF
diff -u mili-23.02/configure.orig mili-23.02/configure
--- mili-23.02/configure.orig   2023-05-09 09:51:35.931686000 -0700
+++ mili-23.02/configure        2023-05-09 09:54:38.169814000 -0700
@@ -4361,24 +4361,28 @@

     case $CC in
       *icc)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O3 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O3 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O3 $WORD_SIZE"
         ;;
       *xlc)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O4 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O4"
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O4 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O4"
         ;;
       *gcc)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O4 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O4 $WORD_SIZE"
-        ;;
-      *cc)
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O4 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O4 $WORD_SIZE"
+        ;;
+      *)
+        CC_FLAGS_DEBUG="$CFLAGS -g $WORD_SIZE "
+        CC_FLAGS_OPT="$CFLAGS -O3 $WORD_SIZE "
+        CC_FLAGS_LD_DEBUG="$CFLAGS -g $WORD_SIZE"
+        CC_FLAGS_LD_OPT="$CFLAGS -O3 $WORD_SIZE"
         ;;
     esac
     case $F77 in
@@ -4395,12 +4399,16 @@
         FC_FLAGS_LD_OPT="-O3 $WORD_SIZE -WF,-DAIX"
         ;;
       *gfortran)
-        CC_FLAGS_DEBUG="-g $WORD_SIZE "
-        CC_FLAGS_OPT="-O3 $WORD_SIZE "
-        CC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
-        CC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
+        FC_FLAGS_DEBUG="-g $WORD_SIZE "
+        FC_FLAGS_OPT="-O3 $WORD_SIZE "
+        FC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
+        FC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
         ;;
-      *cc)
+      *)
+        FC_FLAGS_DEBUG="-g $WORD_SIZE "
+        FC_FLAGS_OPT="-O3 $WORD_SIZE "
+        FC_FLAGS_LD_DEBUG="-g $WORD_SIZE"
+        FC_FLAGS_LD_OPT="-O3 $WORD_SIZE"
         ;;
     esac
     SHELL="/bin/sh"
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply CFLAGS patch to Mili 23.02"
        return 1
    fi

    return 0
}

function apply_mili_2302_blueos_patch
{
    info "Applying Mili 23.02 blueos patch."
    patch -p0 << \EOF
diff -u mili-23.02/src/eprtf.c.orig mili-23.02/src/eprtf.c
--- mili-23.02/src/eprtf.c.orig 2023-05-09 09:47:16.860814000 -0700
+++ mili-23.02/src/eprtf.c      2023-05-09 09:48:11.209704000 -0700
@@ -89,27 +89,23 @@
 #include "win32-regex.h"
 #endif

-#include "mili_enum.h"
+#include "mili_internal.h"
 #include "eprtf.h"

 static char destbuf[CMAX];
 static char *p_cur;
 static int cur_len;
 static va_list val;
-#ifdef HAVE_EPRINT
 static char *t_pattern = "%([0-9]+|[*])t";
 static regex_t all_re;
 static char *all_pattern =
    "%[0 -+#]*([0-9]*|[*])([.]([0-9]*|[*]))?[hlL]?[dioxXucsfeEgGpn%]";

-#endif
 static regex_t t_re;
 static regmatch_t t_match[1];


-#ifdef NOOPTERON
 static regmatch_t all_match[1];
-#endif


 /*****************************************************************
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply blueos patch to Mili 23.02"
        return 1
    fi

    return 0
}

function apply_mili_2302_write_funcs_patch
{
    #
    # write_funcs is not needed and having it in the header leads to
    # multiple definitions, which gcc 10.2 on Debian 11 doesn't like.
    #
    info "Applying Mili 23.02 write funcs patch."
    patch -p0 << \EOF
diff -u mili-23.02/src/mili_internal.h.orig mili-23.02/src/mili_internal.h
--- mili-23.02/src/mili_internal.h.orig 2023-05-09 09:41:20.347561000 -0700
+++ mili-23.02/src/mili_internal.h      2023-05-09 09:42:12.478771000 -0700
@@ -647,7 +647,6 @@
 /* dep.c - routines for handling architecture dependencies. */
 Return_value set_default_io_routines( Mili_family *fam );
 Return_value set_state_data_io_routines( Mili_family *fam );
-extern void (*write_funcs[QTY_PD_ENTRY_TYPES + 1])();

 /* svar.c - routines for managing state variables. */
 Bool_type valid_svar_data( Aggregate_type atype, char *name,
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply write funcs patch to Mili 23.02"
        return 1
    fi

    return 0
}

function apply_mili_patch
{
    if [[ "$OPSYS" == "Darwin" ]]; then
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

    if [[ ${MILI_VERSION} == 22.1 ]] ; then
        apply_mili_221_cflags_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_221_blueos_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_221_write_funcs_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_221_path_length_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    if [[ ${MILI_VERSION} == 23.02 ]] ; then
        apply_mili_2302_cflags_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_2302_blueos_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mili_2302_write_funcs_patch
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
    # 0, already exists, 1 untarred src, 2 error

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
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure Mili
    #
    info "Configuring Mili . . ."
    cd $MILI_BUILD_DIR || error "Can't cd to mili build dir."

    extra_ac_flags=""
    # detect coral systems, which older versions of autoconf don't detect
    if [[ "$(uname -m)" == "ppc64le" ]] ; then
         extra_ac_flags="ac_cv_build=powerpc64le-unknown-linux-gnu"
    elif [[ "$(uname -m)" == "aarch64" ]] ; then
         extra_ac_flags="ac_cv_build=aarch64-unknown-linux-gnu"
    fi

    F77_ARG=""
    TYPEDEFS_ARG=""
    config_script=configure
    if [[ ${MILI_VERSION} == 19.2 && "$OPSYS" == "Darwin" ]]; then
        config_script=configure_15_1
    elif [[ ${MILI_VERSION} == 22.1 && "$OPSYS" == "Darwin" ]] ; then
        # Mili 22.1 configure expects fortran compiler even if no intention to use it.
        # We spoof fortran compiler here to fool configure.
       cat << \EOF > spoof_f77.sh
#!/bin/sh
echo "#!/bin/sh" > conftest.out
chmod 755 conftest.out
EOF
        chmod 755 spoof_f77.sh
        F77_ARG="F77=./spoof_f77.sh"
        TYPEDEFS_ARG="ac_cv_type_mode_t=yes ac_cv_type_off_t=yes ac_cv_type_size_t=yes"
    fi

    info "Invoking command to configure Mili"
    set -x
    ./${config_script} CXX="$CXX_COMPILER" CC="$C_COMPILER" \
                CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
                ac_cv_prog_FOUND_GMAKE=make $extra_ac_flags $F77_ARG $TYPEDEFS_ARG \
                --prefix="$VISITDIR/mili/$MILI_VERSION/$VISITARCH"
    set +x
    if [[ $? != 0 ]] ; then
        warn "Mili configure failed.  Giving up"
        return 1
    fi

    #
    # Build Mili
    #
    info "Building Mili . . . (~2 minutes)"
    cd MILI-*-*
    $MAKE opt fortran=false

    #
    # Install into the VisIt third party location.
    #
    info "Installing Mili . . ." 

    mkdir "$VISITDIR/mili"
    mkdir "$VISITDIR/mili/$MILI_VERSION"
    mkdir "$VISITDIR/mili/$MILI_VERSION/$VISITARCH"
    mkdir "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
    mkdir "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/include"
    cp src/{mili.h,mili_enum.h,misc.h}  "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/include"
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        INSTALLNAMEPATH="$VISITDIR/mili/${MILI_VERSION}/$VISITARCH/lib"

        $C_COMPILER -dynamiclib -o libmili.$SO_EXT objs_opt/*.o \
                    -Wl,-headerpad_max_install_names \
                    -Wl,-install_name,$INSTALLNAMEPATH/libmili.${SO_EXT} \
                    -Wl,-compatibility_version,$MILI_COMPATIBILITY_VERSION \
                    -Wl,-current_version,$MILI_VERSION
        if [[ $? != 0 ]] ; then
            warn "Mili dynamic library build failed.  Giving up"
            return 1
        fi
        cp libmili.$SO_EXT "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
    else
        cp lib_opt/libmili.a "$VISITDIR/mili/$MILI_VERSION/$VISITARCH/lib"
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
