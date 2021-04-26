function bv_vtkh_initialize
{
    export DO_VTKH="no"
}

function bv_vtkh_enable
{
    DO_VTKH="yes"
}

function bv_vtkh_disable
{
    DO_VTKH="no"
}

function bv_vtkh_depends_on
{
    depends_on="vtkm"

    echo ${depends_on}
}

function bv_vtkh_initialize_vars
{
    VTKH_INSTALL_DIR="\${VISITHOME}/vtkh/$VTKH_VERSION/\${VISITARCH}"
}

function bv_vtkh_info
{
    export VTKH_VERSION=${VTKH_VERSION:-"v0.6.6"}
    export VTKH_FILE=${VTKH_FILE:-"vtkh-${VTKH_VERSION}.tar.gz"}
    export VTKH_SRC_DIR=${VTKH_SRC_DIR:-"${VTKH_FILE%.tar*}"}
    export VTKH_BUILD_DIR=${VTKH_BUILD_DIR:-"${VTKH_SRC_DIR}-build"}
    export VTKH_MD5_CHECKSUM="ec9bead5d3bcc317149fb273f7c5a4af"
    export VTKH_SHA256_CHECKSUM="5fe8bae5f55dbeb3047a37499cc41f3b548e4d86f0058993069f1df57f7915a1"
}

function bv_vtkh_print
{
    printf "%s%s\n" "VTKH_FILE=" "${VTKH_FILE}"
    printf "%s%s\n" "VTKH_VERSION=" "${VTKH_VERSION}"
    printf "%s%s\n" "VTKH_SRC_DIR=" "${VTKH_SRC_DIR}"
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
        echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR \${VISITHOME}/vtkh/\${VTKH_VERSION}/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_vtkh_ensure
{
    if [[ "$DO_VTKH" == "yes" ]] ; then
        check_installed_or_have_src "vtk-h" $VTKH_VERSION $VTKH_SRC_DIR $VTKH_FILE $VTKH_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_VTKH="no"
            error "Unable to build VTKh. ${VTKH_FILE} not found."
        fi
    fi
}

function bv_vtkh_dry_run
{
    if [[ "$DO_VTKH" == "yes" ]] ; then
        echo "Dry run option not set for VTKh"
    fi
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
    check_if_installed "vtkh" $VTKH_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_vtkh_build
{
    cd "$START_DIR"

    if [[ "$DO_VTKH" == "yes" ]] ; then
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

# *************************************************************************** #
#                            Function 6, patch_vtk                            #
# *************************************************************************** #
function apply_vtkh_patch
{
    cd ${VTKH_SRC_DIR} || error "Can't cd to VTKh source dir."

#    info "Patching VTKh . . ."

    cd "$START_DIR"

    return 0
}

# *************************************************************************** #
#                            Function 8, build_vtkh                           #
# *************************************************************************** #
function build_vtkh
{
    #
    # Uncompress the source file
    #
    uncompress_src_file $VTKH_SRC_DIR $VTKH_FILE
    untarred_vtkh=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_vtkh == -1 ]] ; then
        warn "Unable to uncompress VTKh source file. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
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

    #
    # Make a build directory for an out-of-source build.
    #
    cd "$START_DIR"
    if [[ ! -d $VTKH_BUILD_DIR ]] ; then
        echo "Making build directory $VTKH_BUILD_DIR"
        mkdir $VTKH_BUILD_DIR
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf ${VTKH_BUILD_DIR}/CMakeCache.txt ${VTKH_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd ${VTKH_BUILD_DIR}

    #
    # Configure VTKH
    #
    info "Configuring VTKh . . ."

    vopts=""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/vtkh/${VTKH_VERSION}/${VISITARCH}"
    vopts="${vopts} -DVTKM_DIR=${VISITDIR}/vtkm/${VTKM_VERSION}/${VISITARCH}"
    vopts="${vopts} -DENABLE_MPI=OFF"
    vopts="${vopts} -DENABLE_OPENMP=OFF"
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
    # Several platforms have had problems with the VTKh cmake configure command
    # issued simply via "issue_command". This was first discovered on
    # BGQ and then showed up in random cases for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - we write a simple
    # script that we invoke with bash which calls cmake with all of the properly
    # arguments. We are now using this strategy for all platforms.
    #
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${vopts} ../${VTKH_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "VTKh configure failed. Giving up"
        return 1
    fi

    #
    # Now build VTKh.
    #
    info "Building VTKh . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "VTKh build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing VTKh . . . "
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "VTKh install failed. Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/vtkh"
        chgrp -R ${GROUP} "$VISITDIR/vtkh"
    fi
    cd "$START_DIR"
    info "Done with VTKh"
    return 0
}
