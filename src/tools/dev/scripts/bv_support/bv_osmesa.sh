function bv_osmesa_initialize
{
    export DO_OSMESA="no"
}

function bv_osmesa_enable
{
    DO_OSMESA="yes"
}

function bv_osmesa_disable
{
    DO_OSMESA="no"
}

function bv_osmesa_depends_on
{
    echo "llvm"
}

function bv_osmesa_info
{
    export OSMESA_VERSION=${OSMESA_VERSION:-"17.3.9"}
    export OSMESA_FILE=${OSMESA_FILE:-"mesa-$OSMESA_VERSION.tar.xz"}
    export OSMESA_URL=${OSMESA_URL:-"https://archive.mesa3d.org/older-versions/17.x/"}
    export OSMESA_BUILD_DIR=${OSMESA_BUILD_DIR:-"mesa-$OSMESA_VERSION"}
    export OSMESA_MD5_CHECKSUM="b8042f9970ea70a36da1ee1fae27c448"
    export OSMESA_SHA256_CHECKSUM="c5beb5fc05f0e0c294fefe1a393ee118cb67e27a4dca417d77c297f7d4b6e479"
}

function bv_osmesa_print
{
    printf "%s%s\n" "OSMESA_FILE=" "${OSMESA_FILE}"
    printf "%s%s\n" "OSMESA_VERSION=" "${OSMESA_VERSION}"
    printf "%s%s\n" "OSMESA_BUILD_DIR=" "${OSMESA_BUILD_DIR}"
}

function bv_osmesa_print_usage
{
    printf "%-20s %s [%s]\n" "--osmesa" "Build OSMesa" "$DO_OSMESA"
}

function bv_osmesa_host_profile
{
    # If we are using osmesa as the GL for VTK in a static build, we'll tell
    # VisIt about osmesa using a different mechanism.
    addhp="yes"
    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
            addhp="no"
        fi
    fi

    if [[ "$DO_OSMESA" == "yes" && "$addhp" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OSMesa" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OSMESA_DIR \${VISITHOME}/osmesa/$OSMESA_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_osmesa_selected
{
    args=$@
    if [[ $args == "--osmesa" ]]; then
        DO_OSMESA="yes"
        return 1
    fi

    return 0
}

function bv_osmesa_initialize_vars
{
    info "initalizing osmesa vars"
    if [[ "$DO_OSMESA" == "yes" ]]; then
        OSMESA_INSTALL_DIR="${VISITDIR}/osmesa/${OSMESA_VERSION}/${VISITARCH}"
        OSMESA_INCLUDE_DIR="${OSMESA_INSTALL_DIR}/include"
        OSMESA_LIB_DIR="${OSMESA_INSTALL_DIR}/lib"
        if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            OSMESA_LIB="${OSMESA_LIB_DIR}/libOSMesa.a"
        else
            OSMESA_LIB="${OSMESA_LIB_DIR}/libOSMesa.${SO_EXT}"
        fi
    fi
}

function bv_osmesa_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_OSMESA" == "yes" ]] ; then
            ensure_built_or_ready "osmesa"   $OSMESA_VERSION   $OSMESA_BUILD_DIR   $OSMESA_FILE $OSMESA_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function apply_osmesa_patch
{
    patch -p0 << \EOF
diff -c configure.ac.orig configure.ac
*** configure.ac.orig   Mon Jul 13 09:47:20 2020
--- configure.ac        Mon Jul 13 09:50:37 2020
***************
*** 2653,2659 ****
      dnl ourselves.
      dnl (See https://llvm.org/bugs/show_bug.cgi?id=6823)
      dnl We can't use $LLVM_VERSION because it has 'svn' stripped out,
!     LLVM_SO_NAME=LLVM-`$LLVM_CONFIG --version`
      AS_IF([test -f "$LLVM_LIBDIR/lib$LLVM_SO_NAME.$IMP_LIB_EXT"], [llvm_have_one_so=yes])

      if test "x$llvm_have_one_so" = xyes; then
--- 2653,2659 ----
      dnl ourselves.
      dnl (See https://llvm.org/bugs/show_bug.cgi?id=6823)
      dnl We can't use $LLVM_VERSION because it has 'svn' stripped out,
!     LLVM_SO_NAME=LLVM-$LLVM_VERSION
      AS_IF([test -f "$LLVM_LIBDIR/lib$LLVM_SO_NAME.$IMP_LIB_EXT"], [llvm_have_one_so=yes])

      if test "x$llvm_have_one_so" = xyes; then
EOF

    if [[ $? != 0 ]] ; then
        warn "OSMesa patch 1 failed."
        return 1
    fi

    #
    # Patch so that displaying graphics to the XWin-32 2018 X server
    # works properly.
    #
    patch -p0 << \EOF
diff -c src/gallium/winsys/sw/xlib/xlib_sw_winsys.c.orig src/gallium/winsys/sw/xlib/xlib_sw_winsys.c
*** src/gallium/winsys/sw/xlib/xlib_sw_winsys.c.orig    Thu Mar  4 13:12:20 2021
--- src/gallium/winsys/sw/xlib/xlib_sw_winsys.c Thu Mar  4 13:14:11 2021
***************
*** 396,401 ****
--- 396,402 ----
  {
     struct xlib_displaytarget *xlib_dt;
     unsigned nblocksy, size;
+    int ignore;
  
     xlib_dt = CALLOC_STRUCT(xlib_displaytarget);
     if (!xlib_dt)
***************
*** 410,416 ****
     xlib_dt->stride = align(util_format_get_stride(format, width), alignment);
     size = xlib_dt->stride * nblocksy;
  
!    if (!debug_get_option_xlib_no_shm()) {
        xlib_dt->data = alloc_shm(xlib_dt, size);
        if (xlib_dt->data) {
           xlib_dt->shm = True;
--- 411,418 ----
     xlib_dt->stride = align(util_format_get_stride(format, width), alignment);
     size = xlib_dt->stride * nblocksy;
  
!    if (!debug_get_option_xlib_no_shm() &&
!        XQueryExtension(xlib_dt->display, "MIT-SHM", &ignore, &ignore, &ignore)) {
        xlib_dt->data = alloc_shm(xlib_dt, size);
        if (xlib_dt->data) {
           xlib_dt->shm = True;
EOF
    if [[ $? != 0 ]] ; then
        warn "OSMesa patch 2 failed."
        return 1
    fi

    #
    # Patch so that building with gcc-10 will work.
    #
    patch -p0 << \EOF
diff -u src/gallium/drivers/swr/rasterizer/common/os.h.orig src/gallium/drivers/swr/rasterizer/common/os.h
--- src/gallium/drivers/swr/rasterizer/common/os.h.orig 2021-06-28 08:51:12.252643000 -0700
+++ src/gallium/drivers/swr/rasterizer/common/os.h      2021-06-28 08:55:32.676722000 -0700
@@ -166,14 +166,15 @@
 #endif
 
 #if !defined( __clang__) && !defined(__INTEL_COMPILER)
-// Intrinsic not defined in gcc
+// Intrinsic not defined in gcc < 10
+#if (__GNUC__) && (GCC_VERSION < 100000)
 static INLINE
 void _mm256_storeu2_m128i(__m128i *hi, __m128i *lo, __m256i a)
 {
     _mm_storeu_si128((__m128i*)lo, _mm256_castsi256_si128(a));
     _mm_storeu_si128((__m128i*)hi, _mm256_extractf128_si256(a, 0x1));
 }
-
+#endif
 // gcc prior to 4.9 doesn't have _mm*_undefined_*
 #if (__GNUC__) && (GCC_VERSION < 409000)
 #define _mm_undefined_si128 _mm_setzero_si128
EOF
    if [[ $? != 0 ]] ; then
        warn "OSMesa patch 3 failed."
        return 1
    fi

    #
    # Patch to increase the maximum image size in the llvmpipe
    # driver to 16K x 16K.
    #
    patch -p0 << \EOF
diff -c src/gallium/drivers/llvmpipe/lp_limits.h.orig src/gallium/drivers/llvmpipe/lp_limits.h
*** src/gallium/drivers/llvmpipe/lp_limits.h.orig       Fri Jul 30 10:03:06 2021
--- src/gallium/drivers/llvmpipe/lp_limits.h    Fri Jul 30 10:04:41 2021
***************
*** 44,50 ****
   * Max texture sizes
   */
  #define LP_MAX_TEXTURE_SIZE (1 * 1024 * 1024 * 1024ULL)  /* 1GB for now */
! #define LP_MAX_TEXTURE_2D_LEVELS 14  /* 8K x 8K for now */
  #define LP_MAX_TEXTURE_3D_LEVELS 12  /* 2K x 2K x 2K for now */
  #define LP_MAX_TEXTURE_CUBE_LEVELS 14  /* 8K x 8K for now */
  #define LP_MAX_TEXTURE_ARRAY_LAYERS 512 /* 8K x 512 / 8K x 8K x 512 */
--- 44,50 ----
   * Max texture sizes
   */
  #define LP_MAX_TEXTURE_SIZE (1 * 1024 * 1024 * 1024ULL)  /* 1GB for now */
! #define LP_MAX_TEXTURE_2D_LEVELS 15  /* 16K x 16K for now */
  #define LP_MAX_TEXTURE_3D_LEVELS 12  /* 2K x 2K x 2K for now */
  #define LP_MAX_TEXTURE_CUBE_LEVELS 14  /* 8K x 8K for now */
  #define LP_MAX_TEXTURE_ARRAY_LAYERS 512 /* 8K x 512 / 8K x 8K x 512 */
EOF
    if [[ $? != 0 ]] ; then
        warn "OSMesa patch 4 failed."
        return 1
    fi

    #
    # Patch to increase the maximum scene temporary storage in the llvmpipe
    # driver. This is required for large image sizes.
    #
    patch -p0 << \EOF
diff -c src/gallium/drivers/llvmpipe/lp_scene.h.orig src/gallium/drivers/llvmpipe/lp_scene.h
*** src/gallium/drivers/llvmpipe/lp_scene.h.orig        Fri Jul 30 12:11:39 2021
--- src/gallium/drivers/llvmpipe/lp_scene.h     Fri Jul 30 12:11:49 2021
***************
*** 60,66 ****
  
  /* Scene temporary storage is clamped to this size:
   */
! #define LP_SCENE_MAX_SIZE (9*1024*1024)
  
  /* The maximum amount of texture storage referenced by a scene is
   * clamped to this size:
--- 60,66 ----
  
  /* Scene temporary storage is clamped to this size:
   */
! #define LP_SCENE_MAX_SIZE (64*1024*1024)
  
  /* The maximum amount of texture storage referenced by a scene is
   * clamped to this size:
EOF
    if [[ $? != 0 ]] ; then
        warn "OSMesa patch 5 failed."
        return 1
    fi

    return 0;
}

function build_osmesa
{
    #
    # prepare build dir
    #
    prepare_build_dir $OSMESA_BUILD_DIR $OSMESA_FILE
    untarred_osmesa=$?
    if [[ $untarred_osmesa == -1 ]] ; then
        warn "Unable to prepare Mesa build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    cd $OSMESA_BUILD_DIR || error "Couldn't cd to osmesa build dir."

    info "Patching OSMesa"
    apply_osmesa_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_osmesa == 1 ]] ; then
            warn "Giving up on OSMesa build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Handle case where python doesn't exist.
    # The magic to determine if python exist comes from
    # https://stackoverflow.com/questions/592620/how-can-i-check-if-a-program-exists-from-a-bash-script
    #
    if ! command -v python > /dev/null 2>&1 ; then
        sed -i "s/python2.7/python3 python2.7/" configure.ac
    fi

    #
    # Build OSMESA.
    #
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        OSMESA_STATIC_DYNAMIC="--disable-shared --disable-shared-glapi --enable-static --enable-static-glapi"
    fi
    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        OSMESA_DEBUG_BUILD="--enable-debug"
    fi
    if [[ "$(uname -m)" == "x86_64" ]] ; then
        OSMESA_GALLIUM_DRIVERS="swrast,swr"
    else
        OSMESA_GALLIUM_DRIVERS="swrast"
    fi

    info "Configuring OSMesa . . ."
    # add -fcommon if gcc >=10 to work around changes in compiler behavior
    # see: https://wiki.gentoo.org/wiki/Project:Toolchain/Gcc_10_porting_notes/fno_common
    # otherwise we would need to patch mesa to fix build problems

    osmesa_c_opt_flags=""
    if [[ "$CXX_COMPILER" == "g++" ]] ; then
        VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-1)
        if [[ ${VERSION} -ge 10 ]] ; then
            osmesa_c_opt_flags="-fcommon"
        fi
    fi

    set -x
    env CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        CXX=${CXX_COMPILER} \
        CFLAGS="${CFLAGS} ${C_OPT_FLAGS} ${osmesa_c_opt_flags}" \
        CC=${C_COMPILER} \
        ./autogen.sh \
        --prefix=${VISITDIR}/osmesa/${OSMESA_VERSION}/${VISITARCH} \
        --disable-gles1 \
        --disable-gles2 \
        --disable-dri \
        --disable-dri3 \
        --disable-glx \
        --disable-glx-tls \
        --disable-egl \
        --disable-gbm \
        --disable-xvmc \
        --disable-vdpau \
        --disable-va \
        --with-platforms= \
        --enable-llvm \
        --with-gallium-drivers=${OSMESA_GALLIUM_DRIVERS} \
        --enable-gallium-osmesa $OSMESA_STATIC_DYNAMIC $OSMESA_DEBUG_BUILD \
        --disable-llvm-shared-libs \
        --with-llvm-prefix=${VISIT_LLVM_DIR}
    set +x

    if [[ $? != 0 ]] ; then
        warn "OSMesa configure failed.  Giving up"
        return 1
    fi

    info "Building OSMesa . . ."
    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "OSMesa build failed.  Giving up"
        return 1
    fi

    info "Installing OSMesa ..."
    ${MAKE} ${MAKE_OPT_FLAGS} install
    if [[ $? != 0 ]] ; then
        warn "OSMesa install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/osmesa"
        chgrp -R ${GROUP} "$VISITDIR/osmesa"
    fi
    cd "$START_DIR"
    info "Done with OSMesa"
    return 0
}

function bv_osmesa_is_enabled
{
    if [[ $DO_OSMESA == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_osmesa_is_installed
{
    check_if_installed "osmesa" $OSMESA_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_osmesa_build
{
    #
    # Build OSMesa
    #
    cd "$START_DIR"
    if [[ "$DO_OSMESA" == "yes" ]] ; then
        check_if_installed "osmesa" $OSMESA_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OSMesa build.  OSMesa is already installed."
            return 0
        fi
        check_if_installed "mesagl" $MESAGL_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OSMesa build.  MesaGL is already installed."
        else
            info "Building OSMesa (~20 minutes)"
            build_osmesa
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OSMesa.  Bailing out."
            fi
            info "Done building OSMesa"
        fi
    fi
}
