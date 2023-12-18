function bv_mesagl_initialize
{
    export DO_MESAGL="no"
}

function bv_mesagl_enable
{
    DO_MESAGL="yes"
    bv_glu_enable
}

function bv_mesagl_disable
{
    DO_MESAGL="no"
}

function bv_mesagl_depends_on
{
    echo "llvm"
}

function bv_mesagl_info
{
    export MESAGL_VERSION=${MESAGL_VERSION:-"17.3.9"}
    export MESAGL_FILE=${MESAGL_FILE:-"mesa-$MESAGL_VERSION.tar.xz"}
    export MESAGL_URL=${MESAGL_URL:-"https://archive.mesa3d.org/older-versions/17.x/"}
    export MESAGL_BUILD_DIR=${MESAGL_BUILD_DIR:-"mesa-$MESAGL_VERSION"}
    export MESAGL_MD5_CHECKSUM="b8042f9970ea70a36da1ee1fae27c448"
    export MESAGL_SHA256_CHECKSUM="c5beb5fc05f0e0c294fefe1a393ee118cb67e27a4dca417d77c297f7d4b6e479"
}

function bv_mesagl_print
{
    printf "%s%s\n" "MESAGL_FILE=" "${MESAGL_FILE}"
    printf "%s%s\n" "MESAGL_VERSION=" "${MESAGL_VERSION}"
    printf "%s%s\n" "MESAGL_BUILD_DIR=" "${MESAGL_BUILD_DIR}"
}

function bv_mesagl_print_usage
{
    printf "%-20s %s [%s]\n" "--mesagl" "Build MesaGL" "$DO_MESAGL"
}

function bv_mesagl_host_profile
{
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MesaGL" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MESAGL_DIR \${VISITHOME}/mesagl/$MESAGL_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_mesagl_selected
{
    args=$@
    if [[ $args == "--mesagl" ]]; then
        DO_MESAGL="yes"
        return 1
    fi

    return 0
}

function bv_mesagl_initialize_vars
{
    info "initalizing mesagl vars"
    if [[ "$DO_MESAGL" == "yes" ]]; then
        MESAGL_INSTALL_DIR="${VISITDIR}/mesagl/${MESAGL_VERSION}/${VISITARCH}"
        MESAGL_INCLUDE_DIR="${MESAGL_INSTALL_DIR}/include"
        MESAGL_LIB_DIR="${MESAGL_INSTALL_DIR}/lib"
        if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            MESAGL_OPENGL_LIB="${MESAGL_LIB_DIR}/libGL.a"
            MESAGL_OSMESA_LIB="${MESAGL_LIB_DIR}/libOSMesa.a"
            # initialized here, because glu's initialize_vars is called first
            # and install location won't be set properly for use with VTK
            MESAGL_GLU_LIB="${MESAGL_LIB_DIR}/libGLU.a"
        else
            MESAGL_OPENGL_LIB="${MESAGL_LIB_DIR}/libGL.${SO_EXT}"
            MESAGL_OSMESA_LIB="${MESAGL_LIB_DIR}/libOSMesa.${SO_EXT}"
            # initialized here, because glu's initialize_vars is called first
            # and install location won't be set properly for use with VTK
            MESAGL_GLU_LIB="${MESAGL_LIB_DIR}/libGLU.${SO_EXT}"
        fi
    fi
}

function bv_mesagl_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_MESAGL" == "yes" ]] ; then
            ensure_built_or_ready "mesagl"   $MESAGL_VERSION   $MESAGL_BUILD_DIR   $MESAGL_FILE $MESAGL_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function apply_mesagl_patch
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
        warn "MesaGL patch 1 failed."
        return 1
    fi

    #
    # Patch so that displaying graphics to the XWin-32 2018 X server
    # works properly.
    #
    patch -p0 << \EOF
diff -c src/gallium/winsys/sw/xlib/xlib_sw_winsys.c.orig src/gallium/winsys/sw/xlib/xlib_sw_winsys.c
*** src/gallium/winsys/sw/xlib/xlib_sw_winsys.c.orig	Thu Mar  4 13:12:20 2021
--- src/gallium/winsys/sw/xlib/xlib_sw_winsys.c	Thu Mar  4 13:14:11 2021
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
        warn "MesaGL patch 2 failed."
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
        warn "MesaGL patch 3 failed."
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
        warn "MesaGL patch 4 failed."
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
        warn "MesaGL patch 5 failed."
        return 1
    fi

    #
    # Patch to address VTK texture buffer error.
    #
    patch -p0 << \EOF
diff -c src/gallium/drivers/llvmpipe/lp_screen.c.orig src/gallium/drivers/llvmpipe/lp_screen.c
*** src/gallium/drivers/llvmpipe/lp_screen.c.orig        Fri Dec 15 14:33:53 PST 2023
--- src/gallium/drivers/llvmpipe/lp_screen.c     Fri Dec 15 14:33:53 PST 2023
***************
*** 236,242 ****
     case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
        return 1;
     case PIPE_CAP_MAX_TEXTURE_BUFFER_SIZE:
!       return 65536;
     case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
        return 1;
     case PIPE_CAP_PREFER_BLIT_BASED_TEXTURE_TRANSFER:
--- 236,242 ----
     case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
        return 1;
     case PIPE_CAP_MAX_TEXTURE_BUFFER_SIZE:
!       return 134217728;
     case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
        return 1;
     case PIPE_CAP_PREFER_BLIT_BASED_TEXTURE_TRANSFER:
EOF
    if [[ $? != 0 ]] ; then
        warn "MesaGL patch 6 failed."
        return 1
    fi

    return 0;
}

function build_mesagl
{
    #
    # prepare build dir
    #
    prepare_build_dir $MESAGL_BUILD_DIR $MESAGL_FILE
    untarred_mesagl=$?
    if [[ $untarred_mesagl == -1 ]] ; then
        warn "Unable to prepare MesaGL build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    cd $MESAGL_BUILD_DIR || error "Couldn't cd to mesagl build dir."

    info "Patching MesaGL"
    apply_mesagl_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_mesagl == 1 ]] ; then
            warn "Giving up on MesaGL build because the patch failed."
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
    # Build MESAGL.
    #
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        MESAGL_STATIC_DYNAMIC="--disable-shared --disable-shared-glapi --enable-static --enable-static-glapi"
    fi
    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        MESAGL_DEBUG_BUILD="--enable-debug"
    fi
    if [[ "$(uname -m)" == "x86_64" ]] ; then
        MESAGL_GALLIUM_DRIVERS="swrast,swr"
    else
        MESAGL_GALLIUM_DRIVERS="swrast"
    fi

    info "Configuring MesaGL . . ."

    # add -fcommon if gcc >=10 to work around changes in compiler behavior
    # see: https://wiki.gentoo.org/wiki/Project:Toolchain/Gcc_10_porting_notes/fno_common
    # otherwise we would need to patch mesa to fix build problems

    mesa_c_opt_flags=""
    if [[ "$CXX_COMPILER" == "g++" ]] ; then
        VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-1)
        if [[ ${VERSION} -ge 10 ]] ; then
            mesa_c_opt_flags="-fcommon"
        fi
    fi

    set -x
    env CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        CXX=${CXX_COMPILER} \
        CFLAGS="${CFLAGS} ${C_OPT_FLAGS} ${mesa_c_opt_flags}" \
        CC=${C_COMPILER} \
        ./autogen.sh \
        --prefix=${VISITDIR}/mesagl/${MESAGL_VERSION}/${VISITARCH} \
        --with-platforms=x11 \
        --disable-dri \
        --disable-dri3 \
        --disable-egl \
        --disable-gbm \
        --disable-gles1 \
        --disable-gles2 \
        --disable-xvmc \
        --disable-vdpau \
        --disable-va \
        --enable-glx \
        --enable-llvm \
        --with-gallium-drivers=${MESAGL_GALLIUM_DRIVERS} \
        --enable-gallium-osmesa $MESAGL_STATIC_DYNAMIC $MESAGL_DEBUG_BUILD \
        --disable-llvm-shared-libs \
        --with-llvm-prefix=${VISIT_LLVM_DIR}
    set +x

    if [[ $? != 0 ]] ; then
        warn "MesaGL configure failed.  Giving up"
        return 1
    fi

    info "Building MesaGL . . ."
    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "MesaGL build failed.  Giving up"
        return 1
    fi

    info "Installing MesaGL ..."
    ${MAKE} ${MAKE_OPT_FLAGS} install
    if [[ $? != 0 ]] ; then
        warn "MesaGL install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/mesagl"
        chgrp -R ${GROUP} "$VISITDIR/mesagl"
    fi
    cd "$START_DIR"
    info "Done with MesaGL"
    return 0
}

function bv_mesagl_is_enabled
{
    if [[ $DO_MESAGL == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mesagl_is_installed
{
    check_if_installed "mesagl" $MESAGL_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mesagl_build
{
    #
    # Build MesaGL
    #
    cd "$START_DIR"
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        check_if_installed "mesagl" $MESAGL_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping MesaGL build.  MesaGL is already installed."
        else
            info "Building MesaGL (~20 minutes)"
            build_mesagl
            if [[ $? != 0 ]] ; then
                error "Unable to build or install MesaGL.  Bailing out."
            fi
            info "Done building MesaGL"
        fi
    fi
}
