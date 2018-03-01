function bv_qt_initialize
{
    export DO_QT="yes"
    export FORCE_QT="no"
    export USE_SYSTEM_QT="no"
    # NOTE: IS_QT4 is used in bv_pyside.sh
    export IS_QT4="no"
    add_extra_commandline_args "qt" "qt4" 0 "Build Qt4 instead of Qt5"
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

    IS_QT4="no"
    if [[ "${QT_VERSION%%.*}" == "4" ]]; then
        IS_QT4="yes"
    fi 
}

function bv_qt_system_qt
{
    echo "using system qt"

    QTEXEC="qmake"
    TEST=`which $QTEXEC`
    if [[ $? != 0 ]]; then
        QTEXEC="qmake-qt4"
        TEST=`which $QTEXEC`
        [ $? != 0 ] && error "System Qt not found"
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
        QTEXEC="qmake-qt4"
        [ ! -e "$1/bin/$QTEXEC" ] && error "qmake was not found in directory: $1/bin"
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
    echo ""
}

function bv_qt_qt4
{
    bv_qt_enable

    # if we are on osx 10.8 or later, we need to use 4.8.6
    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "${MACOSX_DEPLOYMENT_TARGET}" == "10.8" ||
              "${MACOSX_DEPLOYMENT_TARGET}" == "10.9" ||
              "${MACOSX_DEPLOYMENT_TARGET}" == "10.10" ||
              "${MACOSX_DEPLOYMENT_TARGET}" == "10.11" ||
              "${MACOSX_DEPLOYMENT_TARGET}" == "10.12" ]]; then
            QT_VERSION="4.8.6"
            QT_FILE="qt-everywhere-opensource-src-${QT_VERSION}.tar.gz"
            QT_MD5_CHECKSUM="2edbe4d6c2eff33ef91732602f3518eb"
            QT_SHA256_CHECKSUM=""
        fi
    else
        QT_VERSION="4.8.3"
        QT_FILE="qt-everywhere-opensource-src-${QT_VERSION}.tar.gz"
        QT_MD5_CHECKSUM="a663b6c875f8d7caa8ac9c30e4a4ec3b"
        QT_SHA256_CHECKSUM=""
    fi
    
    QT_BUILD_DIR="${QT_FILE%.tar*}"
    QT_BIN_DIR="${QT_BUILD_DIR}/bin"

    info "enabling Qt${QT_VERSION}..."

    IS_QT4="yes"
}

function bv_qt_info
{
    bv_qt_enable

#    Note: Qt 5.8 is not yet compatible with Pyside2.
#    export QT_VERSION=${QT_VERSION:-"5.8.0"}
    
    export QT_VERSION=${QT_VERSION:-"5.6.1"}
    export QT_FILE=${QT_FILE:-"qt-everywhere-opensource-src-${QT_VERSION}.tar.gz"}
    export QT_MD5_CHECKSUM=${QT_MD5_CHECKSUM:-"a9f2494f75f966e2f22358ec367d8f41"}
    export QT_SHA256_CHECKSUM=${QT_SHA256_CHECKSUM:-"9dc5932307ae452855863f6405be1f7273d91173dcbe4257561676a599bd58d3"}

    export QT_BUILD_DIR=${QT_BUILD_DIR:-"${QT_FILE%.tar*}"}
    export QT_BIN_DIR=${QT_BIN_DIR:-"${QT_BUILD_DIR}/bin"}
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
    printf "%-15s %s [%s]\n" "--qt" "Build Qt4" "built by default unless --no-thirdparty flag is used"
    printf "%-15s %s [%s]\n" "--qt4" "Build Qt4 instead of Qt5" "$IS_QT4"
    printf "%-15s %s [%s]\n" "--system-qt" "Use the system installed Qt"
    printf "%-15s %s [%s]\n" "--alt-qt-dir" "Use Qt from alternative directory"
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
                    if [[ "$IS_QT4" == "yes" ]]; then
                        echo "VISIT_OPTION_DEFAULT(VISIT_QT4 ON TYPE BOOL)" >> $HOSTCONF
                    else 
                        echo "VISIT_OPTION_DEFAULT(VISIT_QT5 ON TYPE BOOL)" >> $HOSTCONF
                    fi
                    
                    echo "VISIT_OPTION_DEFAULT(VISIT_QT_DIR \${VISITHOME}/qt/\${QT_VERSION}/\${VISITARCH})" >> $HOSTCONF
                    echo "VISIT_OPTION_DEFAULT(VISIT_QT_BIN \${VISIT_QT_DIR}/bin)" >> $HOSTCONF
                fi
            fi
        fi
    fi
}

function bv_qt_ensure
{
    if [[ "$DO_QT" == "yes"  && "$USE_SYSTEM_QT" == "no" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
        ensure_built_or_ready "qt"     $QT_VERSION    $QT_BUILD_DIR    $QT_FILE
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}

function bv_qt_dry_run
{
    if [[ "$DO_QT" == "yes" ]] ; then
        echo "Dry run option not set for qt."
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

    QT_CONFIRM_MSG="VisIt requires Qt: Please respond with \"yes\" to accept\
                Qt licensing under the terms of the Lesser GNU General \
                Public License (LGPL) version 2.1 or \
                the GNU General Public License (GPL) version 3"
    info $QT_LIC_MSG
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
    if [[ ${QT_VERSION} == 4.8.6 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            if [[ "${MACOSX_DEPLOYMENT_TARGET}" == "10.10" ||
                  "${MACOSX_DEPLOYMENT_TARGET}" == "10.11" ||
                  "${MACOSX_DEPLOYMENT_TARGET}" == "10.12" ]]; then
                apply_qt_486_osx10_patch 
            fi
        fi
    fi

    if [[ ${QT_VERSION} == 5.6.1 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then

            XCODE_VERSION="$(/usr/bin/xcodebuild -version)"
#           info ${XCODE_VERSION}
            if [[ "$XCODE_VERSION" == "Xcode 8"* ]]; then
                apply_qt_561_osx_xcode_8_patch
            fi
        fi
    elif [[ ${QT_VERSION} == 5.8.0 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            apply_qt_580_osx_patch
        fi
    fi

    return 0
}

function apply_qt_486_osx10_patch
{
    # fix for OS X 10.10
    info "Patching qt 4.8.6 for OS X 10.10 and above"
    patch -p0 << \EOF

diff -c src/gui/painting/qpaintengine_mac.cpp.orig src/gui/painting/qpaintengine_mac.cpp
*** src/gui/painting/qpaintengine_mac.cpp.orig  2014-04-10 12:37:12.000000000 -0600
--- src/gui/painting/qpaintengine_mac.cpp       2016-01-05 15:43:29.000000000 -0700
***************
*** 340,352 ****
      }
  
      // Get the color space from the display profile.
!     CGColorSpaceRef colorSpace = 0;
!     CMProfileRef displayProfile = 0;
!     CMError err = CMGetProfileByAVID((CMDisplayIDType)displayID, &displayProfile);
!     if (err == noErr) {
!         colorSpace = CGColorSpaceCreateWithPlatformColorSpace(displayProfile);
!         CMCloseProfile(displayProfile);
!     }
  
      // Fallback: use generic DeviceRGB
      if (colorSpace == 0)
--- 340,346 ----
      }
  
      // Get the color space from the display profile.
!     CGColorSpaceRef colorSpace = CGDisplayCopyColorSpace(displayID);
  
      // Fallback: use generic DeviceRGB
      if (colorSpace == 0)

EOF
    if [[ $? != 0 ]] ; then
        warn "qt 4.8.6 patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_561_osx_xcode_8_patch
{
    # fix for OS X 10.11 or 10.12 with Xcode 8
    info "Patching qt 5.6.1 for OS X and Xcode 8"
    patch -p0 << \EOF
diff -c  qtbase/configure.orig qtbase/configure
*** qtbase/configure.orig       2017-10-16 15:48:39.000000000 -0600
--- qtbase/configure    2017-10-16 15:48:54.000000000 -0600
***************
*** 543,549 ****
          exit 2
      fi
  
!     if ! /usr/bin/xcrun -find xcrun >/dev/null 2>&1; then
          echo >&2
          echo "   Xcode not set up properly. You may need to confirm the license" >&2
          echo "   agreement by running /usr/bin/xcodebuild without arguments." >&2
--- 543,549 ----
          exit 2
      fi
  
!     if ! /usr/bin/xcrun -find xcodebuild >/dev/null 2>&1; then
          echo >&2
          echo "   Xcode not set up properly. You may need to confirm the license" >&2
          echo "   agreement by running /usr/bin/xcodebuild without arguments." >&2

diff -c qtbase/mkspecs/features/mac/default_pre.prf.orig qtbase/mkspecs/features/mac/default_pre.prf
*** qtbase/mkspecs/features/mac/default_pre.prf.orig    2017-10-16 15:33:57.000000000 -0600
--- qtbase/mkspecs/features/mac/default_pre.prf 2017-10-16 15:35:02.000000000 -0600
***************
*** 12,18 ****
          error("Xcode is not installed in $${QMAKE_XCODE_DEVELOPER_PATH}. Please use xcode-select to choose Xcode installation path.")
  
      # Make sure Xcode is set up properly
!     isEmpty($$list($$system("/usr/bin/xcrun -find xcrun 2>/dev/null"))): \
          error("Xcode not set up properly. You may need to confirm the license agreement by running /usr/bin/xcodebuild.")
  }
  
--- 12,18 ----
          error("Xcode is not installed in $${QMAKE_XCODE_DEVELOPER_PATH}. Please use xcode-select to choose Xcode installation path.")
  
      # Make sure Xcode is set up properly
!     isEmpty($$list($$system("/usr/bin/xcrun -find xcodebuild 2>/dev/null"))): \
          error("Xcode not set up properly. You may need to confirm the license agreement by running /usr/bin/xcodebuild.")
  }
  
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.6.1 patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_580_osx_patch
{
    # fix for OS X 10.11 
    info "Patching qt 5.8.0 for OS X"
    patch -p0 << \EOF

diff -c qtbase/mkspecs/features/qt_module.prf.orig qtbase/mkspecs/features/qt_module.prf
*** qtbase/mkspecs/features/qt_module.prf.orig  Wed Jan 18 06:20:58 2017
--- qtbase/mkspecs/features/qt_module.prf       Thu Apr 20 07:42:05 2017
***************
*** 68,76 ****
  
  header_module {
      TEMPLATE     = aux
!     CONFIG      += \
!         force_qt \  # Needed for the headers_clean tests.
!         qt_no_install_library
  } else {
      TEMPLATE     = lib
  }
--- 68,76 ----
  
  header_module {
      TEMPLATE     = aux
!     CONFIG      += force_qt  # Needed for the headers_clean tests.
!     !lib_bundle: \
!         CONFIG += qt_no_install_library 
  } else {
      TEMPLATE     = lib
  }

EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.8.0 patch failed."
        return 1
    fi

    return 0;
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

    #
    # Platform specific configuration
    #
    if [[ "$OPSYS" == "Darwin" ]] ; then
        # Determine if we build with Cocoa
        VER=$(uname -r)
        if [[ ${VER%%.*} -ge 10 ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -cocoa"
        fi

        QT_PLATFORM=${QT_PLATFORM:-"macx-g++"}
        # webkit causes the linker on Hank's mac to run out of memory
        # Hari: for Qt5 I have disabled webkit all together
        if [[ "$IS_QT4" == "yes" ]]; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-webkit -no-phonon -no-phonon-backend"
        fi
        # Figure out whether we need to build 64-bit version of Qt
        echo "int main() {}" >> arch_test.c
        ${C_COMPILER} arch_test.c -o arch_test
        GCC_BUILD_ARCH=$(lipo -info arch_test | sed -e 's/[^x]*//')
        rm arch_test.c arch_test
        if [[ "$GCC_BUILD_ARCH" == "x86_64" ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -arch x86_64"
        fi

    elif [[ "$OPSYS" == "Linux" ]] ; then
        # w/ Qt 4.8.3, these guys will on fail on linux
        # if gstreamer isn't installed ...
        if [[ "$IS_QT4" == "yes" ]]; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-webkit -no-phonon -no-phonon-backend"
        fi
        
        # For OLD versions of linux, disable openssl
        VER=$(uname -r)
        if [[ "${VER:0:3}" == "2.4" ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-openssl"
        # For Fedora, disable openssl
        elif [[ -n "$(cat /proc/version 2>/dev/null | grep -i fedora)" ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-openssl"
        fi

        #
        # select the proper value for QT_PLATFORM 
        #
        # Most of the logic for QT_PLATFORM on various os flavors
        # is still in bv_main.sh, and should be detangled and placed here.
        #
        # For now add support for icc on linux.
        #

        # enable icc qt build on linux
        #
        # Question: could qt auto detect this via the CC and CXX env vars?
        #
        # For osx, and linux - we may also need clang support in the future.
        # We should try to see if we can avoid setting the platform, set
        # CC and CXX and see if that is enough to trigger qt's detection logic.
        #
        #
        if [[ "$(uname -m)" == "x86_64" ]] ; then
            if [[ "$C_COMPILER" == "icc" || "$CXX_COMPILER" == "icpc" ]]; then
                QT_PLATFORM="linux-icc-64"
            else
                QT_PLATFORM="linux-g++-64"
            fi
        else
            if [[ "$C_COMPILER" == "icc" || "$CXX_COMPILER" == "icpc" ]]; then
                QT_PLATFORM="linux-icc"
            else
                QT_PLATFORM="linux-g++"
            fi
        fi
    fi

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

    if [[ $IS_QT4 == "yes" ]]; then
        QT_VER_MSG="Qt4"
        qt_flags="${qt_flags} -fast"
        qt_flags="${qt_flags} -no-libtiff"
        qt_flags="${qt_flags} -no-qt3support"
        qt_flags="${qt_flags} -nomake docs"
        qt_flags="${qt_flags} -nomake demos"
        qt_flags="${qt_flags} -nomake examples"
        qt_flags="${qt_flags} ${EXTRA_QT_FLAGS}"
    else
        QT_VER_MSG="Qt5"
        qt_flags="${qt_flags} -skip 3d"
        qt_flags="${qt_flags} -skip sensors"
        qt_flags="${qt_flags} -skip doc"
        qt_flags="${qt_flags} -skip serialport"
        qt_flags="${qt_flags} -skip quickcontrols"
        qt_flags="${qt_flags} -skip quickcontrols2"
        qt_flags="${qt_flags} -skip connectivity"
        qt_flags="${qt_flags} -skip multimedia"
        qt_flags="${qt_flags} -nomake examples"
        qt_flags="${qt_flags} -nomake tests"
        qt_flags="${qt_flags} -no-qml-debug"

        if [[ "$OPSYS" == "Linux" ]] ; then
            qt_flags="${qt_flags} -qt-xcb -qt-xkbcommon"
        fi
    fi

    info "Configuring ${QT_VER_MSG}: " \
         "CFLAGS=${QT_CFLAGS} CXXFLAGS=${QT_CXXFLAGS}" \
         "./configure -prefix ${QT_INSTALL_DIR}" \
         "-platform ${QT_PLATFORM}" \
         "-make libs -make tools -no-separate-debug-info" \
         "${qt_flags}" 

    (echo "o"; echo "yes") | CFLAGS="${QT_CFLAGS}" CXXFLAGS="${QT_CXXFLAGS}"  \
                                   ./configure -prefix ${QT_INSTALL_DIR} \
                                   -platform ${QT_PLATFORM} \
                                   -make libs -make tools -no-separate-debug-info \
                                   ${qt_flags} | tee qt.config.out
    if [[ $? != 0 ]] ; then
        warn "${QT_VER_MSG} configure failed. Giving up."
        return 1
    fi

    #
    # Figure out if configure found the OpenGL libraries
    #
    if [[ "${DO_DBIO_ONLY}" != "yes" && "${DO_ENGINE_ONLY}" != "yes" && "${DO_SERVER_COMPONENTS_ONLY}" != "yes" ]] ; then
        HAS_OPENGL_SUPPORT=`grep "OpenGL support" qt.config.out | sed -e 's/.*\. //'  | cut -c 1-3`
        if [[ "$IS_QT4" == "yes" && "$HAS_OPENGL_SUPPORT" != "yes" ]]; then
            warn "Qt4 configure did not find OpenGL." \
                 "VisIt needs Qt4 with enabled OpenGL support. Giving up.\n" \
                 "Here are some common reasons why Qt will not build with GL support.\n" \
                 "\t- The OpenGL development environment is not installed.\n" \
                 "\t  (You can check this by searching for /usr/include/GL/GL.h)\n" \
                 "\t- libGLU is not available\n"\
                 "\t- libGLU is available, but only as a shared library\n"\
                 "You can learn more about exactly why Qt failed by doing the following:\n"\
                 "\t- cd $QT_BUILD_DIR\n" \
                 "\t- ./configure -opengl -verbose\n" \
                 "\t  (this will produce the details of the failed OpenGL tests.)\n" \
                 "\t  (also note you will need to respond with \"o\" to opt for\n" \
                 "\t   the open source license and \"yes\" to accept.)\n"
            return 1
        fi
    fi

    #
    # Build Qt4. Config options above make sure we only build the libs & tools.
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
            info "Skipping Qt build.  Qt4 is already installed."
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
