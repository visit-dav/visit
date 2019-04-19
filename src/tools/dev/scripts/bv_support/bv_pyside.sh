function bv_pyside_initialize
{
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        export DO_PYSIDE="yes"
        export USE_SYSTEM_PYSIDE="no"
        add_extra_commandline_args "pyside" "alt-pyside-dir" 1 "Use alternative directory for pyside"
    else
        export DO_PYSIDE="no"
        export USE_SYSTEM_PYSIDE="no"
    fi
}

function bv_pyside_enable
{
    DO_PYSIDE="yes"
    DO_QT="yes"
}

function bv_pyside_disable
{
    DO_PYSIDE="no"
}

function bv_pyside_alt_pyside_dir
{
    bv_pyside_enable 
    USE_SYSTEM_PYSIDE="yes"
    PYSIDE_INSTALL_DIR="$1"
    info "using alternative pyside directory: $PYSIDE_INSTALL_DIR" 
}

function bv_pyside_depends_on
{
    if [[ "$USE_SYSTEM_PYSIDE" == "yes" ]]; then
        echo ""
    else
        echo "cmake python qt"
    fi
}

function bv_pyside_initialize_vars
{
    info "initialize PySide vars"
}

function bv_pyside_info
{
    export PYSIDE_VERSION=${PYSIDE_VERSION:-"2.0.0-2017.08.30"}
    export PYSIDE_FILE=${PYSIDE_FILE:-"pyside2-combined.2017.08.30.tar.gz"}
    export PYSIDE_BUILD_DIR=${PYSIDE_BUILD_DIR:-"pyside2-combined"}
    export PYSIDE_MD5_CHECKSUM="e3916fe9ffa0887c01bb0e22f78a559a"
    export PYSIDE_SHA256_CHECKSUM="9479bc5d0bc1dc7eecdf474701ac8540a6c833cdad81fa22adecda1ff790d9bf"
}

function bv_pyside_print
{
    printf "%s%s\n" "PYSIDE_FILE=" "${PYSIDE_FILE}"
    printf "%s%s\n" "PYSIDE_VERSION=" "${PYSIDE_VERSION}"
    printf "%s%s\n" "PYSIDE_PLATFORM=" "${PYSIDE_PLATFORM}"
    printf "%s%s\n" "PYSIDE_BUILD_DIR=" "${PYSIDE_BUILD_DIR}"
}

function bv_pyside_print_usage
{
    printf "%-20s %s [%s]\n" "--pyside" "Build PySide" "$DO_PYSIDE"
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        printf "%-20s %s [%s]\n" "--alt-pyside-dir" "Use PySide from an alternative directory"
    fi
}

function bv_pyside_host_profile
{
    if [[ "$DO_PYSIDE" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## PySide" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_PYSIDE" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR $PYSIDE_INSTALL_DIR)" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR \${VISITHOME}/pyside/$PYSIDE_VERSION/\${VISITARCH}/)" >> $HOSTCONF
        fi
    fi
}

function bv_pyside_ensure
{
    if [[ "$DO_PYSIDE" = "yes" && "$DO_SERVER_COMPONENTS_ONLY" == "no" && "$USE_SYSTEM_PYSIDE" == "no" ]] ; then
        ensure_built_or_ready "pyside"     $PYSIDE_VERSION    $PYSIDE_BUILD_DIR    $PYSIDE_FILE 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_PYSIDE="no"
            error "Unable to build PySide.  ${PYSIDE_FILE} not found."
        fi
    fi
}

function bv_pyside_dry_run
{
    if [[ "$DO_PYSIDE" == "yes" ]] ; then
        echo "Dry run option not set for pyside."
    fi
}

# *************************************************************************** #
#                          Function 4.2, build_pyside                          #
# *************************************************************************** #

function build_pyside_component
{
    VISIT_PYSIDE_DIR="${VISITDIR}/pyside/${PYSIDE_VERSION}/${VISITARCH}/"

    export PATH=${QT_BIN_DIR}:$PATH
    export PATH=$VISIT_PYSIDE_DIR/bin:$VISIT_PYTHON_DIR/bin:$PATH
    export PYTHONPATH=$VISIT_PYSIDE_DIR/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages:$PYTHONPATH
    export PKG_CONFIG_PATH=$VISIT_PYSIDE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

    ALTERNATIVE_QT_INCLUDE_DIR="$QT_INCLUDE_DIR"
    
    # There is a bug on mac that using system qt
    # where headers say they are in /usr/include
    # when in reality most of the headers are in
    # /Library/Frameworks (except for QtUiTools)
    if [[ "Darwin" == `uname` && 
                "$QT_LIB_DIR" == "/Library/Frameworks" &&
                "$QT_INCLUDE_DIR" == "/usr/include" ]]; then
        ALTERNATIVE_QT_INCLUDE_DIR="$QT_LIB_DIR"
    fi

    cd $1
    #
    # Make sure to pass compilers and compiler flags to cmake
    #
    popts=""
    popts="${popts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    popts="${popts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    popts="${popts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    popts="${popts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    popts="${popts} -DCMAKE_INSTALL_PREFIX:FILEPATH=\"$VISIT_PYSIDE_DIR\""
    popts="${popts} -DCMAKE_SKIP_BUILD_RPATH:BOOL=false"
    popts="${popts} -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=false"
    popts="${popts} -DCMAKE_INSTALL_RPATH:FILEPATH=\"$VISIT_PYSIDE_DIR/lib\""
    popts="${popts} -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=true"
    popts="${popts} -DCMAKE_INSTALL_NAME_DIR:FILEPATH=\"$VISIT_PYSIDE_DIR/lib\""
    popts="${popts} -DCMAKE_BUILD_TYPE:STRING=\"${VISIT_BUILD_MODE}\""
    popts="${popts} -DALTERNATIVE_QT_INCLUDE_DIR:FILEPATH=\"$ALTERNATIVE_QT_INCLUDE_DIR\""
    popts="${popts} -DQT_QMAKE_EXECUTABLE:FILEPATH=\"$QT_QMAKE_COMMAND\""
    popts="${popts} -DENABLE_ICECC:BOOL=false"
    popts="${popts} -DShiboken_DIR:FILEPATH=\"$VISIT_PYSIDE_DIR/lib/\""
    popts="${popts} -DPYTHON_EXECUTABLE:FILEPATH=\"$PYTHON_COMMAND\""
    popts="${popts} -DPYTHON_INCLUDE_PATH:FILEPATH=\"$PYTHON_INCLUDE_DIR\""
    popts="${popts} -DPYTHON_LIBRARY:FILEPATH=\"$PYTHON_LIBRARY\""
    popts="${popts} -DDISABLE_DOCSTRINGS:BOOL=true"

    popts="${popts} -DBUILD_TESTS:BOOL=false"
    popts="${popts} -DENABLE_VERSION_SUFFIX:BOOL=false"
    popts="${popts} -DCMAKE_PREFIX_PATH=${QT_INSTALL_DIR}/lib/cmake"

    info "Configuring pyside/$1 . . ."
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    mkdir -p build
    cd build #PySide fails during in source build..


    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${popts} ../ > bv_run_cmake.sh

    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "pyside/$1 configuration failed."

    if [[ $? != 0 ]] ; then
        warn "Cannot configure pyside/$1, giving up."
        return 1
    fi

    info "Building pyside/$1 . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Cannot build pyside/$1, giving up."
        return 1
    fi

    info "Installing pyside/$1 . . ."
    $MAKE install
    touch "${VISIT_PYSIDE_DIR}/$1_success"
    info "Successfully built pyside/$1"
    cd ../../
}

function apply_pyside_2_0_0_patch
{
    info "Patching Pyside2"
    patch -p0 << \EOF
diff -c pyside2-combined/pyside2/orig/CMakeLists.txt pyside2-combined/pyside2/CMakeLists.txt 
*** pyside2-combined/pyside2/orig/CMakeLists.txt        Thu Dec 14 16:03:50 2017
--- pyside2-combined/pyside2/CMakeLists.txt     Thu Dec 14 16:06:34 2017
***************
*** 273,280 ****
  COLLECT_MODULE_IF_FOUND(Xml)
  COLLECT_MODULE_IF_FOUND(XmlPatterns opt)
  COLLECT_MODULE_IF_FOUND(Help opt)
! COLLECT_MODULE_IF_FOUND(Multimedia opt)
! COLLECT_MODULE_IF_FOUND(MultimediaWidgets opt)
  COLLECT_MODULE_IF_FOUND(OpenGL opt)
  COLLECT_MODULE_IF_FOUND(Qml opt)
  COLLECT_MODULE_IF_FOUND(Quick opt)
--- 273,282 ----
  COLLECT_MODULE_IF_FOUND(Xml)
  COLLECT_MODULE_IF_FOUND(XmlPatterns opt)
  COLLECT_MODULE_IF_FOUND(Help opt)
! #COLLECT_MODULE_IF_FOUND(Multimedia opt)
! set(DISABLE_QtMultimedia 1)
! #COLLECT_MODULE_IF_FOUND(MultimediaWidgets opt)
! set(DISABLE_QtMultimediaWidgets 1)
  COLLECT_MODULE_IF_FOUND(OpenGL opt)
  COLLECT_MODULE_IF_FOUND(Qml opt)
  COLLECT_MODULE_IF_FOUND(Quick opt)
***************
*** 297,304 ****
  # still forgotten:
  #COLLECT_MODULE_IF_FOUND(WebEngineCore opt)
  #COLLECT_MODULE_IF_FOUND(WebEngine opt)
! COLLECT_MODULE_IF_FOUND(WebEngineWidgets opt)
! COLLECT_MODULE_IF_FOUND(WebKit opt)
  if(NOT MSVC)
      # right now this does not build on windows
      COLLECT_MODULE_IF_FOUND(WebKitWidgets opt)
--- 299,306 ----
  # still forgotten:
  #COLLECT_MODULE_IF_FOUND(WebEngineCore opt)
  #COLLECT_MODULE_IF_FOUND(WebEngine opt)
! #COLLECT_MODULE_IF_FOUND(WebEngineWidgets opt)
! #COLLECT_MODULE_IF_FOUND(WebKit opt)
  if(NOT MSVC)
      # right now this does not build on windows
      COLLECT_MODULE_IF_FOUND(WebKitWidgets opt)
EOF
    if [[ $? != 0 ]] ; then
        warn "Pyside2 patch failed."
        return 1
    fi

    return 0
}

function apply_pyside_patch
{
    if [[ ${PYSIDE_VERSION} == 2.0.0-2017.08.30 ]] ; then
        apply_pyside_2_0_0_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_pyside
{
    ##
    ## Prepare the build dir using src file.
    ##

    prepare_build_dir $PYSIDE_BUILD_DIR $PYSIDE_FILE
    untarred_pyside=$?
    ## 0, already exists, 1  untarred src, 2 error

    if [[ untarred_pyside == -1 ]] ; then
        warn "Unable to prepare PySide build directory. Giving Up!"
        return 1
    fi

    ##
    ## Apply patches
    ##

    apply_pyside_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_pyside == 1 ]] ; then
            warn "Giving up on pyside build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing. I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    cd $PYSIDE_BUILD_DIR || error "Can't cd to PySide build dir."


    build_pyside_component shiboken2

    if [[ $? != 0 ]] ; then
        return 1
    fi

    build_pyside_component pyside2

    if [[ $? != 0 ]] ; then
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/pyside"
        chgrp -R ${GROUP} "$VISITDIR/pyside"
    fi

    cd "$START_DIR"

    info "Done with PySide"

    return 0
}

function bv_pyside_is_enabled
{
    if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]]; then
        return 0;
    fi 
    if [[ $DO_PYSIDE == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_pyside_is_installed
{
    if [[ "$USE_SYSTEM_PYSIDE" == "yes" ]]; then 
        return 1
    fi

    VISIT_PYSIDE_DIR="${VISITDIR}/pyside/${PYSIDE_VERSION}/${VISITARCH}/"
    check_if_installed "pyside" $PYSIDE_VERSION
    if [[ $? != 0 ]] ; then
        return 0
    fi

    if  [[ ! -e "${VISIT_PYSIDE_DIR}/shiboken2_success" ||
             ! -e "${VISIT_PYSIDE_DIR}/pyside2_success" ]]; then
        info "pyside not installed completely"
        return 0
    fi
    return 1
}

function bv_pyside_build
{
    #
    # Build PySide
    #
    cd "$START_DIR"
    if [[ "$DO_PYSIDE" == "yes" && "$DO_SERVER_COMPONENTS_ONLY" == "no" && "$USE_SYSTEM_PYSIDE" == "no" ]] ; then
        bv_pyside_is_installed #this returns 1 for true, 0 for false
        if [[ $? != 0 ]] ; then
            info "Skipping PySide build.  PySide is already installed."
        else
            info "Building PySide (~10 minutes)"
            build_pyside
            if [[ $? != 0 ]] ; then
                error "Unable to build or install PySide.  Bailing out."
            fi
            info "Done building PySide"
        fi
    fi
}
