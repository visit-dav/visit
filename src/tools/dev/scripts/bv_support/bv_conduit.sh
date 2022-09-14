function bv_conduit_initialize
{
    export DO_CONDUIT="no"
}

function bv_conduit_enable
{
    DO_CONDUIT="yes"
}

function bv_conduit_disable
{
    DO_CONDUIT="no"
}

function bv_conduit_depends_on
{
    local depends_on=""

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="hdf5"
    fi

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        depends_on="$depends_on zlib"
    fi

    if [[ "$DO_PYTHON" == "yes" ]] ; then
        depends_on="$depends_on python"
    fi

    if [[ "$DO_MPICH" == "yes" ]] ; then
        depends_on="$depends_on mpich"
    fi
    
    echo $depends_on
}

function bv_conduit_info
{
    export CONDUIT_VERSION=${CONDUIT_VERSION:-"v0.8.4"}
    export CONDUIT_FILE=${CONDUIT_FILE:-"conduit-${CONDUIT_VERSION}-src-with-blt.tar.gz"}
    export CONDUIT_COMPATIBILITY_VERSION=${CONDUIT_COMPATIBILITY_VERSION:-"v0.8.0"}
    export CONDUIT_BUILD_DIR=${CONDUIT_BUILD_DIR:-"conduit-${CONDUIT_VERSION}"}
    export CONDUIT_MD5_CHECKSUM="25e6354026185f304f25b91c4348b72f"
    export CONDUIT_SHA256_CHECKSUM="55c37ddc668dbc45d43b60c440192f76e688a530d64f9fe1a9c7fdad8cd525fd"
}

function bv_conduit_print
{
    printf "%s%s\n" "CONDUIT_FILE=" "${CONDUIT_FILE}"
    printf "%s%s\n" "CONDUIT_VERSION=" "${CONDUIT_VERSION}"
    printf "%s%s\n" "CONDUIT_COMPATIBILITY_VERSION=" "${CONDUIT_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CONDUIT_BUILD_DIR=" "${CONDUIT_BUILD_DIR}"
}

function bv_conduit_print_usage
{
    printf "%-20s %s [%s]\n" "--conduit"   "Build Conduit" "$DO_CONDUIT"
}

function bv_conduit_host_profile
{
    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Conduit" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        # Need to remove the 'v' from the version string for cmake
        echo "SETUP_APP_VERSION(CONDUIT ${CONDUIT_VERSION:1})" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_CONDUIT_DIR \${VISITHOME}/conduit/$CONDUIT_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        if [[ "$DO_HDF5" == "yes" ]] ; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_CONDUIT_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_conduit_ensure
{
    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        ensure_built_or_ready "conduit" $CONDUIT_VERSION $CONDUIT_BUILD_DIR $CONDUIT_FILE $CONDUIT_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CONDUIT="no"
            error "Unable to build Conduit.  ${CONDUIT_FILE} not found."
        fi
    fi
}

function apply_conduit_patch
{
    return 0
}

# *************************************************************************** #
# build_conduit
# *************************************************************************** #

function build_conduit
{
    #
    # Conduit uses CMake  -- make sure we have it built.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "Conduit: CMake found"
    else
        build_cmake
        if [[ $? != 0 ]] ; then
            warn "Unable to build cmake.  Giving up"
            return 1
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $CONDUIT_BUILD_DIR $CONDUIT_FILE
    untarred_conduit=$?
    if [[ $untarred_conduit == -1 ]] ; then
        warn "Unable to prepare Conduit build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching Conduit . . ."
    cd $CONDUIT_BUILD_DIR || error "Can't cd to Conduit build dir."
    apply_conduit_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_conduit == 1 ]] ; then
            warn "Giving up on Conduit build because the patch failed."
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
    # Call configure
    #
    info "Configuring Conduit . . ."

    # Make a build directory for an out-of-source build.. Change the
    # VISIT_BUILD_DIR variable to represent the out-of-source build directory. 
    CONDUIT_SRC_DIR=$CONDUIT_BUILD_DIR
    CONDUIT_BUILD_DIR="${CONDUIT_SRC_DIR}-build"
    if [[ ! -d $CONDUIT_BUILD_DIR ]] ; then
        echo "Making build directory $CONDUIT_BUILD_DIR"
        mkdir $CONDUIT_BUILD_DIR
    fi

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf $CONDUIT_BUILD_DIR/CMakeCache.txt $CONDUIT_BUILD_DIR/*/CMakeCache.txt


    conduit_build_mode="${VISIT_BUILD_MODE}"
    conduit_install_path="${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}"


    cfg_opts=""
    # normal stuff
    cfg_opts="${cfg_opts} -DCMAKE_BUILD_TYPE:STRING=${conduit_build_mode}"
    cfg_opts="${cfg_opts} -DCMAKE_INSTALL_PREFIX:PATH=${conduit_install_path}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    
    cfg_opts="${cfg_opts} -DENABLE_TESTS:BOOL=false"
    cfg_opts="${cfg_opts} -DENABLE_DOCS:BOOL=false"
    cfg_opts="${cfg_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    cfg_opts="${cfg_opts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    cfg_opts="${cfg_opts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS} ${PAR_LINKER_FLAGS}\""
    cfg_opts="${cfg_opts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS} ${PAR_LINKER_FLAGS}\""
    if test "${OPSYS}" = "Darwin" ; then
        cfg_opts="${cfg_opts} -DCMAKE_INSTALL_NAME_DIR:PATH=${conduit_install_path}/lib"
        if test "${MACOSX_DEPLOYMENT_TARGET}" = "10.10"; then
            # If building on 10.10 (Yosemite) check if we are building with Xcode 7 ...
            XCODE_VER=$(xcodebuild -version | head -n 1 | awk '{print $2}')
            if test ${XCODE_VER%.*} == 7; then
                # Workaround for Xcode 7 not having a 10.10 SDK: Prevent CMake from linking to 10.11 SDK
                # by using Frameworks installed in root directory.
                echo "Xcode 7 on MacOS 10.10 detected: Enabling CMake workaround"
                cfg_opts="${cfg_opts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=\"\" -DCMAKE_OSX_SYSROOT:STRING=/"
            fi
        fi
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DHDF5_DIR:STRING=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/"
    fi

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DZLIB_DIR:STRING=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/"
    fi

    if [[ "$DO_PYTHON" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DPYTHON_EXECUTABLE:STRING=$PYTHON_COMMAND"
        cfg_opts="${cfg_opts} -DENABLE_PYTHON:STRING=TRUE"
    fi

    if [[ "$FC_COMPILER" != "no" ]] ; then
        cfg_opts="${cfg_opts} -DENABLE_FORTRAN:BOOL=ON"
        cfg_opts="${cfg_opts} -DCMAKE_Fortran_COMPILER:STRING=${FC_COMPILER}"
    fi

    #
    # Conduit Relay MPI Support
    #

    if [[ "$PAR_COMPILER" != "" ]] ; then
        cfg_opts="${cfg_opts} -DENABLE_MPI:BOOL=ON"
        cfg_opts="${cfg_opts} -DMPI_C_COMPILER:STRING=${PAR_COMPILER}"
        cfg_opts="${cfg_opts} -DMPI_CXX_COMPILER:STRING=${PAR_COMPILER}"
    fi
    
    if [[ "$PAR_INCLUDE" != "" ]] ; then
        cfg_opts="${cfg_opts} -DMPI_C_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
        cfg_opts="${cfg_opts} -DMPI_CXX_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
    fi
    
    if [[ "$PAR_LIBS" != "" ]] ; then
        cfg_opts="${cfg_opts} -DMPI_C_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cfg_opts="${cfg_opts} -DMPI_C_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
        cfg_opts="${cfg_opts} -DMPI_CXX_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cfg_opts="${cfg_opts} -DMPI_CXX_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
    fi
    
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    cd ${CONDUIT_BUILD_DIR}

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command".  This was first discovered on 
    # BGQ and then showed up in random cases for both OSX and Linux machines. 
    # Brad resolved this on BGQ  with a simple work around - we write a simple 
    # script that we invoke with bash which calls cmake with all of the properly
    # arguments. We are now using this strategy for all platforms.
    #

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${cfg_opts} ../${CONDUIT_SRC_DIR}/src > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "Conduit configure failed.  Giving up"
        return 1
    fi

    #
    # Build Conduit
    #
    info "Building Conduit . . . (~5 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Conduit build failed.  Giving up"
        return 1
    fi
    
    #
    # Install into the VisIt third party location.
    #
    info "Installing Conduit"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Conduit install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/conduit"
        chgrp -R ${GROUP} "$VISITDIR/conduit"
    fi
    cd "$START_DIR"
    info "Done with Conduit"
    return 0
}

function bv_conduit_is_enabled
{
    if [[ $DO_CONDUIT == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_conduit_is_installed
{
    check_if_installed "conduit" $CONDUIT_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_conduit_build
{
    cd "$START_DIR"
    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        check_if_installed "conduit" $CONDUIT_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Conduit build.  Conduit is already installed."
        else
            info "Building Conduit (~5 minutes)"
            build_conduit
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Conduit.  Bailing out."
            fi
            info "Done building Conduit"
        fi
    fi
}
