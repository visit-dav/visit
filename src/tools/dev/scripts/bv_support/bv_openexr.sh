function bv_openexr_initialize
{
    export DO_OPENEXR="no"
}

function bv_openexr_enable
{
    DO_OPENEXR="yes"
}

function bv_openexr_disable
{
    DO_OPENEXR="no"
}

function bv_openexr_depends_on
{
    echo "cmake"
}

function bv_openexr_info
{
    export OPENEXR_VERSION=${OPENEXR_VERSION:-"3.3.4"}
    export OPENEXR_FILE=${OPENEXR_FILE:-"openexr-${OPENEXR_VERSION}.tar.gz"}
    export OPENEXR_COMPATIBILITY_VERSION=${OPENEXR_COMPATIBILITY_VERSION:-"3.3"}
    export OPENEXR_BUILD_DIR=${OPENEXR_BUILD_DIR:-"openexr-${OPENEXR_VERSION}"}
    export OPENEXR_SHA256_CHECKSUM="73a6d83edcc68333afb95e133f6e12012073815a854bc41abc1a01c1db5f124c"

    export IMATH_VERSION=${IMATH_VERSION:-"3.1.12"}
    export IMATH_FILE=${IMATH_FILE:-"Imath-${IMATH_VERSION}.tar.gz"}
    export IMATH_COMPATIBILITY_VERSION=${IMATH_COMPATIBILITY_VERSION:-"3.1"}
    export IMATH_BUILD_DIR=${IMATH_BUILD_DIR:-"Imath-${IMATH_VERSION}"}
    export IMATH_SHA256_CHECKSUM="cb8ca9ca77ac4338ebbee911fc90c886011ac5b00088630bacf8ef6c6e522f0a"
}

function bv_openexr_print
{
    printf "%s%s\n" "OPENEXR_FILE=" "${OPENEXR_FILE}"
    printf "%s%s\n" "OPENEXR_VERSION=" "${OPENEXR_VERSION}"
    printf "%s%s\n" "OPENEXR_COMPATIBILITY_VERSION=" "${OPENEXR_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "OPENEXR_BUILD_DIR=" "${OPENEXR_BUILD_DIR}"

    printf "%s%s\n" "IMATH_FILE=" "${IMATH_FILE}"
    printf "%s%s\n" "IMATH_VERSION=" "${IMATH_VERSION}"
    printf "%s%s\n" "IMATH_COMPATIBILITY_VERSION=" "${IMATH_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "IMATH_BUILD_DIR=" "${IMATH_BUILD_DIR}"
}

function bv_openexr_host_profile
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OpenEXR" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(OPENEXR $OPENEXR_VERSION)" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_OPENEXR_DIR \${VISITHOME}/openexr/$OPENEXR_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_openexr_print_usage
{
    #openexr does not have an option, it is only dependent on openexr.
    printf "%-20s %s [%s]\n" "--openexr" "Build OpenEXR" "$DO_OPENEXR"
}

function bv_openexr_ensure
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        ensure_built_or_ready "openexr" $OPENEXR_VERSION $OPENEXR_BUILD_DIR $OPENEXR_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENEXR="no"
            error "Unable to build OpenEXR.  ${OPENEXR_FILE} not found."
        fi
        ensure_built_or_ready "openexr" $IMATH_VERSION $IMATH_BUILD_DIR $IMATH_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENEXR="no"
            error "Unable to build OpenEXR.  ${IMATH_FILE} not found."
        fi
    fi
}

# ***************************************************************************
# build_imath
#
# Modifications:
#
# ***************************************************************************

function build_imath
{
    #
    # Prepare build dir
    #
    prepare_build_dir $IMATH_BUILD_DIR $IMATH_FILE SHA256 $IMATH_SHA256_CHECKSUM
    untarred_imath=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_imath == -1 ]] ; then
        warn "Unable to prepare Imath Build Directory. Giving Up"
        return 1
    fi

    # Make a build directory for an out-of-source build. Change the
    # IMATH_BUILD_DIR variable to represent the out-of-source build directory.
    IMATH_SRC_DIR=$IMATH_BUILD_DIR
    IMATH_BUILD_DIR="${IMATH_SRC_DIR}-build"
    if [[ ! -d $IMATH_BUILD_DIR ]] ; then
        echo "Making build directory $IMATH_BUILD_DIR"
        mkdir $IMATH_BUILD_DIR
    fi

   
    #
    # Configure Imath
    #
    cd $IMATH_BUILD_DIR || error "Can't cd to Imath build dir."

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf ${IMATH_BUILD_DIR}/CMakeCache.txt 

    imathopts=""
    imathopts="${imathopts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"

    if test "x${DO_STATIC_BUILD}" == "xyes" ; then
        imathopts="${imathopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        imathopts="${imathopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    imathopts="${imathopts} -DBUILD_TESTING:BOOL=OFF"
    imathopts="${imathopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/openexr/${OPENEXR_VERSION}/${VISITARCH}"

    imathopts="${imathopts} -DIMATH_CXX_STANDARD:STRING=17"
    imathopts="${imathopts} -DIMATH_OUTPUT_SUBDIR:STRING="
    imathopts="${imathopts} -DIMATH_INSTALL_PKG_CONFIG:BOOL=OFF"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_INSTALL}/cmake\"" ${imathopts} ../${IMATH_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "Imath configuration failed."

    #
    # Build Imath
    #
    info "Building Imath . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Imath build failed.  Giving up"
        return 1
    fi
    info "Installing Imath . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Imath build (make install) failed.  Giving up"
        return 1
    fi


    cleanup_build_dirs $IMATH_BUILD_DIR $IMATH_SRC_DIR

    change_install_dir_perms "$VISITDIR/openexr"

    cd "$START_DIR"
    info "Done with Imath"
    return 0
}

# ***************************************************************************
# build_openexr
#
# Modifications:
#
# ***************************************************************************


function apply_openexr_cmakelists_patch
{
    info "Patching OpenEXR to disable website/src which fails to compile with gcc 10"
    patch -p0 << \EOF
--- CMakeLists.txt.orig	2025-07-07 08:28:50.888000000 -0700
+++ CMakeLists.txt	2025-07-07 08:28:18.097045000 -0700
@@ -146,10 +146,10 @@
   add_subdirectory(website)
 endif()
 
-if (OPENEXR_BUILD_LIBS AND NOT OPENEXR_IS_SUBPROJECT)
+#if (OPENEXR_BUILD_LIBS AND NOT OPENEXR_IS_SUBPROJECT)
   # Even if not building the website, still make sure the website example code compiles.
-  add_subdirectory(website/src)
-endif()
+#  add_subdirectory(website/src)
+#endif()
 
 if (OPENEXR_BUILD_PYTHON AND OPENEXR_BUILD_LIBS AND NOT OPENEXR_IS_SUBPROJECT)
   add_subdirectory(src/wrappers/python)
EOF
    if [[ $? != 0 ]] ; then
        warn "OpenEXR patch for CMakeLists.txt failed."
        return 1
    fi

    return 0;
}

function apply_openexr_patch
{
    if [[ ${OPENEXR_VERSION} == 3.3.4 ]] ; then
        apply_openexr_cmakelists_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0;
}


function build_openexr
{
    #
    # Prepare build dir
    #
    prepare_build_dir $OPENEXR_BUILD_DIR $OPENEXR_FILE SHA256 $OPENEXR_SHA256_CHECKSUM
    untarred_openexr=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_openexr == -1 ]] ; then
        warn "Unable to prepare OpenEXR Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching OpenEXR . . ."
    cd $OPENEXR_BUILD_DIR
    apply_openexr_patch

    if [[ $? != 0 ]] ; then
        if [[ $untarred_openexr == 1 ]] ; then
            warn "Giving up on OpenEXR build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi
    cd ../

    # Make a build directory for an out-of-source build. Change the
    # OPENEXR_BUILD_DIR variable to represent the out-of-source build directory.
    OPENEXR_SRC_DIR=$OPENEXR_BUILD_DIR
    OPENEXR_BUILD_DIR="${OPENEXR_SRC_DIR}-build"
    if [[ ! -d $OPENEXR_BUILD_DIR ]] ; then
        echo "Making build directory $OPENEXR_BUILD_DIR"
        mkdir $OPENEXR_BUILD_DIR
    fi

   
    #
    # Configure OpenEXR
    #
    cd $OPENEXR_BUILD_DIR || error "Can't cd to OpenEXR build dir."

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf ${OPENEXR_BUILD_DIR}/CMakeCache.txt 

    openexr_dir="${VISITDIR}/openexr/${OPENEXR_VERSION}/${VISITARCH}"
    openexropts="-DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"

    if test "x${DO_STATIC_BUILD}" = "xyes"; then
        openexropts="${openexropts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        openexropts="${openexropts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    openexropts="${openexropts} -DBUILD_TESTING:BOOL=OFF"
    openexropts="${openexropts} -DCMAKE_INSTALL_PREFIX:PATH=${openexr_dir}"

    openexropts="${openexropts} -DOPENEXR_BUILD_EXAMPLES:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_BUILD_TOOLS:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_BUILD_TOOLS:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_INSTALL_PKG_CONFIG:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_INSTALL_TOOLS:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_OUTPUT_SUBDIR:STRING="
    openexropts="${openexropts} -DOPENEXR_TEST_LIBRARIES:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_TEST_PYTHON:BOOL=OFF"
    openexropts="${openexropts} -DOPENEXR_TEST_TOOLS:BOOL=OFF"

    openexropts="${openexropts} -DFETCHCONTENT_FULL_DISCONNECTED:BOOL=ON"
    if [[ -d ${openexr_dir}/lib64/cmake ]] ; then
        openexropts="${openexropts} -DImath_DIR:PATH=${openexr_dir}/lib64/cmake"
    else
        openexropts="${openexropts} -DImath_DIR:PATH=${openexr_dir}/lib/cmake"
    fi

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_INSTALL}/cmake\"" ${openexropts} ../${OPENEXR_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "OpenEXR configuration failed."
    
    #
    # Build OpenEXR
    #
    info "Building OpenEXR . . . (~5 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "OpenEXR build failed.  Giving up"
        return 1
    fi
    info "Installing OpenEXR . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "OpenEXR build (make install) failed.  Giving up"
        return 1
    fi

    cleanup_build_dirs $OPENEXR_BUILD_DIR $OPENEXR_SRC_DIR

    change_install_dir_perms "$VISITDIR/openexr"

    cd "$START_DIR"
    info "Done with OpenEXR"
    return 0
}

function bv_openexr_is_enabled
{
    if [[ $DO_OPENEXR == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_openexr_is_installed
{
    check_if_installed "openexr" $OPENEXR_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_openexr_build
{
    if [[ "$DO_OPENEXR" == "yes" ]] ; then
        check_if_installed "openexr" $OPENEXR_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of OpenEXR"
        else
            build_imath
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Imath for OpenEXR.  Bailing out."
            fi
            info "Done building Imath"
            build_openexr
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OpenEXR.  Bailing out."
            fi
            info "Done building OpenEXR"
        fi
    fi
}

