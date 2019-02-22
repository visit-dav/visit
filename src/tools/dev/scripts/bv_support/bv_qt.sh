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
    echo ""
}


function bv_qt_info
{
    bv_qt_enable

    export QT_VERSION=${QT_VERSION:-"5.10.1"}
    export QT_FILE=${QT_FILE:-"qt-everywhere-src-${QT_VERSION}.tar.xz"}
    export QT_MD5_CHECKSUM=${QT_MD5_CHECKSUM:-"7e167b9617e7bd64012daaacb85477af"}
    export QT_SHA256_CHECKSUM=${QT_SHA256_CHECKSUM:-"05ffba7b811b854ed558abf2be2ddbd3bb6ddd0b60ea4b5da75d277ac15e740a"}

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
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        if [[ ${QT_VERSION} == 5.10.1 ]] ; then
            if [[ "$OPSYS" == "Linux" ]]; then
                apply_qt_5101_linux_mesagl_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi
    fi

    if [[ ${QT_VERSION} == 5.6.1 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then

            XCODE_VERSION="$(/usr/bin/xcodebuild -version)"
#           info ${XCODE_VERSION}
            if [[ "$XCODE_VERSION" == "Xcode 8"* ||
                  "$XCODE_VERSION" == "Xcode 9"* ]]; then
                apply_qt_561_osx_xcode_8_patch
            fi

            if [[ "${MACOSX_DEPLOYMENT_TARGET}" == "10.13" ]]; then 
                    if [[ "$XCODE_VERSION" == "Xcode 9"* ]]; then
                            apply_qt_561_osx10_13_patch
                    fi
            fi
        fi
    elif [[ ${QT_VERSION} == 5.8.0 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            apply_qt_580_osx_patch
        fi
    fi

    return 0
}

function apply_qt_5101_linux_mesagl_patch
{   
    info "Patching qt 5.10.1 for Linux and Mesa-as-GL"
    patch -p0 << EOF
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
    ! QMAKE_LIBDIR_OPENGL=$MESAGL_LIB_DIR
    ! QMAKE_INCDIR_OPENGL=$MESAGL_INCLUDE_DIR
  
      load(qt_config)
EOF 
    if [[ $? != 0 ]] ; then
        warn "qt 5.10.1 linux conf patch 1 failed."
        return 1
    fi
    
    return 0;
}


function apply_qt_561_osx10_13_patch
{
    info "Patching qt 5.6.1 for OS X 10.13 and Xcode 9"
    patch -p0 << \EOF
diff -c qtbase/src/plugins/platforms/cocoa/orig/qcocoahelpers.h qtbase/src/plugins/platforms/cocoa/qcocoahelpers.h
*** qtbase/src/plugins/platforms/cocoa/orig/qcocoahelpers.h	Thu Aug 23 20:59:43 2018
--- qtbase/src/plugins/platforms/cocoa/qcocoahelpers.h	Thu Aug 23 19:50:04 2018
***************
*** 78,84 ****
  // Creates a mutable shape, it's the caller's responsibility to release.
  HIMutableShapeRef qt_mac_QRegionToHIMutableShape(const QRegion &region);

! OSStatus qt_mac_drawCGImage(CGContextRef inContext, const CGRect *inBounds, CGImageRef inImage);

  NSDragOperation qt_mac_mapDropAction(Qt::DropAction action);
  NSDragOperation qt_mac_mapDropActions(Qt::DropActions actions);
--- 78,84 ----
  // Creates a mutable shape, it's the caller's responsibility to release.
  HIMutableShapeRef qt_mac_QRegionToHIMutableShape(const QRegion &region);

! void qt_mac_drawCGImage(CGContextRef inContext, const CGRect *inBounds, CGImageRef inImage);

  NSDragOperation qt_mac_mapDropAction(Qt::DropAction action);
  NSDragOperation qt_mac_mapDropActions(Qt::DropActions actions);

diff -c qtbase/src/plugins/platforms/cocoa/orig/qcocoahelpers.mm qtbase/src/plugins/platforms/cocoa/qcocoahelpers.mm
*** qtbase/src/plugins/platforms/cocoa/orig/qcocoahelpers.mm	Thu Aug 23 20:59:43 2018
--- qtbase/src/plugins/platforms/cocoa/qcocoahelpers.mm	Thu Aug 23 19:52:25 2018
***************
*** 539,553 ****
      return NSMakeRect(rect.x(), flippedY, rect.width(), rect.height());
  }

! OSStatus qt_mac_drawCGImage(CGContextRef inContext, const CGRect *inBounds, CGImageRef inImage)
  {
-     // Verbatim copy if HIViewDrawCGImage (as shown on Carbon-Dev)
-     OSStatus err = noErr;
-
-     require_action(inContext != NULL, InvalidContext, err = paramErr);
-     require_action(inBounds != NULL, InvalidBounds, err = paramErr);
-     require_action(inImage != NULL, InvalidImage, err = paramErr);
-
      CGContextSaveGState( inContext );
      CGContextTranslateCTM (inContext, 0, inBounds->origin.y + CGRectGetMaxY(*inBounds));
      CGContextScaleCTM(inContext, 1, -1);
--- 539,546 ----
      return NSMakeRect(rect.x(), flippedY, rect.width(), rect.height());
  }

! void qt_mac_drawCGImage(CGContextRef inContext, const CGRect *inBounds, CGImageRef inImage)
  {
      CGContextSaveGState( inContext );
      CGContextTranslateCTM (inContext, 0, inBounds->origin.y + CGRectGetMaxY(*inBounds));
      CGContextScaleCTM(inContext, 1, -1);
***************
*** 555,564 ****
      CGContextDrawImage(inContext, *inBounds, inImage);

      CGContextRestoreGState(inContext);
- InvalidImage:
- InvalidBounds:
- InvalidContext:
-         return err;
  }

  Qt::MouseButton cocoaButton2QtButton(NSInteger buttonNum)
--- 548,553 ----
EOF
    if [[ $? != 0 ]] ; then
        warn "qt 5.6.1 OSX 10.13 patch failed."
        return 1
    fi

    return 0;
}

function apply_qt_561_osx_xcode_8_patch
{
    # fix for OS X 10.11 or 10.12 with Xcode 8
    info "Patching qt 5.6.1 for OS X and Xcode 8 or Xcode 9"
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
    qt_flags="${qt_flags} -skip 3d"
    qt_flags="${qt_flags} -skip canvas3d"
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

    if [[ "$OPSYS" == "Linux" ]] ; then
        qt_flags="${qt_flags} -qt-xcb -qt-xkbcommon"
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
    #if [[ "${DO_DBIO_ONLY}" != "yes" && "${DO_ENGINE_ONLY}" != "yes" && "${DO_SERVER_COMPONENTS_ONLY}" != "yes" ]] ; then
    #    HAS_OPENGL_SUPPORT=`grep "OpenGL support" qt.config.out | sed -e 's/.*\. //'  | cut -c 1-3`
    #    if [[ "$HAS_OPENGL_SUPPORT" != "yes" ]]; then
    #        warn "Qt configure did not find OpenGL." \
    #             "VisIt needs Qt with enabled OpenGL support. Giving up.\n" \
    #             "Here are some common reasons why Qt will not build with GL support.\n" \
    #             "\t- The OpenGL development environment is not installed.\n" \
    #             "\t  (You can check this by searching for /usr/include/GL/GL.h)\n" \
    #             "\t- libGLU is not available\n"\
    #             "\t- libGLU is available, but only as a shared library\n"\
    #             "You can learn more about exactly why Qt failed by doing the following:\n"\
    #             "\t- cd $QT_BUILD_DIR\n" \
    #             "\t- ./configure -opengl -verbose\n" \
    #             "\t  (this will produce the details of the failed OpenGL tests.)\n" \
    #             "\t  (also note you will need to respond with \"o\" to opt for\n" \
    #             "\t   the open source license and \"yes\" to accept.)\n"
    #        return 1
    #    fi
    #fi

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
