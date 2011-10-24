function bv_pyside_initialize
{
export DO_PYSIDE="no"
export ON_PYSIDE="off"
}

function bv_pyside_enable
{
DO_PYSIDE="yes"
ON_PYSIDE="on"
}

function bv_pyside_disable
{
DO_PYSIDE="no"
ON_PYSIDE="off"
}

function bv_pyside_depends_on
{
return "qt"
}

function bv_pyside_info
{
export PYSIDE_FILE=${PYSIDE_FILE:-"pyside-combined-1.0.7.tar.gz"}
export PYSIDE_VERSION=${PYSIDE_VERSION:-"1.0.7"}
export PYSIDE_BUILD_DIR=${PYSIDE_BUILD_DIR:-"${PYSIDE_FILE%.tar*}"}
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
        echo "##" >> $HOSTCONF
        echo "## Specify the PySide dir. " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PYSIDE_DIR \${VISITHOME}/pyside/$PYSIDE_VERSION/\${VISITARCH}/)" >> $HOSTCONF
        echo >> $HOSTCONF
    fi
    echo "##" >> $HOSTCONF
}

function bv_pyside_ensure
{
    if [[ "$DO_PYSIDE" = "yes" ]] ; then
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
    CMAKE_BIN="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin/cmake"
    VISIT_PYSIDE_DIR=${VISITDIR}/pyside/${PYSIDE_VERSION}/${VISITARCH}/
    QTDIR=${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}/
    QTLIBDIR=${QTDIR}/lib/
    
    export PATH=${QTBINDIR}:$PATH
    export PATH=$VISIT_PYSIDE_DIR/bin:$VISIT_PYTHON_DIR/bin:$PATH
    export PYTHONPATH=$VISIT_PYSIDE_DIR/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages:$PYTHONPATH
    export PKG_CONFIG_PATH=$VISIT_PYSIDE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

    
    QMAKEEXE=${QTDIR}/bin/qmake
    QMAKEINCDIR=${QTDIR}/include
    CMAKE_PROG=${VISITDIR}/cmake/${CMAKE_VERSION}/$VISITARCH/bin/cmake
    PYTHON_DIR=${VISITDIR}/python/${PYTHON_VERSION}/$VISITARCH/

    PYTHON_BINARY=${PYTHON_DIR}/bin/python
    PYTHON_LIBRARY_DIR=${PYTHON_DIR}/lib
    PYTHON_INCLUDE_DIR=${PYTHON_DIR}/include/python${PYTHON_COMPATIBILITY_VERSION}
    PYTHON_LIBRARY=${PYTHON_DIR}/lib/libpython${PYTHON_COMPATIBILITY_VERSION}.${SO_EXT}
    PYTHON_LIBS=${PYTHON_DIR}/lib/

    cd $1
    info "Configuring pyside/$1 . . ."
    $CMAKE_BIN . \
        -DCMAKE_INSTALL_PREFIX:FILEPATH=$VISIT_PYSIDE_DIR \
        -DCMAKE_SKIP_BUILD_RPATH:BOOL=FALSE\
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=FALSE\
        -DCMAKE_INSTALL_RPATH:FILEPATH=$VISIT_PYSIDE_DIR/lib \
        -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=TRUE\
        -DCMAKE_INSTALL_NAME_DIR:FILEPATH=$VISIT_PYSIDE_DIR/lib \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DALTERNATIVE_QT_INCLUDE_DIR:FILEPATH=$QMAKEINCDIR \
        -DQT_QMAKE_EXECUTABLE:FILEPATH=$QMAKEEXE \
        -DENABLE_ICECC:BOOL=0 \
        -DShiboken_DIR:FILEPATH=$VISIT_PYSIDE_DIR/lib/\
        -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON_BINARY\
        -DPYTHON_INCLUDE_PATH:FILEPATH=$PYTHON_INCLUDE_DIR\
        -DPYTHON_LIBRARY:FILEPATH=$PYTHON_LIBRARY\
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
    info "Successfully built pyside/$1"
    cd ../
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
    ln -s $VISIT_PYSIDE_DIR/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/PySide $PYTHON_DIR/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/PySide
    info "Done with PySide"

    return 0
}

function bv_pyside_build
{
#
# Build PySide
#
cd "$START_DIR"
if [[ "$DO_PYSIDE" == "yes" ]] ; then
        check_if_installed "pyside" $PYSIDE_VERSION
    if [[ $? == 0 ]] ; then
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
