function bv_advio_initialize
{
    export DO_ADVIO="no"
}

function bv_advio_enable
{
    DO_ADVIO="yes"
}

function bv_advio_disable
{
    DO_ADVIO="no"
}

function bv_advio_depends_on
{
    echo ""
}

function bv_advio_info
{
    export ADVIO_FILE=${ADVIO_FILE:-"AdvIO-1.2.tar.gz"}
    export ADVIO_VERSION=${ADVIO_VERSION:-"1.2"}
    export ADVIO_COMPATIBILITY_VERSION=${ADVIO_COMPATIBILITY_VERSION:-"1.2"}
    export ADVIO_BUILD_DIR=${ADVIO_BUILD_DIR:-AdvIO-1.2}
    export ADVIO_SHA256_CHECKSUM="cd89d8a7f1fe94c1bd2d04888028d8b2b98a37853c4a8d5b2b7417b83ea1e803"
}

function bv_advio_print
{
    printf "%s%s\n" "ADVIO_FILE=" "${ADVIO_FILE}"
    printf "%s%s\n" "ADVIO_VERSION=" "${ADVIO_VERSION}"
    printf "%s%s\n" "ADVIO_COMPATIBILITY_VERSION=" "${ADVIO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ADVIO_BUILD_DIR=" "${ADVIO_BUILD_DIR}"
}

function bv_advio_print_usage
{
    printf "%-20s %s [%s]\n" "--advio"   "Build AdvIO" "$DO_ADVIO"
}

function bv_advio_host_profile
{
    if [[ "$DO_ADVIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## AdvIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR \${VISITHOME}/AdvIO/$ADVIO_VERSION/\${VISITARCH})"\
            >> $HOSTCONF
    fi

}

function bv_advio_ensure
{
    if [[ "$DO_ADVIO" == "yes" ]] ; then
        ensure_built_or_ready "AdvIO" $ADVIO_VERSION $ADVIO_BUILD_DIR $ADVIO_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ADVIO="no"
            error "Unable to build AdvIO. ${ADVIO_FILE} not found. You can register and download it from: http://adventure.sys.t.u-tokyo.ac.jp/download/IO.html"
        fi
    fi
}

function apply_advio_12_darwin_patch
{
    patch -p0 << \EOF
--- AdvIO-1.2/configure	2006-02-14 05:19:56.000000000 -0800
+++ AdvIO-1.2/configure.new	2024-02-09 16:28:49.000000000 -0800
@@ -1897,11 +1897,12 @@
 #line 1898 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(short));
+  fprintf(f, "%d\n", (int)sizeof(short));
   exit(0);
 }
 EOF
@@ -1936,11 +1937,12 @@
 #line 1937 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(int));
+  fprintf(f, "%d\n", (int)sizeof(int));
   exit(0);
 }
 EOF
@@ -1975,11 +1977,12 @@
 #line 1976 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(long));
+  fprintf(f, "%d\n", (int)sizeof(long));
   exit(0);
 }
 EOF
@@ -2014,11 +2017,12 @@
 #line 2015 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(long long));
+  fprintf(f, "%d\n", (int)sizeof(long long));
   exit(0);
 }
 EOF
@@ -2053,11 +2057,12 @@
 #line 2054 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(float));
+  fprintf(f, "%d\n", (int)sizeof(float));
   exit(0);
 }
 EOF
@@ -2092,11 +2097,12 @@
 #line 2093 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(double));
+  fprintf(f, "%d\n", (int)sizeof(double));
   exit(0);
 }
 EOF
@@ -2131,11 +2137,12 @@
 #line 2132 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(long double));
+  fprintf(f, "%d\n", (int)sizeof(long double));
   exit(0);
 }
 EOF
@@ -2170,11 +2177,12 @@
 #line 2171 "configure"
 #include "confdefs.h"
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(size_t));
+  fprintf(f, "%d\n", (int)sizeof(size_t));
   exit(0);
 }
 EOF
@@ -2309,15 +2317,13 @@
   cat > conftest.$ac_ext <<EOF
 #line 2311 "configure"
 #include "confdefs.h"
-#ifdef __cplusplus
-extern "C" void exit(int);
-#endif
 #include <stdio.h>
+#include <stdlib.h>
 main()
 {
   FILE *f=fopen("conftestval", "w");
   if (!f) exit(1);
-  fprintf(f, "%d\n", sizeof(bool));
+  fprintf(f, "%d\n", (int)sizeof(bool));
   exit(0);
 }
 EOF
@@ -3969,7 +3975,7 @@
 #============================================================
 
 
-subdirs="${subdirs} Base FileIO IDL DocIO doc"
+subdirs="${subdirs} Base FileIO DocIO doc"
 #if test "${adv_cv_lib_micogtk}" = "yes"; then
 #  subdirs="${subdirs} Frame"
 #  subdirs="${subdirs} Compo"
EOF
    if [[ $? != 0 ]] ; then
        echo "Failed applying Darwin patch"
        return 1
    fi

    return 0 
}

function apply_advio_12_patch
{
    if [[ "$OPSYS" == "Darwin" ]] ; then
        apply_advio_12_darwin_patch
    fi

    return $?
}

function apply_advio_patch
{
    if [[ ${ADVIO_VERSION} == 1.2 ]] ; then
        apply_advio_12_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.18, build_advio                          #
#                                                                             #
# Kevin Griffin, Tue Nov 18 18:25:38 PST 2014                                 #
# Added patch for OS X 10.9 Mavericks.                                        #
#                                                                             #
# Kevin Griffin, Mon Aug 8 17:34:52 PDT 2016                                  #
# Updated patch for OS X 10.1*.                                               #
# *************************************************************************** #

function build_advio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADVIO_BUILD_DIR $ADVIO_FILE
    untarred_ADVIO=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ADVIO == -1 ]] ; then
        warn "Unable to prepare AdvIO Build Directory. Giving up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching AdvIO . . ."
    apply_advio_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_ADVIO == 1 ]] ; then
            warn "Giving up on AdvIO build because the patch failed."
            return 1
        else 
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure AdvIO
    #
    info "Configuring AdvIO . . ."
    cd $ADVIO_BUILD_DIR || error "Can't cd to AdvIO build dir."
    # Remove IDL dependencies from the build process
    sed "s%@idldir@%%g" Makefile.in > m2
    mv m2 Makefile.in
    sed "s%FileIO IDL DocIO%FileIO DocIO%g" configure > c2
    mv c2 configure
    chmod 750 ./configure
    info "Invoking command to configure AdvIO"
    ADVIO_DARWIN=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        ADVIO_DARWIN="--host=darwin"
    fi
    ADVIO_DEBUG=""
    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        ADVIO_DEBUG="--enable-debug"
    fi
    C_OPT_FLAGS="-Wno-error=implicit-function-declaration"
    set -x
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        ./configure --prefix="$VISITDIR/AdvIO/$ADVIO_VERSION/$VISITARCH" --disable-gtktest $ADVIO_DARWIN $ADVIO_DEBUG
    set +x
    if [[ $? != 0 ]] ; then
        warn "AdvIO configure failed.  Giving up"
        return 1
    fi

    #
    # Build AdvIO
    #
    info "Building AdvIO . . . (~1 minute)"

    $MAKE
    if [[ $? != 0 ]] ; then
        warn "AdvIO build failed.  Giving up"
        return 1
    fi

    # Install AdvIO
    info "Installing AdvIO"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "AdvIO install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/AdvIO"
        chgrp -R ${GROUP} "$VISITDIR/AdvIO"
    fi

    cd "$START_DIR"
    info "Done with AdvIO"
    return 0
}

function bv_advio_is_enabled
{
    if [[ $DO_ADVIO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_advio_is_installed
{
    check_if_installed "AdvIO" $ADVIO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_advio_build
{
    cd "$START_DIR"
    if [[ "$DO_ADVIO" == "yes" ]] ; then
        check_if_installed "AdvIO" $ADVIO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping AdvIO build.  AdvIO is already installed."
        else
            info "Building AdvIO (~1 minutes)"
            build_advio
            if [[ $? != 0 ]] ; then
                error "Unable to build or install AdvIO.  Bailing out."
            fi
            info "Done building AdvIO"
        fi
    fi
}
