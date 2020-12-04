function bv_clang_initialize
{
    export DO_CLANG="no"
}

function bv_clang_enable
{
    DO_CLANG="yes"
}

function bv_clang_disable
{
    DO_CLANG="no"
}

function bv_clang_depends_on
{
    depends_on="cmake llvm"
    echo ${depends_on}
}

function bv_clang_info
{
    export BV_CLANG_VERSION=${BV_CLANG_VERSION:-"6.0.1"}
    export BV_CLANG_FILE=${BV_CLANG_FILE:-"cfe-${BV_CLANG_VERSION}.src.tar.xz"}
    export BV_CLANG_URL=${BV_CLANG_URL:-"http://releases.llvm.org/${BV_CLANG_VERSION}/"}
    export BV_CLANG_BUILD_DIR=${BV_CLANG_BUILD_DIR:-"cfe-${BV_CLANG_VERSION}.src"}
    #export CLANG_MD5_CHECKSUM="5ce9c5ad55243347ea0fdb4c16754be0"
    #export CLANG_SHA256_CHECKSUM="e35dcbae6084adcf4abb32514127c5eabd7d63b733852ccdb31e06f1373136da"
}

function bv_clang_print
{
    printf "%s%s\n" "BV_CLANG_FILE=" "${BV_CLANG_FILE}"
    printf "%s%s\n" "BV_CLANG_VERSION=" "${BV_CLANG_VERSION}"
    printf "%s%s\n" "CLANG_TARGET=" "${CLANG_TARGET}"
    printf "%s%s\n" "BV_CLANG_BUILD_DIR=" "${BV_CLANG_BUILD_DIR}"
}

function bv_clang_print_usage
{
    printf "%-20s %s [%s]\n" "--clang" "Build clang" "$DO_CLANG"
}

function bv_clang_host_profile
{

    echo "nada"

    #if [[ "$DO_CLANG" == "yes" ]] ; then
    #    echo >> $HOSTCONF
    #    echo "##" >> $HOSTCONF
    #    echo "## CLANG" >> $HOSTCONF
    #    echo "##" >> $HOSTCONF
    #    echo "VISIT_OPTION_DEFAULT(VISIT_CLANG_DIR \${VISITHOME}/clang/$BV_CLANG_VERSION/\${VISITARCH})" >> $HOSTCONF
    #fi

}

function bv_clang_initialize_vars
{
    export VISIT_CLANG_DIR=${VISIT_CLANG_DIR:-"$VISITDIR/clang/${BV_CLANG_VERSION}/${VISITARCH}"}
    CLANG_INCLUDE_DIR="${VISIT_CLANG_DIR}/include"
    export CLANG_LIB_DIR="${VISIT_CLANG_DIR}/lib"
    export CLANG_INSTALL_DIR="${VISIT_CLANG_DIR}"
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        CLANG_LIB="${CLANG_LIB_DIR}/libclang.a"
    else
        CLANG_LIB="${CLANG_LIB_DIR}/libclang.${SO_EXT}"
    fi
}

function bv_clang_selected
{
    args=$@
    if [[ $args == "--clang" ]]; then
        DO_CLANG="yes"
        return 1
    fi

    return 0
}

function bv_clang_ensure
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_CLANG" == "yes" ]] ; then
            ensure_built_or_ready "clang"   $BV_CLANG_VERSION   $BV_CLANG_BUILD_DIR   $BV_CLANG_FILE $BV_CLANG_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function bv_clang_dry_run
{
    if [[ "$DO_CLANG" == "yes" ]] ; then
        echo "Dry run option not set for clang."
    fi
}

function apply_clang_patch
{
    info "Currently no patches for clang"
}

function build_clang
{
    #
    # prepare build dir
    #
    prepare_build_dir $BV_CLANG_BUILD_DIR $BV_CLANG_FILE
    untarred_clang=$?
    if [[ $untarred_clang == -1 ]] ; then
        warn "Unable to prepare clang build directory. Giving Up!"
        return 1
    fi

    #
    # Build clang
    #

    #
    # clang must be built with an out of source build.
    #
    BV_CLANG_SRC_DIR=${BV_CLANG_BUILD_DIR}
    BV_CLANG_BUILD_DIR="${BV_CLANG_SRC_DIR}-build"
    if [[ ! -d ${BV_CLANG_BUILD_DIR} ]] ; then
        info "Making build directory ${BV_CLANG_BUILD_DIR}"
        mkdir ${BV_CLANG_BUILD_DIR}
    fi

    #
    # Patch clang
    #
    
    cd "$BV_CLANG_SRC_DIR" || error "Couldn't cd to clang src dir."
    apply_clang_patch
    if [[ $? != 0 ]] ; then
	if [[ $untarred_clang == 1 ]] ; then
	    warn "Giving up on clang build because the patch failed."
	    return 1
	else
	    warn "Patch failed, but continuing.  I believe that this script\n" \
		 "tried to apply a patch to an existing directory that had\n" \
		 "already been patched ... that is, the patch is\n" \
		 "failing harmlessly on a second application."
        fi
    fi

    cd "$START_DIR"
    cd ${BV_CLANG_BUILD_DIR} || error "Couldn't cd to clang build dir."

    #
    # Remove any CMakeCache.txt files just to be safe.
    #
    rm -f CMakeCache.txt */CMakeCache.txt

    info "Configuring clang . . ."
    clang_opts=""
    if [[ $DO_PYTHON == "yes" ]] ; then
        clang_opts="${clang_opts} -DPYTHON_EXECUTABLE:FILEPATH=$PYTHON_COMMAND"
    fi

    if test "${DO_STATIC_BUILD}" = "yes" ; then
        clang_opts="${clang_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        clang_opts="${clang_opts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
 
    clang_opts="${clang_opts} -DCMAKE_INSTALL_PREFIX:PATH=${VISIT_CLANG_DIR}" 
    clang_opts="${clang_opts} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}" 
    clang_opts="${clang_opts} -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON"
    clang_opts="${clang_opts} -DCMAKE_CXX_FLAGS:STRING=\"${CXXFLAGS} ${CXX_OPT_FLAGS}\""
    clang_opts="${clang_opts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    clang_opts="${clang_opts} -DCMAKE_C_FLAGS:STRING=\"${CFLAGS} ${C_OPT_FLAGS}\""
    clang_opts="${clang_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    clang_opts="${clang_opts} -DLLVM_ENABLE_RTTI:BOOL=ON"
    clang_opts="${clang_opts} -DLLVM_CONFIG:FILEPATH=${VISIT_LLVM_DIR}/bin/llvm-config"

    # minimize the amount of clang that gets built, can reevaluate if necessary
    clang_opts="${clang_opts} -DCLANG_BUILD_TOOLS:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_ENABLE_ARCMT:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_ENABLE_STATIC_ANALYZER:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_ANALYZER_BUILD_Z3:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_INCLUDE_TESTS:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_INSTALL_SCANBUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_INSTALL_SCANVIEW:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_ARCMT_TEST_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_CHECK_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_DIFF_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_FORMAT_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_FORMAT_VS_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_FUNC_MAPPING_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_FUZZER_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_IMPORT_TEST_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_OFFLOAD_BUNDLER_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_REFACTOR_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_CLANG_RENAME_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_C_ARCMT_TEST_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_C_INDEX_TEST_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_DIAGTOOL_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_DRIVER_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_HANDLE_CXX_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_SCAN_BUILD_BUILD:BOOL=OFF"
    clang_opts="${clang_opts} -DCLANG_TOOL_SCAN_VIEW_BUILD:BOOL=OFF"


    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo ${CMAKE_COMMAND} ${clang_opts} ../${BV_CLANG_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "clang configuration failed."

    info "Building clang . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "clang build failed.  Giving up"
        return 1
    fi

    info "Installing clang . . ."
    $MAKE $MAKE_OPT_FLAGS install
    if [[ $? != 0 ]] ; then
        warn "clang install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/clang"
        chgrp -R ${GROUP} "$VISITDIR/clang"
    fi
    cd "$START_DIR"
    info "Done with clang"
    return 0
}

function bv_clang_is_enabled
{
    if [[ "$DO_CLANG" == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_clang_is_installed
{
    check_if_installed "clang" $BV_CLANG_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_clang_build
{
    #
    # Build clang
    #
    cd "$START_DIR"
    if [[ "$DO_CLANG" == "yes" ]] ; then
        check_if_installed "clang" $BV_CLANG_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping clang build.  clang is already installed."
        else
            info "Building clang (~60 minutes)"
            build_clang
            if [[ $? != 0 ]] ; then
                error "Unable to build or install clang.  Bailing out."
            fi
            info "Done building clang"
        fi
    fi
}
