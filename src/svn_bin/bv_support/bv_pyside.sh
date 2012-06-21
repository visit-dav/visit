function bv_pyside_initialize
{
export DO_PYSIDE="yes"
export ON_PYSIDE="on"
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

function bv_pyside_depends_on
{
echo "cmake python qt"
}

function bv_pyside_info
{
export PYSIDE_FILE=${PYSIDE_FILE:-"pyside-combined-1.0.7.tar.gz"}
export PYSIDE_VERSION=${PYSIDE_VERSION:-"1.0.7"}
export PYSIDE_BUILD_DIR=${PYSIDE_BUILD_DIR:-"${PYSIDE_FILE%.tar*}"}
export PYSIDE_MD5_CHECKSUM="ebc8f4c479d36772e5a34e3be6402972"
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
        echo "VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR \${VISITHOME}/pyside/$PYSIDE_VERSION/\${VISITARCH}/)" >> $HOSTCONF
    fi
}

function bv_pyside_ensure
{
    if [[ "$DO_PYSIDE" = "yes" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
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

    cd $1
    info "Configuring pyside/$1 . . ."
    $CMAKE_COMMAND . \
        -DCMAKE_INSTALL_PREFIX:FILEPATH="$VISIT_PYSIDE_DIR" \
        -DCMAKE_SKIP_BUILD_RPATH:BOOL=FALSE\
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=FALSE\
        -DCMAKE_INSTALL_RPATH:FILEPATH="$VISIT_PYSIDE_DIR/lib" \
        -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=TRUE\
        -DCMAKE_INSTALL_NAME_DIR:FILEPATH="$VISIT_PYSIDE_DIR/lib" \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DALTERNATIVE_QT_INCLUDE_DIR:FILEPATH="$QT_INCLUDE_DIR" \
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
    cd ../
}

function patch_pyside_qt47_107
{
   patch -f -p0 pyside-qt4.7+1.0.7/PySide/QtScript/typesystem_script.xml <<\EOF
42a43,44
>         <!-- Not supported BUG #957-->
>         <modify-function signature="scriptValueFromQMetaObject()" remove="all" />
EOF
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

    
    build_pyside_component apiextractor-0.10.7
    if [[ $? != 0 ]] ; then
       return 1
    fi

    build_pyside_component generatorrunner-0.6.13       
    if [[ $? != 0 ]] ; then
       return 1
    fi
    
    build_pyside_component shiboken-1.0.7
    if [[ $? != 0 ]] ; then
       return 1
    fi
    
    patch_pyside_qt47_107 #remove if component is updated to latest
    build_pyside_component pyside-qt4.7+1.0.7
    if [[ $? != 0 ]] ; then
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/pyside"
       chgrp -R ${GROUP} "$VISITDIR/pyside"
    fi

    cd "$START_DIR"
    info "Linking PySide to Python Installation"
    ln -s $VISIT_PYSIDE_DIR/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/PySide $VISIT_PYTHON_DIR/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/PySide
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
    VISIT_PYSIDE_DIR="${VISITDIR}/pyside/${PYSIDE_VERSION}/${VISITARCH}/"
    check_if_installed "pyside" $PYSIDE_VERSION
    if [[ $? != 0 ]] ; then
        return 0
    fi

    if [[ ! -e "${VISIT_PYSIDE_DIR}/apiextractor-0.10.7_success" ||
          ! -e "${VISIT_PYSIDE_DIR}/generatorrunner-0.6.13_success" ||
          ! -e "${VISIT_PYSIDE_DIR}/shiboken-1.0.7_success" ||
          ! -e "${VISIT_PYSIDE_DIR}/pyside-qt4.7+1.0.7_success" ]]; then
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
if [[ "$DO_PYSIDE" == "yes" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
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
