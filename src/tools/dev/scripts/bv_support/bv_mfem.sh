function bv_mfem_initialize
{
    export DO_MFEM="no"
}

function bv_mfem_enable
{
    DO_MFEM="yes"
}

function bv_mfem_disable
{
    DO_MFEM="no"
}

function bv_mfem_depends_on
{
    local depends_on="zlib"

    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        depends_on="$depends_on conduit"
    fi
    if [[ "$DO_FMS" == "yes" ]] ; then
        depends_on="$depends_on fms"
    fi

    echo $depends_on
}

function bv_mfem_info
{
    # NOTE: we are using a special version of MFEM that has not yet been in an MFEM release.
    export MFEM_VERSION=${MFEM_VERSION:-"Add_FMS_support"}
    export MFEM_FILE=${MFEM_FILE:-"mfem-${MFEM_VERSION}.tgz"}
    export MFEM_BUILD_DIR=${MFEM_BUILD_DIR:-"mfem-${MFEM_VERSION}"}
    # For the time being, get MFEM tarball from IL website if it cannot be downloaded from third-party
    export MFEM_URL=${MFEM_URL:-"http://visit.ilight.com/assets"}
    #export MFEM_URL=${MFEM_URL:-"https://bit.ly/mfem-4-0"}
    export MFEM_MD5_CHECKSUM="a8124f5a43bef277aeb659a492c05600"
    export MFEM_SHA256_CHECKSUM="0dec3ebd2163bb2f104bdb6f4009da09a05ac27108b0d2f820c7f67cfda6921a"
}

function bv_mfem_print
{
    printf "%s%s\n" "MFEM_FILE=" "${MFEM_FILE}"
    printf "%s%s\n" "MFEM_VERSION=" "${MFEM_VERSION}"
    printf "%s%s\n" "MFEM_BUILD_DIR=" "${MFEM_BUILD_DIR}"
}

function bv_mfem_print_usage
{
    printf "%-20s %s [%s]\n" "--mfem" "Build mfem support" "$DO_MFEM"
}

function bv_mfem_host_profile
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MFEM " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MFEM_DIR \${VISITHOME}/mfem/$MFEM_VERSION/\${VISITARCH})" \
            >> $HOSTCONF

        ZLIB_LIBDEP="\${VISITHOME}/zlib/\${ZLIB_VERSION}/\${VISITARCH}/lib z"

        CONDUIT_LIBDEP=""
        INCDEP=""
        if [[ "$DO_CONDUIT" == "yes" ]] ; then
            CONDUIT_LIBDEP="\${VISIT_CONDUIT_LIBDEP}"
            INCDEP="CONDUIT_INCLUDE_DIR"
        fi
        if [[ "$DO_FMS" == "yes" ]] ; then
            INCDEP="$INCDEP FMS_INCLUDE_DIR"
        fi

        if [[ "$INCDEP" != "" ]] ; then
             echo \
                "VISIT_OPTION_DEFAULT(VISIT_MFEM_INCDEP $INCDEP TYPE STRING)" \
                    >> $HOSTCONF
        fi
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MFEM_LIBDEP $CONDUIT_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
    fi
}

function bv_mfem_ensure
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        ensure_built_or_ready "mfem" $MFEM_VERSION $MFEM_BUILD_DIR $MFEM_FILE $MFEM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MFEM="no"
            error "Unable to build mfem.  ${MFEM_FILE} not found."
        fi
    fi
}

function bv_mfem_dry_run
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        echo "Dry run option not set for mfem."
    fi
}

# *************************************************************************** #
#                            Function 8, build_mfem
# *************************************************************************** #
function build_mfem
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MFEM_BUILD_DIR $MFEM_FILE
    untarred_mfem=$?
    if [[ $untarred_mfem == -1 ]] ; then
        warn "Unable to prepare mfem build directory. Giving Up!"
        return 1
    fi

    cd $MFEM_BUILD_DIR || error "Can't cd to mfem build dir."
    mkdir build
    cd build || error "Can't cd to MFEM build dir."

    # Version 4.0 now requires c++11
    CXXFLAGS="-std=c++11 ${CXXFLAGS}"

    vopts="-DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS} $CXXFLAGS\""
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/mfem/${MFEM_VERSION}/${VISITARCH}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    vopts="${vopts} -DMFEM_USE_EXCEPTIONS:BOOL=ON"
    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        vopts="${vopts} -DMFEM_USE_CONDUIT=ON -DCONDUIT_DIR=${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}"
    fi
    if [[ "$DO_FMS" == "yes" ]] ; then
        vopts="${vopts} -DMFEM_USE_FMS=ON -DFMS_DIR=${VISITDIR}/fms/${FMS_VERSION}/${VISITARCH}"
    fi
    vopts="${vopts} -DMFEM_USE_ZLIB=ON"

    #
    # Call configure
    #
    info "Configuring mfem . . ."
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    CMS=bv_run_cmake.sh
    echo "#!/bin/bash" > $CMS
    echo "# Find the right ZLIB" >> $CMS
    echo "export CMAKE_PREFIX_PATH=${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}" >> $CMS
    if [[ "$DO_HDF5" == "yes" ]] ; then
        echo "# Find the right HDF5" >> $CMS
        echo "export HDF5_ROOT=${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}" >> $CMS
    fi
    echo "\"${CMAKE_BIN}\" ${vopts} .." >> $CMS
    cat $CMS
    issue_command bash $CMS || error "FMS configuration failed."

    #
    # Build mfem
    #
    info "Building mfem . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "mfem build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing mfem"
    $MAKE install

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/mfem"
        chgrp -R ${GROUP} "$VISITDIR/mfem"
    fi
    cd "$START_DIR"
    info "Done with mfem"
    return 0
}


function bv_mfem_is_enabled
{
    if [[ $DO_MFEM == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mfem_is_installed
{
    check_if_installed "mfem" $MFEM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mfem_build
{
    cd "$START_DIR"
    if [[ "$DO_MFEM" == "yes" ]] ; then
        check_if_installed "mfem" $MFEM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping mfem build.  mfem is already installed."
        else
            info "Building mfem (~2 minutes)"
            build_mfem
            if [[ $? != 0 ]] ; then
                error "Unable to build or install mfem.  Bailing out."
            fi
            info "Done building mfem"
        fi
    fi
}
