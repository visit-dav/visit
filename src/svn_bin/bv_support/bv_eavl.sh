
function bv_eavl_initialize
{
export DO_EAVL="no"
export ON_EAVL="off"
}

function bv_eavl_enable
{
DO_EAVL="yes"
ON_EAVL="on"
}

function bv_eavl_disable
{
DOEAVL_="no"
ON_EAVL="off"
}

function bv_eavl_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_eavl_info
{
export EAVL_VERSION=${EAVL_VERSION:-"ac39232"}
export EAVL_FILE=${EAVL_FILE:-"EAVL-${EAVL_VERSION}.tar.gz"}
export EAVL_BUILD_DIR=${EAVL_BUILD_DIR:-"EAVL-${EAVL_VERSION}"}
}

function bv_eavl_print
{
  printf "%s%s\n" "EAVL_FILE=" "${EAVL_FILE}"
  printf "%s%s\n" "EAVL_VERSION=" "${EAVL_VERSION}"
  printf "%s%s\n" "EAVL_BUILD_DIR=" "${EAVL_BUILD_DIR}"
}

function bv_eavl_print_usage
{
printf "%-15s %s [%s]\n" "--eavl" "Build eavl support" "$DO_EAVL"
}

function bv_eavl_graphical
{
local graphical_out="eavl     $EAVL_VERSION($EAVL_FILE)      $ON_EAVL"
echo "$graphical_out"
}

function bv_eavl_host_profile
{
    if [[ "$DO_EAVL" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## eavl " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_EAVL_DIR \${VISITHOME}/eavl/$EAVL_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_eavl_ensure
{
    if [[ "$DO_EAVL" == "yes" ]] ; then
        ensure_built_or_ready "eavl" $EAVL_VERSION $EAVL_BUILD_DIR $EAVL_FILE $EAVL_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_EAVL="no"
            error "Unable to build eavl.  ${EAVL_FILE} not found."
        fi
    fi
}

function bv_eavl_dry_run
{
  if [[ "$DO_EAVL" == "yes" ]] ; then
    echo "Dry run option not set for eavl."
  fi
}

# *************************************************************************** #
#                            Function 8, build_EAVL
#
#
# *************************************************************************** #

function build_EAVL
{
    #
    # Prepare build dir
    #
    prepare_build_dir $EAVL_BUILD_DIR $EAVL_FILE
    untarred_EAVL=$?
    if [[ $untarred_EAVL == -1 ]] ; then
       warn "Unable to prepare eavl build directory. Giving Up!"
       return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring eavl . . ."
    cd $EAVL_BUILD_DIR || error "Can't cd to eavl build dir."
    
    #
    # expedient hack: use VISIT_CUDA_TOOLKIT env var to select cuda
    #
    if test "x${VISIT_CUDA_TOOLKIT}" = "x"; then
        export EAVL_EXTRA_ARGS=""
    else
        export EAVL_CUDA_TOOLKIT_ARGS=" --with-cuda=$VISIT_CUDA_TOOLKIT"
    fi

    if test "x${SYSTEM_VTK_DIR}" = "x"; then
        export EAVL_VTK_ARGS=""
    else
        export EAVL_VTK_ARGS=" --with-vtk=$SYSTEM_VTK_DIR"
    fi
        
VISIT_VTK_DIR
    
#info ./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
    #                 CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
    #                 CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
    #                 $EAVL_CUDA_TOOLKIT_ARGS \
    #                 --prefix="$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH"

    #./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
    #                 CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" \
    #                 CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
    #                 $EAVL_CUDA_TOOLKIT_ARGS \
    #                 --prefix="$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH"
    # avoid passing flags for now
    info ./configure $EAVL_CUDA_TOOLKIT_ARGS $EAVL_VTK_ARGS --with-openmp \
                     --prefix="$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH"

    ./configure $EAVL_CUDA_TOOLKIT_ARGS $EAVL_VTK_ARGS --with-openmp \
                 --prefix="$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH"

    #
    # Build eavl
    #
    info "Building eavl . . . (~2 minutes)"
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "eavl build failed.  Giving up"
       return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing eavl"
    #$MAKE $MAKE_OPT_FLAGS install
    # eavl doesn't yet support make install ...
    mkdir "$VISITDIR/eavl"
    mkdir "$VISITDIR/eavl/$EAVL_VERSION"
    mkdir "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH"
    mkdir "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    mkdir "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/lib"

    cp lib/libeavl.a      "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/lib"
    cp src/common/*.h     "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/exporters/*.h  "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/importers/*.h  "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/rendering/*.h  "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/filters/*.h    "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/math/*.h       "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/fonts/*.h      "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/operations/*.h "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/vtk/*.h        "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"


    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/eavl"
       chgrp -R ${GROUP} "$VISITDIR/eavl"
    fi
    cd "$START_DIR"
    info "Done with eavl"
    return 0
}

function bv_eavl_is_enabled
{
    if [[ $DO_EAVL == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_eavl_is_installed
{
    check_if_installed "eavl" $EAVL_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_eavl_build
{
cd "$START_DIR"
if [[ "$DO_EAVL" == "yes" ]] ; then
    check_if_installed "eavl" $EAVL_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping eavl build.  eavl is already installed."
    else
        info "Building eavl (~2 minutes)"
        build_EAVL
        if [[ $? != 0 ]] ; then
            error "Unable to build or install eavl.  Bailing out."
        fi
        info "Done building eavl"
    fi
fi
}

