function bv_llvm_initialize
{
    export DO_LLVM="no"
}

function bv_llvm_enable
{
    DO_LLVM="yes"
}

function bv_llvm_disable
{
    DO_LLVM="no"
}

function bv_llvm_depends_on
{
    depends_on="cmake"
    if [[ $DO_PYTHON == "yes" ]] ; then
        depends_on="$depends_on python"
    fi

    echo ${depends_on}
}

function bv_llvm_info
{
    export BV_LLVM_VERSION=${BV_LLVM_VERSION:-"6.0.1"}
    export BV_LLVM_FILE=${BV_LLVM_FILE:-"llvm-${BV_LLVM_VERSION}.src.tar.xz"}
    export BV_LLVM_URL=${BV_LLVM_URL:-"http://releases.llvm.org/${BV_LLVM_VERSION}/"}
    export BV_LLVM_BUILD_DIR=${BV_LLVM_BUILD_DIR:-"llvm-${BV_LLVM_VERSION}.src"}
    export BV_LLVM_MD5_CHECKSUM="c88c98709300ce2c285391f387fecce0"
    export BV_LLVM_SHA256_CHECKSUM="b6d6c324f9c71494c0ccaf3dac1f16236d970002b42bb24a6c9e1634f7d0f4e2"

    export BV_CLANG_URL=${BV_LLVM_URL}
    export BV_CLANG_FILE="cfe-${BV_LLVM_VERSION}.src.tar.xz"
    export BV_CLANG_BUILD_DIR="cfe-${BV_LLVM_VERSION}.src"
    export BV_CLANG_MD5_CHECKSUM="4e419bd4e3b55aa06d872320f754bd85"
    export BV_CLANG_SHA256_CHECKSUM="7c243f1485bddfdfedada3cd402ff4792ea82362ff91fbdac2dae67c6026b667"
}

function bv_llvm_print
{
    printf "%s%s\n" "BV_LLVM_FILE=" "${BV_LLVM_FILE}"
    printf "%s%s\n" "BV_LLVM_VERSION=" "${BV_LLVM_VERSION}"
    printf "%s%s\n" "LLVM_TARGET=" "${LLVM_TARGET}"
    printf "%s%s\n" "BV_LLVM_BUILD_DIR=" "${BV_LLVM_BUILD_DIR}"
}

function bv_llvm_print_usage
{
    printf "%-20s %s [%s]\n" "--llvm" "Build LLVM" "$DO_LLVM"
}

function bv_llvm_host_profile
{
    if [[ "$DO_LLVM" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## LLVM" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_LLVM_DIR \${VISITHOME}/llvm/$BV_LLVM_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_llvm_initialize_vars
{
    export VISIT_LLVM_DIR=${VISIT_LLVM_DIR:-"$VISITDIR/llvm/${BV_LLVM_VERSION}/${VISITARCH}"}
    LLVM_INCLUDE_DIR="${VISIT_LLVM_DIR}/include"
    LLVM_LIB_DIR="${VISIT_LLVM_DIR}/lib"
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        LLVM_LIB="${LLVM_LIB_DIR}/libLLVM.a"
    else
        LLVM_LIB="${LLVM_LIB_DIR}/libLLVM.${SO_EXT}"
    fi
    # needed for clang and pyside
    export LLVM_INSTALL_DIR="${VISIT_LLVM_DIR}"
}

function bv_llvm_selected
{
    args=$@
    if [[ $args == "--llvm" ]]; then
        DO_LLVM="yes"
        return 1
    fi

    return 0
}

function bv_llvm_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_LLVM" == "yes" ]] ; then
            ensure_built_or_ready "llvm"   $BV_LLVM_VERSION   $BV_LLVM_BUILD_DIR   $BV_LLVM_FILE $BV_LLVM_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function apply_llvm_patch
{
    info "Currently no patches for llvm"
}

function build_llvm
{
    #
    # prepare build dir
    #
    prepare_build_dir $BV_LLVM_BUILD_DIR $BV_LLVM_FILE
    untarred_llvm=$?
    if [[ $untarred_llvm == -1 ]] ; then
        warn "Unable to prepare LLVM build directory. Giving Up!"
        return 1
    fi

    # download clang
    if ! test -f ${BV_CLANG_FILE} ; then
        download_file ${BV_CLANG_FILE} ${BV_CLANG_URL}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${BV_CLANG_FILE}"
            return 1
        fi
    fi

    # extract clang
    if ! test -d clang ; then
        info "Extracting clang ..."
        uncompress_untar ${BV_CLANG_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${BV_CLANG_FILE}"
            return 1
        fi
        # llvm build system expects the directory to be named clang
        mv ${BV_CLANG_BUILD_DIR} clang
    fi

    #
    # Build LLVM.
    #

    #
    # LLVM must be built with an out of source build.
    #
    BV_LLVM_SRC_DIR=${BV_LLVM_BUILD_DIR}
    BV_LLVM_BUILD_DIR="${BV_LLVM_SRC_DIR}-build"
    if [[ ! -d ${BV_LLVM_BUILD_DIR} ]] ; then
        info "Making build directory ${BV_LLVM_BUILD_DIR}"
        mkdir ${BV_LLVM_BUILD_DIR}
    fi

    #
    # Patch LLVM
    #
    
    cd "$BV_LLVM_SRC_DIR" || error "Couldn't cd to llvm src dir."
    apply_llvm_patch
    if [[ $? != 0 ]] ; then
	if [[ $untarred_llvm == 1 ]] ; then
	    warn "Giving up on LLVM build because the patch failed."
	    return 1
	else
	    warn "Patch failed, but continuing.  I believe that this script\n" \
		 "tried to apply a patch to an existing directory that had\n" \
		 "already been patched ... that is, the patch is\n" \
		 "failing harmlessly on a second application."
        fi
    fi

    cd "$START_DIR"
    cd ${BV_LLVM_BUILD_DIR} || error "Couldn't cd to llvm build dir."

    #
    # Remove any CMakeCache.txt files just to be safe.
    #
    rm -f CMakeCache.txt */CMakeCache.txt

    info "Configuring LLVM . . ."

    llvm_opts=""
    # standard cmake options
    llvm_opts="${llvm_opts} -DCMAKE_INSTALL_PREFIX:PATH=${VISIT_LLVM_DIR}"
    llvm_opts="${llvm_opts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    llvm_opts="${llvm_opts} -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON"
    llvm_opts="${llvm_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    llvm_opts="${llvm_opts} -DCMAKE_CXX_FLAGS:STRING=\"${CXXFLAGS} ${CXX_OPT_FLAGS}\""
    llvm_opts="${llvm_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    llvm_opts="${llvm_opts} -DCMAKE_C_FLAGS:STRING=\"${CFLAGS} ${C_OPT_FLAGS}\""

    # python?
    if [[ $DO_PYTHON == "yes" ]] ; then
        llvm_opts="${llvm_opts} -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON_COMMAND"
    fi

    #
    # Determine the LLVM_TARGET_TO_BUILD.
    #
    if [[ "$(uname -m)" == "ppc64" || "$(uname -m)" == "ppc64le" ]]; then
        llvm_opts="${llvm_opts} -DLLVM_TARGETS_TO_BUILD:STRING=PowerPC"
    else
        llvm_opts="${llvm_opts} -DLLVM_TARGETS_TO_BUILD:STRING=X86"
    fi

    llvm_opts="${llvm_opts} -DLLVM_ENABLE_RTTI:BOOL=ON"
    llvm_opts="${llvm_opts} -DLLVM_BUILD_LLVM_DYLIB:BOOL=ON"

    # turn off things we don't need?
    llvm_opts="${llvm_opts} -DLLVM_INCLUDE_DOCS:BOOL=OFF"
    llvm_opts="${llvm_opts} -DLLVM_INCLUDE_EXAMPLES:BOOL=OFF"
    llvm_opts="${llvm_opts} -DLLVM_INCLUDE_GO_TESTS:BOOL=OFF"
    llvm_opts="${llvm_opts} -DLLVM_INCLUDE_TESTS:BOOL=OFF"
    llvm_opts="${llvm_opts} -DLLVM_INCLUDE_UTILS:BOOL=OFF"


    # options for building libclang
    llvm_opts="${llvm_opts} -DLLVM_ENABLE_PROJECTS:STRING=clang"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_LIBCLANG_BUILD:BOOL=ON"
    # turning off unnecessary tools
    llvm_opts="${llvm_opts} -DCLANG_BUILD_TOOLS:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_ENABLE_ARCMT:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_ENABLE_STATIC_ANALYZER:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_INSTALL_SCANBUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_INSTALL_SCANVIEW:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_PLUGIN_SUPPORT:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_ARCMT_TEST_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_CHECK_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_DIFF_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_FORMAT_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_FORMAT_VS_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_FUNC_MAPPING_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_FUZZER_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_IMPORT_TEST_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_OFFLOAD_BUNDLER_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_REFACTOR_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_CLANG_RENAME_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_C_ARCMT_TEST_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_C_INDEX_TEST_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_DIAGTOOL_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_DRIVER_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_HANDLE_CXX_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_SCAN_BUILD_BUILD:BOOL=OFF"
    llvm_opts="${llvm_opts} -DCLANG_TOOL_SCAN_VIEW_BUILD:BOOL=OFF"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_COMMAND}\"" ${llvm_opts} ../${BV_LLVM_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "LLVM configuration failed"

    info "Building LLVM . . ."
    ${MAKE} ${MAKE_OPT_FLAGS}
    if [[ $? != 0 ]] ; then
        warn "LLVM build failed.  Giving up"
        return 1
    fi

    info "Installing LLVM . . ."
    ${MAKE} ${MAKE_OPT_FLAGS} install
    if [[ $? != 0 ]] ; then
        warn "LLVM install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/llvm"
        chgrp -R ${GROUP} "$VISITDIR/llvm"
    fi
    cd "$START_DIR"
    info "Done with LLVM"
    return 0
}

function bv_llvm_is_enabled
{
    if [[ $DO_LLVM == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_llvm_is_installed
{
    check_if_installed "llvm" $BV_LLVM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_llvm_build
{
    #
    # Build LLVM
    #
    cd "$START_DIR"
    if [[ "$DO_LLVM" == "yes" ]] ; then
        check_if_installed "llvm" $BV_LLVM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping LLVM build.  LLVM is already installed."
        else
            info "Building LLVM (~60 minutes)"
            build_llvm
            if [[ $? != 0 ]] ; then
                error "Unable to build or install LLVM.  Bailing out."
            fi
            info "Done building LLVM"
        fi
    fi
}
