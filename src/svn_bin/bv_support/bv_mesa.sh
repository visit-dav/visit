function bv_mesa_initialize
{
export DO_MESA="no"
export ON_MESA="off"
}

function bv_mesa_enable
{
DO_MESA="yes"
ON_MESA="on"
}

function bv_mesa_disable
{
DO_MESA="no"
ON_MESA="off"
}

function bv_mesa_depends_on
{
echo ""
}

function bv_mesa_info
{
export MESA_FILE=${MESA_FILE:-"MesaLib-7.8.2.tar.gz"}
export MESA_VERSION=${MESA_VERSION:-"7.8.2"}
export MESA_BUILD_DIR=${MESA_BUILD_DIR:-"Mesa-7.8.2"}
export MESA_URL="ftp://ftp.freedesktop.org/pub/mesa/7.8.2/"
export MESA_MD5_CHECKSUM="c89b63d253605ed40e8ac370d25a833c"
export MESA_SHA256_CHECKSUM=""
}

function bv_mesa_print
{
printf "%s%s\n" "MESA_FILE=" "${MESA_FILE}"
printf "%s%s\n" "MESA_VERSION=" "${MESA_VERSION}"
printf "%s%s\n" "MESA_TARGET=" "${MESA_TARGET}"
printf "%s%s\n" "MESA_BUILD_DIR=" "${MESA_BUILD_DIR}"
}

function bv_mesa_print_usage
{
printf "\t\t%15s\n" "NOTE: not available for download from web"
printf "%-15s %s [%s]\n" "--mesa" "Build Mesa" "$DO_MESA"
}

function bv_mesa_graphical
{
local graphical_out="Mesa     $MESA_VERSION($MESA_FILE)      $ON_MESA"
echo $graphical_out
}

function bv_mesa_host_profile
{
    if [[ "$DO_MESA" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Mesa" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MESA_DIR \${VISITHOME}/mesa/$MESA_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_mesa_selected
{
    args=$@
    if [[ $args == "--mesa" ]]; then
        DO_MESA="yes"
        ON_MESA="on"
        return 1
    fi

    return 0
}

function bv_mesa_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_MESA" == "yes" ]] ; then
            ensure_built_or_ready "mesa"   $MESA_VERSION   $MESA_BUILD_DIR   $MESA_FILE $MESA_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function bv_mesa_dry_run
{
    if [[ "$DO_MESA" == "yes" ]] ; then
        echo "Dry run option not set for mesa."
    fi
}

function apply_mesa_75_patch_1
{
   patch -f -p0 <<\EOF
diff -c a/src/mesa/main/config.h Mesa-7.5/src/mesa/main/config.h
*** a/src/mesa/main/config.h
--- Mesa-7.5/src/mesa/main/config.h
***************
*** 138,146 ****
  /** 
   * Maximum viewport/image width. Must accomodate all texture sizes too. 
   */
! #define MAX_WIDTH 4096
  /** Maximum viewport/image height */
! #define MAX_HEIGHT 4096
  
  /** Maxmimum size for CVA.  May be overridden by the drivers.  */
  #define MAX_ARRAY_LOCK_SIZE 3000
--- 138,146 ----
  /** 
   * Maximum viewport/image width. Must accomodate all texture sizes too. 
   */
! #define MAX_WIDTH 16384
  /** Maximum viewport/image height */
! #define MAX_HEIGHT 16384
  
  /** Maxmimum size for CVA.  May be overridden by the drivers.  */
  #define MAX_ARRAY_LOCK_SIZE 3000
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 1 to Mesa 7.5."
        return 1
   else
        return 0
   fi
}

function apply_mesa_75_patch_2
{
    patch -f -p0 <<\EOF
diff -c a/configure.ac Mesa-7.5/configure.ac
*** a/configure.ac
--- Mesa-7.5/configure.ac
***************
*** 224,229 ****
--- 224,231 ----
          LIB_EXTENSION='dylib' ;;
      cygwin* )
          LIB_EXTENSION='dll' ;;
+     aix* )
+         LIB_EXTENSION='a' ;;
      * )
          LIB_EXTENSION='so' ;;
      esac
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 2 to Mesa 7.5."
        return 1
    else
        return 0
   fi
}

function apply_mesa_75_patch_3
{
    patch -f -p0 <<\EOF
diff -c a/progs/Makefile Mesa-7.5/progs/Makefile
*** a/progs/Makefile
--- Mesa-7.5/progs/Makefile
***************
*** 4,10 ****
  
  include $(TOP)/configs/current
  
! SUBDIRS = $(PROGRAM_DIRS)
  
  
  default: message subdirs
--- 4,10 ----
  
  include $(TOP)/configs/current
  
! SUBDIRS ="$(PROGRAM_DIRS)"
  
  
  default: message subdirs
***************
*** 15,32 ****
  
  
  subdirs:
! 	@for dir in $(SUBDIRS) ; do \
! 		if [ -d $$dir ] ; then \
! 			(cd $$dir && $(MAKE)) || exit 1 ; \
! 		fi \
! 	done
  
  # Dummy install target
  install:
  
  clean:
! 	-@for dir in $(SUBDIRS) tests ; do \
! 		if [ -d $$dir ] ; then \
! 			(cd $$dir && $(MAKE) clean) ; \
! 		fi \
! 	done
--- 15,36 ----
  
  
  subdirs:
! 	@if test -n "$(SUBDIRS)" ; then \
! 		for dir in $(SUBDIRS) ; do \
! 			if [ -d $$dir ] ; then \
! 				(cd $$dir && $(MAKE)) || exit 1 ; \
! 			fi \
! 		done \
! 	fi
  
  # Dummy install target
  install:
  
  clean:
! 	-@if test -n "$(SUBDIRS)" ; then \
! 		for dir in $(SUBDIRS) tests ; do \
! 			if [ -d $$dir ] ; then \
! 				(cd $$dir && $(MAKE) clean) ; \
! 			fi \
! 		done \
! 	fi
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 3 to Mesa 7.5."
        return 1
    else
        return 0
   fi
}


function apply_mesa_75_patch_4
{
    patch -f -p0 <<\EOF
diff -c a/src/mesa/main/compiler.h Mesa-7.5/src/mesa/main/compiler.h
*** a/src/mesa/main/compiler.h
--- Mesa-7.5/src/mesa/main/compiler.h
***************
*** 233,239 ****
  #elif defined(__APPLE__)
  #include <CoreFoundation/CFByteOrder.h>
  #define CPU_TO_LE32( x )	CFSwapInt32HostToLittle( x )
! #else /*__linux__ __APPLE__*/
  #include <sys/endian.h>
  #define CPU_TO_LE32( x )	bswap32( x )
  #endif /*__linux__*/
--- 233,244 ----
  #elif defined(__APPLE__)
  #include <CoreFoundation/CFByteOrder.h>
  #define CPU_TO_LE32( x )	CFSwapInt32HostToLittle( x )
! #elif defined(_AIX)
! #define CPU_TO_LE32( x )	x = ((x & 0x000000ff) << 24) | \
! 				    ((x & 0x0000ff00) <<  8) | \
! 				    ((x & 0x00ff0000) >>  8) | \
! 				    ((x & 0xff000000) >> 24);
! #else /*__linux__*/
  #include <sys/endian.h>
  #define CPU_TO_LE32( x )	bswap32( x )
  #endif /*__linux__*/
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 4 to Mesa 7.5."
        return 1
    else
        return 0
   fi
}


function apply_mesa_782_patch_1
{
   patch -f -p0 <<\EOF
diff -c a/src/mesa/drivers/osmesa/osmesa.c Mesa-7.8.2/src/mesa/drivers/osmesa/osmesa.c
*** a/src/mesa/drivers/osmesa/osmesa.c
--- Mesa-7.8.2/src/mesa/drivers/osmesa/osmesa.c
***************
*** 1328,1336 ****
      * that converts rendering from CHAN_BITS to the user-requested channel
      * size.
      */
!    osmesa->rb = new_osmesa_renderbuffer(&osmesa->mesa, osmesa->format, type);
!    _mesa_add_renderbuffer(osmesa->gl_buffer, BUFFER_FRONT_LEFT, osmesa->rb);
!    assert(osmesa->rb->RefCount == 2);
  
     /* Set renderbuffer fields.  Set width/height = 0 to force 
      * osmesa_renderbuffer_storage() being called by _mesa_resize_framebuffer()
--- 1328,1339 ----
      * that converts rendering from CHAN_BITS to the user-requested channel
      * size.
      */
!    if (!osmesa->rb) {
!       osmesa->rb = new_osmesa_renderbuffer(&osmesa->mesa, osmesa->format, type);
!       _mesa_remove_renderbuffer(osmesa->gl_buffer, BUFFER_FRONT_LEFT);
!       _mesa_add_renderbuffer(osmesa->gl_buffer, BUFFER_FRONT_LEFT, osmesa->rb);
!       assert(osmesa->rb->RefCount == 2);
!    }
  
     /* Set renderbuffer fields.  Set width/height = 0 to force 
      * osmesa_renderbuffer_storage() being called by _mesa_resize_framebuffer()
EOF

   if [[ $? != 0 ]] ; then
      warn "Unable to apply patch 1 to Mesa 7.8.2."
      return 1
   else
      return 0
   fi
}


function apply_mesa_782_patch_2
{
   patch -f -p1 <<\EOF
From cc32ff741c5d32a66531a586b1f9268b94846c58 Mon Sep 17 00:00:00 2001
From: Tom Fogal <tfogal@alumni.unh.edu>
Date: Sun, 26 Sep 2010 18:57:59 -0600
Subject: [PATCH] Implement x86_64 atomics for compilers w/o intrinsics.

Really old gcc's (3.3, at least) don't have support for the
intrinsics we need.  This implements a fallback for that case.
---
 src/gallium/auxiliary/util/u_atomic.h |   47 +++++++++++++++++++++++++++++++++
 1 files changed, 47 insertions(+), 0 deletions(-)

diff --git a/src/gallium/auxiliary/util/u_atomic.h b/Mesa-7.8.2/src/gallium/auxiliary/util/u_atomic.h
index a156823..8434491 100644
--- a/src/gallium/auxiliary/util/u_atomic.h
+++ b/Mesa-7.8.2/src/gallium/auxiliary/util/u_atomic.h
@@ -29,6 +29,8 @@
 #define PIPE_ATOMIC_ASM_MSVC_X86                
 #elif (defined(PIPE_CC_GCC) && defined(PIPE_ARCH_X86))
 #define PIPE_ATOMIC_ASM_GCC_X86
+#elif (defined(PIPE_CC_GCC) && defined(PIPE_ARCH_X86_64))
+#define PIPE_ATOMIC_ASM_GCC_X86_64
 #elif defined(PIPE_CC_GCC) && (PIPE_CC_GCC_VERSION >= 401)
 #define PIPE_ATOMIC_GCC_INTRINSIC
 #else
@@ -36,6 +38,51 @@
 #endif
 
 
+#if defined(PIPE_ATOMIC_ASM_GCC_X86_64)
+#define PIPE_ATOMIC "GCC x86_64 assembly"
+
+#ifdef __cplusplus
+extern "C" {
+#endif
+
+#define p_atomic_set(_v, _i) (*(_v) = (_i))
+#define p_atomic_read(_v) (*(_v))
+
+static INLINE boolean
+p_atomic_dec_zero(int32_t *v)
+{
+   unsigned char c;
+
+   __asm__ __volatile__("lock; decl %0; sete %1":"+m"(*v), "=qm"(c)
+			::"memory");
+
+   return c != 0;
+}
+
+static INLINE void
+p_atomic_inc(int32_t *v)
+{
+   __asm__ __volatile__("lock; incl %0":"+m"(*v));
+}
+
+static INLINE void
+p_atomic_dec(int32_t *v)
+{
+   __asm__ __volatile__("lock; decl %0":"+m"(*v));
+}
+
+static INLINE int32_t
+p_atomic_cmpxchg(int32_t *v, int32_t old, int32_t _new)
+{
+   return __sync_val_compare_and_swap(v, old, _new);
+}
+
+#ifdef __cplusplus
+}
+#endif
+
+#endif /* PIPE_ATOMIC_ASM_GCC_X86_64 */
+
 
 #if defined(PIPE_ATOMIC_ASM_GCC_X86)
 
-- 
1.7.0.2
EOF

   if [[ $? != 0 ]] ; then
      warn "Unable to apply patch 2 to Mesa 7.8.2."
      return 1
   else
      return 0
   fi
}


function apply_mesa_7102_patch_1
{
   patch -f -p0 <<\EOF
diff -c a/src/mesa/main/APIspec.py Mesa-7.10.2/src/mesa/main/APIspec.py
*** a/src/mesa/main/APIspec.py
--- Mesa-7.10.2/src/mesa/main/APIspec.py
***************
*** 262,268 ****
          args = []
          if declaration:
              for param in self.params:
!                 sep = "" if param.type.endswith("*") else " "
                  args.append("%s%s%s" % (param.type, sep, param.name))
              if not args:
                  args.append("void")
--- 262,272 ----
          args = []
          if declaration:
              for param in self.params:
!                 #sep = "" if param.type.endswith("*") else " "
!                 if param.type.endswith("*"):
!                     sep = ""
!                 else:
!                     sep = " "
                  args.append("%s%s%s" % (param.type, sep, param.name))
              if not args:
                  args.append("void")
EOF

   if [[ $? != 0 ]] ; then
      warn "Unable to apply patch 1 to Mesa 7.10.2."
      return 1
   else
      return 0
   fi
}


function apply_mesa_7102_patch_2
{
   patch -f -p0 <<\EOF
diff -c a/src/mesa/main/APIspecutil.py Mesa-7.10.2/src/mesa/main/APIspecutil.py
*** a/src/mesa/main/APIspecutil.py
--- Mesa-7.10.2/src/mesa/main/APIspecutil.py
***************
*** 129,135 ****
              if dep_desc.checker.switches:
                  print >>sys.stderr, "%s: deep nested dependence" % func.name

!             convert = None if dep_desc.convert else "noconvert"
              for val in desc.values:
                  valid_values.append((val, dep_desc.size_str, dep_desc.name,
                                       dep_desc.values, dep_desc.error, convert))
--- 129,139 ----
              if dep_desc.checker.switches:
                  print >>sys.stderr, "%s: deep nested dependence" % func.name

!             #convert = None if dep_desc.convert else "noconvert"
!             if dep_desc.convert:
!                 convert = None
!             else:
!                 convert = "noconvert"
              for val in desc.values:
                  valid_values.append((val, dep_desc.size_str, dep_desc.name,
                                       dep_desc.values, dep_desc.error, convert))
***************
*** 241,247 ****
      """Return a C-style parameter declaration string."""
      string = []
      for p in params:
!         sep = "" if p[1].endswith("*") else " "
          string.append("%s%s%s" % (p[1], sep, p[0]))
      if not string:
          return "void"
--- 245,255 ----
      """Return a C-style parameter declaration string."""
      string = []
      for p in params:
!         #sep = "" if p[1].endswith("*") else " "
!         if p[1].endswith("*"):
!             sep = ""
!         else:
!             sep = " "
          string.append("%s%s%s" % (p[1], sep, p[0]))
      if not string:
          return "void"
***************
*** 257,269 ****
  def Alias(funcname):
      """Return the name of the function the named function is an alias of."""
      alias, need_conv = __aliases[funcname]
!     return alias.name if not need_conv else None


  def ConversionFunction(funcname):
      """Return the name of the function the named function converts to."""
      alias, need_conv = __aliases[funcname]
!     return alias.name if need_conv else None


  def Categories(funcname):
--- 265,285 ----
  def Alias(funcname):
      """Return the name of the function the named function is an alias of."""
      alias, need_conv = __aliases[funcname]
!     if need_conv:
!         return None
!     else:
!         return alias.name
!     #return alias.name if not need_conv else None


  def ConversionFunction(funcname):
      """Return the name of the function the named function converts to."""
      alias, need_conv = __aliases[funcname]
!     if need_conv:
!         return alias.name
!     else:
!         return None
!     #return alias.name if need_conv else None


  def Categories(funcname):
EOF

   if [[ $? != 0 ]] ; then
      warn "Unable to apply patch 2 to Mesa 7.10.2."
      return 1
   else
      return 0
   fi
}


function apply_mesa_7102_patch_3
{
   patch -f -p0 <<\EOF
diff -c a/src/mesa/main/es_generator.py Mesa-7.10.2/src/mesa/main/es_generator.py
*** a/src/mesa/main/es_generator.py
--- Mesa-7.10.2/src/mesa/main/es_generator.py
***************
*** 741,747 ****
  """ % (shortname, shortname, shortname, shortname)

  for func in keys:
!     prefix = "_es_" if func not in allSpecials else "_check_"
      for spec in apiutil.Categories(func):
          ext = spec.split(":")
          # version does not match
--- 741,751 ----
  """ % (shortname, shortname, shortname, shortname)

  for func in keys:
!     #prefix = "_es_" if func not in allSpecials else "_check_"
!     if func in allSpecials:
!         prefix = "_check_"
!     else:
!         prefix = "_es_"
      for spec in apiutil.Categories(func):
          ext = spec.split(":")
          # version does not match
EOF

   if [[ $? != 0 ]] ; then
      warn "Unable to apply patch 3 to Mesa 7.10.2."
      return 1
   else
      return 0
   fi
}


function apply_mesa_patch
{
    info "Patching Mesa . . ."
    if [[ ${MESA_VERSION} == "7.5" ]] ; then
        apply_mesa_75_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mesa_75_patch_2
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mesa_75_patch_3
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mesa_75_patch_4
        if [[ $? != 0 ]] ; then
            return 1
        fi
    elif [[ ${MESA_VERSION} == "7.8.2" ]] ; then
        apply_mesa_782_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mesa_782_patch_2
        if [[ $? != 0 ]] ; then
            return 1
        fi
    elif [[ ${MESA_VERSION} == "7.10.2" ]] ; then
        apply_mesa_7102_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mesa_7102_patch_2
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_mesa_7102_patch_3
        if [[ $? != 0 ]] ; then
            return 1
        fi
    else
        warn "Unsupported Mesa Version ${MESA_VERSION}"
        return 1
    fi

    return 0
}


function build_mesa
{
    #
    # prepare build dir
    #
    prepare_build_dir $MESA_BUILD_DIR $MESA_FILE
    untarred_mesa=$?

    if [[ $untarred_mesa == -1 ]] ; then
        warn "Unable to prepare Mesa build directory. Giving Up!"
        return 1
    fi

    #
    # Patch mesa
    #
    apply_mesa_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_mesa == 1 ]] ; then
            warn "Giving up on Mesa build because the patch failed."
            return 1
         else
            warn "Patch failed, but continuing.  I believe that this script\n"\
                 "tried to apply a patch to an existing directory which had\n"\
                 "already been patched ... that is, that the patch is\n"\
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build Mesa.
    #
    info "Building Mesa . . . (~2 minutes)"
    cd $MESA_BUILD_DIR || error "Couldn't cd to mesa build dir."
    PF="${VISITDIR}/mesa/${MESA_VERSION}/${VISITARCH}"

    # We do the build twice due to a VTK issue.  VTK can establish a
    # rendering context via the system's GL using glX, via mangled Mesa
    # using glX, and via offscreen mangled Mesa.  For VisIt, we use
    # either the system's GL, or offscreen mangled Mesa.  To placate
    # VTK, we'll build a mangled+glX version, but then we'll build the
    # offscreen one that we really want.  This ensures we have the 'MesaGL'
    # that VTK needs to link, but if we use 'OSMesa' we get a real, OSMesa
    # library with no glX dependency.
    #
    # Due to this issue, it is imperative that one links "-lOSMesa
    # -lMesaGL" when they want to render/link to an offscreen Mesa
    # context.  The two libraries will have a host of duplicate
    # symbols, and it is important that we pick up the ones from OSMesa.
    info "Configuring Mesa (Mangled glX) ..."
    if [[ "$OPSYS" == "AIX" ]]; then
        export AIX_MESA_CFLAGS="-qcpluscmt -qlanglvl=extc99"
        autoconf
        if [[ $? != 0 ]] ; then
            error "Mesa: AIX autoconf failed!"
        fi
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        MESA_STATIC_DYNAMIC="--disable-shared --enable-static"
    fi

    info 
    
    # Neither of these should be necessary, but we use them as a temporary
    # workaround for a mesa issue.
    if test `uname` = "Linux" ; then
        HACK_FLAGS="-fPIC -DGLX_USE_TLS"
    fi
    ./configure \
      CC="${C_COMPILER}" \
      CXX="${CXX_COMPILER}" \
      CFLAGS="${C_OPT_FLAGS} ${CFLAGS} ${AIX_MESA_CFLAGS} -DUSE_MGL_NAMESPACE ${HACK_FLAGS}" \
      CXXFLAGS="${CXX_OPT_FLAGS} ${CXXFLAGS} -DUSE_MGL_NAMESPACE ${HACK_FLAGS}" \
      --prefix=${PF}               \
      --without-demos              \
      --disable-gallium            \
      --with-driver=xlib           \
      --enable-gl-osmesa           \
      --enable-glx-tls             \
      --disable-glw                \
      --disable-glu                \
      --disable-egl ${MESA_STATIC_DYNAMIC}
    if [[ $? != 0 ]] ; then
        warn "Mesa: 'configure' for Mangled glX failed.  Giving up"
        return 1
    fi

    # Make sure we build 'MesaGL*' libraries, to avoid conflict with GL
    # libraries supplied by the vendor.
    cat configs/autoconf | sed -e "s,GL_LIB = GL,GL_LIB=MesaGL,g" > configs/autoconf.edit || return 1
    mv configs/autoconf.edit configs/autoconf
    cat configs/autoconf | sed -e "s,GLU_LIB = GLU,GLU_LIB=MesaGLU,g" > configs/autoconf.edit || return 1
    mv configs/autoconf.edit configs/autoconf

    info "Building Mesa (Mangled glX) ..."
    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "Mesa: 'make' for Mangled glX failed.  Giving up"
        return 1
    fi
    info "Installing Mesa (Mangled glX) ..."
    ${MAKE} install
    if [[ $? != 0 ]] ; then
        warn "Mesa: 'make install' for Mangled glX failed.  Giving up"
        return 1
    fi

    # Now install #2, the OSMesa that we want/need.
    ${MAKE} clean &>/dev/null
    info "Configuring Mesa (Mangled Offscreen) ..."

    # Do not build libMesaGLU unless we're on MacOS X
    DISABLE_GLU="--disable-glu"
    if [[ "$OPSYS" == "Darwin" ]]; then
        DISABLE_GLU=""
        # If we're on 10.4 or earlier, change the GLU exports file
        VER=$(uname -r)
        if [[ ${VER%%.*} -le 9 ]]; then
            rm src/glu/sgi/glu.exports.darwin.edit
            sed "s/_\*/_m/g" src/glu/sgi/glu.exports.darwin > src/glu/sgi/glu.exports.darwin.edit
            cp src/glu/sgi/glu.exports.darwin.edit src/glu/sgi/glu.exports.darwin
        fi
    fi

    ./configure \
      CC="${C_COMPILER}" \
      CXX="${CXX_COMPILER}" \
      CFLAGS="${C_OPT_FLAGS} ${CFLAGS} ${AIX_MESA_CFLAGS} -DUSE_MGL_NAMESPACE ${HACK_FLAGS}" \
      CXXFLAGS="${CXX_OPT_FLAGS} ${CXXFLAGS} -DUSE_MGL_NAMESPACE ${HACK_FLAGS}" \
      --prefix=${PF}                    \
      --without-demos                   \
      --with-driver=osmesa              \
      --disable-gallium                 \
      --with-max-width=16384            \
      --with-max-height=16384           \
      --enable-glx-tls                  \
      --disable-glw                     \
      ${DISABLE_GLU}                    \
      --disable-egl  ${MESA_STATIC_DYNAMIC}
    if [[ $? != 0 ]] ; then
        warn "Mesa: 'configure' for Mangled Offscreen failed.  Giving up"
        return 1
    fi
    # Make sure we build 'MesaGL*' libraries, to avoid conflict with GL
    # libraries supplied by the vendor.
    cat configs/autoconf | sed -e "s,GL_LIB = GL,GL_LIB=MesaGL,g" > configs/autoconf.edit || return 1
    mv configs/autoconf.edit configs/autoconf
    cat configs/autoconf | sed -e "s,GLU_LIB = GLU,GLU_LIB=MesaGLU,g" > configs/autoconf.edit || return 1
    mv configs/autoconf.edit configs/autoconf

    info "Building Mesa (Mangled Offscreen) ..."
    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "Mesa: 'make' for Mangled Offscreen failed.  Giving up"
        return 1
    fi
    info "Installing Mesa (Mangled Offscreen) ..."
    ${MAKE} install
    if [[ $? != 0 ]] ; then
        warn "Mesa: 'make install' for Mangled Offscreen failed.  Giving up"
        return 1
    fi

    # Some versions of Mesa erroneously install GLEW as well.  We need to make
    # sure we get VisIt's GLEW when we include it, so remove the ones Mesa
    # installs.
    rm -f ${PF}/include/GL/gl*ew.h

    if [[ $? != 0 ]] ; then
        warn "Mesa build failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/mesa"
        chgrp -R ${GROUP} "$VISITDIR/mesa"
    fi
    cd "$START_DIR"
    info "Done with Mesa"
    return 0
}

function bv_mesa_is_enabled
{
    if [[ $DO_MESA == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mesa_is_installed
{
    check_if_installed "mesa" $MESA_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mesa_build
{
    #
    # Build Mesa
    #
    cd "$START_DIR"
    if [[ "$DO_MESA" == "yes" ]] ; then
        check_if_installed "mesa" $MESA_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Mesa build.  Mesa is already installed."
        else
            info "Building Mesa (~2 minutes)"
            build_mesa
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Mesa.  Bailing out."
            fi
            info "Done building Mesa"
        fi
    fi
}

