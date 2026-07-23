
function bv_qt_initialize
{
    export DO_QT="yes"
}

function bv_qt_enable
{
    DO_QT="yes"
}

function bv_qt_disable
{
    DO_QT="no"
}

function bv_qt_depends_on
{
    QT_DEPENDS=""
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        QT_DEPENDS="mesagl glu"
    fi
    if [[ "$DO_XKBCOMMON" == "yes" ]] ; then
        QT_DEPENDS="$QT_DEPENDS xkbcommon"
    fi
    if [[ "$DO_XCB" == "yes" ]] ; then
        QT_DEPENDS="$QT_DEPENDS xcb"
    fi

    echo $QT_DEPENDS
}

function bv_qt_info
{
    export QT_VERSION=${QT_VERSION:-"6.4.2"}
    export QT_SHORT_VERSION=${QT_SHORT_VERSION:-"6.4"}
    export QT_BASE_FILE=${QT_BASE_FILE:-"qtbase-everywhere-src-${QT_VERSION}.tar.xz"}
    export QT_BASE_SOURCE_DIR=${QT_BASE_SOURCE_DIR:-"qtbase-everywhere-src-${QT_VERSION}"}
    export QT_BASE_SHA256_CHECKSUM="a88bc6cedbb34878a49a622baa79cace78cfbad4f95fdbd3656ddb21c705525d"

    # Other submodules
    export QT_TOOLS_FILE=${QT_TOOLS_FILE:-"qttools-everywhere-src-${QT_VERSION}.tar.xz"}
    export QT_TOOLS_SOURCE_DIR=${QT_TOOLS_SOURCE_DIR:-"qttools-everywhere-src-${QT_VERSION}"}
    export QT_TOOLS_SHA256_CHECKSUM="a31387916184e4a5ef522d3ea841e8e931cc0f88be0824a7a354a572d5826c68"
    export QT_SVG_FILE=${QT_SVG_FILE:-"qtsvg-everywhere-src-${QT_VERSION}.tar.xz"}
    export QT_SVG_SOURCE_DIR=${QT_SVG_SOURCE_DIR:-"qtsvg-everywhere-src-${QT_VERSION}"}
    export QT_SVG_SHA256_CHECKSUM="b746af3cb1793621d8ed7eae38d9ad5a15541dc2742031069f2ae3fe87590314"
}

function bv_qt_print
{
    printf "%s%s\n" "QT_BASE_FILE=" "${QT_BASE_FILE}"
    printf "%s%s\n" "QT_VERSION=" "${QT_VERSION}"
    printf "%s%s\n" "QT_SHORT_VERSION=" "${QT_SHORT_VERSION}"
    printf "%s%s\n" "QT_BASE_SOURCE_DIR=" "${QT_BASE_SOURCE_DIR}"
}

function bv_qt_print_usage
{
    printf "%-20s %s [%s]\n" "--qt"   "Build QT" "$DO_QT"
}

function bv_qt_host_profile
{
    # bv_qt_host_profile has tests for ENGINE_ONLY etc, but if any of those conditions
    # are set, then DO_QT is "no", so no need to test for them here.
    if [[ "$DO_QT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## QT" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(QT $QT_VERSION)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_QT_DIR \${VISITHOME}/qt/$QT_VERSION/\${VISITARCH})" >> $HOSTCONF
     fi
}

function bv_qt_initialize_vars
{
    info "initializing qt vars"
    QT_INSTALL_DIR="${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}"
    QT_BIN_DIR="$QT_INSTALL_DIR/bin"
    QT_INCLUDE_DIR="$QT_INSTALL_DIR/include"
    info "qt_BIN dir ${QT_BIN_DIR}"
}

#prepare the module and check whether it is built or is ready to be built.
function bv_qt_ensure
{
    if [[ "$DO_QT" == "yes" ]]; then
        if [[ "$DOWNLOAD_ONLY" == "yes" ]] ; then
            download_file ${QT_TOOLS_FILE} ${QT_URL}
            download_file ${QT_SVG_FILE} ${QT_URL}
        fi
        ensure_built_or_ready "qt"     $QT_VERSION    $QT_BASE_SOURCE_DIR    $QT_BASE_FILE    $QT_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
    return 0
}


function apply_qt_base_patch
{
    if [[ ${QT_VERSION} == 6.4.2 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]] ; then

            qt6_macos_13_cpp_stdlib_issue_patch
            if [[ $? != 0 ]] ; then
                return 1
            fi

            qt6_macos_14_xcode_15_patch
            if [[ $? != 0 ]] ; then
                return 1
            fi

            qt6_macos_15_opengl_patch
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi

        qt6_xkbcommon_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        qt6_libpng_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}

function qt6_macos_15_opengl_patch
{
    info "Patching qt 6 for macos opengl linking issue"
    patch -p0 << \EOF
    From 0efea8020c1d221635aaa0a71529edb392cfe3cc Mon Sep 17 00:00:00 2001
From: Joerg Bornemann <joerg.bornemann@qt.io>
Date: Mon, 11 Sep 2023 14:48:32 +0200
Subject: [PATCH] CMake: Fix build with CMake 3.28 on macOS

FindWrapOpenGL.cmake assumed that IMPORTED_LOCATION is the absolute path
of the library within the framework. That's not the case with CMake 3.28
anymore. There, IMPORTED_LOCATION is the absolute path of the framework
directory.

The relevant upstream CMake change is
6b01a27f901b5eb392955fea322cde44a1b782a3.

Pick-to: 6.2 6.5 6.6
Change-Id: I6b702a28318e0978c56dec83c398965aa77ef020
Reviewed-by: Alexandru Croitor <alexandru.croitor@qt.io>
---
 qtbase-everywhere-src-6.4.2/cmake/FindWrapOpenGL.cmake | 8 ++++++--
 1 file changed, 6 insertions(+), 2 deletions(-)

diff --git qtbase-everywhere-src-6.4.2/cmake/FindWrapOpenGL.cmake qtbase-everywhere-src-6.4.2/cmake/FindWrapOpenGL.cmake
index 3e6abaf4dda7..7295a159caf6 100644
--- qtbase-everywhere-src-6.4.2/cmake/FindWrapOpenGL.cmake
+++ qtbase-everywhere-src-6.4.2/cmake/FindWrapOpenGL.cmake
@@ -17,14 +17,18 @@ if (OpenGL_FOUND)

     add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED)
     if(APPLE)
+        # CMake 3.27 and older:
         # On Darwin platforms FindOpenGL sets IMPORTED_LOCATION to the absolute path of the library
         # within the framework. This ends up as an absolute path link flag, which we don't want,
         # because that makes our .prl files un-relocatable.
         # Extract the framework path instead, and use that in INTERFACE_LINK_LIBRARIES,
-        # which CMake ends up transforming into a reloctable -framework flag.
+        # which CMake ends up transforming into a relocatable -framework flag.
         # See https://gitlab.kitware.com/cmake/cmake/-/issues/20871 for details.
+        #
+        # CMake 3.28 and above:
+        # IMPORTED_LOCATION is the absolute path the the OpenGL.framework folder.
         get_target_property(__opengl_fw_lib_path OpenGL::GL IMPORTED_LOCATION)
-        if(__opengl_fw_lib_path)
+        if(__opengl_fw_lib_path AND NOT __opengl_fw_lib_path MATCHES "/([^/]+)\\.framework$")
             get_filename_component(__opengl_fw_path "${__opengl_fw_lib_path}" DIRECTORY)
         endif()

@@ -30,16 +34,17 @@
             set(__opengl_fw_path "-framework OpenGL")
         endif()
 
-        find_library(WrapOpenGL_AGL NAMES AGL)
-        if(WrapOpenGL_AGL)
-            set(__opengl_agl_fw_path "${WrapOpenGL_AGL}")
-        endif()
-        if(NOT __opengl_agl_fw_path)
-            set(__opengl_agl_fw_path "-framework AGL")
-        endif()
+        # On Darwin XCode 26 and macOS versions moving forward, do not provide AGL
+        # find_library(WrapOpenGL_AGL NAMES AGL)
+        # if(WrapOpenGL_AGL)
+        #     set(__opengl_agl_fw_path "${WrapOpenGL_AGL}")
+        # endif()
+        # if(NOT __opengl_agl_fw_path)
+        #     set(__opengl_agl_fw_path "-framework AGL")
+        # endif()
 
         target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE ${__opengl_fw_path})
-        target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE ${__opengl_agl_fw_path})
+        # target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE ${__opengl_agl_fw_path})
     else()
         target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE OpenGL::GL)
     endif()
EOF

}

function qt6_libpng_patch
{
    info "Patching qt 6 for libpng header issue"
    patch -p0 << \EOF
diff --git qtbase-everywhere-src-6.4.2/src/3rdparty/libpng/pngpriv.h qtbase-everywhere-src-6.4.2/src/3rdparty/libpng//pngpriv.h
index 6c7280cf53..190eb85cbf 100644
--- qtbase-everywhere-src-6.4.2/src/3rdparty/libpng/a/pngpriv.h
+++ qtbase-everywhere-src-6.4.2/src/3rdparty/libpng//pngpriv.h
@@ -556,18 +556,8 @@
     */
 #  include <float.h>

-#  if (defined(__MWERKS__) && defined(macintosh)) || defined(applec) || \
-    defined(THINK_C) || defined(__SC__) || defined(TARGET_OS_MAC)
-   /* We need to check that <math.h> hasn't already been included earlier
-    * as it seems it doesn't agree with <fp.h>, yet we should really use
-    * <fp.h> if possible.
-    */
-#    if !defined(__MATH_H__) && !defined(__MATH_H) && !defined(__cmath__)
-#      include <fp.h>
-#    endif
-#  else
-#    include <math.h>
-#  endif
+#  include <math.h>
+
 #  if defined(_AMIGA) && defined(__SASC) && defined(_M68881)
    /* Amiga SAS/C: We must include builtin FPU functions when compiling using
     * MATH=68881
EOF

}

function qt6_xkbcommon_patch
{
    info "Patching qt 6 for xkbcommon issue"
    patch -p0 << \EOF
-- qtbase-everywhere-src-6.4.2/src/gui/platform/unix/qxkbcommon.cpp.orig	2024-05-21 08:51:16.000000000 -0700
+++ qtbase-everywhere-src-6.4.2/src/gui/platform/unix/qxkbcommon.cpp	2024-05-21 08:50:33.000000000 -0700
@@ -236,16 +236,20 @@
         Xkb2Qt<XKB_KEY_dead_O,                  Qt::Key_Dead_O>,
         Xkb2Qt<XKB_KEY_dead_u,                  Qt::Key_Dead_u>,
         Xkb2Qt<XKB_KEY_dead_U,                  Qt::Key_Dead_U>,
         Xkb2Qt<XKB_KEY_dead_small_schwa,        Qt::Key_Dead_Small_Schwa>,
         Xkb2Qt<XKB_KEY_dead_capital_schwa,      Qt::Key_Dead_Capital_Schwa>,
         Xkb2Qt<XKB_KEY_dead_greek,              Qt::Key_Dead_Greek>,
+/* The following for XKB_KEY_dead keys got removed in libxkbcommon 1.6.0
+   The define check is kind of version check here. */
+#ifdef XKB_KEY_dead_lowline
         Xkb2Qt<XKB_KEY_dead_lowline,            Qt::Key_Dead_Lowline>,
         Xkb2Qt<XKB_KEY_dead_aboveverticalline,  Qt::Key_Dead_Aboveverticalline>,
         Xkb2Qt<XKB_KEY_dead_belowverticalline,  Qt::Key_Dead_Belowverticalline>,
         Xkb2Qt<XKB_KEY_dead_longsolidusoverlay, Qt::Key_Dead_Longsolidusoverlay>,
+#endif

         // Special keys from X.org - This include multimedia keys,
         // wireless/bluetooth/uwb keys, special launcher keys, etc.
         Xkb2Qt<XKB_KEY_XF86Back,                Qt::Key_Back>,
         Xkb2Qt<XKB_KEY_XF86Forward,             Qt::Key_Forward>,
         Xkb2Qt<XKB_KEY_XF86Stop,                Qt::Key_Stop>,
EOF
    if [[ $? != 0 ]] ; then
        warn "Patching qt 6 for xkbcommon issue failed"
        return 1
    fi
}


function qt6_macos_13_cpp_stdlib_issue_patch
{
    info "Patching qt 6 for macOS c++ stdlib issue"

    patch -p0 << \EOF
diff -crB qtbase-everywhere-src-6.4.2/src/corelib/tools/qduplicatetracker_p.h qtbase-everywhere-src-6.4.2-patched/src/corelib/tools/qduplicatetracker_p.h
*** qtbase-everywhere-src-6.4.2/src/corelib/tools/qduplicatetracker_p.h	Tue Nov 15 23:54:24 2022
--- qtbase-everywhere-src-6.4.2-patched/src/corelib/tools/qduplicatetracker_p.h	Wed Oct 25 13:14:40 2023
***************
*** 16,33 ****

  #include <private/qglobal_p.h>

! #if __has_include(<memory_resource>)
! #  include <unordered_set>
! #  include <memory_resource>
! #  include <qhash.h> // for the hashing helpers
! #else
! #  include <qset.h>
! #endif

  QT_BEGIN_NAMESPACE

  template <typename T, size_t Prealloc = 32>
  class QDuplicateTracker {
  #ifdef __cpp_lib_memory_resource
      template <typename HT>
      struct QHasher {
--- 16,38 ----

  #include <private/qglobal_p.h>

! // Only supported on macOS 14 and iOS 17
! // #if __has_include(<memory_resource>)
! // #  include <unordered_set>
! // #  include <memory_resource>
! // #  include <qhash.h> // for the hashing helpers
! // #else
! // #  include <qset.h>
! // #endif

+ #undef __cpp_lib_memory_resource // Only supported on macOS 14 and iOS 17
+ #include <qset.h>
+
  QT_BEGIN_NAMESPACE

  template <typename T, size_t Prealloc = 32>
  class QDuplicateTracker {
+ #undef __cpp_lib_memory_resource // Only supported on macOS 14 and iOS 17
  #ifdef __cpp_lib_memory_resource
      template <typename HT>
      struct QHasher {
***************
*** 70,75 ****
--- 75,81 ----
      Q_DISABLE_COPY_MOVE(QDuplicateTracker);
  public:
      static constexpr inline bool uses_pmr =
+ #undef __cpp_lib_memory_resource // Only supported on macOS 14 and iOS 17
          #ifdef __cpp_lib_memory_resource
              true
          #else
***************
*** 78,83 ****
--- 84,90 ----
              ;
      QDuplicateTracker() = default;
      explicit QDuplicateTracker(qsizetype n)
+ #undef __cpp_lib_memory_resource // Only supported on macOS 14 and iOS 17
  #ifdef __cpp_lib_memory_resource
          : set{size_t(n), &res}
  #else
diff -crB qtbase-everywhere-src-6.4.2/src/gui/image/qxpmhandler.cpp qtbase-everywhere-src-6.4.2-patched/src/gui/image/qxpmhandler.cpp
*** qtbase-everywhere-src-6.4.2/src/gui/image/qxpmhandler.cpp	Tue Nov 15 23:54:24 2022
--- qtbase-everywhere-src-6.4.2-patched/src/gui/image/qxpmhandler.cpp	Wed Oct 25 12:58:52 2023
***************
*** 1078,1083 ****
--- 1078,1084 ----
      else
          image = sourceImage;

+ #undef __cpp_lib_memory_resource
  #ifdef __cpp_lib_memory_resource
      char buffer[1024];
      std::pmr::monotonic_buffer_resource res{&buffer, sizeof buffer};

EOF
    if [[ $? != 0 ]] ; then
        warn "Patching qt 6for macOS c++ stdlib issue failed"
        return 1
    fi
}

function qt6_macos_14_xcode_15_patch
{
    info "Patching qt 6 for macOS xcode 15 with toolchain fix"

    patch -p0 << \EOF
diff --git qtbase-everywhere-src-6.4.2/mkspecs/features/toolchain.prf qtbase-everywhere-src-6.4.2-patched/mkspecs/features/toolchain.prf
index 0040b6c..bfad10d 100644
--- qtbase-everywhere-src-6.4.2/mkspecs/features/toolchain.prf
+++ qtbase-everywhere-src-6.4.2-patched/mkspecs/features/toolchain.prf
@@ -288,9 +288,12 @@
                 }
             }
         }
-        isEmpty(QMAKE_DEFAULT_LIBDIRS)|isEmpty(QMAKE_DEFAULT_INCDIRS): \
+        isEmpty(QMAKE_DEFAULT_INCDIRS): \
             !integrity: \
-                error("failed to parse default search paths from compiler output")
+                error("failed to parse default include paths from compiler output")
+        isEmpty(QMAKE_DEFAULT_LIBDIRS): \
+            !integrity:!darwin: \
+                error("failed to parse default library paths from compiler output")
         QMAKE_DEFAULT_LIBDIRS = $$unique(QMAKE_DEFAULT_LIBDIRS)
     } else: ghs {
         cmd = $$QMAKE_CXX $$QMAKE_CXXFLAGS -$${LITERAL_HASH} -o /tmp/fake_output /tmp/fake_input.cpp
@@ -411,7 +414,7 @@
         QMAKE_DEFAULT_INCDIRS = $$split(INCLUDE, $$QMAKE_DIRLIST_SEP)
     }

-    unix:if(!cross_compile|host_build) {
+    unix:!darwin:if(!cross_compile|host_build) {
         isEmpty(QMAKE_DEFAULT_INCDIRS): QMAKE_DEFAULT_INCDIRS = /usr/include /usr/local/include
         isEmpty(QMAKE_DEFAULT_LIBDIRS): QMAKE_DEFAULT_LIBDIRS = /lib /usr/lib
     }
EOF
    if [[ $? != 0 ]] ; then
        warn "Patching qt 6 for macOS xcode 15 with toolchain fix failed"
        return 1
    fi
}


function build_qt_base
{
    echo "Build Qt 6 base module"
    prepare_build_dir $QT_BASE_SOURCE_DIR $QT_BASE_FILE SHA256 $QTBASE_SHA256_CHECKSUM

    untarred_qt=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_qt == -1 ]] ; then
        warn "Unable to prepare Qt 6 build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching qt . . ."

    apply_qt_base_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_qt == 1 ]] ; then
            warn "Giving up on Qt build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    QT_BASE_BUILD_DIR="${QT_BASE_SOURCE_DIR}-build"
    if ! test -f $QT_BASE_BUILD_DIR ; then
        echo "Making build directory $QT_BASE_BUILD_DIR"
        mkdir $QT_BASE_BUILD_DIR
    fi

    # Qt has a check for symlinks in its build dir,
    # so use 'cd -P' to resolve them if they are present
    cd -P ${QT_BASE_BUILD_DIR}

    #
    # Platform specific configuration
    #

    #
    # Select the proper value for QT_PLATFORM
    #
    # Question: Could qt auto detect this via the CC and CXX env vars?
    #
    # We should try to see if we can avoid setting the platform, set
    # CC and CXX and see if that is enough to trigger qt's detection logic.
    #
    # NOTE, KSB 03-20-2023, setting CC and CXX, on LC toss4, QT chose as
    # QT_QMAKE_TARGET_MKSPEC: linux-g++, not linux-g++-x64
    #


    if [[ "$OPSYS" == "Darwin" ]]; then
        QT_PLATFORM="macx-clang"
    elif [[ "$OPSYS" == "Linux" ]] ; then
        if [[ "$C_COMPILER" == "clang" ]]; then
            QT_PLATFORM="linux-clang"
        elif [[ "$C_COMPILER" == "llvm" ]]; then
            QT_PLATFORM="linux-llvm"
        elif [[ "$(uname -m)" == "ia64" ]]; then
            QT_PLATFORM="linux-g++-64"
        elif [[ "$(uname -m)" == "x86_64" ]] ; then
            if [[ "$C_COMPILER" == "icc" || "$CXX_COMPILER" == "icpc" ]]; then
                QT_PLATFORM="linux-icc-64"
            else
                QT_PLATFORM="linux-g++-64"
            fi
        elif [[ "$(uname -m)" == "ppc64" || "$(uname -m)" == "ppc64le" ]]; then
            QT_PLATFORM="linux-g++-64"
        else
            if [[ "$C_COMPILER" == "icc" || "$CXX_COMPILER" == "icpc" ]]; then
                QT_PLATFORM="linux-icc-32"
            else
                QT_PLATFORM="linux-g++-32"
            fi
        fi
        # For Fedora, disable openssl
        if [[ -n "$(cat /proc/version 2>/dev/null | grep -i fedora)" ]]; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-openssl"
        fi
    fi

    #
    # Call configure
    #

    if [[ "$DO_XKBCOMMON" == "yes" ]] ; then
        export PKG_CONFIG_PATH=$XKBCOMMON_INSTALL_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
    fi
    if [[ "$DO_XCB" == "yes" ]] ; then
        export PKG_CONFIG_PATH=$XCB_INSTALL_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
    fi

    QT_CFLAGS="${CFLAGS} ${C_OPT_FLAGS}"
    QT_CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}"

    qt_flags=""
    qt_flags="${qt_flags} -no-dbus"
    qt_flags="${qt_flags} -no-egl"
    qt_flags="${qt_flags} -no-eglfs"
    qt_flags="${qt_flags} -no-sql-db2"
    qt_flags="${qt_flags} -no-sql-ibase"
    qt_flags="${qt_flags} -no-sql-mysql"
    qt_flags="${qt_flags} -no-sql-oci"
    qt_flags="${qt_flags} -no-sql-odbc"
    qt_flags="${qt_flags} -no-sql-psql"
    qt_flags="${qt_flags} -no-sql-sqlite"
    qt_flags="${qt_flags} -no-libjpeg"
    qt_flags="${qt_flags} -qt-libpng"
    qt_flags="${qt_flags} -qt-zlib"
    qt_flags="${qt_flags} -nomake examples"
    qt_flags="${qt_flags} -nomake tests"
    qt_flags="${qt_flags} -opensource"
    qt_flags="${qt_flags} -confirm-license"
    # should test for static
    qt_flags="${qt_flags} -shared"

    if [[ "$VISIT_BUILD_MODE" == "Release" ]] ; then
        qt_flags="${qt_flags} -release"
    else
        qt_flags="${qt_flags} -debug"
    fi

    qt_cmake_flags="$(qt_mesagl_cmake_flags)"
    info "Configuring Qt base: . . . "
    set -x
    (echo "o"; echo "yes") | env PATH="${CMAKE_INSTALL}:$PATH" \
                             CFLAGS="${QT_CFLAGS}" CXXFLAGS="${QT_CXXFLAGS}"  \
                             CC="${C_COMPILER}" CXX="${CXX_COMPILER}"  \
                             ../${QT_BASE_SOURCE_DIR}/configure \
                             -prefix ${QT_INSTALL_DIR} ${qt_flags} \
                             ${qt_cmake_flags} | tee qt.config.out
    set +x
    if [[ $? != 0 ]] ; then
        warn "Qt base configure failed. Giving up."
        return 1
    fi

   #
    # Build Qt. Config options above make sure we only build the libs & tools.
    #
    info "Building Qt6 base . . . "
    ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS

    if [[ $? != 0 ]] ; then
        warn "Qt base build failed.  Giving up"
        return 1
    fi

    info "Installing Qt  base . . . "
    ${CMAKE_COMMAND} --install .

    # Qt screws up permissions in some cases.  Try to fix that.
    chmod -R a+rX ${VISITDIR}/qt/${QT_VERSION}
   if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/qt"
        chgrp -R ${GROUP} "$VISITDIR/qt"
    fi

    cd "$START_DIR"
    info "Done with Qt base "

    return 0
}

function qt_mesagl_cmake_flags
{
    local qt_cmake_flags=""
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        qt_cmake_flags=" -- -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_gl_LIBRARY:STRING=${MESAGL_OPENGL_LIB}"
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_opengl_LIBRARY:STRING="
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_glu_LIBRARY:STRING=${MESAGL_GLU_LIB}"
        qt_cmake_flags="${qt_cmake_flags} -DOpenGL_GL_PREFERENCE:STRING=LEGACY"
    fi
    echo "${qt_cmake_flags}"
}


function build_qt_tools
{
    cd "$START_DIR"
    echo "Build Qt 6 tools module"

    if ! test -f ${QT_TOOLS_FILE} ; then
        download_file ${QT_TOOLS_FILE} ${QT_URL}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${QT_TOOLS_FILE}"
            return 1
        fi
    fi

    if ! test -d ${QT_TOOLS_SOURCE_DIR} ; then
        info "Extracting qt tools ..."
        uncompress_untar ${QT_TOOLS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${QT_TOOLS_FILE}"
            return 1
        fi
    fi

    # Make a build directory for an out-of-source build.
    QT_TOOLS_BUILD_DIR="${QT_TOOLS_SOURCE_DIR}-build"
    if [[ ! -d $QT_TOOLS_BUILD_DIR ]] ; then
        echo "Making build directory $QT_TOOLS_BUILD_DIR"
        mkdir $QT_TOOLS_BUILD_DIR
    fi

    cd ${QT_TOOLS_BUILD_DIR}

    qt_module_cmake_flags="$(qt_mesagl_cmake_flags)"

    info "Configuring Qt tools . . . "
    env CC="${C_COMPILER}" CXX="${CXX_COMPILER}"  \
        ${QT_INSTALL_DIR}/bin/qt-configure-module  ../${QT_TOOLS_SOURCE_DIR} \
        ${qt_module_cmake_flags}

    info "Building Qt6 tools . . . "
    ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS

    info "Installing Qt tools . . . "
    ${CMAKE_COMMAND} --install .

    return 0;
}

function build_qt_svg
{
    cd "$START_DIR"
    echo "Build Qt 6 svg module"

    if ! test -f ${QT_SVG_FILE} ; then
        download_file ${QT_SVG_FILE} ${QT_URL}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${QT_SVG_FILE}"
            return 1
        fi
    fi

    if ! test -d ${QT_SVG_SOURCE_DIR} ; then
        info "Extracting qt svg ..."
        uncompress_untar ${QT_SVG_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${QT_SVG_FILE}"
            return 1
        fi
    fi

    # Make a build directory for an out-of-source build.
    QT_SVG_BUILD_DIR="${QT_SVG_SOURCE_DIR}-build"
    if [[ ! -d $QT_SVG_BUILD_DIR ]] ; then
        echo "Making build directory $QT_SVG_BUILD_DIR"
        mkdir $QT_SVG_BUILD_DIR
    fi

    cd ${QT_SVG_BUILD_DIR}

    qt_module_cmake_flags="$(qt_mesagl_cmake_flags)"

    info "Configuring Qt svg . . . "
    env CC="${C_COMPILER}" CXX="${CXX_COMPILER}"  \
        ${QT_INSTALL_DIR}/bin/qt-configure-module  ../${QT_SVG_SOURCE_DIR} \
        ${qt_module_cmake_flags}

    info "Building Qt6 svg . . . "
    ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS

    info "Installing Qt svg . . . "
    ${CMAKE_COMMAND} --install .

    return 0;
}

function bv_qt_is_enabled
{
    if [[ $DO_QT == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_qt_is_installed
{
    check_if_installed "qt" $QT_VERSION
    if [[ $? != 0 ]] ; then
        return 0
    fi

    # check submodules

    if ! test -f ${QT_INSTALL_DIR}/modules/Tools.json ; then
        return 0
    fi

    if ! test -f ${QT_INSTALL_DIR}/modules/Svg.json ; then
        return 0
    fi
    return 1
}

function bv_qt_build
{
    cd "$START_DIR"
    if [[ "$DO_QT" == "yes" ]] ; then

        # checks if qtbase and all required modules are installed
        bv_qt_is_installed
        if [[ $? == 1 ]] ; then
             info "Skipping Qt build.  Qt is already installed."
        else

            # check qt proper, then submodules
            check_if_installed "qt" $QT_VERSION
            if [[ $? != 0 ]] ; then
                info "Building QT base (~10 minutes)"
                build_qt_base
                if [[ $? != 0 ]] ; then
                    error "Unable to build or install QT.  Bailing out."
                fi
                info "Done building Qt base"
            else
                info "Qt base already exists"
            fi


            # tools submodule
            if test -f ${QT_INSTALL_DIR}/modules/Tools.json ; then
                info "Qt 6 submodule tools already exists"
            else
                info "Building QT tools (~4 minutes)"
                build_qt_tools
            fi

            # svg submodule
            if test -f ${QT_INSTALL_DIR}/modules/Svg.json ; then
                info "Qt 6 submodule svg already exists"
            else
                info "Building QT svg (~2 minutes)"
                build_qt_svg
           fi
        fi
    fi
}
