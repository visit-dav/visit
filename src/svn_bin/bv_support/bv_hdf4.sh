function bv_hdf4_initialize
{
export DO_HDF4="no"
export ON_HDF4="off"
}

function bv_hdf4_enable
{
DO_HDF4="yes"
ON_HDF4="on"
DO_SZIP="yes"
ON_SZIP="on"
}

function bv_hdf4_disable
{
DO_HDF4="no"
ON_HDF4="off"
}

function bv_hdf4_depends_on
{
return ""
}

function bv_hdf4_info
{
export HDF4_FILE=${HDF4_FILE:-"hdf-4.2.5.tar.gz"}
export HDF4_VERSION=${HDF4_VERSION:-"4.2.5"}
export HDF4_COMPATIBILITY_VERSION=${HDF4_COMPATIBILITY_VERSION:-"4.2"}
export HDF4_BUILD_DIR=${HDF4_BUILD_DIR:-"hdf-4.2.5"}
export HDF4_URL=${HDF4_URL:-"http://www.hdfgroup.org/ftp/HDF/HDF_Current/src"}
}

function bv_hdf4_print
{
  printf "%s%s\n" "HDF4_FILE=" "${HDF4_FILE}"
  printf "%s%s\n" "HDF4_VERSION=" "${HDF4_VERSION}"
  printf "%s%s\n" "HDF4_COMPATIBILITY_VERSION=" "${HDF4_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "HDF4_BUILD_DIR=" "${HDF4_BUILD_DIR}"
}

function bv_hdf4_print_usage
{
printf "%-15s %s [%s]\n" "--hdf4" "Build HDF4" "${DO_HDF4}"
}

function bv_hdf4_graphical
{
local graphical_out="HDF4     $HDF4_VERSION($HDF4_FILE)      $ON_HDF4"
echo $graphical_out
}

function bv_hdf4_host_profile
{
    if [[ "$DO_HDF4" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## HDF4" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_HDF4_DIR \${VISITHOME}/hdf4/$HDF4_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        if [[ "$DO_SZIP" == "yes" ]] ; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP \${VISITHOME}/szip/$SZIP_VERSION/\${VISITARCH}/lib sz \${VISITHOME}/vtk/$VTK_VERSION/\${VISITARCH}/lib vtkjpeg TYPE STRING)" \
            >> $HOSTCONF
          else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_HDF4_LIBDEP \${VISITHOME}/vtk/$VTK_VERSION/\${VISITARCH}/lib vtkjpeg TYPE STRING)" \
            >> $HOSTCONF
        fi
        >> $HOSTCONF
    fi
}

function bv_hdf4_ensure
{
    if [[ "$DO_HDF4" == "yes" ]] ; then
        ensure_built_or_ready "hdf4" $HDF4_VERSION $HDF4_BUILD_DIR $HDF4_FILE $HDF4_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_HDF4="no"
            error "Unable to build HDF4.  ${HDF4_FILE} not found."
        fi
    fi
}

function bv_hdf4_dry_run
{
  if [[ "$DO_HDF4" == "yes" ]] ; then
    echo "Dry run option not set for hdf4."
  fi
}


# *************************************************************************** #
#                          Function 8.3, build_hdf4                           #
# *************************************************************************** #
function apply_hdf4_421_darwin_patch
{
   patch -p0 << \EOF
*** HDF4.2r1.orig/configure	Tue Feb  8 10:29:27 2005
--- HDF4.2r1/configure	Thu Apr 26 13:30:56 2007
*************** done
*** 5656,5711 ****
  
  echo "$as_me:$LINENO: checking for jpeg_start_decompress in -ljpeg" >&5
  echo $ECHO_N "checking for jpeg_start_decompress in -ljpeg... $ECHO_C" >&6
! if test "${ac_cv_lib_jpeg_jpeg_start_decompress+set}" = set; then
!   echo $ECHO_N "(cached) $ECHO_C" >&6
! else
!   ac_check_lib_save_LIBS=$LIBS
! LIBS="-ljpeg  $LIBS"
! cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! 
! /* Override any gcc2 internal prototype to avoid an error.  */
! #ifdef __cplusplus
! extern "C"
! #endif
! /* We use char because int might match the return type of a gcc2
!    builtin and then its argument prototype would still apply.  */
! char jpeg_start_decompress ();
! int
! main ()
! {
! jpeg_start_decompress ();
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest.$ac_objext conftest$ac_exeext
! if { (eval echo "$as_me:$LINENO: \"$ac_link\"") >&5
!   (eval $ac_link) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } &&
!          { ac_try='test -s conftest$ac_exeext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_cv_lib_jpeg_jpeg_start_decompress=yes
! else
!   echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! ac_cv_lib_jpeg_jpeg_start_decompress=no
! fi
! rm -f conftest.$ac_objext conftest$ac_exeext conftest.$ac_ext
! LIBS=$ac_check_lib_save_LIBS
! fi
  echo "$as_me:$LINENO: result: $ac_cv_lib_jpeg_jpeg_start_decompress" >&5
  echo "${ECHO_T}$ac_cv_lib_jpeg_jpeg_start_decompress" >&6
  if test $ac_cv_lib_jpeg_jpeg_start_decompress = yes; then
--- 5656,5712 ----
  
  echo "$as_me:$LINENO: checking for jpeg_start_decompress in -ljpeg" >&5
  echo $ECHO_N "checking for jpeg_start_decompress in -ljpeg... $ECHO_C" >&6
! #if test "${ac_cv_lib_jpeg_jpeg_start_decompress+set}" = set; then
! #  echo $ECHO_N "(cached) $ECHO_C" >&6
! #else
! #  ac_check_lib_save_LIBS=$LIBS
! #LIBS="-ljpeg  $LIBS"
! #cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #
! #/* Override any gcc2 internal prototype to avoid an error.  */
! ##ifdef __cplusplus
! #extern "C"
! ##endif
! #/* We use char because int might match the return type of a gcc2
! #   builtin and then its argument prototype would still apply.  */
! #char jpeg_start_decompress ();
! #int
! #main ()
! #{
! #jpeg_start_decompress ();
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest.$ac_objext conftest$ac_exeext
! #if { (eval echo "$as_me:$LINENO: \"$ac_link\"") >&5
! #  (eval $ac_link) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } &&
! #         { ac_try='test -s conftest$ac_exeext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_cv_lib_jpeg_jpeg_start_decompress=yes
! #else
! #  echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #ac_cv_lib_jpeg_jpeg_start_decompress=no
! #fi
! #rm -f conftest.$ac_objext conftest$ac_exeext conftest.$ac_ext
! #LIBS=$ac_check_lib_save_LIBS
! #fi
! ac_cv_lib_jpeg_jpeg_start_decompress=yes
  echo "$as_me:$LINENO: result: $ac_cv_lib_jpeg_jpeg_start_decompress" >&5
  echo "${ECHO_T}$ac_cv_lib_jpeg_jpeg_start_decompress" >&6
  if test $ac_cv_lib_jpeg_jpeg_start_decompress = yes; then
*************** echo "${ECHO_T}$ac_cv_type_intp" >&6
*** 6874,7183 ****
  
  echo "$as_me:$LINENO: checking size of int*" >&5
  echo $ECHO_N "checking size of int*... $ECHO_C" >&6
! if test "${ac_cv_sizeof_intp+set}" = set; then
!   echo $ECHO_N "(cached) $ECHO_C" >&6
! else
!   if test "$ac_cv_type_intp" = yes; then
!   # The cast to unsigned long works around a bug in the HP C Compiler
!   # version HP92453-01 B.11.11.23709.GP, which incorrectly rejects
!   # declarations like `int a3[[(sizeof (unsigned char)) >= 0]];'.
!   # This bug is HP SR number 8606223364.
!   if test "$cross_compiling" = yes; then
!   # Depending upon the size, compute the lo and hi bounds.
! cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! $ac_includes_default
! int
! main ()
! {
! static int test_array [1 - 2 * !(((long) (sizeof (int*))) >= 0)];
! test_array [0] = 0
! 
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest.$ac_objext
! if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
!   (eval $ac_compile) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } &&
!          { ac_try='test -s conftest.$ac_objext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_lo=0 ac_mid=0
!   while :; do
!     cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! $ac_includes_default
! int
! main ()
! {
! static int test_array [1 - 2 * !(((long) (sizeof (int*))) <= $ac_mid)];
! test_array [0] = 0
! 
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest.$ac_objext
! if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
!   (eval $ac_compile) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } &&
!          { ac_try='test -s conftest.$ac_objext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_hi=$ac_mid; break
! else
!   echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! ac_lo=`expr $ac_mid + 1`
!                     if test $ac_lo -le $ac_mid; then
!                       ac_lo= ac_hi=
!                       break
!                     fi
!                     ac_mid=`expr 2 '*' $ac_mid + 1`
! fi
! rm -f conftest.$ac_objext conftest.$ac_ext
!   done
! else
!   echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! $ac_includes_default
! int
! main ()
! {
! static int test_array [1 - 2 * !(((long) (sizeof (int*))) < 0)];
! test_array [0] = 0
! 
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest.$ac_objext
! if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
!   (eval $ac_compile) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } &&
!          { ac_try='test -s conftest.$ac_objext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_hi=-1 ac_mid=-1
!   while :; do
!     cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! $ac_includes_default
! int
! main ()
! {
! static int test_array [1 - 2 * !(((long) (sizeof (int*))) >= $ac_mid)];
! test_array [0] = 0
! 
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest.$ac_objext
! if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
!   (eval $ac_compile) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } &&
!          { ac_try='test -s conftest.$ac_objext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_lo=$ac_mid; break
! else
!   echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! ac_hi=`expr '(' $ac_mid ')' - 1`
!                        if test $ac_mid -le $ac_hi; then
!                          ac_lo= ac_hi=
!                          break
!                        fi
!                        ac_mid=`expr 2 '*' $ac_mid`
! fi
! rm -f conftest.$ac_objext conftest.$ac_ext
!   done
! else
!   echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! ac_lo= ac_hi=
! fi
! rm -f conftest.$ac_objext conftest.$ac_ext
! fi
! rm -f conftest.$ac_objext conftest.$ac_ext
! # Binary search between lo and hi bounds.
! while test "x$ac_lo" != "x$ac_hi"; do
!   ac_mid=`expr '(' $ac_hi - $ac_lo ')' / 2 + $ac_lo`
!   cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! $ac_includes_default
! int
! main ()
! {
! static int test_array [1 - 2 * !(((long) (sizeof (int*))) <= $ac_mid)];
! test_array [0] = 0
! 
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest.$ac_objext
! if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
!   (eval $ac_compile) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } &&
!          { ac_try='test -s conftest.$ac_objext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_hi=$ac_mid
! else
!   echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! ac_lo=`expr '(' $ac_mid ')' + 1`
! fi
! rm -f conftest.$ac_objext conftest.$ac_ext
! done
! case $ac_lo in
! ?*) ac_cv_sizeof_intp=$ac_lo;;
! '') { { echo "$as_me:$LINENO: error: cannot compute sizeof (int*), 77
! See \`config.log' for more details." >&5
! echo "$as_me: error: cannot compute sizeof (int*), 77
! See \`config.log' for more details." >&2;}
!    { (exit 1); exit 1; }; } ;;
! esac
! else
!   if test "$cross_compiling" = yes; then
!   { { echo "$as_me:$LINENO: error: cannot run test program while cross compiling
! See \`config.log' for more details." >&5
! echo "$as_me: error: cannot run test program while cross compiling
! See \`config.log' for more details." >&2;}
!    { (exit 1); exit 1; }; }
! else
!   cat >conftest.$ac_ext <<_ACEOF
! #line $LINENO "configure"
! /* confdefs.h.  */
! _ACEOF
! cat confdefs.h >>conftest.$ac_ext
! cat >>conftest.$ac_ext <<_ACEOF
! /* end confdefs.h.  */
! $ac_includes_default
! long longval () { return (long) (sizeof (int*)); }
! unsigned long ulongval () { return (long) (sizeof (int*)); }
! #include <stdio.h>
! #include <stdlib.h>
! int
! main ()
! {
! 
!   FILE *f = fopen ("conftest.val", "w");
!   if (! f)
!     exit (1);
!   if (((long) (sizeof (int*))) < 0)
!     {
!       long i = longval ();
!       if (i != ((long) (sizeof (int*))))
! 	exit (1);
!       fprintf (f, "%ld\n", i);
!     }
!   else
!     {
!       unsigned long i = ulongval ();
!       if (i != ((long) (sizeof (int*))))
! 	exit (1);
!       fprintf (f, "%lu\n", i);
!     }
!   exit (ferror (f) || fclose (f) != 0);
! 
!   ;
!   return 0;
! }
! _ACEOF
! rm -f conftest$ac_exeext
! if { (eval echo "$as_me:$LINENO: \"$ac_link\"") >&5
!   (eval $ac_link) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); } && { ac_try='./conftest$ac_exeext'
!   { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
!   (eval $ac_try) 2>&5
!   ac_status=$?
!   echo "$as_me:$LINENO: \$? = $ac_status" >&5
!   (exit $ac_status); }; }; then
!   ac_cv_sizeof_intp=`cat conftest.val`
! else
!   echo "$as_me: program exited with status $ac_status" >&5
! echo "$as_me: failed program was:" >&5
! sed 's/^/| /' conftest.$ac_ext >&5
! 
! ( exit $ac_status )
! { { echo "$as_me:$LINENO: error: cannot compute sizeof (int*), 77
! See \`config.log' for more details." >&5
! echo "$as_me: error: cannot compute sizeof (int*), 77
! See \`config.log' for more details." >&2;}
!    { (exit 1); exit 1; }; }
! fi
! rm -f core core.* *.core gmon.out bb.out conftest$ac_exeext conftest.$ac_objext conftest.$ac_ext
! fi
! fi
! rm -f conftest.val
! else
!   ac_cv_sizeof_intp=0
! fi
! fi
  echo "$as_me:$LINENO: result: $ac_cv_sizeof_intp" >&5
  echo "${ECHO_T}$ac_cv_sizeof_intp" >&6
  cat >>confdefs.h <<_ACEOF
--- 6875,7185 ----
  
  echo "$as_me:$LINENO: checking size of int*" >&5
  echo $ECHO_N "checking size of int*... $ECHO_C" >&6
! #if test "${ac_cv_sizeof_intp+set}" = set; then
! #  echo $ECHO_N "(cached) $ECHO_C" >&6
! #else
! #  if test "$ac_cv_type_intp" = yes; then
! #  # The cast to unsigned long works around a bug in the HP C Compiler
! #  # version HP92453-01 B.11.11.23709.GP, which incorrectly rejects
! #  # declarations like `int a3[[(sizeof (unsigned char)) >= 0]];'.
! #  # This bug is HP SR number 8606223364.
! #  if test "$cross_compiling" = yes; then
! #  # Depending upon the size, compute the lo and hi bounds.
! #cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #$ac_includes_default
! #int
! #main ()
! #{
! #static int test_array [1 - 2 * !(((long) (sizeof (int*))) >= 0)];
! #test_array [0] = 0
! #
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest.$ac_objext
! #if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
! #  (eval $ac_compile) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } &&
! #         { ac_try='test -s conftest.$ac_objext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_lo=0 ac_mid=0
! #  while :; do
! #    cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #$ac_includes_default
! #int
! #main ()
! #{
! #static int test_array [1 - 2 * !(((long) (sizeof (int*))) <= $ac_mid)];
! #test_array [0] = 0
! #
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest.$ac_objext
! #if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
! #  (eval $ac_compile) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } &&
! #         { ac_try='test -s conftest.$ac_objext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_hi=$ac_mid; break
! #else
! #  echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #ac_lo=`expr $ac_mid + 1`
! #                    if test $ac_lo -le $ac_mid; then
! #                      ac_lo= ac_hi=
! #                      break
! #                    fi
! #                    ac_mid=`expr 2 '*' $ac_mid + 1`
! #fi
! #rm -f conftest.$ac_objext conftest.$ac_ext
! #  done
! #else
! #  echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #$ac_includes_default
! #int
! #main ()
! #{
! #static int test_array [1 - 2 * !(((long) (sizeof (int*))) < 0)];
! #test_array [0] = 0
! #
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest.$ac_objext
! #if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
! #  (eval $ac_compile) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } &&
! #         { ac_try='test -s conftest.$ac_objext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_hi=-1 ac_mid=-1
! #  while :; do
! #    cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #$ac_includes_default
! #int
! #main ()
! #{
! #static int test_array [1 - 2 * !(((long) (sizeof (int*))) >= $ac_mid)];
! #test_array [0] = 0
! #
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest.$ac_objext
! #if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
! #  (eval $ac_compile) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } &&
! #         { ac_try='test -s conftest.$ac_objext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_lo=$ac_mid; break
! #else
! #  echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #ac_hi=`expr '(' $ac_mid ')' - 1`
! #                       if test $ac_mid -le $ac_hi; then
! #                         ac_lo= ac_hi=
! #                         break
! #                       fi
! #                       ac_mid=`expr 2 '*' $ac_mid`
! #fi
! #rm -f conftest.$ac_objext conftest.$ac_ext
! #  done
! #else
! #  echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #ac_lo= ac_hi=
! #fi
! #rm -f conftest.$ac_objext conftest.$ac_ext
! #fi
! #rm -f conftest.$ac_objext conftest.$ac_ext
! ## Binary search between lo and hi bounds.
! #while test "x$ac_lo" != "x$ac_hi"; do
! #  ac_mid=`expr '(' $ac_hi - $ac_lo ')' / 2 + $ac_lo`
! #  cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #$ac_includes_default
! #int
! #main ()
! #{
! #static int test_array [1 - 2 * !(((long) (sizeof (int*))) <= $ac_mid)];
! #test_array [0] = 0
! #
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest.$ac_objext
! #if { (eval echo "$as_me:$LINENO: \"$ac_compile\"") >&5
! #  (eval $ac_compile) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } &&
! #         { ac_try='test -s conftest.$ac_objext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_hi=$ac_mid
! #else
! #  echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #ac_lo=`expr '(' $ac_mid ')' + 1`
! #fi
! #rm -f conftest.$ac_objext conftest.$ac_ext
! #done
! #case $ac_lo in
! #?*) ac_cv_sizeof_intp=$ac_lo;;
! #'') { { echo "$as_me:$LINENO: error: cannot compute sizeof (int*), 77
! #See \`config.log' for more details." >&5
! #echo "$as_me: error: cannot compute sizeof (int*), 77
! #See \`config.log' for more details." >&2;}
! #   { (exit 1); exit 1; }; } ;;
! #esac
! #else
! #  if test "$cross_compiling" = yes; then
! #  { { echo "$as_me:$LINENO: error: cannot run test program while cross compiling
! #See \`config.log' for more details." >&5
! #echo "$as_me: error: cannot run test program while cross compiling
! #See \`config.log' for more details." >&2;}
! #   { (exit 1); exit 1; }; }
! #else
! #  cat >conftest.$ac_ext <<_ACEOF
! ##line $LINENO "configure"
! #/* confdefs.h.  */
! #_ACEOF
! #cat confdefs.h >>conftest.$ac_ext
! #cat >>conftest.$ac_ext <<_ACEOF
! #/* end confdefs.h.  */
! #$ac_includes_default
! #long longval () { return (long) (sizeof (int*)); }
! #unsigned long ulongval () { return (long) (sizeof (int*)); }
! ##include <stdio.h>
! ##include <stdlib.h>
! #int
! #main ()
! #{
! #
! #  FILE *f = fopen ("conftest.val", "w");
! #  if (! f)
! #    exit (1);
! #  if (((long) (sizeof (int*))) < 0)
! #    {
! #      long i = longval ();
! #      if (i != ((long) (sizeof (int*))))
! #	exit (1);
! #      fprintf (f, "%ld\n", i);
! #    }
! #  else
! #    {
! #      unsigned long i = ulongval ();
! #      if (i != ((long) (sizeof (int*))))
! #	exit (1);
! #      fprintf (f, "%lu\n", i);
! #    }
! #  exit (ferror (f) || fclose (f) != 0);
! #
! #  ;
! #  return 0;
! #}
! #_ACEOF
! #rm -f conftest$ac_exeext
! #if { (eval echo "$as_me:$LINENO: \"$ac_link\"") >&5
! #  (eval $ac_link) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); } && { ac_try='./conftest$ac_exeext'
! #  { (eval echo "$as_me:$LINENO: \"$ac_try\"") >&5
! #  (eval $ac_try) 2>&5
! #  ac_status=$?
! #  echo "$as_me:$LINENO: \$? = $ac_status" >&5
! #  (exit $ac_status); }; }; then
! #  ac_cv_sizeof_intp=`cat conftest.val`
! #else
! #  echo "$as_me: program exited with status $ac_status" >&5
! #echo "$as_me: failed program was:" >&5
! #sed 's/^/| /' conftest.$ac_ext >&5
! #
! #( exit $ac_status )
! #{ { echo "$as_me:$LINENO: error: cannot compute sizeof (int*), 77
! #See \`config.log' for more details." >&5
! #echo "$as_me: error: cannot compute sizeof (int*), 77
! #See \`config.log' for more details." >&2;}
! #   { (exit 1); exit 1; }; }
! #fi
! #rm -f core core.* *.core gmon.out bb.out conftest$ac_exeext conftest.$ac_objext conftest.$ac_ext
! #fi
! #fi
! #rm -f conftest.val
! #else
! #  ac_cv_sizeof_intp=0
! #fi
! #fi
! ac_cv_sizeof_intp=4
  echo "$as_me:$LINENO: result: $ac_cv_sizeof_intp" >&5
  echo "${ECHO_T}$ac_cv_sizeof_intp" >&6
  cat >>confdefs.h <<_ACEOF
*** HDF4.2r1.orig/hdf/src/hdfi.h	Mon Jan 24 19:36:44 2005
--- HDF4.2r1/hdf/src/hdfi.h	Thu Apr 26 13:39:53 2007
***************
*** 10,16 ****
   *                                                                          *
   ****************************************************************************/
  
! /* $Id: hdfi.h,v 1.156 2005/01/25 03:36:44 epourmal Exp $ */
  
  #ifndef HDFI_H
  #define HDFI_H
--- 10,16 ----
   *                                                                          *
   ****************************************************************************/
  
! /* $Id: hdfi.h 4798 2006-12-06 20:51:13Z epourmal $ */
  
  #ifndef HDFI_H
  #define HDFI_H
***************
*** 62,67 ****
--- 62,68 ----
  #define     DFMT_MIPSEL         0x4441
  #define     DFMT_PC             0x4441
  #define     DFMT_APPLE          0x1111
+ #define     DFMT_APPLE_INTEL    0x4441
  #define     DFMT_MAC            0x1111
  #define     DFMT_SUN386         0x4441
  #define     DFMT_NEXT           0x1111
*************** typedef int               hdf_pint_t;   
*** 607,612 ****
--- 608,626 ----
  
  #endif /* CRAYMPP */
  
+ /* CRAY XT3
+  * Note from RedStorm helpdesk,
+  * When I compile a C code with the '-v' option, it indicates that the compile
+  * is done with the macros __QK_USER__ and __LIBCATAMOUNT__ defined.  In
+  * addition, there are other macros like __x86_64__ defined as well, to
+  * indicate processor type.  __QK_USER__ might be a good check for Catamount,
+  * and __x86_64__ might be good for Opteron node.  You might try something
+  * like the following in a header file:
+  */
+ #if ((defined(__QK_USER__)) && (defined(__x86_64__)))
+ #define __CRAY_XT3__
+ #endif
+ 
  #if defined(VMS) || defined(vms)
  
  #ifdef GOT_MACHINE
*************** Please check your Makefile.
*** 736,742 ****
  #include <sys/types.h>
  #include <sys/file.h>               /* for unbuffered i/o stuff */
  #include <sys/stat.h>
! #define DF_MT   DFMT_APPLE 
  typedef void            VOID;
  typedef void            *VOIDP;
  typedef char            *_fcd;
--- 750,764 ----
  #include <sys/types.h>
  #include <sys/file.h>               /* for unbuffered i/o stuff */
  #include <sys/stat.h>
! #ifdef __i386
! #ifndef INTEL86
! #define INTEL86   /* we need this Intel define or bad things happen later */
! #endif /* INTEL86 */
! #define DF_MT   DFMT_APPLE_INTEL
! #else
! #define DF_MT   DFMT_APPLE
! #endif /* __i386 */
! 
  typedef void            VOID;
  typedef void            *VOIDP;
  typedef char            *_fcd;
*************** void exit(int status);
*** 886,892 ****
  #endif /*MAC*/
  
  /* Metrowerks Mac compiler defines some PC stuff so need to exclude this on the Mac */
! #if !(defined(macintosh) || defined(MAC))
  
  #if defined _M_ALPHA || defined _M_IX86 || defined INTEL86 || defined M_I86 || defined M_I386 || defined DOS386 || defined __i386 || defined UNIX386 || defined i386
  #ifndef INTEL86
--- 908,914 ----
  #endif /*MAC*/
  
  /* Metrowerks Mac compiler defines some PC stuff so need to exclude this on the Mac */
! #if !(defined(macintosh) || defined(MAC) || defined (__APPLE__))
  
  #if defined _M_ALPHA || defined _M_IX86 || defined INTEL86 || defined M_I86 || defined M_I386 || defined DOS386 || defined __i386 || defined UNIX386 || defined i386
  #ifndef INTEL86
EOF
   if [[ $? != 0 ]] ; then
      warn "Unable to patch HDF4. Wrong version?"
      return 1
   fi
}

# Sets up defines so that HDF4 can build on Linux-ppc64.
function apply_hdf4_421_ppc_patch
{
    patch -p0 << \EOF
--- HDF4.2r1/hdf/src/hdfi.h.bak	2004-06-11 21:28:20.763821223 +0200
+++ HDF4.2r1/hdf/src/hdfi.h	2004-06-11 21:43:34.853673152 +0200
@@ -1318,6 +1318,55 @@
 
 #endif /* IA-64 */
 
+#if defined(__powerpc__)
+
+#ifdef GOT_MACHINE
+#error If you get an error on this line more than one machine type has been defined. Please check your Makefile.
+#endif
+#define GOT_MACHINE
+
+#include <sys/file.h>               /* for unbuffered i/o stuff */
+#include <sys/stat.h>
+#define DF_MT           DFMT_MAC
+typedef void            VOID;
+typedef void *          VOIDP;
+typedef char *          _fcd;
+typedef char            char8;
+typedef unsigned char   uchar8;
+typedef char            int8;
+typedef unsigned char   uint8;
+typedef short int       int16;
+typedef unsigned short  uint16;
+typedef int             int32;
+typedef unsigned int    uint32;
+typedef int             intn;
+typedef unsigned int    uintn;
+typedef long            intf;       /* size of INTEGERs in Fortran compiler  */
+typedef float           float32;
+typedef double          float64;
+typedef int             hdf_pint_t; /* an integer the same size as a pointer */
+#define FNAME_POST_UNDERSCORE
+#define _fcdtocp(desc) (desc)
+#ifdef  HAVE_FMPOOL
+#define FILELIB PAGEBUFIO  /* enable page buffering */
+#else
+#define FILELIB UNIXBUFIO
+#endif
+
+/* JPEG #define's - Look in the JPEG docs before changing - (Q) */
+
+/* Determine the memory manager we are going to use. Valid values are: */
+/*  MEM_DOS, MEM_ANSI, MEM_NAME, MEM_NOBS.  See the JPEG docs for details on */
+/*  what each does */
+#define JMEMSYS         MEM_ANSI
+
+#ifdef __GNUC__
+#define HAVE_STDC
+#define INCLUDES_ARE_ANSI
+#endif
+
+#endif /* ppc */
+
 #ifndef GOT_MACHINE
 #error No machine type has been defined.  Your Makefile needs to have someing like -DSUN or -DUNICOS in order for the HDF internal structures to be defined correctly.
 #endif
EOF
}

# Switches a define for the endianness on PPC systems.
function apply_hdf4_421_ppc_patch_endianness
{
  patch -p0 << \EOF
--- HDF4.2r1/hdf/fmpool/config/fmplinux.h.orig	2009-03-17 21:10:59.240084436 -0700
+++ HDF4.2r1/hdf/fmpool/config/fmplinux.h	2009-03-17 21:11:24.868152481 -0700
@@ -36,7 +36,7 @@
 #define HAVE_STAT
 #define HAVE_MIN_MAX
 #define HAVE_CDEFS_H
-#define	BYTE_ORDER  LITTLE_ENDIAN	
+#define	BYTE_ORDER  BIG_ENDIAN	
 
 #endif /* _FMPCONF_H */
EOF
}

function apply_hdf4_421_patch
{
   if [[ "$OPSYS" == "Darwin" ]]; then
       apply_hdf4_421_darwin_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi

   return 0
}

function apply_hdf4_425_patch
{
   patch -p0 << \EOF
diff -c a/configure hdf-4.2.5/configure
*** a/configure
--- hdf-4.2.5/configure
***************
*** 6770,6782 ****

  done

!     { $as_echo "$as_me:${as_lineno-$LINENO}: checking for jpeg_start_decompress in -ljpeg" >&5
! $as_echo_n "checking for jpeg_start_decompress in -ljpeg... " >&6; }
! if test "${ac_cv_lib_jpeg_jpeg_start_decompress+set}" = set; then :
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-ljpeg  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */

--- 6770,6782 ----

  done

!     { $as_echo "$as_me:${as_lineno-$LINENO}: checking for vtk_jpeg_start_decompress in -lvtkjpeg" >&5
! $as_echo_n "checking for vtk_jpeg_start_decompress in -lvtkjpeg... " >&6; }
! if test "${ac_cv_lib_jpeg_vtk_jpeg_start_decompress+set}" = set; then :
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-lvtkjpeg  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */

***************
*** 6786,6792 ****
  #ifdef __cplusplus
  extern "C"
  #endif
! char jpeg_start_decompress ();
  #ifdef F77_DUMMY_MAIN

  #  ifdef __cplusplus
--- 6786,6792 ----
  #ifdef __cplusplus
  extern "C"
  #endif
! char vtk_jpeg_start_decompress ();
  #ifdef F77_DUMMY_MAIN

  #  ifdef __cplusplus
***************
*** 6798,6825 ****
  int
  main ()
  {
! return jpeg_start_decompress ();
    ;
    return 0;
  }
  _ACEOF
  if ac_fn_c_try_link "$LINENO"; then :
!   ac_cv_lib_jpeg_jpeg_start_decompress=yes
  else
!   ac_cv_lib_jpeg_jpeg_start_decompress=no
  fi
  rm -f core conftest.err conftest.$ac_objext \
      conftest$ac_exeext conftest.$ac_ext
  LIBS=$ac_check_lib_save_LIBS
  fi
! { $as_echo "$as_me:${as_lineno-$LINENO}: result: $ac_cv_lib_jpeg_jpeg_start_decompress" >&5
! $as_echo "$ac_cv_lib_jpeg_jpeg_start_decompress" >&6; }
! if test "x$ac_cv_lib_jpeg_jpeg_start_decompress" = x""yes; then :
    cat >>confdefs.h <<_ACEOF
  #define HAVE_LIBJPEG 1
  _ACEOF

!   LIBS="-ljpeg $LIBS"

  else
    unset HAVE_JPEG
--- 6798,6825 ----
  int
  main ()
  {
! return vtk_jpeg_start_decompress ();
    ;
    return 0;
  }
  _ACEOF
  if ac_fn_c_try_link "$LINENO"; then :
!   ac_cv_lib_jpeg_vtk_jpeg_start_decompress=yes
  else
!   ac_cv_lib_jpeg_vtk_jpeg_start_decompress=no
  fi
  rm -f core conftest.err conftest.$ac_objext \
      conftest$ac_exeext conftest.$ac_ext
  LIBS=$ac_check_lib_save_LIBS
  fi
! { $as_echo "$as_me:${as_lineno-$LINENO}: result: $ac_cv_lib_jpeg_vtk_jpeg_start_decompress" >&5
! $as_echo "$ac_cv_lib_jpeg_vtk_jpeg_start_decompress" >&6; }
! if test "x$ac_cv_lib_jpeg_vtk_jpeg_start_decompress" = x""yes; then :
    cat >>confdefs.h <<_ACEOF
  #define HAVE_LIBJPEG 1
  _ACEOF

!   LIBS="-lvtkjpeg $LIBS"

  else
    unset HAVE_JPEG
***************
*** 6878,6890 ****
        LDFLAGS="$LDFLAGS -L$jpeg_lib"
      fi

!     { $as_echo "$as_me:${as_lineno-$LINENO}: checking for jpeg_start_decompress in -ljpeg" >&5
! $as_echo_n "checking for jpeg_start_decompress in -ljpeg... " >&6; }
! if test "${ac_cv_lib_jpeg_jpeg_start_decompress+set}" = set; then :
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-ljpeg  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */

--- 6878,6890 ----
        LDFLAGS="$LDFLAGS -L$jpeg_lib"
      fi

!     { $as_echo "$as_me:${as_lineno-$LINENO}: checking for vtk_jpeg_start_decompress in -lvtkjpeg" >&5
! $as_echo_n "checking for vtk_jpeg_start_decompress in -lvtkjpeg... " >&6; }
! if test "${ac_cv_lib_jpeg_vtk_jpeg_start_decompress+set}" = set; then :
    $as_echo_n "(cached) " >&6
  else
    ac_check_lib_save_LIBS=$LIBS
! LIBS="-lvtkjpeg  $LIBS"
  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
  /* end confdefs.h.  */

***************
*** 6894,6900 ****
  #ifdef __cplusplus
  extern "C"
  #endif
! char jpeg_start_decompress ();
  #ifdef F77_DUMMY_MAIN

  #  ifdef __cplusplus
--- 6894,6900 ----
  #ifdef __cplusplus
  extern "C"
  #endif
! char vtk_jpeg_start_decompress ();
  #ifdef F77_DUMMY_MAIN

  #  ifdef __cplusplus
***************
*** 6906,6933 ****
  int
  main ()
  {
! return jpeg_start_decompress ();
    ;
    return 0;
  }
  _ACEOF
  if ac_fn_c_try_link "$LINENO"; then :
!   ac_cv_lib_jpeg_jpeg_start_decompress=yes
  else
!   ac_cv_lib_jpeg_jpeg_start_decompress=no
  fi
  rm -f core conftest.err conftest.$ac_objext \
      conftest$ac_exeext conftest.$ac_ext
  LIBS=$ac_check_lib_save_LIBS
  fi
! { $as_echo "$as_me:${as_lineno-$LINENO}: result: $ac_cv_lib_jpeg_jpeg_start_decompress" >&5
! $as_echo "$ac_cv_lib_jpeg_jpeg_start_decompress" >&6; }
! if test "x$ac_cv_lib_jpeg_jpeg_start_decompress" = x""yes; then :
    cat >>confdefs.h <<_ACEOF
  #define HAVE_LIBJPEG 1
  _ACEOF

!   LIBS="-ljpeg $LIBS"

  else
    unset HAVE_JPEG
--- 6906,6933 ----
  int
  main ()
  {
! return vtk_jpeg_start_decompress ();
    ;
    return 0;
  }
  _ACEOF
  if ac_fn_c_try_link "$LINENO"; then :
!   ac_cv_lib_jpeg_vtk_jpeg_start_decompress=yes
  else
!   ac_cv_lib_jpeg_vtk_jpeg_start_decompress=no
  fi
  rm -f core conftest.err conftest.$ac_objext \
      conftest$ac_exeext conftest.$ac_ext
  LIBS=$ac_check_lib_save_LIBS
  fi
! { $as_echo "$as_me:${as_lineno-$LINENO}: result: $ac_cv_lib_jpeg_vtk_jpeg_start_decompress" >&5
! $as_echo "$ac_cv_lib_jpeg_vtk_jpeg_start_decompress" >&6; }
! if test "x$ac_cv_lib_jpeg_vtk_jpeg_start_decompress" = x""yes; then :
    cat >>confdefs.h <<_ACEOF
  #define HAVE_LIBJPEG 1
  _ACEOF

!   LIBS="-lvtkjpeg $LIBS"

  else
    unset HAVE_JPEG
EOF
   if [[ $? != 0 ]] ; then
      warn "HDF4 patch failed."
      return 1
   fi

   return 0
}

function apply_hdf4_patch
{
   if [[ ${HDF4_VERSION} == 4.2.1 ]] ; then
       apply_hdf4_421_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi
   if [[ ${HDF4_VERSION} == 4.2.5 ]] ; then
       apply_hdf4_425_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi
   if [[ `uname -m` == "ppc64" ]]; then
       apply_hdf4_421_ppc_patch
       apply_hdf4_421_ppc_patch_endianness
   fi

   return 0
}

function build_hdf4
{
    #
    # Prepare build dir
    #
    prepare_build_dir $HDF4_BUILD_DIR $HDF4_FILE
    untarred_hdf4=$?
    if [[ $untarred_hdf4 == -1 ]] ; then
       warn "Unable to prepare HDF4 Build Directory. Giving Up"
       return 1
    fi
    #
    # Apply patches
    #
    info "Patching HDF . . ." 
    apply_hdf4_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_hdf4 == 1 ]] ; then
          warn "Giving up on HDF4 build because the patch failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script\n" \
               "tried to apply a patch to an existing directory which had\n" \
               "already been patched ... that is, that the patch is\n" \
               "failing harmlessly on a second application."
       fi
    fi

    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS="--disable-fortran"
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    info "Configuring HDF4 . . ."
    cd $HDF4_BUILD_DIR || error "Can't cd to hdf4 build dir."
    info "Invoking command to configure HDF4"
    MAKEOPS=""
    if [[ "$OPSYS" == "Darwin" || "$OPSYS" == "AIX" ]]; then
        export DYLD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
        # In order to ensure $FORTRANARGS is expanded to build the arguments to
        # configure, we wrap the invokation in 'sh -c "..."' syntax
        sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
        CPPFLAGS=\"-I$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/include/ \
        -I$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/include/vtkjpeg\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH\" \
        --with-jpeg=\"$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH\" \
        --with-szlib=\"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH\" \
        --disable-dependency-tracking"
        if [[ $? != 0 ]] ; then
           warn "HDF4 configure failed.  Giving up"\
                "You can see the details of the build failure at $HDF4_BUILD_DIR/config.log\n"
           return 1
        fi
        MAKEOPS="-i"
    else
        export LD_LIBRARY_PATH="$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/lib":"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
        # In order to ensure $FORTRANARGS is expanded to build the arguments to
        # configure, we wrap the invokation in 'sh -c "..."' syntax
        sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" LIBS=\"-lm\" \
        CPPFLAGS=\"-I$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/include/ \
        -I$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/include/vtkjpeg\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH\" \
        --with-jpeg=\"$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH\" \
        --with-szlib=\"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH\""
        if [[ $? != 0 ]] ; then
           warn "HDF4 configure failed.  Giving up.\n"\
                "You can see the details of the build failure at $HDF4_BUILD_DIR/config.log\n"
           return 1
        fi
    fi

    #
    # Build HDF4
    #
    info "Building HDF4 . . . (~2 minutes)"

    $MAKE $MAKEOPS
    if [[ $? != 0 ]] ; then
       warn "HDF4 build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing HDF4 . . ."
    $MAKE $MAKEOPS install
    if [[ $? != 0 ]] ; then
       warn "HDF4 install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for HDF4 . . ."
        # Relink libdf.
        INSTALLNAMEPATH="$VISITDIR/hdf4/${HDF4_VERSION}/$VISITARCH/lib"

        ${C_COMPILER} -dynamiclib -o libdf.${SO_EXT} hdf/src/*.o \
           -Wl,-headerpad_max_install_names \
           -Wl,-install_name,$INSTALLNAMEPATH/libdf.${SO_EXT} \
           -Wl,-compatibility_version,$HDF4_COMPATIBILITY_VERSION \
           -Wl,-current_version,$HDF4_VERSION \
           -L"$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/lib" \
           -L"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib" \
           -lvtkjpeg -lsz -lz
        if [[ $? != 0 ]] ; then
           warn \
"HDF4 dynamic library build failed for libdf.${SO_EXT}.  Giving up"
           return 1
        fi
        cp libdf.${SO_EXT} "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib"

        # Relink libmfhdf.
        ${C_COMPILER} -dynamiclib -o libmfhdf.${SO_EXT} mfhdf/libsrc/*.o \
           -Wl,-headerpad_max_install_names \
           -Wl,-install_name,$INSTALLNAMEPATH/libmfhdf.${SO_EXT} \
           -Wl,-compatibility_version,$HDF4_COMPATIBILITY_VERSION \
           -Wl,-current_version,$HDF4_VERSION \
           -L"$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/lib" \
           -L"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib" \
           -L"$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib" \
           -lvtkjpeg -ldf -lsz -lz
        if [[ $? != 0 ]] ; then
           warn \
"HDF4 dynamic library build failed for libmfhdf.${SO_EXT}.  Giving up"
           return 1
        fi
        cp libmfhdf.${SO_EXT} "$VISITDIR/hdf4/$HDF4_VERSION/$VISITARCH/lib"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/hdf4"
       chgrp -R ${GROUP} "$VISITDIR/hdf4"
    fi
    cd "$START_DIR"
    info "Done with HDF4"
    return 0
}

function bv_hdf4_build
{
cd "$START_DIR"
if [[ "$DO_HDF4" == "yes" ]] ; then
    check_if_installed "hdf4" $HDF4_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping HDF4 build.  HDF4 is already installed."
    else
        info "Building HDF4 (~2 minutes)"
        build_hdf4
        if [[ $? != 0 ]] ; then
            error "Unable to build or install HDF4.  Bailing out."
        fi
        info "Done building HDF4"
    fi
fi
}

