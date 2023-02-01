function bv_vtkh_initialize
{
    export DO_VTKH="no"
    export USE_SYSTEM_VTKH="no"
    add_extra_commandline_args "vtkh" "alt-vtkh-dir" 1 "Use alternative directory for VTKh"
}

function bv_vtkh_enable
{
    DO_VTKH="yes"
}

function bv_vtkh_disable
{
    DO_VTKH="no"
}

function bv_vtkh_alt_vtkh_dir
{
    bv_vtkh_enable
    USE_SYSTEM_VTKH="yes"
    VTKH_INSTALL_DIR="$1"
    info "Using Alternate VTKH: $VTKH_INSTALL_DIR"
}

function bv_vtkh_depends_on
{
    depends_on="vtkm"

    echo ${depends_on}
}

function bv_vtkh_initialize_vars
{
    if [[ "$USE_SYSTEM_VTKH" == "no" ]]; then
        VTKH_INSTALL_DIR="\${VISITHOME}/vtkh/$VTKH_VERSION/\${VISITARCH}"
    fi
}

function bv_vtkh_info
{
    export VTKH_VERSION=${VTKH_VERSION:-"v0.8.0"}
    export VTKH_FILE=${VTKH_FILE:-"vtkh-${VTKH_VERSION}.tar.gz"}
    export VTKH_BUILD_DIR=${VTKH_BUILD_DIR:-"vtkh-${VTKH_VERSION}"}
    export VTKH_MD5_CHECKSUM="9e231ce66cf483116423540f64163a26"
    export VTKH_SHA256_CHECKSUM="8366ebfe094c258555f343ba1f9bbad1d8e4804f844768b639f6ff13a6390f29"
}

function bv_vtkh_print
{
    printf "%s%s\n" "VTKH_FILE=" "${VTKH_FILE}"
    printf "%s%s\n" "VTKH_VERSION=" "${VTKH_VERSION}"
    printf "%s%s\n" "VTKH_BUILD_DIR=" "${VTKH_BUILD_DIR}"
}

function bv_vtkh_print_usage
{
    printf "%-20s %s [%s]\n" "--vtkh" "Build VTKh support" "$DO_VTKH"
}

function bv_vtkh_host_profile
{
    if [[ "$DO_VTKH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VTKH" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_VTKH_DIR ${VTKH_INSTALL_DIR})" \
            >> $HOSTCONF
    fi
}

function bv_vtkh_ensure
{
    if [[ "$DO_VTKH" == "yes" && "$USE_SYSTEM_VTKH" == "no" ]] ; then
        ensure_built_or_ready "vtk-h" $VTKH_VERSION $VTKH_BUILD_DIR $VTKH_FILE $VTKH_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_VTKH="no"
            error "Unable to build VTKh. ${VTKH_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_vtkh
#
#
# *************************************************************************** #

function apply_vtkh_patch
{
    info "Patching VTKh . . ."

    return 0
}

function build_vtkh
{
    #
    # Extract the sources
    #
    if [[ -d $VTKH_BUILD_DIR ]] ; then
        if [[ ! -f $VTKH_FILE ]] ; then
            warn "The directory VTKH exists, deleting before uncompressing"
            rm -Rf $VTKH_BUILD_DIR
            ensure_built_or_ready $VTKH_INSTALL_DIR $VTKH_VERSION $VTKH_BUILD_DIR $VTKH_FILE
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $VTKH_BUILD_DIR $VTKH_FILE
    untarred_vtkh=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_vtkh == -1 ]] ; then
        warn "Unable to prepare VTKh build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    cd $VTKH_BUILD_DIR || error "Can't cd to VTKh build dir."
    apply_vtkh_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_vtkh == 1 ]] ; then
            warn "Giving up on VTKh build because the patch failed."
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
    # Configure VTKH
    #
    info "Configuring VTKh . . ."

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    # Make a build directory for an out-of-source build.. Change the
    # VTKH_BUILD_DIR variable to represent the out-of-source build directory.
    VTKH_SRC_DIR=$VTKH_BUILD_DIR
    VTKH_BUILD_DIR="${VTKH_SRC_DIR}-build"
    if [[ ! -d $VTKH_BUILD_DIR ]] ; then
        echo "Making build directory $VTKH_BUILD_DIR"
        mkdir $VTKH_BUILD_DIR
    fi

    cd $VTKH_BUILD_DIR || error "Can't cd to VTKh build dir."

    vopts=""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/vtkh/${VTKH_VERSION}/${VISITARCH}"
    vopts="${vopts} -DVTKM_DIR=${VISITDIR}/vtkm/${VTKM_VERSION}/${VISITARCH}"
    vopts="${vopts} -DENABLE_MPI=OFF"
    vopts="${vopts} -DENABLE_OPENMP=OFF"
    vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    vopts="${vopts} -DCMAKE_BUILD_TYPE=${VISIT_BUILD_MODE}"
    # Disable CUDA support for now since it requires using the CUDA compiler
    # to build all of VisIt, which we don't want to do.
    #if [[ -d $CUDA_HOME ]]; then
    #    # Turn off shared libraries with CUDA support, since VTKm
    #    # requires static libraries with CUDA.
    #    echo "Building with CUDA support."
    #    vopts="${vopts} -DENABLE_CUDA=ON"
    #    vopts="${vopts} -DBUILD_SHARED_LIBS=OFF"
    #else
    #    vopts="${vopts} -DBUILD_SHARED_LIBS=ON"
    #fi
    vopts="${vopts} -DBUILD_SHARED_LIBS=ON"

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
    echo "\"${CMAKE_BIN}\"" ${vopts} ../${VTKH_SRC_DIR}/src > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "VTKh configuration failed."

    #
    # Build vtkh
    #
    info "Building VTKh . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS || error "VTKh did not build correctly. Giving up."

    info "Installing VTKh . . . (~2 minutes)"
    $MAKE install || error "VTKh did not install correctly."

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/vtkh"
        chgrp -R ${GROUP} "$VISITDIR/vtkh"
    fi
    cd "$START_DIR"
    info "Done with VTKh"
    return 0
}

function bv_vtkh_is_enabled
{
    if [[ $DO_VTKH == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_vtkh_is_installed
{
    if [[ "$USE_SYSTEM_VTKH" == "yes" ]]; then
        return 1
    fi

    check_if_installed "vtkh" $VTKH_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_vtkh_build
{
    cd "$START_DIR"
    if [[ "$DO_VTKH" == "yes" && "$USE_SYSTEM_VTKH" == "no" ]] ; then
        check_if_installed "vtkh" $VTKH_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping VTKh build. VTKh is already installed."
        else
            info "Building VTKh (~2 minutes)"
            build_vtkh
            if [[ $? != 0 ]] ; then
                error "Unable to build or install VTKh.  Bailing out."
            fi
            info "Done building VTKh"
        fi
    fi
}
