function bv_cfitsio_initialize
{
    export DO_CFITSIO="no"
}

function bv_cfitsio_enable
{
    DO_CFITSIO="yes"
}

function bv_cfitsio_disable
{
    DO_CFITSIO="no"
}

function bv_cfitsio_depends_on
{
    echo "cmake zlib"
}

function bv_cfitsio_info
{
    export CFITSIO_FILE=${CFITSIO_FILE:-"cfitsio-4.6.3.tar.gz"}
    export CFITSIO_VERSION=${CFITSIO_VERSION:-"4.6.3"}
    export CFITSIO_COMPATIBILITY_VERSION=${CFITSIO_COMPATIBILITY_VERSION:-"4.0"}
    export CFITSIO_BUILD_DIR=${CFITSIO_BUILD_DIR:-"cfitsio-${CFITSIO_VERSION}"}
    export CFITSIO_SHA256_CHECKSUM="fad44fff274fdda5ffcc0c0fff3bc3c596362722b9292fc8944db91187813600"
}

function bv_cfitsio_print
{
    printf "%s%s\n" "CFITSIO_FILE=" "${CFITSIO_FILE}"
    printf "%s%s\n" "CFITSIO_VERSION=" "${CFITSIO_VERSION}"
    printf "%s%s\n" "CFITSIO_COMPATIBILITY_VERSION=" "${CFITSIO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CFITSIO_BUILD_DIR=" "${CFITSIO_BUILD_DIR}"
}

function bv_cfitsio_print_usage
{
    printf "%-20s %s [%s]\n" "--cfitsio" "Build CFITSIO" "$DO_CFITSIO"
}

function bv_cfitsio_host_profile
{
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CFITSIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR \${VISITHOME}/cfitsio/$CFITSIO_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi

}
function bv_cfitsio_ensure
{
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        ensure_built_or_ready "cfitsio" $CFITSIO_VERSION $CFITSIO_BUILD_DIR $CFITSIO_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CFITSIO="no"
            error "Unable to build CFITSIO.  ${CFITSIO_FILE} not found."
        fi
    fi
}
# *************************************************************************** #
#                         Function 8.9, build_cfitsio                         #
# *************************************************************************** #

function build_cfitsio
{
    #
    # CFITSIO uses CMake -- make sure we have it.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "CFITSIO: CMake found"
    else
        warn "Unable to find cmake, cannot build CFITSIO. Giving up."
        return 1
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $CFITSIO_BUILD_DIR $CFITSIO_FILE SHA256 $CFITSIO_SHA256_CHECKSUM
    untarred_cfitsio=$?
    if [[ $untarred_cfitsio == -1 ]] ; then
        warn "Unable to prepare CFITSIO Build Directory. Giving Up"
        return 1
    fi

    #
    # Configure CFITSIO (CMake)
    #
    info "Configuring CFITSIO . . ."

    CFITSIO_SRC_DIR="${CFITSIO_BUILD_DIR}"
    CFITSIO_BUILD_SUBDIR="${CFITSIO_SRC_DIR}-build"
    CFITSIO_INSTALL_DIR="${VISITDIR}/cfitsio/${CFITSIO_VERSION}/${VISITARCH}"

    if [[ ! -d "${CFITSIO_BUILD_SUBDIR}" ]] ; then
        mkdir "${CFITSIO_BUILD_SUBDIR}" || error "Unable to create CFITSIO build directory."
    fi

    cd "${CFITSIO_BUILD_SUBDIR}" || error "Can't cd to CFITSIO build dir."

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf CMakeCache.txt */CMakeCache.txt

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cfitsio_cmake_build_type="Debug"
    else
        cfitsio_cmake_build_type="Release"
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cfitsio_build_shared="OFF"
    else
        cfitsio_build_shared="ON"
    fi

    cmake_opts=""
    cmake_opts="${cmake_opts} -DCMAKE_INSTALL_PREFIX:PATH=${CFITSIO_INSTALL_DIR}"
    cmake_opts="${cmake_opts} -DCMAKE_BUILD_TYPE:STRING=${cfitsio_cmake_build_type}"
    cmake_opts="${cmake_opts} -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON"
    cmake_opts="${cmake_opts} -DBUILD_SHARED_LIBS:BOOL=${cfitsio_build_shared}"
    cmake_opts="${cmake_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    cmake_opts="${cmake_opts} -DCMAKE_C_FLAGS:STRING=\"${CFLAGS} ${C_OPT_FLAGS}\""

    # cfitsio options
    cmake_opts="${cmake_opts} -DTESTS:BOOL=OFF"
    cmake_opts="${cmake_opts} -DUSE_CURL:BOOL=OFF"
    cmake_opts="${cmake_opts} -DUTILS:BOOL=OFF"

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        cmake_opts="${cmake_opts} -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}"
        cmake_opts="${cmake_opts} -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}"
    fi

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_COMMAND}\"" ${cmake_opts} "../${CFITSIO_SRC_DIR}" >> bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "CFITSIO configuration failed."

    #
    # Build CFITSIO
    #
    info "Building CFITSIO . . . (~2 minutes)"

    ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS || error "CFITSIO did not build correctly. Giving up."

    #
    # Install into the VisIt third party location.
    #
    info "Installing CFITSIO . . ."
    ${CMAKE_COMMAND} --install . || error "CFITSIO did not install correctly. Giving up."

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/cfitsio"
        chgrp -R ${GROUP} "$VISITDIR/cfitsio"
    fi
    cd "$START_DIR"
    info "Done with CFITSIO"
    return 0
}

function bv_cfitsio_is_enabled
{
    if [[ $DO_CFITSIO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cfitsio_is_installed
{
    check_if_installed "cfitsio" $CFITSIO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cfitsio_build
{
    cd "$START_DIR"
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        check_if_installed "cfitsio" $CFITSIO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CFITSIO build.  CFITSIO is already installed."
        else
            info "Building CFITSIO (~2 minutes)"
            build_cfitsio
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CFITSIO.  Bailing out."
            fi
            info "Done building CFITSIO"
        fi
    fi
}
