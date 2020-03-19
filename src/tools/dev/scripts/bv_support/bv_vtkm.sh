function bv_vtkm_initialize
{
    export DO_VTKM="no"
    export USE_SYSTEM_VTKM="no"
    add_extra_commandline_args "vtkm" "alt-vtkm-dir" 1 "Use alternative directory for VTKm"
}

function bv_vtkm_enable
{
    DO_VTKM="yes"
}

function bv_vtkm_disable
{
    DO_VTKM="no"
}

function bv_vtkm_alt_vtkm_dir
{
    bv_vtkm_enable
    USE_SYSTEM_VTKM="yes"
    VTKM_INSTALL_DIR="$1"
    info "Using Alternate VTKM: $SYSTEM_VTKM_DIR"
}

function bv_vtkm_depends_on
{
    depends_on="cmake"

    echo ${depends_on}
}

function bv_vtkm_initialize_vars
{
    if [[ "$USE_SYSTEM_VTKM" == "no" ]]; then
        VTKM_INSTALL_DIR="\${VISITHOME}/vtkm/$VTKM_VERSION/\${VISITARCH}"
    fi
}

function bv_vtkm_info
{
    export VTKM_VERSION=${VTKM_VERSION:-"0d141c"}
    export VTKM_FILE=${VTKM_FILE:-"vtkm-${VTKM_VERSION}.tar.gz"}
    export VTKM_BUILD_DIR=${VTKM_BUILD_DIR:-"vtkm-${VTKM_VERSION}"}
    export VTKM_MD5_CHECKSUM="72f862af47da3578370734f3ba0a4c59"
    export VTKM_SHA256_CHECKSUM="965b6bb7a1a579b8494523b993f060b6bc461a5dc0296b8ad6033ff10916628f"
}

function bv_vtkm_print
{
    printf "%s%s\n" "VTKM_FILE=" "${VTKM_FILE}"
    printf "%s%s\n" "VTKM_VERSION=" "${VTKM_VERSION}"
    printf "%s%s\n" "VTKM_BUILD_DIR=" "${VTKM_BUILD_DIR}"
}

function bv_vtkm_print_usage
{
    printf "%-20s %s [%s]\n" "--vtkm" "Build VTKm support" "$DO_VTKM"
    printf "%-20s %s [%s]\n" "--alt-vtkm-dir" "Use VTKm from an alternative directory"
}

function bv_vtkm_host_profile
{
    if [[ "$DO_VTKM" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VTKM" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_VTKM_DIR ${VTKM_INSTALL_DIR})" \
            >> $HOSTCONF
    fi
}

function bv_vtkm_ensure
{
    if [[ "$DO_VTKM" == "yes" && "$USE_SYSTEM_VTKM" == "no" ]] ; then
        ensure_built_or_ready "vtk-m" $VTKM_VERSION $VTKM_BUILD_DIR $VTKM_FILE $VTKM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_VTKM="no"
            error "Unable to build VTKm. ${VTKM_FILE} not found."
        fi
    fi
}

function bv_vtkm_dry_run
{
    if [[ "$DO_VTKM" == "yes" ]] ; then
        echo "Dry run option not set for VTKm"
    fi
}

# *************************************************************************** #
#                            Function 8, build_vtkm
#
#
# *************************************************************************** #

function apply_patch_1
{
   patch -p0 << \EOF
diff -c ./vtkm/TypeListTag.h.orig ./vtkm/TypeListTag.h
*** ./vtkm/TypeListTag.h.orig	Tue Nov  6 16:17:57 2018
--- ./vtkm/TypeListTag.h	Tue Nov  6 16:15:29 2018
***************
*** 204,210 ****
  /// A list of the most commonly used types across multiple domains. Includes
  /// integers, floating points, and 3 dimensional vectors of floating points.
  ///
! struct TypeListTagCommon : vtkm::ListTagBase<vtkm::Int32,
                                               vtkm::Int64,
                                               vtkm::Float32,
                                               vtkm::Float64,
--- 204,211 ----
  /// A list of the most commonly used types across multiple domains. Includes
  /// integers, floating points, and 3 dimensional vectors of floating points.
  ///
! struct TypeListTagCommon : vtkm::ListTagBase<vtkm::UInt8,
!                                              vtkm::Int32,
                                               vtkm::Int64,
                                               vtkm::Float32,
                                               vtkm::Float64,
EOF

    if [[ $? != 0 ]] ; then
      warn "vtkm patch 1 failed."
      return 1
    fi
    return 0;
}

function apply_patch_2
{
   patch -p0 << \EOF
diff -c ./vtkm/cont/arg/TransportTagTopologyFieldIn.h.orig ./vtkm/cont/arg/TransportTagTopologyFieldIn.h
*** ./vtkm/cont/arg/TransportTagTopologyFieldIn.h.orig	Tue Nov  6 16:18:09 2018
--- ./vtkm/cont/arg/TransportTagTopologyFieldIn.h	Tue Nov  6 16:16:18 2018
***************
*** 98,104 ****
--- 98,106 ----
    {
      if (object.GetNumberOfValues() != detail::TopologyDomainSize(inputDomain, TopologyElementTag()))
      {
+ #if 0
        throw vtkm::cont::ErrorBadValue("Input array to worklet invocation the wrong size.");
+ #endif
      }
  
      return object.PrepareForInput(Device());
EOF

    if [[ $? != 0 ]] ; then
      warn "vtkm patch 2 failed."
      return 1
    fi
    return 0;
}

function apply_vtkm_patch
{
    info "Patching VTKm . . ."

    apply_patch_1
    if [[ $? != 0 ]] ; then
       return 1
    fi

    apply_patch_2
    if [[ $? != 0 ]] ; then
       return 1
    fi

    return 0
}

function build_vtkm
{
    #
    # Extract the sources
    #
    if [[ -d $VTKM_BUILD_DIR ]] ; then
        if [[ ! -f $VTKM_FILE ]] ; then
            warn "The directory VTKM exists, deleting before uncompressing"
            rm -Rf $VTKM_BUILD_DIR
            ensure_built_or_ready $VTKM_INSTALL_DIR $VTKM_VERSION $VTKM_BUILD_DIR $VTKM_FILE
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $VTKM_BUILD_DIR $VTKM_FILE
    untarred_vtkm=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_vtkm == -1 ]] ; then
        warn "Unable to prepare VTKm build directory. Giving Up!"
        return 1
    fi
    
    #
    # Apply patches
    #
    cd $VTKM_BUILD_DIR || error "Can't cd to VTKm build dir."
    apply_vtkm_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_vtkm == 1 ]] ; then
            warn "Giving up on VTKm build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi
    # move back up to the start dir
    cd "$START_DIR"

    #
    # Configure VTKM
    #
    info "Configuring VTKm . . ."
    
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    # Make a build directory for an out-of-source build.. Change the
    # VTKM_BUILD_DIR variable to represent the out-of-source build directory.
    VTKM_SRC_DIR=$VTKM_BUILD_DIR
    VTKM_BUILD_DIR="${VTKM_SRC_DIR}-build"
    if [[ ! -d $VTKM_BUILD_DIR ]] ; then
        echo "Making build directory $VTKM_BUILD_DIR"
        mkdir $VTKM_BUILD_DIR
    fi

    cd $VTKM_BUILD_DIR || error "Can't cd to VTKm build dir."

    vopts=""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/vtkm/${VTKM_VERSION}/${VISITARCH}"
    vopts="${vopts} -DVTKm_ENABLE_TESTING:BOOL=OFF"
    vopts="${vopts} -DVTKm_ENABLE_RENDERING:BOOL=ON"
    vopts="${vopts} -DVTKm_USE_64BIT_IDS:BOOL=OFF"
    vopts="${vopts} -DVTKm_USE_DOUBLE_PRECISION:BOOL=ON"
    vopts="${vopts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    if [[ -d $CUDA_HOME ]]; then
        echo "Building with CUDA support."
        vopts="${vopts} -DVTKm_ENABLE_CUDA:BOOL=ON"
        vopts="${vopts} -DVTKm_CUDA_Architecture=kepler"
    fi

    #
    # Several platforms have had problems with the VTK cmake configure
    # command issued simply via "issue_command".  This was first discovered
    # on BGQ and then showed up in random cases for both OSX and Linux
    # machines. Brad resolved this on BGQ  with a simple work around - we
    # write a simple script that we invoke with bash which calls cmake with
    # all of the properly arguments. We are now using this strategy for all
    # platforms.
    #
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${vopts} ../${VTKM_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "VTKm configuration failed."

    #
    # Build vtkm
    #
    info "Building VTKm . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS || error "VTKm did not build correctly. Giving up."

    info "Installing VTKm . . . (~2 minutes)"
    $MAKE install || error "VTKm did not install correctly."

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/vtkm"
        chgrp -R ${GROUP} "$VISITDIR/vtkm"
    fi
    cd "$START_DIR"
    info "Done with vtkm"
    return 0
}

function bv_vtkm_is_enabled
{
    if [[ $DO_VTKM == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_vtkm_is_installed
{
    if [[ "$USE_SYSTEM_VTKM" == "yes" ]]; then
        return 1
    fi

    check_if_installed "vtkm" $VTKM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_vtkm_build
{
    cd "$START_DIR"
    if [[ "$DO_VTKM" == "yes" && "$USE_SYSTEM_VTKM" == "no" ]] ; then
        check_if_installed "vtkm" $VTKM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping VTKm build. VTKm is already installed."
        else
            info "Building VTKm (~2 minutes)"
            build_vtkm
            if [[ $? != 0 ]] ; then
                error "Unable to build or install VTKm.  Bailing out."
            fi
            info "Done building VTKm"
        fi
    fi
}
