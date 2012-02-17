function bv_boxlib_initialize
{
export DO_BOXLIB="no"
export ON_BOXLIB="off"
}

function bv_boxlib_enable
{
DO_BOXLIB="yes"
ON_BOXLIB="on"
}

function bv_boxlib_disable
{
DO_BOXLIB="no"
ON_BOXLIB="off"
}

function bv_boxlib_depends_on
{
return ""
}

function bv_boxlib_info
{
export BOXLIB_VERSION=${BOXLIB_VERSION:-"0.1.8"}
export BOXLIB_FILE=${BOXLIB_FILE:-"ccse-${BOXLIB_VERSION}.tar.gz"}
export BOXLIB_COMPATIBILITY_VERSION=${BOXLIB_COMPATIBILITY_VERSION:-"0.1.8"}
export BOXLIB_URL=${BOXLIB_URL:-"https://ccse.lbl.gov/Software/tarfiles/"}
export BOXLIB_BUILD_DIR=${BOXLIB_BUILD_DIR:-"ccse-${BOXLIB_VERSION}/Src/C_BaseLib"}
}

function bv_boxlib_print
{
  printf "%s%s\n" "BOXLIB_FILE=" "${BOXLIB_FILE}"
  printf "%s%s\n" "BOXLIB_VERSION=" "${BOXLIB_VERSION}"
  printf "%s%s\n" "BOXLIB_COMPATIBILITY_VERSION=" "${BOXLIB_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "BOXLIB_BUILD_DIR=" "${BOXLIB_BUILD_DIR}"
}

function bv_boxlib_print_usage
{
printf "%-15s %s [%s]\n" "--boxlib"  "Build Boxlib" "$DO_BOXLIB" 
}

function bv_boxlib_graphical
{
  local graphical_output="Boxlib   $BOXLIB_VERSION($BOXLIB_FILE)    $ON_BOXLIB"
  echo $graphical_output
}

function bv_boxlib_host_profile
{
    if [[ "$DO_BOXLIB" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Boxlib" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR \${VISITHOME}/boxlib/$BOXLIB_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        >> $HOSTCONF
    fi

}

function bv_boxlib_ensure
{
    if [[ "$DO_BOXLIB" == "yes" ]] ; then
        ensure_built_or_ready "boxlib" $BOXLIB_VERSION $BOXLIB_BUILD_DIR $BOXLIB_FILE $BOXLIB_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_BOXLIB="no"
            error "Unable to build Boxlib.  ${BOXLIB_FILE} not found."
        fi
    fi
}

function bv_boxlib_dry_run
{
  if [[ "$DO_BOXLIB" == "yes" ]] ; then
    echo "Dry run option not set for boxlib."
  fi
}
# *************************************************************************** #
#                         Function 8.8, build_boxlib                          #
# *************************************************************************** #

function build_boxlib
{
    #
    # Prepare build dir
    #
    prepare_build_dir $BOXLIB_BUILD_DIR $BOXLIB_FILE
    untarred_boxlib=$?
    if [[ $untarred_boxlib == -1 ]] ; then
       warn "Unable to prepare Boxlib Build Directory. Giving Up"
       return 1
    fi

    cd $BOXLIB_BUILD_DIR || error "Can't cd to BoxLib build dir."

    #
    # Build BoxLib
    #
    info "Building Boxlib. . . (~4 minutes)"

    if [[ "$OPSYS" == "AIX" ]]; then
        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
           DEBUG="FALSE" DIM=3 COMP="xlC" USE_MPI="FALSE" \
           BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"

        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
           DEBUG="FALSE" DIM=2 COMP="xlC" USE_MPI="FALSE" \
           BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"
    elif [[ "$OPSYS" == "Darwin" ]]; then
        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
           DEBUG="FALSE" DIM=3 USE_MPI="FALSE" BL_MANGLE_SYMBOLS_WITH_DIM="TRUE" \
           BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"

        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
           DEBUG="FALSE" DIM=2 USE_MPI="FALSE" BL_MANGLE_SYMBOLS_WITH_DIM="TRUE" \
           BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"
    else
        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
           DEBUG="FALSE" DIM=3 USE_MPI="FALSE" \
           BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"

        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
           DEBUG="FALSE" DIM=2 USE_MPI="FALSE" \
           BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"
    fi

    #
    # Create dynamic library for Darwin
    #
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        INSTALLNAMEPATH="$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib"

        $CXX_COMPILER -dynamiclib -o libbox3D.$SO_EXT o/3d.Darwin.*/*.o \
          -lSystem -Wl,-headerpad_max_install_names \
          -Wl,-install_name,$INSTALLNAMEPATH/libbox3D.$SO_EXT \
          -Wl,-compatibility_version,$BOXLIB_COMPATIBILITY_VERSION \
          -Wl,-current_version,$BOXLIB_VERSION || \
          error "Creation of dynamic 3D Boxlib library failed. Giving up!"
        $CXX_COMPILER -dynamiclib -o libbox2D.$SO_EXT o/2d.Darwin.*/*.o \
          -lSystem -Wl,-headerpad_max_install_names \
          -Wl,-install_name,$INSTALLNAMEPATH/libbox2D.$SO_EXT \
          -Wl,-compatibility_version,$BOXLIB_COMPATIBILITY_VERSION \
          -Wl,-current_version,$BOXLIB_VERSION || \
          error "Creation of dynamic 2D Boxlib library failed. Giving up!"
        boxlib_ext=$SO_EXT
    else
        mv libbox3d.*.a libbox3D.a
        mv libbox2d.*.a libbox2D.a
        boxlib_ext=a
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing Boxlib . . ."

    mkdir "$VISITDIR/boxlib"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/include"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib"

    cp libbox3D.$boxlib_ext \
       "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib/" || \
       error "Boxlib install failed. Giving up!"

    cp libbox2D.$boxlib_ext \
       "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib/" || \
       error "Boxlib install failed. Giving up!"

    cp *.H "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/include" || \
       error "Boxlib install failed. Giving up!"

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/boxlib"
       chgrp -R ${GROUP} "$VISITDIR/boxlib"
    fi

    cd "$START_DIR"
    info "Done with BoxLib"
    return 0
}

function bv_boxlib_build
{
cd "$START_DIR"
if [[ "$DO_BOXLIB" == "yes" ]] ; then
    check_if_installed "boxlib" $BOXLIB_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Boxlib build.  Boxlib is already installed."
    else
        info "Building Boxlib (~2 minutes)"
        build_boxlib
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Boxlib.  Bailing out."
        fi
        info "Done building Boxlib"
    fi
fi
}
