function bv_qt_initialize
{
    export DO_QT="yes"
    export FORCE_QT="no"
    export USE_SYSTEM_QT="no"
    add_extra_commandline_args "qt" "system-qt" 0 "Use qt found on system"
    add_extra_commandline_args "qt" "alt-qt-dir" 1 "Use qt found in alternative directory"
}

function bv_qt_enable
{
    DO_QT="yes"
    FORCE_QT="yes"
}

function bv_qt_disable
{
    DO_QT="no"
    FORCE_QT="no"
}

function bv_qt_force
{
    if [[ "$FORCE_QT" == "yes" ]]; then
        return 0;
    fi
    return 1;
}

function qt_set_vars_helper
{
    QT_VERSION=`$QT_QMAKE_COMMAND -query QT_VERSION`
    QT_INSTALL_DIR=`$QT_QMAKE_COMMAND -query QT_INSTALL_PREFIX`
    QT_BIN_DIR=`$QT_QMAKE_COMMAND -query QT_INSTALL_BINS`
    QT_INCLUDE_DIR=`$QT_QMAKE_COMMAND -query QT_INSTALL_HEADERS`
    QT_LIB_DIR=`"$QT_QMAKE_COMMAND" -query QT_INSTALL_LIBS`
    QT_QTUITOOLS_INCLUDE_DIR="$QT_INCLUDE_DIR/QtUiTools"
}

function bv_qt_system_qt
{
    echo "using system qt"

    QTEXEC="qmake"
    TEST=`which $QTEXEC`
    if [[ $? != 0 ]]; then
        error "System Qt not found"
    fi

    bv_qt_enable

    USE_SYSTEM_QT="yes"
    QT_QMAKE_COMMAND="$QTEXEC"
    qt_set_vars_helper #set vars..
    QT_FILE=""
}

function bv_qt_alt_qt_dir
{
    info "using qt from alternative directory $1"

    QTEXEC="qmake"
    if [[ ! -e "$1/bin/$QTEXEC" ]]; then
        error "qmake was not found in directory: $1/bin"
    fi

    bv_qt_enable
    USE_SYSTEM_QT="yes"

    QT_ALT_DIR="$1"
    QT_QMAKE_COMMAND="$QT_ALT_DIR/bin/$QTEXEC"
    qt_set_vars_helper #set vars..
    QT_FILE=""
}

function bv_qt_initialize_vars
{
    info "initalizing qt vars"
    if [[ $USE_SYSTEM_QT != "yes" ]]; then
        QT_INSTALL_DIR="${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}"
        QT_QMAKE_COMMAND="${QT_INSTALL_DIR}/bin/qmake"
        if [[ -e "$QT_QMAKE_COMMAND" ]]; then
            QT_BIN_DIR=`$QT_QMAKE_COMMAND -query QT_INSTALL_BINS`
            QT_INCLUDE_DIR=`$QT_QMAKE_COMMAND -query QT_INSTALL_HEADERS`
            QT_LIB_DIR=`"$QT_QMAKE_COMMAND" -query QT_INSTALL_LIBS`
        else
            QT_BIN_DIR="$QT_INSTALL_DIR/bin"
            QT_INCLUDE_DIR="$QT_INSTALL_DIR/include"
            QT_LIB_DIR="$QT_INSTALL_DIR/lib"
        fi
        QT_QTUITOOLS_INCLUDE_DIR="$QT_INCLUDE_DIR/QtUiTools"
    fi
}

function bv_qt_depends_on
{
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        echo "mesagl glu"
    else
        echo ""
    fi
}


function bv_qt_info
{
    bv_qt_enable

    if [[ "$DO_QT510" == "yes" ]] ; then
        export QT_VERSION=${QT_VERSION:-"5.10.1"}
        export QT_SHORT_VERSION=${QT_SHORT_VERSION:-"5.10"}
        export QT_FILE=${QT_FILE:-"qt-everywhere-src-${QT_VERSION}.tar.xz"}
        export QT_BUILD_DIR=${QT_BUILD_DIR:-"${QT_FILE%.tar*}"}
        export QT_BIN_DIR=${QT_BIN_DIR:-"${QT_BUILD_DIR}/bin"}
        export QT_URL=${QT_URL:-"http://download.qt.io/archive/qt/${QT_SHORT_VERSION}/${QT_VERSION}/single/"}
        export QT_MD5_CHECKSUM="7e167b9617e7bd64012daaacb85477af"
        export QT_SHA256_CHECKSUM="05ffba7b811b854ed558abf2be2ddbd3bb6ddd0b60ea4b5da75d277ac15e740a"
    else
        export QT_VERSION=${QT_VERSION:-"5.14.2"}
        export QT_SHORT_VERSION=${QT_SHORT_VERSION:-"5.14"}
        export QT_FILE=${QT_FILE:-"qt-everywhere-src-${QT_VERSION}.tar.xz"}
        export QT_BUILD_DIR=${QT_BUILD_DIR:-"${QT_FILE%.tar*}"}
        export QT_BIN_DIR=${QT_BIN_DIR:-"${QT_BUILD_DIR}/bin"}
        export QT_URL=${QT_URL:-"http://download.qt.io/archive/qt/${QT_SHORT_VERSION}/${QT_VERSION}/single/"}
        export QT_MD5_CHECKSUM="b3d2b6d00e6ca8a8ede6d1c9bdc74daf"
        export QT_SHA256_CHECKSUM="c6fcd53c744df89e7d3223c02838a33309bd1c291fcb6f9341505fe99f7f19fa"
    fi
}

function bv_qt_print
{
    printf "%s%s\n" "QT_FILE=" "${QT_FILE}"
    printf "%s%s\n" "QT_VERSION=" "${QT_VERSION}"
    printf "%s%s\n" "QT_PLATFORM=" "${QT_PLATFORM}"
    printf "%s%s\n" "QT_BUILD_DIR=" "${QT_BUILD_DIR}"
    printf "%s%s\n" "QT_BIN_DIR=" "${QT_BIN_DIR}"
}

function bv_qt_print_usage
{
    printf "%-20s %s\n" "--qt" "Build Qt5" 
    printf "%-20s %s [%s]\n" "--system-qt" "Use the system installed Qt"
    printf "%-20s %s [%s]\n" "--alt-qt-dir" "Use Qt from alternative directory"
}

function bv_qt_host_profile
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then 
                echo >> $HOSTCONF
                echo "##" >> $HOSTCONF
                echo "## Qt" >> $HOSTCONF
                echo "##" >> $HOSTCONF
                echo "SETUP_APP_VERSION(QT $QT_VERSION)" >> $HOSTCONF

                if [[ $USE_SYSTEM_QT == "yes" ]]; then
                    echo "VISIT_OPTION_DEFAULT(QT_QTUITOOLS_INCLUDE_DIR ${QT_QTUITOOLS_INCLUDE_DIR})" >> $HOSTCONF
                    echo "VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${QT_BIN_DIR})" >> $HOSTCONF
                    echo "SET(VISIT_QT_SKIP_INSTALL ON)" >> $HOSTCONF
                else
                    echo "VISIT_OPTION_DEFAULT(VISIT_QT_DIR \${VISITHOME}/qt/\${QT_VERSION}/\${VISITARCH})" >> $HOSTCONF
                fi
            fi
        fi
    fi
}

function bv_qt_ensure
{
    if [[ "$DO_QT" == "yes"  && "$USE_SYSTEM_QT" == "no" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
        ensure_built_or_ready "qt"     $QT_VERSION    $QT_BUILD_DIR    $QT_FILE    $QT_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}

# *************************************************************************** #
#                          Function 4, build_qt                               #
# *************************************************************************** #

function qt_license_prompt
{
    QT_LIC_MSG="During the build process this script will build Qt and confirm\
            that you accept Trolltech's license for the Qt Open Source\
            Edition. Please respond \"yes\" to accept (in advance) either\
            the Lesser GNU General Public License (LGPL) version 2.1 or \
            the GNU General Public License (GPL) version 3. Visit \
            http://www.qt.io/qt-licensing-terms to view these licenses."

    QT_CONFIRM_MSG="VisIt requires Qt: Do you accept\
                Qt licensing under the terms of the Lesser GNU General \
                Public License (LGPL) version 2.1 or \
                the GNU General Public License (GPL) version 3? [yes/no]"
    info $QT_LIC_MSG
    info $QT_CONFIRM_MSG
    read RESPONSE
    if [[ "$RESPONSE" != "yes" ]] ; then
        info $QT_CONFIRM_MSG
        read RESPONSE
        if [[ $RESPONSE != "yes" ]] ; then
            return 1
        fi
    fi

    return 0
}


function apply_qt_patch
{
    if [[ ${QT_VERSION} == 5.10.1 ]] ; then
        if [[ "$DO_MESAGL" == "yes" ]] ; then
            if [[ "$OPSYS" == "Linux" ]]; then
                apply_qt_5101_linux_mesagl_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi

        if [[ -f /etc/centos-release ]] ; then
            VER=`cat /etc/centos-release | cut -d' ' -f 4`
            if [[ "${VER:0:2}" == "8." ]] ; then
                apply_qt_5101_centos8_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        elif [[ -f /etc/lsb-release ]] ; then
            VER=`cat /etc/lsb-release | grep "DISTRIB_RELEASE" | cut -d'=' -f 2`
            if [[ "${VER:0:3}" == "19." || "${VER:0:3}" == "20." ]] ; then
                apply_qt_5101_centos8_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        elif [[ -f /etc/os-release ]] ; then
            REDHAT_VER=`cat /etc/os-release | grep "REDHAT_SUPPORT_PRODUCT_VERSION" | cut -d'=' -f 2`
            DEBIAN_VER=`cat /etc/os-release | grep "PRETTY_NAME" | cut -d' ' -f 3`
            if [[ "${REDHAT_VER:1:1}" == "8"  || \
                  "${REDHAT_VER:0:2}" == "31" || \
                  "${DEBIAN_VER:0:2}" == "10" ]] ; then
                apply_qt_5101_centos8_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi

        if [[ "$OPSYS" == "Linux" ]]; then
            apply_qt_5101_blueos_patch
            if [[ $? != 0 ]] ; then
                return 1
            fi

            if [[ "$C_COMPILER" == "gcc" ]]; then
                apply_qt_5101_gcc_9_2_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi

        if [[ "$OPSYS" == "Darwin" ]]; then
            productVersion=`sw_vers -productVersion`
            if [[ $productVersion == 10.14.[0-9]* || \
                  $productVersion == 10.15.[0-9]* ]] ; then
                apply_qt_5101_macos_mojave_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi

        #
        # If python doesn't exist, substitute python2 for python.
        #
        python -c "print(1)"
        if [[ $? != 0 ]] ; then
            info "Patching qt 5.10.1 for python to python2"
            cd qtdeclarative
            find . -type f -exec sed -i "s/python/python2/g" {} \;
            cd ..
        fi
    elif [[ ${QT_VERSION} == 5.14.2 ]] ; then
        apply_qt_5142_numeric_limits_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_qt_5142_cmath_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        if [[ "$OPSYS" == "Linux" ]]; then
            if [[ "$DO_MESAGL" == "yes" ]] ; then
                apply_qt_5142_linux_mesagl_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            else
                apply_qt_5142_linux_opengl_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
            #
            # If /usr/lib64 doesn't exist, replace references to it with
            # /usr/lib.
            #
            if [[ ! -d /usr/lib64 ]] ; then
                sed -i "s/\/usr\/lib64/\/usr\/lib/" ./qtbase/mkspecs/linux-g++-64/qmake.conf
                sed -i "s/\/usr\/lib64/\/usr\/lib/" ./qtbase/mkspecs/linux-icc-64/qmake.conf
            fi
        fi
        if [[ "$OPSYS" == "Darwin" ]]; then
            if [[ $(echo $MACOSX_DEPLOYMENT_TARGET | tr -d '.') -ge 1014 ]]; then
                apply_qt_5142_macOS_1014_hunspell_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi
        if [[ "$OPSYS" == "Darwin" ]]; then
            apply_qt_5142_missing_cgcolorspace_header
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
    return 0
}

function apply_qt_5101_linux_mesagl_patch
{
    info "Patching qt 5.10.1 for Linux and Mesa-as-GL"
    patch -p0 <<EOF
    diff -c qtbase/mkspecs/linux-g++-64/qmake.conf.orig  qtbase/mkspecs/linux-g++-64/qmake.conf
    *** qtbase/mkspecs/linux-g++-64/qmake.conf.orig     Thu Feb  8 18:24:48 2018
    --- qtbase/mkspecs/linux-g++-64/qmake.conf  Fri Feb 22 22:04:50 2019
    ***************
    *** 19,24 ****
  
  
      QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
    ! QMAKE_LIBDIR_OPENGL     = /usr/X11R6/lib64
  
      load(qt_config)
    --- 19,25 ----
  
  
      QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
    ! QMAKE_LIBDIR_OPENGL=$MESAGL_LIB_DIR $LLVM_LIB_DIR
    ! QMAKE_INCDIR_OPENGL=$MESAGL_INCLUDE_DIR
  
      load(qt_config)
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 linux mesagl patch failed."
        return 1
    fi

    patch -p0 <<EOF
diff -c qtbase/mkspecs/linux-icc-64/qmake.conf.orig qtbase/mkspecs/linux-icc-64/qmake.conf
*** qtbase/mkspecs/linux-icc-64/qmake.conf.orig Fri Nov 22 15:24:45 2019
--- qtbase/mkspecs/linux-icc-64/qmake.conf      Fri Nov 22 15:25:11 2019
***************
*** 13,16 ****
  # Change the all LIBDIR variables to use lib64 instead of lib
  
  QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/X11R6/lib64
--- 13,17 ----
  # Change the all LIBDIR variables to use lib64 instead of lib
  
  QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
! QMAKE_LIBDIR_OPENGL=$MESAGL_LIB_DIR $LLVM_LIB_DIR
! QMAKE_INCDIR_OPENGL=$MESAGL_INCLUDE_DIR
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 linux mesagl patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_5101_centos8_patch
{
    info "Patching qt 5.10.1 for Centos8"
    patch -p0 <<EOF
diff -c qtbase/src/corelib/io/qfilesystemengine_unix.cpp.orig qtbase/src/corelib/io/qfilesystemengine_unix.cpp
*** qtbase/src/corelib/io/qfilesystemengine_unix.cpp.orig       Thu Oct 17 13:54:59 2019
--- qtbase/src/corelib/io/qfilesystemengine_unix.cpp    Thu Oct 17 13:57:20 2019
***************
*** 97,102 ****
--- 97,103 ----
  #  define FICLONE       _IOW(0x94, 9, int)
  #endif
  
+ #if 0
  #  if !QT_CONFIG(renameat2) && defined(SYS_renameat2)
  static int renameat2(int oldfd, const char *oldpath, int newfd, const char *newpath, unsigned flags)
  { return syscall(SYS_renameat2, oldfd, oldpath, newfd, newpath, flags); }
***************
*** 108,117 ****
--- 109,121 ----
  { return syscall(SYS_statx, dirfd, pathname, flag, mask, statxbuf); }
  #  endif
  #endif
+ #endif
  
+ #if 0
  #ifndef STATX_BASIC_STATS
  struct statx { mode_t stx_mode; };
  #endif
+ #endif
  
  QT_BEGIN_NAMESPACE
  
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 centos8 patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_5101_blueos_patch
{
    info "Patching qt 5.10.1 for Blueos"
    sed -i "s/PNG_ARM_NEON_OPT=0/PNG_ARM_NEON_OPT=0 PNG_POWERPC_VSX_OPT=0/" qtbase/src/3rdparty/libpng/libpng.pro
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 blueos patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_5101_macos_mojave_patch
{
    info "Patching qt 5.10.1 for macOS 10.14 (Mojave)..."
    patch -p0 <<EOF
diff -c qtbase/src/platformsupport/fontdatabases/mac/qfontengine_coretext.mm.orig qtbase/src/platformsupport/fontdatabases/mac/qfontengine_coretext.mm
*** qtbase/src/platformsupport/fontdatabases/mac/qfontengine_coretext.mm.orig   2020-01-16 11:06:12.000000000 -0800
--- qtbase/src/platformsupport/fontdatabases/mac/qfontengine_coretext.mm        2020-01-16 11:06:38.000000000 -0800
***************
*** 830,836 ****
  
  QFixed QCoreTextFontEngine::emSquareSize() const
  {
!     return QFixed::QFixed(int(CTFontGetUnitsPerEm(ctfont)));
  }
  
  QFontEngine *QCoreTextFontEngine::cloneWithSize(qreal pixelSize) const
--- 830,836 ----
  
  QFixed QCoreTextFontEngine::emSquareSize() const
  {
!     return QFixed(int(CTFontGetUnitsPerEm(ctfont)));
  }
  
  QFontEngine *QCoreTextFontEngine::cloneWithSize(qreal pixelSize) const
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 macOS patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_5101_gcc_9_2_patch
{
    info "Patching qt 5.10.1 for gcc 9.2"
    patch -p0 <<EOF
diff -c qtbase/src/corelib/global/qrandom.cpp.orig qtbase/src/corelib/global/qrandom.cpp
*** qtbase/src/corelib/global/qrandom.cpp.orig  Mon Mar  9 17:09:47 2020
--- qtbase/src/corelib/global/qrandom.cpp       Mon Mar  9 17:10:42 2020
***************
*** 220,225 ****
--- 220,226 ----
  #endif // Q_OS_WINRT
  
      static SystemGenerator &self();
+     typedef quint32 result_type;
      void generate(quint32 *begin, quint32 *end) Q_DECL_NOEXCEPT_EXPR(FillBufferNoexcept);
  
      // For std::mersenne_twister_engine implementations that use something
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 gcc 9.2 patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_5142_numeric_limits_patch
{   
    info "Patching qt 5.14.2 for numeric-limits" 
    patch -p0 <<EOF
diff -c qtbase/src/corelib/global/qendian.h.orig c qtbase/src/corelib/global/qendian.h
*** qtbase/src/corelib/global/qendian.h.orig	Wed Oct 13 20:10:58 2021
--- qtbase/src/corelib/global/qendian.h	Wed Oct 13 20:11:12 2021
***************
*** 47,52 ****
--- 47,53 ----
  // include stdlib.h and hope that it defines __GLIBC__ for glibc-based systems
  #include <stdlib.h>
  #include <string.h>
+ #include <limits>
  
  #ifdef min // MSVC
  #undef min
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 for numeric-limits patch1 failed"
        return 1
    fi

    patch -p0 <<EOF
diff -c qtbase/src/corelib/global/qfloat16.h.orig c qtbase/src/corelib/global/qfloat16.h
*** qtbase/src/corelib/global/qfloat16.h.orig	Wed Oct 13 20:09:19 2021
--- qtbase/src/corelib/global/qfloat16.h	Wed Oct 13 20:09:50 2021
***************
*** 44,49 ****
--- 44,50 ----
  #include <QtCore/qglobal.h>
  #include <QtCore/qmetatype.h>
  #include <string.h>
+ #include <limits>
  
  #if defined(QT_COMPILER_SUPPORTS_F16C) && defined(__AVX2__) && !defined(__F16C__)
  // All processors that support AVX2 do support F16C too. That doesn't mean
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 for numeric-limits patch2 failed"
        return 1
    fi
    patch -p0 <<EOF
diff -c qtbase/src/corelib/text/qbytearraymatcher.h.orig c qtbase/src/corelib/text/qbytearraymatcher.h
*** qtbase/src/corelib/text/qbytearraymatcher.h.orig	Wed Oct 13 20:11:58 2021
--- qtbase/src/corelib/text/qbytearraymatcher.h	Wed Oct 13 20:12:21 2021
***************
*** 41,46 ****
--- 41,47 ----
  #define QBYTEARRAYMATCHER_H
  
  #include <QtCore/qbytearray.h>
+ #include <limits>
  
  QT_BEGIN_NAMESPACE
  
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 for numeric-limits patch3 failed"
        return 1
    fi

    return 0
}

function apply_qt_5142_cmath_patch
{
    info "Patching qt 5.14.2 for qjp2handler cmath include"
    patch -p0 <<EOF
diff -c qtimageformats/src/plugins/imageformats/jp2/qjp2handler.cpp.orig qtimageformats/src/plugins/imageformats/jp2/qjp2handler.cpp
*** qtimageformats/src/plugins/imageformats/jp2/qjp2handler.cpp.orig
--- qtimageformats/src/plugins/imageformats/jp2/qjp2handler.cpp
***************
*** 45,50 ****
--- 45,51 ----
  #include "qcolor.h"

  #include <jasper/jasper.h>
+ #include <cmath>

  QT_BEGIN_NAMESPACE

EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 for qjp2handler cmath include"
        return 1
    fi

    return 0
}

function apply_qt_5142_linux_mesagl_patch
{   
    info "Patching qt 5.14.2 for Linux and Mesa-as-GL"
    patch -p0 <<EOF
diff -c qtbase/mkspecs/linux-g++-64/qmake.conf.orig  qtbase/mkspecs/linux-g++-64/qmake.conf
*** qtbase/mkspecs/linux-g++-64/qmake.conf.orig
--- qtbase/mkspecs/linux-g++-64/qmake.conf
***************
*** 18,24 ****
  include(../common/g++-unix.conf)
  
  
! QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/X11R6/lib64
  
  load(qt_config)
--- 18,25 ----
  include(../common/g++-unix.conf)
  
  
! QMAKE_LIBDIR_X11        = /usr/lib64
! QMAKE_LIBDIR_OPENGL     = $MESAGL_LIB_DIR $LLVM_LIB_DIR 
! QMAKE_INCDIR_OPENGL     = $MESAGL_INCLUDE_DIR
  
  load(qt_config)

EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 linux mesagl patch 1 failed."
        return 1
    fi
    
    patch -p0 <<EOF
diff -c qtbase/mkspecs/linux-icc-64/qmake.conf.orig qtbase/mkspecs/linux-icc-64/qmake.conf
*** qtbase/mkspecs/linux-icc-64/qmake.conf.orig
--- qtbase/mkspecs/linux-icc-64/qmake.conf
***************
*** 12,16 ****
  
  # Change the all LIBDIR variables to use lib64 instead of lib
  
! QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/X11R6/lib64
--- 12,18 ----
  
  # Change the all LIBDIR variables to use lib64 instead of lib
  
! QMAKE_LIBDIR_X11        = /usr/lib64
! QMAKE_LIBDIR_OPENGL     = $MESAGL_LIB_DIR $LLVM_LIB_DIR
! QMAKE_INCDIR_OPENGL     = $MESAGL_INCLUDE_DIR
! 
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 linux mesagl patch 2 failed."
        return 1
    fi

    patch -p0 <<EOF
diff -c qtbase/mkspecs/common/linux.conf.orig qtbase/mkspecs/common/linux.conf
*** qtbase/mkspecs/common/linux.conf.orig
--- qtbase/mkspecs/common/linux.conf
***************
*** 30,36 ****
  QMAKE_LIBS_DYNLOAD      = -ldl
  QMAKE_LIBS_X11          = -lXext -lX11 -lm
  QMAKE_LIBS_EGL          = -lEGL
! QMAKE_LIBS_OPENGL       = -lGL
  QMAKE_LIBS_OPENGL_ES2   = -lGLESv2
  QMAKE_LIBS_OPENVG       = -lOpenVG
  QMAKE_LIBS_THREAD       = -lpthread
--- 30,36 ----
  QMAKE_LIBS_DYNLOAD      = -ldl
  QMAKE_LIBS_X11          = -lXext -lX11 -lm
  QMAKE_LIBS_EGL          = -lEGL
! QMAKE_LIBS_OPENGL       = -lGL -lLLVM
  QMAKE_LIBS_OPENGL_ES2   = -lGLESv2
  QMAKE_LIBS_OPENVG       = -lOpenVG
  QMAKE_LIBS_THREAD       = -lpthread
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 linux mesagl patch 3 failed."
        return 1
    fi

    return 0
}

function apply_qt_5142_linux_opengl_patch
{
    info "Patching qt 5.14.2 for Linux and OpenGL"
    patch -p0 <<EOF
diff -c qtbase/mkspecs/linux-g++-64/qmake.conf.orig  qtbase/mkspecs/linux-g++-64/qmake.conf
*** qtbase/mkspecs/linux-g++-64/qmake.conf.orig
--- qtbase/mkspecs/linux-g++-64/qmake.conf
***************
*** 18,24 ****
  include(../common/g++-unix.conf)


! QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/X11R6/lib64

  load(qt_config)
--- 18,24 ----
  include(../common/g++-unix.conf)


! QMAKE_LIBDIR_X11        = /usr/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/lib64

  load(qt_config)

EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 linux opengl patch 1 failed."
        return 1
    fi

    patch -p0 <<EOF
diff -c qtbase/mkspecs/linux-icc-64/qmake.conf.orig qtbase/mkspecs/linux-icc-64/qmake.conf
*** qtbase/mkspecs/linux-icc-64/qmake.conf.orig
--- qtbase/mkspecs/linux-icc-64/qmake.conf
***************
*** 12,16 ****

  # Change the all LIBDIR variables to use lib64 instead of lib

! QMAKE_LIBDIR_X11        = /usr/X11R6/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/X11R6/lib64
--- 12,16 ----

  # Change the all LIBDIR variables to use lib64 instead of lib

! QMAKE_LIBDIR_X11        = /usr/lib64
! QMAKE_LIBDIR_OPENGL     = /usr/lib64
EOF

    if [[ $? != 0 ]] ; then
        warn "qt 5.14.2 linux opengl patch 2 failed."
        return 1
    fi

    return 0
}

function apply_qt_5142_macOS_1014_hunspell_patch
{   
    info "Patching qt 5.14.2 for macOS 10.14 hunspell"
    patch -p0 <<EOF
diff -c qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellworker_p.h.orig qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellworker_p.h
*** qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellworker_p.h.orig
--- qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellworker_p.h
***************
*** 48,54 ****
  #include <QSharedPointer>
  #include <QVector>
  #include <QLoggingCategory>
! #include <hunspell/hunspell.h>
  #include <QtHunspellInputMethod/qhunspellinputmethod_global.h>
  
  QT_BEGIN_NAMESPACE
--- 48,54 ----
  #include <QSharedPointer>
  #include <QVector>
  #include <QLoggingCategory>
! #include <hunspell.h>
  #include <QtHunspellInputMethod/qhunspellinputmethod_global.h>
  
  QT_BEGIN_NAMESPACE
EOF
    if [[ $? != 0 ]] ; then
        warn "Patching qt 5.14.2 for macOS 10.14 hunspell failed."
        return 1
    fi

    patch -p0 <<EOF
diff -c qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellinputmethod_p.cpp.orig qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellinputmethod_p.cpp
*** qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellinputmethod_p.cpp.orig
--- qtvirtualkeyboard/src/plugins/hunspell/hunspellinputmethod/hunspellinputmethod_p.cpp
***************
*** 29,35 ****
  
  #include <QtHunspellInputMethod/private/hunspellinputmethod_p_p.h>
  #include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
! #include <hunspell/hunspell.h>
  #include <QStringList>
  #include <QDir>
  #include <QTextCodec>
--- 29,35 ----
  
  #include <QtHunspellInputMethod/private/hunspellinputmethod_p_p.h>
  #include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
! #include <hunspell.h>
  #include <QStringList>
  #include <QDir>
  #include <QTextCodec>
EOF
    if [[ $? != 0 ]] ; then
        warn "Patching qt 5.14.2 for macOS 10.14 hunspell failed."
        return 1
    fi

    return 0
}

function apply_qt_5142_missing_cgcolorspace_header
{
    info "Patching qt 5.14.2 for macOS missing <CoreGraphics/CGColorSpace.h> header"
    patch -p0 << \EOF
--- qtbase/src/plugins/platforms/cocoa/qiosurfacegraphicsbuffer.h.orig	2023-04-14 17:11:54.000000000 -0700
+++ qtbase/src/plugins/platforms/cocoa/qiosurfacegraphicsbuffer.h	2023-04-14 17:12:08.000000000 -0700
@@ -40,6 +40,7 @@
 #ifndef QIOSURFACEGRAPHICSBUFFER_H
 #define QIOSURFACEGRAPHICSBUFFER_H
 
+#include <CoreGraphics/CGColorSpace.h>
 #include <qpa/qplatformgraphicsbuffer.h>
 #include <private/qcore_mac_p.h>
 
EOF
    if [[ $? != 0 ]] ; then
        warn "Patching qt 5.14.2 for macOS missing <CoreGraphics/CGColorSpace.h> header failed."
        return 1
    fi

    return 0

}

function build_qt
{
    #
    # Prepare the build dir using src file.
    #
    prepare_build_dir $QT_BUILD_DIR $QT_FILE
    untarred_qt=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ untarred_qt == -1 ]] ; then
        warn "Unable to prepare Qt build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching qt . . ."
    cd $QT_BUILD_DIR || error "Can't cd to Qt build dir."
    apply_qt_patch
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

    # move back up to the start dir
    cd "$START_DIR"

    # Make a build directory for an out-of-source build. Change the
    # QT_BUILD_DIR variable to represent the out-of-source build directory.
    QT_SRC_DIR=$QT_BUILD_DIR
    QT_BUILD_DIR="${QT_SRC_DIR}-build"
    if [[ ! -d $QT_BUILD_DIR ]] ; then
        echo "Making build directory $QT_BUILD_DIR"
        mkdir $QT_BUILD_DIR
    fi

    #
    # Handle case where python doesn't exist.
    # The magic to determine if python exist comes from
    # https://stackoverflow.com/questions/592620/how-can-i-check-if-a-program-exists-from-a-bash-script
    #
    pushd ${QT_SRC_DIR} > /dev/null 2>&1
    if ! command -v python > /dev/null 2>&1 ; then
        if [[ "$OPSYS" == "Darwin" ]]; then       
            sed -i orig -e 's/python/python3/' ./qtdeclarative/src/3rdparty/masm/masm.pri
            sed -i orig -e 's/python -c/python3 -c/' ./qtdeclarative/qtdeclarative.pro
        else
            sed -i.orig -e 's/python/python3/' ./qtdeclarative/src/3rdparty/masm/masm.pri
            sed -i.orig -e 's/python -c/python3 -c/' ./qtdeclarative/qtdeclarative.pro
        fi
    fi
    popd > /dev/null 2>&1

    cd ${QT_BUILD_DIR}

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
    #   
    if [[ "$OPSYS" == "Darwin" ]]; then       
        QT_PLATFORM="macx-clang"

    elif [[ "$OPSYS" == "AIX" ]]; then
        if [[ "$OBJECT_MODE" == 32 ]]; then
            QT_PLATFORM="aix-g++"
        else
            QT_PLATFORM="aix-g++-64"
        fi
    elif [[ "$OPSYS" == "SunOS" ]]; then
        if [[ "$OBJECT_MODE" == 32 ]]; then
            QT_PLATFORM="aix-solaris"
        else
            QT_PLATFORM="aix-solaris-64"
        fi
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

        # For OLD versions of linux, disable openssl
        VER=$(uname -r)
        if [[ "${VER:0:3}" == "2.4" ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-openssl"
        # For Fedora, disable openssl
        elif [[ -n "$(cat /proc/version 2>/dev/null | grep -i fedora)" ]]; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-openssl"
        fi
    fi

    QT_PLATFORM=${QT_PLATFORM:-"linux-g++"}

    # We may be building statically.
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        # Protect the build by NOT using the system versions of the I/O libraries.
        EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -static -qt-libtiff -qt-libpng -qt-libmng -qt-libjpeg"
    fi

    #
    # Call configure
    #

    QT_CFLAGS="${CFLAGS} ${C_OPT_FLAGS}"
    QT_CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}"

    qt_flags=""
    qt_flags="${qt_flags} -no-dbus"
    qt_flags="${qt_flags} -no-sql-db2 -no-sql-ibase -no-sql-mysql -no-sql-oci"
    qt_flags="${qt_flags} -no-sql-odbc -no-sql-psql -no-sql-sqlite"
    qt_flags="${qt_flags} -no-sql-sqlite2 -no-sql-tds"
    qt_flags="${qt_flags} -no-libjpeg"
    qt_flags="${qt_flags} -opensource"
    qt_flags="${qt_flags} -confirm-license"

    QT_VER_MSG="Qt5"
    qt_flags="${qt_flags} -skip qtwebengine"
    qt_flags="${qt_flags} -skip 3d"
    qt_flags="${qt_flags} -skip charts"
    qt_flags="${qt_flags} -skip connectivity"
    qt_flags="${qt_flags} -skip datavis3d"
    qt_flags="${qt_flags} -skip doc"
    qt_flags="${qt_flags} -skip gamepad"
    qt_flags="${qt_flags} -skip graphicaleffects"
    qt_flags="${qt_flags} -skip location"
    qt_flags="${qt_flags} -skip multimedia"
    qt_flags="${qt_flags} -skip networkauth"
    qt_flags="${qt_flags} -skip purchasing"
    qt_flags="${qt_flags} -skip quickcontrols"
    qt_flags="${qt_flags} -skip quickcontrols2"
    qt_flags="${qt_flags} -skip remoteobjects"
    qt_flags="${qt_flags} -skip scxml"
    qt_flags="${qt_flags} -skip sensors"
    qt_flags="${qt_flags} -skip serialport"
    qt_flags="${qt_flags} -skip speech"
    qt_flags="${qt_flags} -skip wayland"
    qt_flags="${qt_flags} -nomake examples"
    qt_flags="${qt_flags} -nomake tests"
    qt_flags="${qt_flags} -no-qml-debug"
    qt_flags="${qt_flags} -qt-zlib"
    qt_flags="${qt_flags} -qt-libpng"
    if [[ ${QT_VERSION} == 5.10.1 ]] ; then
        qt_flags="${qt_flags} -skip canvas3d"
    elif [[ ${QT_VERSION} == 5.14.2 ]] ; then
        qt_flags="${qt_flags} -skip activeqt"
        qt_flags="${qt_flags} -skip androidextras"
        qt_flags="${qt_flags} -skip lottie"
        qt_flags="${qt_flags} -skip quick3d"
        qt_flags="${qt_flags} -skip quicktimeline"
    fi

    if [[ ${QT_VERSION} == 5.10.1 ]] ; then
        if [[ "$OPSYS" == "Linux" ]] ; then
            qt_flags="${qt_flags} -qt-xcb -qt-xkbcommon"
        fi
    elif [[ ${QT_VERSION} == 5.14.2 ]] ; then
        if [[ "$OPSYS" == "Linux" ]] ; then
            qt_flags="${qt_flags} -qt-xcb --xkbcommon=yes -xcb-native-painting"
        fi
    fi

    if [[ "$VISIT_BUILD_MODE" == "Release" ]] ; then
        qt_flags="${qt_flags} -release"
    else
        qt_flags="${qt_flags} -debug"
    fi

    info "Configuring ${QT_VER_MSG}: "
    set -x
    (echo "o"; echo "yes") | CFLAGS="${QT_CFLAGS}" CXXFLAGS="${QT_CXXFLAGS}"  \
                                   ../${QT_SRC_DIR}/configure -prefix ${QT_INSTALL_DIR} \
                                   -platform ${QT_PLATFORM} \
                                   -make libs -make tools -no-separate-debug-info \
                                   ${qt_flags} | tee qt.config.out
    set +x
    if [[ $? != 0 ]] ; then
        warn "${QT_VER_MSG} configure failed. Giving up."
        return 1
    fi

    if [[ "$DO_MESAGL" == "yes" ]] ; then
        if [[ ${QT_VERSION} == 5.10.1 ]] ; then
            if [[ "$OPSYS" == "Linux" ]]; then
                sed -i 's/-o Makefile/-o Makefile -after "QMAKE_LIBS_OPENGL+=-lLLVM"/' Makefile
            fi
        fi
    fi

    #
    # When running Ubuntu 18 under WSL (Windows Subsystem for Linux), for
    # whatever reason, the libQt5Core.so library has something in it that
    # misidentifies the system and the file is not recognized. I first
    # build the library and then strip out the offending information.
    # I found the fix at https://github.com/microsoft/WSL/issues/3023.
    #
    if [[ "$OPSYS" == "Linux" ]] ; then
        if [[ "$REL" == *"Microsoft" ]] ; then
            cd qtbase
            bin/qmake -o Makefile qtbase.pro
            if [[ "${DO_QT_SILENT}" == "yes" ]] ; then
                $MAKE -s $MAKE_OPT_FLAGS
            else
                $MAKE $MAKE_OPT_FLAGS
            fi
            strip --remove-section=.note.ABI-tag lib/libQt5Core.so.5
            cd ..
        fi
    fi

    #
    # Build Qt. Config options above make sure we only build the libs & tools.
    #
    info "Building ${QT_VER_MSG} . . . (~60 minutes)"
    if [[ "${DO_QT_SILENT}" == "yes" ]] ; then
        $MAKE -s $MAKE_OPT_FLAGS
    else
        $MAKE $MAKE_OPT_FLAGS
    fi

    if [[ $? != 0 ]] ; then
        warn "${QT_VER_MSG} build failed.  Giving up"
        return 1
    fi

    info "Installing ${QT_VER_MSG} . . . "
    if [[ "${DO_QT_SILENT}" == "yes" ]] ; then
        $MAKE -s install
    else
        $MAKE install
    fi

    # Qt screws up permissions in some cases.  Try to fix that.
    chmod -R a+rX ${VISITDIR}/qt/${QT_VERSION}

    #
    # Visit expects .so suffix on qt libs but xlc uses .a suffixe even 
    # for shared libs (however subversioned qts libs end up with a .so suffix)
    #
    # Fix this by creating .so simlinks to the .a versions
    #

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "AIX" ]]; then
        cd ${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}/lib/
        for f in *.a; do ln -s $f ${f%\.*}.so; done

    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/qt"
        chgrp -R ${GROUP} "$VISITDIR/qt"
    fi

    cd "$START_DIR"
    info "Done with ${QT_VER_MSG}"

    return 0
}

function bv_qt_is_enabled
{
    if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]]; then
        return 0
    fi 
    if [[ $DO_QT == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_qt_is_installed
{
    if [[ "$USE_SYSTEM_QT" == "yes" ]]; then
        return 1    
    fi

    check_if_installed "qt" $QT_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_qt_build
{
    #
    # Build Qt
    #
    cd "$START_DIR"
    if [[ "$DO_QT" == "yes"  && "$USE_SYSTEM_QT" == "no" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
        check_if_installed "qt" $QT_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Qt build.  Qt is already installed."
        else
            info "Building Qt (~60 minutes)"
            build_qt
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Qt.  Bailing out."
            fi
            info "Done building Qt"
        fi
    fi
}
