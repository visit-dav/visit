function bv_pyside_initialize
{
if [[ "$DO_STATIC_BUILD" == "no" ]]; then
    export DO_PYSIDE="yes"
    export ON_PYSIDE="on"
    export USE_SYSTEM_PYSIDE="no"
    add_extra_commandline_args "pyside" "alt-pyside-dir" 1 "Use alternative directory for pyside"
else
    export DO_PYSIDE="no"
    export ON_PYSIDE="off"
    export USE_SYSTEM_PYSIDE="no"
fi
}

function bv_pyside_enable
{
DO_PYSIDE="yes"
ON_PYSIDE="on"
DO_QT="yes"
ON_QT="on"
}

function bv_pyside_disable
{
DO_PYSIDE="no"
ON_PYSIDE="off"
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
    info "initialize Qt vars"
    if [[ $IS_QT5 == "yes" && $DO_PYSIDE == "yes" ]]; then
        error "PySide is not supported with Qt5, please disable"
    fi
}

function bv_pyside_info
{
export PYSIDE_VERSION=${PYSIDE_VERSION:-"1.2.2"}
export PYSIDE_FILE=${PYSIDE_FILE:-"pyside-combined-${PYSIDE_VERSION}.tar.gz"}
export PYSIDE_BUILD_DIR=${PYSIDE_BUILD_DIR:-"${PYSIDE_FILE%.tar*}"}
export PYSIDE_MD5_CHECKSUM="b33dde999cc4eb13933be43f49c1e890"
export PYSIDE_SHA256_CHECKSUM=""
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
printf "%-15s %s [%s]\n" "--pyside" "Build PySide" "$DO_PYSIDE"
}

function bv_pyside_graphical
{
local graphical_out="PySide    $PYSIDE_VERSION($PYSIDE_FILE)     $ON_PYSIDE"
echo "$graphical_out"
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
    mkdir -p build
    cd build #PySide fails during in source build..
    info "Configuring pyside/$1 . . ."
    $CMAKE_COMMAND .. \
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER} \
        -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}" \
        -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        -DCMAKE_INSTALL_PREFIX:FILEPATH="$VISIT_PYSIDE_DIR" \
        -DCMAKE_SKIP_BUILD_RPATH:BOOL=FALSE\
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=FALSE\
        -DCMAKE_INSTALL_RPATH:FILEPATH="$VISIT_PYSIDE_DIR/lib" \
        -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=TRUE\
        -DCMAKE_INSTALL_NAME_DIR:FILEPATH="$VISIT_PYSIDE_DIR/lib" \
        -DCMAKE_BUILD_TYPE:STRING="${VISIT_BUILD_MODE}" \
        -DALTERNATIVE_QT_INCLUDE_DIR:FILEPATH="$ALTERNATIVE_QT_INCLUDE_DIR" \
        -DQT_QMAKE_EXECUTABLE:FILEPATH="$QT_QMAKE_COMMAND" \
        -DENABLE_ICECC:BOOL=0 \
        -DShiboken_DIR:FILEPATH="$VISIT_PYSIDE_DIR/lib/"\
        -DPYTHON_EXECUTABLE:FILEPATH="$PYTHON_COMMAND"\
        -DPYTHON_INCLUDE_PATH:FILEPATH="$PYTHON_INCLUDE_DIR"\
        -DPYTHON_LIBRARY:FILEPATH="$PYTHON_LIBRARY"\
        -DDISABLE_DOCSTRINGS:BOOL=True

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

    cd $PYSIDE_BUILD_DIR || error "Can't cd to PySide build dir."


    build_pyside_component shiboken-${PYSIDE_VERSION}
    if [[ $? != 0 ]] ; then
       return 1
    fi

    build_pyside_component pyside-qt4.8+${PYSIDE_VERSION}
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

    if  [[ ! -e "${VISIT_PYSIDE_DIR}/shiboken-${PYSIDE_VERSION}_success" ||
           ! -e "${VISIT_PYSIDE_DIR}/pyside-qt4.8+${PYSIDE_VERSION}_success" ]]; then
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
