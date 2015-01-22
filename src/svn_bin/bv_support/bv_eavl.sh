
function bv_eavl_initialize
{
export DO_EAVL="no"
export ON_EAVL="off"
export USE_SYSTEM_EAVL="no"
add_extra_commandline_args "eavl" "alt-eavl-dir" 1 "Use alternative directory for EAVL"
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

function bv_eavl_alt_eavl_dir
{
    bv_eavl_enable
    USE_SYSTEM_EAVL="yes"
    EAVL_INSTALL_DIR="$1"
}

function bv_eavl_depends_on
{
    if [[ "$USE_SYSTEM_EAVL" == "yes" ]]; then
        echo ""
    else
        local depends_on=""

        echo $depends_on
    fi
}

function bv_eavl_initialize_vars
{
    if [[ "$USE_SYSTEM_EAVL" == "no" ]]; then
        EAVL_INSTALL_DIR="\${VISITHOME}/eavl/$EAVL_VERSION/\${VISITARCH}"
    fi
}

function bv_eavl_info
{
export EAVL_VERSION=${EAVL_VERSION:-"5f53629eed"}
export EAVL_FILE=${EAVL_FILE:-"EAVL-${EAVL_VERSION}.tar.gz"}
export EAVL_BUILD_DIR=${EAVL_BUILD_DIR:-"EAVL-${EAVL_VERSION}"}
export EAVL_MD5_CHECKSUM="b18015799f81384be0ac91ce11d842bd"
export EAVL_SHA256_CHECKSUM="d1911e6790858c1ab8fa29da4ee451295f79bd0e0795fc33afd5732769c14f7f"
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
        echo "## EAVL" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_EAVL_DIR ${EAVL_INSTALL_DIR})" \
        >> $HOSTCONF
    fi
}

function bv_eavl_ensure
{
    if [[ "$DO_EAVL" == "yes" && "$USE_SYSTEM_EAVL" == "no" ]] ; then
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

function apply_EAVL_5f53629eed_patch
{
   patch -p0 <<\EOF
diff -c a/configure EAVL-5f53629eed/configure
*** a/configure	Tue Aug 12 16:25:47 2014
--- EAVL-5f53629eed/configure	Tue Aug 12 16:26:02 2014
***************
*** 4435,4444 ****
  
  # don't use everything -- we only need enough to do data set conversion.
  VTK_ALL_LIBS="
!     vtkIOLegacy-6.0
!     vtkIOCore-6.0
!     vtkCommonDataModel-6.0
!     vtkCommonCore-6.0
      "
  
  VTK_CPPFLAGS=""
--- 4435,4444 ----
  
  # don't use everything -- we only need enough to do data set conversion.
  VTK_ALL_LIBS="
!     vtkIOLegacy-6.1
!     vtkIOCore-6.1
!     vtkCommonDataModel-6.1
!     vtkCommonCore-6.1
      "
  
  VTK_CPPFLAGS=""
***************
*** 4454,4460 ****
  
  if test "$VTK" != "yes" -a "$VTK" != "no"; then
     # specified path
!    VTK_CPPFLAGS="-I""$VTK""/include/vtk-6.0 -I""$VTK""/include"
     VTK_LDFLAGS="-L""$VTK""/lib"
     if test "$UNAME" = "Darwin"; then
               VTK_LDFLAGS="$VTK_LDFLAGS"
--- 4454,4460 ----
  
  if test "$VTK" != "yes" -a "$VTK" != "no"; then
     # specified path
!    VTK_CPPFLAGS="-I""$VTK""/include/vtk-6.1 -I""$VTK""/include"
     VTK_LDFLAGS="-L""$VTK""/lib"
     if test "$UNAME" = "Darwin"; then
               VTK_LDFLAGS="$VTK_LDFLAGS"
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply patch to EAVL 5f53629eed"
        return 1
   else
        return 0
   fi
}

function apply_EAVL_patch
{
   info "Patching EAVL . . ."
   if [[ ${EAVL_VERSION} == "5f53629eed" ]] ; then
      apply_EAVL_5f53629eed_patch
      if [[ $? != 0 ]] ; then
        return 1
      fi
   fi

   return 0
}

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
    
    apply_EAVL_patch

    #
    # Call configure
    #
    info "Configuring eavl . . ."
    cd $EAVL_BUILD_DIR || error "Can't cd to eavl build dir."
    
    #
    # expedient hack: use VISIT_CUDA_TOOLKIT env var to select cuda
    #
    if [[ "${VISIT_CUDA_TOOLKIT}" == "" ]] ; then
        export EAVL_EXTRA_ARGS=""
    else
        export EAVL_CUDA_TOOLKIT_ARGS=" --with-cuda=$VISIT_CUDA_TOOLKIT"
    fi

    if [[ "${SYSTEM_VTK_DIR}" == "" ]] ; then
        if [[ "${VTK_INSTALL_DIR}" == "" ]] ; then
            export EAVL_VTK_ARGS=""
        else
            export EAVL_VTK_ARGS=" --with-vtk=$VISITDIR/$VTK_INSTALL_DIR/$VTK_VERSION/$VISITARCH"
        fi
    else
        export EAVL_VTK_ARGS=" --with-vtk=$SYSTEM_VTK_DIR"
    fi
        
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

    cp lib/libeavl.a         "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/lib"
    cp config/eavlConfig.h   "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp config/eavlPlatform.h "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/common/*.h        "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/exporters/*.h     "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/importers/*.h     "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/rendering/*.h     "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/filters/*.h       "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/math/*.h          "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/fonts/*.h         "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/operations/*.h    "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"
    cp src/vtk/*.h           "$VISITDIR/eavl/$EAVL_VERSION/$VISITARCH/include"


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
    if [[ "$USE_SYSTEM_EAVL" == "yes" ]]; then
        return 1
    fi

    check_if_installed "eavl" $EAVL_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_eavl_build
{
cd "$START_DIR"
if [[ "$DO_EAVL" == "yes" && "$USE_SYSTEM_EAVL" == "no" ]] ; then
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

