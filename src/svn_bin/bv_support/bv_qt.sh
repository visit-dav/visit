function bv_qt_initialize
{
    export DO_QT="yes"
    export ON_QT="on"
    export FORCE_QT="no"
    export USE_SYSTEM_QT="no"
    add_extra_commandline_args "qt" "system-qt" 0 "Use qt found on system"
    add_extra_commandline_args "qt" "alt-qt-dir" 1 "Use qt found in alternative directory"
}

function bv_qt_enable
{
DO_QT="yes"
ON_QT="on"
FORCE_QT="yes"
}

function bv_qt_disable
{
DO_QT="no"
ON_QT="off"
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
   echo "Using System Qt"

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
    info "Using qt from alternative directory $1"

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
      QT_INSTALL_DIR="${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}/"
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
export QT_FILE=${QT_FILE:-"qt-everywhere-opensource-src-4.8.3.tar.gz"}
export QT_VERSION=${QT_VERSION:-"4.8.3"}
export QT_BUILD_DIR=${QT_BUILD_DIR:-"${QT_FILE%.tar*}"}
export QT_BIN_DIR="${QT_BUILD_DIR}/bin"
export QT_MD5_CHECKSUM="a663b6c875f8d7caa8ac9c30e4a4ec3b"
export QT_SHA256_CHECKSUM=""
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
printf "%-15s %s [%s]\n" "--qt" "Build Qt" "built by default unless --no-thirdparty flag is used"
printf "%-15s %s [%s]\n" "--system-qt" "Use System Qt" "Used by default unless --no-thirdparty flag is used"
printf "%-15s %s [%s]\n" "--alt-qt-dir" "Use Qt from alternative directory" "Used by default unless --no-thirdparty flag is used"
}

function bv_qt_host_profile
{
echo >> $HOSTCONF
echo "##" >> $HOSTCONF
echo "## Qt" >> $HOSTCONF
echo "##" >> $HOSTCONF
if [[ $USE_SYSTEM_QT == "yes" ]]; then
    echo "VISIT_OPTION_DEFAULT(QT_QTUITOOLS_INCLUDE_DIR ${QT_QTUITOOLS_INCLUDE_DIR})" >> $HOSTCONF
    echo "VISIT_OPTION_DEFAULT(VISIT_QT_BIN ${QT_BIN_DIR})" >> $HOSTCONF
else
    echo "VISIT_OPTION_DEFAULT(VISIT_QT_BIN \${VISITHOME}/qt/$QT_VERSION/\${VISITARCH}/bin)" >> $HOSTCONF
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
            that you accept Trolltech's license for the Qt4 Open Source\
            Edition. Please respond \"yes\" to accept (in advance) either\
            the Lesser GNU General Public License (LGPL) version 2.1 or \
            the GNU General Public License (GPL) version 3. Visit\
            http://trolltech.com/products/appdev/licensing to view these\
            licenses."

QT_CONFIRM_MSG="VisIt requires Qt4: Please respond with \"yes\" to accept\
                Qt licensing under the terms of the Lesser GNU General \
                Public License (LGPL) version 2.1 or \
                the GNU General Public License (GPL) version 3"
if [[ "$GRAPHICAL" == "yes" ]] ; then
    if [[ "$REDIRECT_ACTIVE" == "yes" ]] ; then
        $DLG --backtitle "$DLG_BACKTITLE" --yesno "$QT_LIC_MSG" 0 0 1>&3
    else
        $DLG --backtitle "$DLG_BACKTITLE" --yesno "$QT_LIC_MSG" 0 0 
    fi
    if [[ $? == 1 ]] ; then
        if [[ "$REDIRECT_ACTIVE" == "yes" ]] ; then
            $DLG --backtitle "$DLG_BACKTITLE" --yesno "$QT_CONFIRM_MSG" 0 0 1>&3
        else
            $DLG --backtitle "$DLG_BACKTITLE" --yesno "$QT_CONFIRM_MSG" 0 0 
        fi
        if [[ $? == 1 ]] ; then
            return 1
        fi
    fi
else
    info $QT_LIC_MSG
    read RESPONSE
    if [[ "$RESPONSE" != "yes" ]] ; then
        info $QT_CONFIRM_MSG
        read RESPONSE
        if [[ $RESPONSE != "yes" ]] ; then
            return 1
        fi
    fi
fi

return 0
}


function apply_qt_patch
{
   return 0
}

function build_qt
{
    #
    # Prepare the build dir using src file.
    #

    prepare_build_dir $QT_BUILD_DIR $QT_FILE
    untarred_qt=$?
    # 0, already exists, 1  untarred src, 2 error

    if [[ untarred_qt == -1 ]] ; then
       warn "Unable to prepare Qt4 build directory. Giving Up!"
       return 1
    fi

    #
    # Apply patches
    #
    info "Patching qt . . ."
    apply_qt_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_qt == 1 ]] ; then
          warn "Giving up on Qt build because the patch failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script\n" \
               "tried to apply a patch to an existing directory which had " \
               "already been patched ... that is, that the patch is " \
               "failing harmlessly on a second application."
       fi
    fi

    cd $QT_BUILD_DIR || error "Can't cd to Qt build dir."

    #
    # Platform specific configuration
    #
    if [[ "$OPSYS" == "Darwin" ]] ; then
        # Determine if we build with Cocoa
        VER=$(uname -r)
        if [[ ${VER%%.*} -ge 10 ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -cocoa"
        fi

        # webkit causes the linker on Hank's mac to run out of memory
        EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-webkit -no-phonon -no-phonon-backend"

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
        EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-webkit -no-phonon -no-phonon-backend"
        # For OLD versions of linux, disable openssl
        VER=$(uname -r)
        if [[ "${VER:0:3}" == "2.4" ]] ; then
            EXTRA_QT_FLAGS="$EXTRA_QT_FLAGS -no-openssl"
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
    qt_flags=""
    qt_flags="${qt_flags} -no-qt3support"
    qt_flags="${qt_flags} -no-dbus"
    qt_flags="${qt_flags} -no-sql-db2 -no-sql-ibase -no-sql-mysql -no-sql-oci"
    qt_flags="${qt_flags} -no-sql-odbc -no-sql-psql -no-sql-sqlite"
    qt_flags="${qt_flags} -no-sql-sqlite2 -no-sql-tds"
    qt_flags="${qt_flags} -no-libtiff"
    qt_flags="${qt_flags} -no-libjpeg"
    qt_flags="${qt_flags} -nomake docs"
    qt_flags="${qt_flags} -nomake examples"
    qt_flags="${qt_flags} -nomake demos"
    qt_flags="${qt_flags} -opensource"
    qt_flags="${qt_flags} -confirm-license"
    info "Configuring Qt4: ./configure --prefix=${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}/" \
         "-platform ${QT_PLATFORM}" \
         "-make libs -make tools -fast -no-separate-debug-info" \
         "${qt_flags}" \
         "${EXTRA_QT_FLAGS}"
   (echo "o"; echo "yes") | ./configure --prefix=${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}/ \
                    -platform ${QT_PLATFORM} \
                    -make libs -make tools -fast -no-separate-debug-info \
                    ${qt_flags} \
                    ${EXTRA_QT_FLAGS} | tee qt.config.out
    
    if [[ $? != 0 ]] ; then
       warn "Qt4 configure failed. Giving up."
       return 1
    fi

    #
    # Figure out if configure found the OpenGL libraries
    #
    if [[ "${DO_DBIO_ONLY}" != "yes" && "${DO_ENGINE_ONLY}" != "yes" && "${DO_SERVER_COMPONENTS_ONLY}" != "yes" ]] ; then
       HAS_OPENGL_SUPPORT=`grep "OpenGL support" qt.config.out | sed -e 's/.*\. //'  | cut -c 1-3`
       if [[ "$HAS_OPENGL_SUPPORT" != "yes" ]]; then
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
    info "Building Qt4 . . . (~60 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Qt4 build failed.  Giving up"
       return 1
    fi

    info "Installing Qt4 . . . "
    $MAKE install

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
    info "Done with Qt4"

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
        info "Skipping Qt4 build.  Qt4 is already installed."
   else
      info "Building Qt4 (~60 minutes)"
      build_qt
      if [[ $? != 0 ]] ; then
         error "Unable to build or install Qt4.  Bailing out."
      fi
      info "Done building Qt4"
   fi
fi
}

