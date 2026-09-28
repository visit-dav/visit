function bv_adios_initialize
{
    export DO_ADIOS="no"
    export USE_SYSTEM_ADIOS="no"
    add_extra_commandline_args "adios" "alt-adios-dir" 1 "Use alternative directory for adios"

}

function bv_adios_enable
{
    DO_ADIOS="yes"
}

function bv_adios_disable
{
    DO_ADIOS="no"
}

function bv_adios_alt_adios_dir
{
    echo "Using alternate Adios directory"

    # Check to make sure the directory or a particular include file exists.
    #    [ ! -e "$1" ] && error "Adios not found in $1"

    bv_adios_enable
    USE_SYSTEM_ADIOS="yes"
    ADIOS_INSTALL_DIR="$1"
}

function bv_adios_depends_on
{
    if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
        echo ""
    else
        depends_on=""

        if [[ "$DO_MPICH" == "yes" ]] ; then
            depends_on="$depends_on mpich"
        fi

        if [[ "$DO_HDF5" == "yes" ]] ; then
            depends_on="$depends_on hdf5"
        fi

        if [[ "$DO_BLOSC" == "yes" ]] ; then
            depends_on="$depends_on blosc"
        fi

        echo $depends_on
    fi
}

function bv_adios_initialize_vars
{
    if [[ "$parallel" == "no" ]]; then
        bv_adios_disable
        warn "Adios requested by default but the parallel flag has not been set. Adios will not be built."
        return
    fi

    if [[ "$USE_SYSTEM_ADIOS" == "no" ]]; then
        ADIOS_INSTALL_DIR="${VISITDIR}/adios/$ADIOS_VERSION/$VISITARCH"
    fi
}

function bv_adios_info
{
    export ADIOS_VERSION=${ADIOS_VERSION:-"1.13.1"}
    export ADIOS_FILE=${ADIOS_FILE:-"adios-${ADIOS_VERSION}.tar.gz"}
    export ADIOS_COMPATIBILITY_VERSION=${ADIOS_COMPATIBILITY_VERSION:-"${ADIOS_VERSION}"}
    export ADIOS_BUILD_DIR=${ADIOS_BUILD_DIR:-"adios-${ADIOS_VERSION}"}
    export ADIOS_SHA256_CHECKSUM="684096cd7e5a7f6b8859601d4daeb1dfaa416dfc2d9d529158a62df6c5bcd7a0"
}

function bv_adios_print
{
    printf "%s%s\n" "ADIOS_FILE=" "${ADIOS_FILE}"
    printf "%s%s\n" "ADIOS_VERSION=" "${ADIOS_VERSION}"
    printf "%s%s\n" "ADIOS_COMPATIBILITY_VERSION=" "${ADIOS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ADIOS_BUILD_DIR=" "${ADIOS_BUILD_DIR}"
}

function bv_adios_print_usage
{
    printf "%-20s %s [%s]\n" "--adios" "Build ADIOS" "$DO_ADIOS"
    printf "%-20s %s [%s]\n" "--alt-adios-dir" "Use ADIOS from an alternative directory"
}

function bv_adios_host_profile
{
    if [[ "$DO_ADIOS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## ADIOS" >> $HOSTCONF
        if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
            echo "## (configured w/ mpi compiler wrapper)" >> $HOSTCONF
        fi
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
            warn "Assuming version 1.11.0 for Adios"
            echo "SETUP_APP_VERSION(ADIOS 1.11.0)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR $ADIOS_INSTALL_DIR)" >> $HOSTCONF
        else
            echo "SETUP_APP_VERSION(ADIOS $ADIOS_VERSION)" >> $HOSTCONF
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR \${VISITHOME}/adios/\${ADIOS_VERSION}/\${VISITARCH})" \
                >> $HOSTCONF
        fi
    fi
}

function bv_adios_ensure
{
    if [[ "$DO_ADIOS" == "yes" && "$USE_SYSTEM_ADIOS" == "no" ]] ; then
        ensure_built_or_ready "adios" $ADIOS_VERSION $ADIOS_BUILD_DIR $ADIOS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ADIOS="no"
            error "Unable to build ADIOS.  ${ADIOS_FILE} not found."
        fi
    fi
}

# ***************************************************************************
#                         Function 8.22, build_ADIOS
#
# Modifications:
#
# ***************************************************************************


function apply_adios_1131_bool_patch
{
    if [[ "$ADIOS_1131_BOOL_PATCH_APPLIED" == "yes" ]] ; then
        return 0
    fi

    if [[ -f utils/bpls/bpls.h ]] && grep -q "#  include <stdbool.h>" utils/bpls/bpls.h ; then
        ADIOS_1131_BOOL_PATCH_APPLIED="yes"
        return 0
    fi

    info "Patching ADIOS 1.13.1 bool definitions"
    patch -p0 << \EOF
--- examples/staging/stage_write/utils.h	2018-01-22 14:57:53.000000000 -0800
+++ examples/staging/stage_write/utils.h	2026-07-28 13:53:11.985004000 -0700
@@ -15,9 +15,15 @@
 #define print(...) fprintf (stderr, __VA_ARGS__); 
 #define print0(...) if (!rank) fprintf (stderr, __VA_ARGS__); 
 
-#define bool int
-#define false 0
-#define true 1
+#if defined(__cplusplus)
+/* bool, true, and false are built in. */
+#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
+#  define bool int
+#  define false 0
+#  define true 1
+#endif
 
 void ints_to_str (int n, int *values, char *s);
 void int64s_to_str (int n, uint64_t *values, char *s);
@@ -47,4 +53,3 @@
 int createdir_recursive( char* path);
 
 #endif
-
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch1 failed."
        return 1
    fi

    patch -p0 << \EOF
--- examples/staging/stage_write_varyingsize/utils.h	2018-01-22 14:57:53.000000000 -0800
+++ examples/staging/stage_write_varyingsize/utils.h	2026-07-28 13:53:12.570246000 -0700
@@ -15,9 +15,15 @@
 #define print(...) fprintf (stderr, __VA_ARGS__); 
 #define print0(...) if (!rank) fprintf (stderr, __VA_ARGS__); 
 
-#define bool int
-#define false 0
-#define true 1
+#if defined(__cplusplus)
+/* bool, true, and false are built in. */
+#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
+#  define bool int
+#  define false 0
+#  define true 1
+#endif
 
 void ints_to_str (int n, int *values, char *s);
 void int64s_to_str (int n, uint64_t *values, char *s);
@@ -47,4 +53,3 @@
 int createdir_recursive( char* path);
 
 #endif
-
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch2 failed."
        return 1
    fi

    patch -p0 << \EOF
--- utils/bp2h5/bp2h5.c	2018-01-22 14:57:57.000000000 -0800
+++ utils/bp2h5/bp2h5.c	2026-07-28 13:53:10.417327000 -0700
@@ -43,10 +43,18 @@
 #include "dmalloc.h"
 #endif
 
-#ifndef bool
-    typedef int bool;
-#   define false 0
-#   define true  1
+#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#   include <stdbool.h>
+#else
+#   ifndef bool
+typedef int bool;
+#   endif
+#   ifndef false
+#      define false 0
+#   endif
+#   ifndef true
+#      define true  1
+#   endif
 #endif
 
 bool noindex = false;              // do no print array indices with data
@@ -641,4 +649,3 @@
     }
     return status;
 }
-
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch3 failed."
        return 1
    fi


    patch -p0 << \EOF
--- utils/bpdiff/utils.h	2018-01-22 14:57:57.000000000 -0800
+++ utils/bpdiff/utils.h	2026-07-28 13:53:11.482481000 -0700
@@ -16,9 +16,15 @@
 #define print(...) fprintf (stderr, __VA_ARGS__); 
 #define print0(...) if (!rank) fprintf (stderr, __VA_ARGS__); 
 
-#define bool int
-#define false 0
-#define true 1
+#if defined(__cplusplus)
+/* bool, true, and false are built in. */
+#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
+#  define bool int
+#  define false 0
+#  define true 1
+#endif
 
 void ints_to_str (int n, int *values, char *s);
 void int64s_to_str (int n, uint64_t *values, char *s);
@@ -48,4 +54,3 @@
 int createdir_recursive( char* path);
 
 #endif
-
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch4 failed."
        return 1
    fi


    patch -p0 << \EOF
--- utils/bpls/bpls.h	2018-01-22 14:57:57.000000000 -0800
+++ utils/bpls/bpls.h	2026-07-28 13:53:10.953449000 -0700
@@ -17,9 +17,15 @@
 #  define strndup(str,len) strdup(str)
 #endif
 
+#if defined(__cplusplus)
+/* bool, true, and false are built in. */
+#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
 typedef int bool;
-#define false 0
-#define true  1
+#  define false 0
+#  define true  1
+#endif
 
 #define CUT_TO_BYTE(x) (x < 0 ? 0 : (x > 255 ? 255 : x))
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch5 failed."
        return 1
    fi

 
    patch -p0 << \EOF
--- utils/bpsplit/bpappend.c	2018-01-22 14:57:57.000000000 -0800
+++ utils/bpsplit/bpappend.c	2026-07-28 13:53:09.321559000 -0700
@@ -50,11 +50,19 @@
 
 #define DIVIDER "========================================================\n"
 
-#ifndef bool
+#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
+#  ifndef bool
 typedef int bool;
+#  endif
+#  ifndef true
+#    define true 1
+#  endif
+#  ifndef false
+#    define false 0
+#  endif
 #endif
-#define true 1
-#define false 0
 
 /** Prototypes */
 int bpappend(char *filein, char *fileout);
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch6 failed."
        return 1
    fi


    patch -p0 << \EOF
--- utils/bpsplit/bpgettime.c	2018-01-22 14:57:57.000000000 -0800
+++ utils/bpsplit/bpgettime.c	2026-07-28 13:53:09.883540000 -0700
@@ -44,11 +44,19 @@
 
 #define DIVIDER "========================================================\n"
 
-#ifndef bool
+#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
+#  ifndef bool
 typedef int bool;
+#  endif
+#  ifndef true
+#    define true 1
+#  endif
+#  ifndef false
+#    define false 0
+#  endif
 #endif
-#define true 1
-#define false 0
 
 /** Prototypes */
 int bpgettime(char *filein);
@@ -203,4 +211,3 @@
 
     return excode;
 }
-
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch7 failed."
        return 1
    fi


    patch -p0 << \EOF
--- utils/bpsplit/bpsplit.c	2018-01-22 14:57:57.000000000 -0800
+++ utils/bpsplit/bpsplit.c	2026-07-28 13:53:08.768578000 -0700
@@ -50,11 +50,19 @@
 
 #define DIVIDER "========================================================\n"
 
-#ifndef bool
+#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
+#  include <stdbool.h>
+#else
+#  ifndef bool
 typedef int bool;
+#  endif
+#  ifndef true
+#    define true 1
+#  endif
+#  ifndef false
+#    define false 0
+#  endif
 #endif
-#define true 1
-#define false 0
 
 /** Prototypes */
 int bpsplit(char *filein, char *fileout, char *recordfile, int from_in, int to_in, bool skiplast);
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bool patch8 failed."
        return 1
    fi

    ADIOS_1131_BOOL_PATCH_APPLIED="yes"
    return 0
}

function apply_adios_1131_bpls_hidden_attrs_patch
{
    if [[ -f utils/bpls/bpls.c ]] && ! grep -q "bool hidden_attrs" utils/bpls/bpls.c ; then
        return 0
    fi

    info "Patching ADIOS 1.13.1 bpls hidden_attrs option flag"
    patch -p0 << \EOF
--- utils/bpls/bpls.c	2018-01-22 14:57:57.000000000 -0800
+++ utils/bpls/bpls.c	2026-07-28 16:48:00.000000000 -0700
@@ -67,7 +67,7 @@
 bool noindex;              // do no print array indices with data
 bool printByteAsChar;      // print 8 bit integer arrays as string
 bool plot;                 // dump histogram related information
-bool hidden_attrs;         // show hidden attrs in BP file
+int  hidden_attrs;         // show hidden attrs in BP file; getopt_long requires int flags
 bool show_decomp;          // show decomposition of arrays
 
 // other global variables
EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS 1.13.1 bpls hidden_attrs patch failed."
        return 1
    fi

    return 0
}

function apply_adios_patch
{
    if [[ "$ADIOS_VERSION" == "1.13.1" ]] ; then
        apply_adios_1131_bool_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_adios_1131_bpls_hidden_attrs_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_adios
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADIOS_BUILD_DIR $ADIOS_FILE SHA256 $ADIOS_SHA256_CHECKSUM
    untarred_ADIOS=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ADIOS == -1 ]] ; then
        warn "Unable to prepare ADIOS Build Directory. Giving Up"
        return 1
    fi

    cd $ADIOS_BUILD_DIR || error "Can't cd to ADIOS build dir."

    #
    # Apply patches
    #
    apply_adios_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_ADIOS == 1 ]] ; then
            warn "Giving up on ADIOS build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Apply configure
    #
    info "Configuring ADIOS . . ."

    info "Invoking command to configure ADIOS"

    # MPI support
    if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
        WITH_MPI_ARGS="MPICC=\"$VISIT_MPI_COMPILER\" MPICXX=\"$VISIT_MPI_COMPILER_CXX\" LDFLAGS=\"-lpthread $PAR_LINKER_FLAGS\""
        WITH_MPI_INC="$PAR_INCLUDE"
        if [[ "$PAR_COMPILER" != "" ]] ; then
            C_OPT_FLAGS="-I${PAR_HOME}/include"
            MAKE_OPT2_FLAGS="CPPFLAGS=-U_NOMPI"
        fi
    else
        WITH_MPI_ARGS="--without-mpi"
    fi

    # HDF5 support
    if [[ "$DO_HDF5" == "yes" ]] ; then
        export HDF5ROOT="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        WITH_HDF5_ARGS="--with-hdf5=$HDF5ROOT"
    else
        WITH_HDF5_ARGS="--without-hdf5"
        #HDF5_DYLIB=""
    fi

    if [[ "$DO_ZLIB" == "yes" && "$DO_HDF5" == "yes" ]] ; then
        WITH_ZLIB_ARGS="--with-zlib=${VISIT_ZLIB_DIR}"
    else
        WITH_ZLIB_ARGS="--without-zlib"
    fi

    # blosc support
    if [[ "$DO_BLOSC" == "yes" ]]; then
        WITH_BLOSC_ARGS="--with-blosc=$BLOSCDIR"
    else
        WITH_BLOSC_ARGS="--without-blosc"
    fi

    # Fix compilation error on newer Darwin
    if [[ "$OPSYS" == "Darwin" && $(uname -r | cut -d'.' -f1) -ge 23 ]]; then
        sed -i '' 's/^libparse_test_query_xml_a_LIBADD/#libparse_test_query_xml_a_LIBADD/' tests/C/query/common/Makefile.in
    fi

    pyenv=""
    if [[ "$DO_PYTHON" == "yes" ]]; then
        pyenv="env PYTHON=$PYTHON_COMMAND"
    fi

    set -x
    ${pyenv} sh -c "./configure ${OPTIONAL} CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
           CFLAGS=\"$CFLAGS $C_OPT_FLAGS $WITH_MPI_INC\" \
           CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS $WITH_MPI_INC\" \
           $WITH_MPI_ARGS $WITH_HDF5_ARGS $WITH_BLOSC_ARGS $WITH_ZLIB_ARGS \
           --disable-fortran \
           --without-netcdf --without-nc4par --without-phdf5 --without-mxml \
           --prefix=\"$VISITDIR/adios/$ADIOS_VERSION/$VISITARCH\""

    set +x
    if [[ $? != 0 ]] ; then
        warn "ADIOS configure failed.  Giving up"
        return 1
    fi

    #
    # Build ADIOS
    #
    info "Building ADIOS . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "ADIOS build failed. Attempting install anyways..."
    fi

    info "Installing ADIOS . . ."
    $MAKE $MAKE_OPT2_FLAGS install
    if [[ $? != 0 ]] ; then
        warn "ADIOS build (make install) failed.  Giving up"
        return 1
    fi

    cleanup_build_dirs $ADIOS_BUILD_DIR

    change_install_dir_perms "$VISITDIR/adios"

    cd "$START_DIR"
    info "Done with ADIOS"
    return 0
}

function bv_adios_is_enabled
{
    if [[ $DO_ADIOS == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_adios_is_installed
{
    if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
        return 1
    fi

    check_if_installed "adios" $ADIOS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_adios_build
{
    cd "$START_DIR"

    if [[ "$DO_ADIOS" == "yes" && "$USE_SYSTEM_ADIOS" == "no" ]] ; then
        check_if_installed "adios" $ADIOS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping ADIOS build.  ADIOS is already installed."
        else
            info "Building ADIOS (~1 minutes)"
            build_adios
            if [[ $? != 0 ]] ; then
                error "Unable to build or install ADIOS.  Bailing out."
            fi
            info "Done building ADIOS"
        fi
    fi
}
