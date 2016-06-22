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
*** mili/src/mesh_u.c   2015-09-22 13:20:42.000000000 -0700
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
    patch -p0 << \EOF
*** mili/Makefile.Library       2013-12-10 12:55:55.000000000 -0800
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
    patch -p0 << \EOF
*** mili/src/mili_internal.h    2015-09-17 13:26:32.000000000 -0700
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

function apply_mili_151_darwin_patch4
{
    patch -p0 << \EOF
--- mili/configure	2015-09-11 06:32:00.000000000 -0700
+++ mili/configure_minus_fortran	2016-06-22 09:34:12.000000000 -0700
@@ -1,5 +1,5 @@
 #! /bin/sh
-# From configure.ac Revision: 1.26.4.1 .
+# From configure.ac Revision: 1.26.4.2 .
 # Guess values for system-dependent variables and create Makefiles.
 # Generated by GNU Autoconf 2.63 for Mili V15_01.
 #
@@ -721,16 +721,13 @@
 EGREP
 GREP
 CPP
+OBJEXT
+EXEEXT
 ac_ct_CC
 CPPFLAGS
+LDFLAGS
 CFLAGS
 CC
-OBJEXT
-EXEEXT
-ac_ct_F77
-LDFLAGS
-FFLAGS
-F77
 HOSTNAME
 HOSTDIR
 HDF_DEBUG_ENABLE
@@ -830,12 +827,10 @@
       ac_precious_vars='build_alias
 host_alias
 target_alias
-F77
-FFLAGS
-LDFLAGS
-LIBS
 CC
 CFLAGS
+LDFLAGS
+LIBS
 CPPFLAGS
 CPP'
 
@@ -1484,13 +1479,11 @@
   --with-usrbuild=PATH    Use given PATH for BUILD DIRECTORY
 
 Some influential environment variables:
-  F77         Fortran 77 compiler command
-  FFLAGS      Fortran 77 compiler flags
+  CC          C compiler command
+  CFLAGS      C compiler flags
   LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a
               nonstandard directory <lib dir>
   LIBS        libraries to pass to the linker, e.g. -l<library>
-  CC          C compiler command
-  CFLAGS      C compiler flags
   CPPFLAGS    C/C++/Objective C preprocessor flags, e.g. -I<include dir> if
               you have headers in a nonstandard directory <include dir>
   CPP         C preprocessor
@@ -2096,20 +2089,20 @@
 # Variable Initialization
 
 cat >>confdefs.h <<\_ACEOF
-#define PACKAGE_DATE "2015/09/11"
+#define PACKAGE_DATE "2015/09/18"
 _ACEOF
 
 
 cat >>confdefs.h <<\_ACEOF
-#define PACKAGE_TIME "06:00:00"
+#define PACKAGE_TIME "09:00:00"
 _ACEOF
 
 
 cat >>confdefs.h <<\_ACEOF
-#define PACKAGE_DATETIME "2015/09/11 06:00:00"
+#define PACKAGE_DATETIME "2015/09/18 09:00:00"
 _ACEOF
 
-PACKAGE_DATETIME="2015/09/11 06:00:00"
+PACKAGE_DATETIME="2015/09/18 09:00:00"
 
 
 #############################################################################
@@ -2441,22 +2434,24 @@
 # select compiler
 
 #AC_PROG_FC
-ac_ext=f
-ac_compile='$F77 -c $FFLAGS conftest.$ac_ext >&5'
-ac_link='$F77 -o conftest$ac_exeext $FFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&5'
-ac_compiler_gnu=$ac_cv_f77_compiler_gnu
+#AC_PROG_F77([ifort xlf gfortran])
+ac_ext=c
+ac_cpp='$CPP $CPPFLAGS'
+ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext >&5'
+ac_link='$CC -o conftest$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&5'
+ac_compiler_gnu=$ac_cv_c_compiler_gnu
 if test -n "$ac_tool_prefix"; then
-  for ac_prog in ifort xlf gfortran
+  for ac_prog in icc xlc gcc
   do
     # Extract the first word of "$ac_tool_prefix$ac_prog", so it can be a program name with args.
 set dummy $ac_tool_prefix$ac_prog; ac_word=$2
 { $as_echo "$as_me:$LINENO: checking for $ac_word" >&5
 $as_echo_n "checking for $ac_word... " >&6; }
-if test "${ac_cv_prog_F77+set}" = set; then
+if test "${ac_cv_prog_CC+set}" = set; then
   $as_echo_n "(cached) " >&6
 else
-  if test -n "$F77"; then
-  ac_cv_prog_F77="$F77" # Let the user override the test.
+  if test -n "$CC"; then
+  ac_cv_prog_CC="$CC" # Let the user override the test.
 else
 as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
 for as_dir in $PATH
@@ -2465,7 +2460,7 @@
   test -z "$as_dir" && as_dir=.
   for ac_exec_ext in '' $ac_executable_extensions; do
   if { test -f "$as_dir/$ac_word$ac_exec_ext" && $as_test_x "$as_dir/$ac_word$ac_exec_ext"; }; then
-    ac_cv_prog_F77="$ac_tool_prefix$ac_prog"
+    ac_cv_prog_CC="$ac_tool_prefix$ac_prog"
     $as_echo "$as_me:$LINENO: found $as_dir/$ac_word$ac_exec_ext" >&5
     break 2
   fi
@@ -2475,32 +2470,32 @@
 
 fi
 fi
-F77=$ac_cv_prog_F77
-if test -n "$F77"; then
-  { $as_echo "$as_me:$LINENO: result: $F77" >&5
-$as_echo "$F77" >&6; }
+CC=$ac_cv_prog_CC
+if test -n "$CC"; then
+  { $as_echo "$as_me:$LINENO: result: $CC" >&5
+$as_echo "$CC" >&6; }
 else
   { $as_echo "$as_me:$LINENO: result: no" >&5
 $as_echo "no" >&6; }
 fi
 
 
-    test -n "$F77" && break
+    test -n "$CC" && break
   done
 fi
-if test -z "$F77"; then
-  ac_ct_F77=$F77
-  for ac_prog in ifort xlf gfortran
+if test -z "$CC"; then
+  ac_ct_CC=$CC
+  for ac_prog in icc xlc gcc
 do
   # Extract the first word of "$ac_prog", so it can be a program name with args.
 set dummy $ac_prog; ac_word=$2
 { $as_echo "$as_me:$LINENO: checking for $ac_word" >&5
 $as_echo_n "checking for $ac_word... " >&6; }
-if test "${ac_cv_prog_ac_ct_F77+set}" = set; then
+if test "${ac_cv_prog_ac_ct_CC+set}" = set; then
   $as_echo_n "(cached) " >&6
 else
-  if test -n "$ac_ct_F77"; then
-  ac_cv_prog_ac_ct_F77="$ac_ct_F77" # Let the user override the test.
+  if test -n "$ac_ct_CC"; then
+  ac_cv_prog_ac_ct_CC="$ac_ct_CC" # Let the user override the test.
 else
 as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
 for as_dir in $PATH
@@ -2509,7 +2504,7 @@
   test -z "$as_dir" && as_dir=.
   for ac_exec_ext in '' $ac_executable_extensions; do
   if { test -f "$as_dir/$ac_word$ac_exec_ext" && $as_test_x "$as_dir/$ac_word$ac_exec_ext"; }; then
-    ac_cv_prog_ac_ct_F77="$ac_prog"
+    ac_cv_prog_ac_ct_CC="$ac_prog"
     $as_echo "$as_me:$LINENO: found $as_dir/$ac_word$ac_exec_ext" >&5
     break 2
   fi
@@ -2519,21 +2514,21 @@
 
 fi
 fi
-ac_ct_F77=$ac_cv_prog_ac_ct_F77
-if test -n "$ac_ct_F77"; then
-  { $as_echo "$as_me:$LINENO: result: $ac_ct_F77" >&5
-$as_echo "$ac_ct_F77" >&6; }
+ac_ct_CC=$ac_cv_prog_ac_ct_CC
+if test -n "$ac_ct_CC"; then
+  { $as_echo "$as_me:$LINENO: result: $ac_ct_CC" >&5
+$as_echo "$ac_ct_CC" >&6; }
 else
   { $as_echo "$as_me:$LINENO: result: no" >&5
 $as_echo "no" >&6; }
 fi
 
 
-  test -n "$ac_ct_F77" && break
+  test -n "$ac_ct_CC" && break
 done
 
-  if test "x$ac_ct_F77" = x; then
-    F77=""
+  if test "x$ac_ct_CC" = x; then
+    CC=""
   else
     case $cross_compiling:$ac_tool_warned in
 yes:)
@@ -2541,13 +2536,21 @@
 $as_echo "$as_me: WARNING: using cross tools not prefixed with host triplet" >&2;}
 ac_tool_warned=yes ;;
 esac
-    F77=$ac_ct_F77
+    CC=$ac_ct_CC
   fi
 fi
 
 
+test -z "$CC" && { { $as_echo "$as_me:$LINENO: error: in \`$ac_pwd':" >&5
+$as_echo "$as_me: error: in \`$ac_pwd':" >&2;}
+{ { $as_echo "$as_me:$LINENO: error: no acceptable C compiler found in \$PATH
+See \`config.log' for more details." >&5
+$as_echo "$as_me: error: no acceptable C compiler found in \$PATH
+See \`config.log' for more details." >&2;}
+   { (exit 1); exit 1; }; }; }
+
 # Provide some information about the compiler.
-$as_echo "$as_me:$LINENO: checking for Fortran 77 compiler version" >&5
+$as_echo "$as_me:$LINENO: checking for C compiler version" >&5
 set X $ac_compile
 ac_compiler=$2
 { (ac_try="$ac_compiler --version >&5"
@@ -2583,20 +2586,29 @@
   ac_status=$?
   $as_echo "$as_me:$LINENO: \$? = $ac_status" >&5
   (exit $ac_status); }
-rm -f a.out
 
 cat >conftest.$ac_ext <<_ACEOF
-      program main
+/* confdefs.h.  */
+_ACEOF
+cat confdefs.h >>conftest.$ac_ext
+cat >>conftest.$ac_ext <<_ACEOF
+/* end confdefs.h.  */
 
-      end
+int
+main ()
+{
+
+  ;
+  return 0;
+}
 _ACEOF
 ac_clean_files_save=$ac_clean_files
 ac_clean_files="$ac_clean_files a.out a.out.dSYM a.exe b.out"
 # Try to create an executable without -o first, disregard a.out.
 # It will help us diagnose broken compilers, and finding out an intuition
 # of exeext.
-{ $as_echo "$as_me:$LINENO: checking for Fortran 77 compiler default output file name" >&5
-$as_echo_n "checking for Fortran 77 compiler default output file name... " >&6; }
+{ $as_echo "$as_me:$LINENO: checking for C compiler default output file name" >&5
+$as_echo_n "checking for C compiler default output file name... " >&6; }
 ac_link_default=`$as_echo "$ac_link" | sed 's/ -o *conftest[^ ]*//'`
 
 # The possible output files:
@@ -2667,9 +2679,9 @@
 
 { { $as_echo "$as_me:$LINENO: error: in \`$ac_pwd':" >&5
 $as_echo "$as_me: error: in \`$ac_pwd':" >&2;}
-{ { $as_echo "$as_me:$LINENO: error: Fortran 77 compiler cannot create executables
+{ { $as_echo "$as_me:$LINENO: error: C compiler cannot create executables
 See \`config.log' for more details." >&5
-$as_echo "$as_me: error: Fortran 77 compiler cannot create executables
+$as_echo "$as_me: error: C compiler cannot create executables
 See \`config.log' for more details." >&2;}
    { (exit 77); exit 77; }; }; }
 fi
@@ -2678,8 +2690,8 @@
 
 # Check that the compiler produces executables we can run.  If not, either
 # the compiler is broken, or we cross compile.
-{ $as_echo "$as_me:$LINENO: checking whether the Fortran 77 compiler works" >&5
-$as_echo_n "checking whether the Fortran 77 compiler works... " >&6; }
+{ $as_echo "$as_me:$LINENO: checking whether the C compiler works" >&5
+$as_echo_n "checking whether the C compiler works... " >&6; }
 # FIXME: These cross compiler hacks should be removed for Autoconf 3.0
 # If not cross compiling, check that we can run a simple program.
 if test "$cross_compiling" != yes; then
@@ -2701,10 +2713,10 @@
     else
 	{ { $as_echo "$as_me:$LINENO: error: in \`$ac_pwd':" >&5
 $as_echo "$as_me: error: in \`$ac_pwd':" >&2;}
-{ { $as_echo "$as_me:$LINENO: error: cannot run Fortran 77 compiled programs.
+{ { $as_echo "$as_me:$LINENO: error: cannot run C compiled programs.
 If you meant to cross compile, use \`--host'.
 See \`config.log' for more details." >&5
-$as_echo "$as_me: error: cannot run Fortran 77 compiled programs.
+$as_echo "$as_me: error: cannot run C compiled programs.
 If you meant to cross compile, use \`--host'.
 See \`config.log' for more details." >&2;}
    { (exit 1); exit 1; }; }; }
@@ -2772,9 +2784,19 @@
   $as_echo_n "(cached) " >&6
 else
   cat >conftest.$ac_ext <<_ACEOF
-      program main
+/* confdefs.h.  */
+_ACEOF
+cat confdefs.h >>conftest.$ac_ext
+cat >>conftest.$ac_ext <<_ACEOF
+/* end confdefs.h.  */
+
+int
+main ()
+{
 
-      end
+  ;
+  return 0;
+}
 _ACEOF
 rm -f conftest.o conftest.obj
 if { (ac_try="$ac_compile"
@@ -2815,280 +2837,6 @@
 $as_echo "$ac_cv_objext" >&6; }
 OBJEXT=$ac_cv_objext
 ac_objext=$OBJEXT
-# If we don't use `.F' as extension, the preprocessor is not run on the
-# input file.  (Note that this only needs to work for GNU compilers.)
-ac_save_ext=$ac_ext
-ac_ext=F
-{ $as_echo "$as_me:$LINENO: checking whether we are using the GNU Fortran 77 compiler" >&5
-$as_echo_n "checking whether we are using the GNU Fortran 77 compiler... " >&6; }
-if test "${ac_cv_f77_compiler_gnu+set}" = set; then
-  $as_echo_n "(cached) " >&6
-else
-  cat >conftest.$ac_ext <<_ACEOF
-      program main
-#ifndef __GNUC__
-       choke me
-#endif
-
-      end
-_ACEOF
-rm -f conftest.$ac_objext
-if { (ac_try="$ac_compile"
-case "(($ac_try" in
-  *\"* | *\`* | *\\*) ac_try_echo=\$ac_try;;
-  *) ac_try_echo=$ac_try;;
-esac
-eval ac_try_echo="\"\$as_me:$LINENO: $ac_try_echo\""
-$as_echo "$ac_try_echo") >&5
-  (eval "$ac_compile") 2>conftest.er1
-  ac_status=$?
-  grep -v '^ *+' conftest.er1 >conftest.err
-  rm -f conftest.er1
-  cat conftest.err >&5
-  $as_echo "$as_me:$LINENO: \$? = $ac_status" >&5
-  (exit $ac_status); } && {
-	 test -z "$ac_f77_werror_flag" ||
-	 test ! -s conftest.err
-       } && test -s conftest.$ac_objext; then
-  ac_compiler_gnu=yes
-else
-  $as_echo "$as_me: failed program was:" >&5
-sed 's/^/| /' conftest.$ac_ext >&5
-
-	ac_compiler_gnu=no
-fi
-
-rm -f core conftest.err conftest.$ac_objext conftest.$ac_ext
-ac_cv_f77_compiler_gnu=$ac_compiler_gnu
-
-fi
-{ $as_echo "$as_me:$LINENO: result: $ac_cv_f77_compiler_gnu" >&5
-$as_echo "$ac_cv_f77_compiler_gnu" >&6; }
-ac_ext=$ac_save_ext
-ac_test_FFLAGS=${FFLAGS+set}
-ac_save_FFLAGS=$FFLAGS
-FFLAGS=
-{ $as_echo "$as_me:$LINENO: checking whether $F77 accepts -g" >&5
-$as_echo_n "checking whether $F77 accepts -g... " >&6; }
-if test "${ac_cv_prog_f77_g+set}" = set; then
-  $as_echo_n "(cached) " >&6
-else
-  FFLAGS=-g
-cat >conftest.$ac_ext <<_ACEOF
-      program main
-
-      end
-_ACEOF
-rm -f conftest.$ac_objext
-if { (ac_try="$ac_compile"
-case "(($ac_try" in
-  *\"* | *\`* | *\\*) ac_try_echo=\$ac_try;;
-  *) ac_try_echo=$ac_try;;
-esac
-eval ac_try_echo="\"\$as_me:$LINENO: $ac_try_echo\""
-$as_echo "$ac_try_echo") >&5
-  (eval "$ac_compile") 2>conftest.er1
-  ac_status=$?
-  grep -v '^ *+' conftest.er1 >conftest.err
-  rm -f conftest.er1
-  cat conftest.err >&5
-  $as_echo "$as_me:$LINENO: \$? = $ac_status" >&5
-  (exit $ac_status); } && {
-	 test -z "$ac_f77_werror_flag" ||
-	 test ! -s conftest.err
-       } && test -s conftest.$ac_objext; then
-  ac_cv_prog_f77_g=yes
-else
-  $as_echo "$as_me: failed program was:" >&5
-sed 's/^/| /' conftest.$ac_ext >&5
-
-	ac_cv_prog_f77_g=no
-fi
-
-rm -f core conftest.err conftest.$ac_objext conftest.$ac_ext
-
-fi
-{ $as_echo "$as_me:$LINENO: result: $ac_cv_prog_f77_g" >&5
-$as_echo "$ac_cv_prog_f77_g" >&6; }
-if test "$ac_test_FFLAGS" = set; then
-  FFLAGS=$ac_save_FFLAGS
-elif test $ac_cv_prog_f77_g = yes; then
-  if test "x$ac_cv_f77_compiler_gnu" = xyes; then
-    FFLAGS="-g -O2"
-  else
-    FFLAGS="-g"
-  fi
-else
-  if test "x$ac_cv_f77_compiler_gnu" = xyes; then
-    FFLAGS="-O2"
-  else
-    FFLAGS=
-  fi
-fi
-
-if test $ac_compiler_gnu = yes; then
-  G77=yes
-else
-  G77=
-fi
-ac_ext=c
-ac_cpp='$CPP $CPPFLAGS'
-ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext >&5'
-ac_link='$CC -o conftest$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&5'
-ac_compiler_gnu=$ac_cv_c_compiler_gnu
-
-ac_ext=c
-ac_cpp='$CPP $CPPFLAGS'
-ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext >&5'
-ac_link='$CC -o conftest$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&5'
-ac_compiler_gnu=$ac_cv_c_compiler_gnu
-if test -n "$ac_tool_prefix"; then
-  for ac_prog in icc xlc gcc
-  do
-    # Extract the first word of "$ac_tool_prefix$ac_prog", so it can be a program name with args.
-set dummy $ac_tool_prefix$ac_prog; ac_word=$2
-{ $as_echo "$as_me:$LINENO: checking for $ac_word" >&5
-$as_echo_n "checking for $ac_word... " >&6; }
-if test "${ac_cv_prog_CC+set}" = set; then
-  $as_echo_n "(cached) " >&6
-else
-  if test -n "$CC"; then
-  ac_cv_prog_CC="$CC" # Let the user override the test.
-else
-as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
-for as_dir in $PATH
-do
-  IFS=$as_save_IFS
-  test -z "$as_dir" && as_dir=.
-  for ac_exec_ext in '' $ac_executable_extensions; do
-  if { test -f "$as_dir/$ac_word$ac_exec_ext" && $as_test_x "$as_dir/$ac_word$ac_exec_ext"; }; then
-    ac_cv_prog_CC="$ac_tool_prefix$ac_prog"
-    $as_echo "$as_me:$LINENO: found $as_dir/$ac_word$ac_exec_ext" >&5
-    break 2
-  fi
-done
-done
-IFS=$as_save_IFS
-
-fi
-fi
-CC=$ac_cv_prog_CC
-if test -n "$CC"; then
-  { $as_echo "$as_me:$LINENO: result: $CC" >&5
-$as_echo "$CC" >&6; }
-else
-  { $as_echo "$as_me:$LINENO: result: no" >&5
-$as_echo "no" >&6; }
-fi
-
-
-    test -n "$CC" && break
-  done
-fi
-if test -z "$CC"; then
-  ac_ct_CC=$CC
-  for ac_prog in icc xlc gcc
-do
-  # Extract the first word of "$ac_prog", so it can be a program name with args.
-set dummy $ac_prog; ac_word=$2
-{ $as_echo "$as_me:$LINENO: checking for $ac_word" >&5
-$as_echo_n "checking for $ac_word... " >&6; }
-if test "${ac_cv_prog_ac_ct_CC+set}" = set; then
-  $as_echo_n "(cached) " >&6
-else
-  if test -n "$ac_ct_CC"; then
-  ac_cv_prog_ac_ct_CC="$ac_ct_CC" # Let the user override the test.
-else
-as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
-for as_dir in $PATH
-do
-  IFS=$as_save_IFS
-  test -z "$as_dir" && as_dir=.
-  for ac_exec_ext in '' $ac_executable_extensions; do
-  if { test -f "$as_dir/$ac_word$ac_exec_ext" && $as_test_x "$as_dir/$ac_word$ac_exec_ext"; }; then
-    ac_cv_prog_ac_ct_CC="$ac_prog"
-    $as_echo "$as_me:$LINENO: found $as_dir/$ac_word$ac_exec_ext" >&5
-    break 2
-  fi
-done
-done
-IFS=$as_save_IFS
-
-fi
-fi
-ac_ct_CC=$ac_cv_prog_ac_ct_CC
-if test -n "$ac_ct_CC"; then
-  { $as_echo "$as_me:$LINENO: result: $ac_ct_CC" >&5
-$as_echo "$ac_ct_CC" >&6; }
-else
-  { $as_echo "$as_me:$LINENO: result: no" >&5
-$as_echo "no" >&6; }
-fi
-
-
-  test -n "$ac_ct_CC" && break
-done
-
-  if test "x$ac_ct_CC" = x; then
-    CC=""
-  else
-    case $cross_compiling:$ac_tool_warned in
-yes:)
-{ $as_echo "$as_me:$LINENO: WARNING: using cross tools not prefixed with host triplet" >&5
-$as_echo "$as_me: WARNING: using cross tools not prefixed with host triplet" >&2;}
-ac_tool_warned=yes ;;
-esac
-    CC=$ac_ct_CC
-  fi
-fi
-
-
-test -z "$CC" && { { $as_echo "$as_me:$LINENO: error: in \`$ac_pwd':" >&5
-$as_echo "$as_me: error: in \`$ac_pwd':" >&2;}
-{ { $as_echo "$as_me:$LINENO: error: no acceptable C compiler found in \$PATH
-See \`config.log' for more details." >&5
-$as_echo "$as_me: error: no acceptable C compiler found in \$PATH
-See \`config.log' for more details." >&2;}
-   { (exit 1); exit 1; }; }; }
-
-# Provide some information about the compiler.
-$as_echo "$as_me:$LINENO: checking for C compiler version" >&5
-set X $ac_compile
-ac_compiler=$2
-{ (ac_try="$ac_compiler --version >&5"
-case "(($ac_try" in
-  *\"* | *\`* | *\\*) ac_try_echo=\$ac_try;;
-  *) ac_try_echo=$ac_try;;
-esac
-eval ac_try_echo="\"\$as_me:$LINENO: $ac_try_echo\""
-$as_echo "$ac_try_echo") >&5
-  (eval "$ac_compiler --version >&5") 2>&5
-  ac_status=$?
-  $as_echo "$as_me:$LINENO: \$? = $ac_status" >&5
-  (exit $ac_status); }
-{ (ac_try="$ac_compiler -v >&5"
-case "(($ac_try" in
-  *\"* | *\`* | *\\*) ac_try_echo=\$ac_try;;
-  *) ac_try_echo=$ac_try;;
-esac
-eval ac_try_echo="\"\$as_me:$LINENO: $ac_try_echo\""
-$as_echo "$ac_try_echo") >&5
-  (eval "$ac_compiler -v >&5") 2>&5
-  ac_status=$?
-  $as_echo "$as_me:$LINENO: \$? = $ac_status" >&5
-  (exit $ac_status); }
-{ (ac_try="$ac_compiler -V >&5"
-case "(($ac_try" in
-  *\"* | *\`* | *\\*) ac_try_echo=\$ac_try;;
-  *) ac_try_echo=$ac_try;;
-esac
-eval ac_try_echo="\"\$as_me:$LINENO: $ac_try_echo\""
-$as_echo "$ac_try_echo") >&5
-  (eval "$ac_compiler -V >&5") 2>&5
-  ac_status=$?
-  $as_echo "$as_me:$LINENO: \$? = $ac_status" >&5
-  (exit $ac_status); }
-
 { $as_echo "$as_me:$LINENO: checking whether we are using the GNU C compiler" >&5
 $as_echo_n "checking whether we are using the GNU C compiler... " >&6; }
 if test "${ac_cv_c_compiler_gnu+set}" = set; then
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply Darwin patch 4 to Mili 15.1"
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
        apply_mili_151_darwin_patch4
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
