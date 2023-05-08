
function bv_qt6_initialize
{
    export DO_QT6="no"
}

function bv_qt6_enable
{ 
    DO_QT6="yes"
}

function bv_qt6_disable
{
    DO_QT6="no"
}

function bv_qt6_depends_on
{
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        echo "mesagl glu"
    else
        echo ""
    fi
}

function bv_qt6_info
{
    export QT6_VERSION=${QT6_VERSION:-"6.4.2"}
    export QT6_SHORT_VERSION=${QT6_SHORT_VERSION:-"6.4"}
    export QT6_URL=${QT6_URL:-"https://download.qt.io/archive/qt/${QT6_SHORT_VERSION}/${QT6_VERSION}/submodules"}
    export QT6_BASE_FILE=${QT6_BASE_FILE:-"qtbase-everywhere-src-${QT6_VERSION}.tar.xz"}
    export QT6_BASE_SOURCE_DIR=${QT6_BASE_SOURCE_DIR:-"qtbase-everywhere-src-${QT6_VERSION}"}
    export QT6_BASE_SHA256_CHECKSUM="a88bc6cedbb34878a49a622baa79cace78cfbad4f95fdbd3656ddb21c705525d"

    # Other submodules
    export QT6_TOOLS_FILE=${QT6_TOOLS_FILE:-"qttools-everywhere-src-${QT6_VERSION}.tar.xz"}
    export QT6_TOOLS_SOURCE_DIR=${QT6_TOOLS_SOURCE_DIR:-"qttools-everywhere-src-${QT6_VERSION}"}
    export QT6_TOOLS_SHA256_CHECKSUM="a31387916184e4a5ef522d3ea841e8e931cc0f88be0824a7a354a572d5826c68"
    export QT6_SVG_FILE=${QT6_SVG_FILE:-"qtsvg-everywhere-src-${QT6_VERSION}.tar.xz"}
    export QT6_SVG_SOURCE_DIR=${QT6_SVG_SOURCE_DIR:-"qtsvg-everywhere-src-${QT6_VERSION}"}
    export QT6_SVG_SHA256_CHECKSUM="b746af3cb1793621d8ed7eae38d9ad5a15541dc2742031069f2ae3fe87590314"
}

function bv_qt6_print
{
    printf "%s%s\n" "QT6_BASE_FILE=" "${QT6_BASE_FILE}"
    printf "%s%s\n" "QT6_VERSION=" "${QT6_VERSION}"
    printf "%s%s\n" "QT6_SHORT_VERSION=" "${QT6_SHORT_VERSION}"
    printf "%s%s\n" "QT6_BASE_SOURCE_DIR=" "${QT6_BASE_SOURCE_DIR}"
}

function bv_qt6_print_usage
{
    printf "%-20s %s [%s]\n" "--qt6"   "Build QT6" "$DO_QT6"
}

function bv_qt6_host_profile
{
    # b_qt_host_profile has tests for ENGINE_ONLY etc, but if any of those conditions are set, then
    # DO_QT6 is "no"
    if [[ "$DO_QT6" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## QT6" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_QT_DIR \${VISITHOME}/qt/$QT6_VERSION/\${VISITARCH})" >> $HOSTCONF
     fi
}

function bv_qt6_initialize_vars
{
    info "initializing qt6 vars"
    QT6_INSTALL_DIR="${VISITDIR}/qt/${QT6_VERSION}/${VISITARCH}"
    QT6_BIN_DIR="$QT6_INSTALL_DIR/bin"
    QT6_INCLUDE_DIR="$QT6_INSTALL_DIR/include"
    info "qt6_BIN dir ${QT6_BIN_DIR}"
}

#prepare the module and check whether it is built or is ready to be built.
function bv_qt6_ensure
{
    if [[ "$DO_QT6" == "yes" ]]; then
        ensure_built_or_ready "qt"     $QT6_VERSION    $QT6_BASE_SOURCE_DIR    $QT6_BASE_FILE    $QT6_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
    return 0
}

function build_qt6_base
{
    echo "Build Qt 6 base module"
    prepare_build_dir $QT6_BASE_SOURCE_DIR $QT6_BASE_FILE

    untarred_qt6=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_qt6 == -1 ]] ; then
        warn "Unable to prepare Qt 6 build directory. Giving Up!"
        return 1
    fi

    QT6_BASE_BUILD_DIR="${QT6_BASE_SOURCE_DIR}-build"
    if ! test -f $QT6_BASE_BUILD_DIR ; then
        echo "Making build directory $QT6_BASE_BUILD_DIR"
        mkdir $QT6_BASE_BUILD_DIR
    fi

    cd ${QT6_BASE_BUILD_DIR}


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

    QT6_CFLAGS="${CFLAGS} ${C_OPT_FLAGS}"
    QT6_CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}"

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

    qt_cmake_flags=""
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        # '--' separates the qt-configure-style flags from the cmake flags
        qt_cmake_flags=" -- -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_gl_LIBRARY:STRING=${MESAGL_OPENGL_LIB}"
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_opengl_LIBRARY:STRING="
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_glu_LIBRARY:STRING=${MESAGL_GLU_LIB}"
        qt_cmake_flags="${qt_cmake_flags} -DOPENGL_GL_PREFERENCE:STRING=LEGACY"
    fi
    info "Configuring Qt6 base: . . . "
    set -x
    (echo "o"; echo "yes") | env PATH="${CMAKE_INSTALL}:$PATH" \
                             CFLAGS="${QT6_CFLAGS}" CXXFLAGS="${QT6_CXXFLAGS}"  \
                             CC="${C_COMPILER}" CXX="${CXX_COMPILER}"  \
                             ../${QT6_BASE_SOURCE_DIR}/configure \
                             -prefix ${QT6_INSTALL_DIR} ${qt_flags} \
                             ${qt_cmake_flags} | tee qt.config.out
    set +x
    if [[ $? != 0 ]] ; then
        warn "Qt6 base configure failed. Giving up."
        return 1
    fi

   #
    # Build Qt. Config options above make sure we only build the libs & tools.
    #
    info "Building Qt6 base . . . "
    $MAKE $MAKE_OPT_FLAGS

    if [[ $? != 0 ]] ; then
        warn "Qt6 base build failed.  Giving up"
        return 1
    fi

    info "Installing Qt6  base . . . "
    $MAKE install

    # Qt screws up permissions in some cases.  Try to fix that.
    chmod -R a+rX ${VISITDIR}/qt/${QT6_VERSION}
   if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/qt"
        chgrp -R ${GROUP} "$VISITDIR/qt"
    fi

    cd "$START_DIR"
    info "Done with Qt6 base "

    return 0
}


function build_qt6_tools
{
    cd "$START_DIR"
    echo "Build Qt 6 tools module"

    if ! test -f ${QT6_TOOLS_FILE} ; then
        download_file ${QT6_TOOLS_FILE} ${QT6_URL}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${QT6_TOOLS_FILE}"
            return 1
        fi
    fi

    if ! test -d ${QT6_TOOLS_SOURCE_DIR} ; then
        info "Extracting qt6 tools ..."
        uncompress_untar ${QT6_TOOLS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${QT6_TOOLS_FILE}"
            return 1
        fi
    fi 

    # Make a build directory for an out-of-source build.
    QT6_TOOLS_BUILD_DIR="${QT6_TOOLS_SOURCE_DIR}-build"
    if [[ ! -d $QT6_TOOLS_BUILD_DIR ]] ; then
        echo "Making build directory $QT6_TOOLS_BUILD_DIR"
        mkdir $QT6_TOOLS_BUILD_DIR
    fi

    cd ${QT6_TOOLS_BUILD_DIR}
    copts="-DQt6_DIR:PATH=${QT6_INSTALL_DIR}/lib/cmake/Qt6"
    copts="${copts} -DCMAKE_INSTALL_PREFIX:PATH=${QT6_INSTALL_DIR}"
    copts="${copts} -DCMAKE_CXX_STANDARD:STRING=17"
    copts="${copts} -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=ON"
    info "qt6 tools options: $copts"
    info "cmake_install ${CMAKE_INSTALL}"
    info "cmake_command ${CMAKE_COMMAND}"
    qt6_path="${CMAKE_INSTALL}:$PATH"
    info "qt6 tools path: $qt6_path"
   
    info "Configuring Qt6 tools . . . "
    env PATH="${qt6_path}" CC="${C_COMPILER}" CXX="${CXX_COMPILER}"  \
        ${CMAKE_COMMAND} ${copts} ../${QT6_TOOLS_SOURCE_DIR}

    info "Building Qt6 tools . . . "
    $MAKE $MAKE_OPT_FLAGS 

    info "Installing Qt6 tools . . . "
    $MAKE install 

    return 0;
}

function build_qt6_svg
{
    cd "$START_DIR"
    echo "Build Qt 6 svg module"

    if ! test -f ${QT6_SVG_FILE} ; then
        download_file ${QT6_SVG_FILE} ${QT6_URL}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${QT6_SVG_FILE}"
            return 1
        fi
    fi

    if ! test -d ${QT6_SVG_SOURCE_DIR} ; then
        info "Extracting qt6 svg ..."
        uncompress_untar ${QT6_SVG_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${QT6_SVG_FILE}"
            return 1
        fi
    fi 

    # Make a build directory for an out-of-source build.
    QT6_SVG_BUILD_DIR="${QT6_SVG_SOURCE_DIR}-build"
    if [[ ! -d $QT6_SVG_BUILD_DIR ]] ; then
        echo "Making build directory $QT6_SVG_BUILD_DIR"
        mkdir $QT6_SVG_BUILD_DIR
    fi

    cd ${QT6_SVG_BUILD_DIR}
    copts="-DQt6_DIR:PATH=${QT6_INSTALL_DIR}/lib/cmake/Qt6"
    copts="${copts} -DCMAKE_INSTALL_PREFIX:PATH=${QT6_INSTALL_DIR}"
    copts="${copts} -DCMAKE_CXX_STANDARD:STRING=17"
    copts="${copts} -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=ON"
    qt6_path="${CMAKE_INSTALL}:$PATH"
   
    info "Configuring Qt6 svg . . . "
    env PATH="${qt6_path}" CC="${C_COMPILER}" CXX="${CXX_COMPILER}"  \
        ${CMAKE_COMMAND} ${copts} ../${QT6_SVG_SOURCE_DIR}

    info "Building Qt6 svg . . . "
    $MAKE $MAKE_OPT_FLAGS 

    info "Installing Qt6 svg . . . "
    $MAKE install 

    return 0;
}

function bv_qt6_is_enabled
{
    if [[ $DO_QT6 == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_qt6_is_installed
{
    check_if_installed "qt" $QT6_VERSION
    if [[ $? != 0 ]] ; then
        return 0
    fi

    # check submodules

    if ! test -f ${QT6_INSTALL_DIR}/modules/Tools.json ; then
        return 0 
    fi

    if ! test -f ${QT6_INSTALL_DIR}/modules/Svg.json ; then
        return 0 
    fi
    return 1
}

function bv_qt6_build
{
    cd "$START_DIR"
    if [[ "$DO_QT6" == "yes" ]] ; then

        # checks if qtbase and all required modules are installed
        bv_qt6_is_installed
        if [[ $? == 1 ]] ; then
             info "Skipping Qt6 build.  Qt6 is already installed."
        else 

            # check qt proper, then submodules
            check_if_installed "qt" $QT6_VERSION
            if [[ $? != 0 ]] ; then
                info "Building QT6 base (~10 minutes)"
                build_qt6_base
                if [[ $? != 0 ]] ; then
                    error "Unable to build or install QT6.  Bailing out."
                fi
                info "Done building Qt6 base"
            else
                info "Qt6 base already exists"
            fi


            # tools submodule
            if test -f QT6_INSTALL_DIR/modules/Tools.json ; then
                info "Qt 6 submodule tools already exists"
            else
                info "Building QT6 tools (~4 minutes)"
                build_qt6_tools
            fi

            # svg submodule
            if test -f QT6_INSTALL_DIR/modules/Svg.json ; then
                info "Qt 6 submodule svg already exists"
            else
                info "Building QT6 svg (~2 minutes)"
                build_qt6_svg
           fi
        fi
    fi
}
