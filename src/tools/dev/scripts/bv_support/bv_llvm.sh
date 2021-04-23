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
    export LLVM_VERSION=${LLVM_VERSION:-"6.0.1"}
    export LLVM_FILE=${LLVM_FILE:-"llvm-${LLVM_VERSION}.src.tar.xz"}
    export LLVM_URL=${LLVM_URL:-"http://releases.llvm.org/${LLVM_VERSION}/"}
    export LLVM_SRC_DIR=${LLVM_SRC_DIR:-"${LLVM_FILE%.tar*}"}
    export LLVM_BUILD_DIR=${LLVM_BUILD_DIR:-"${LLVM_SRC_DIR}-build"}
    export LLVM_MD5_CHECKSUM="c88c98709300ce2c285391f387fecce0"
    export LLVM_SHA256_CHECKSUM="b6d6c324f9c71494c0ccaf3dac1f16236d970002b42bb24a6c9e1634f7d0f4e2"

    export CLANG_URL=${LLVM_URL}
    export CLANG_FILE="cfe-${LLVM_VERSION}.src.tar.xz"
    export CLANG_SRC_DIR="cfe-${LLVM_VERSION}.src"
    export CLANG_BUILD_DIR=${CLANG_BUILD_DIR:-"${CLANG_SRC_DIR}-build"}
    export CLANG_MD5_CHECKSUM="4e419bd4e3b55aa06d872320f754bd85"
    export CLANG_SHA256_CHECKSUM="7c243f1485bddfdfedada3cd402ff4792ea82362ff91fbdac2dae67c6026b667"
}

function bv_llvm_print
{
    printf "%s%s\n" "LLVM_FILE=" "${LLVM_FILE}"
    printf "%s%s\n" "LLVM_VERSION=" "${LLVM_VERSION}"
    printf "%s%s\n" "LLVM_TARGET=" "${LLVM_TARGET}"
    printf "%s%s\n" "LLVM_SRC_DIR=" "${LLVM_SRC_DIR}"
    printf "%s%s\n" "LLVM_BUILD_DIR=" "${LLVM_BUILD_DIR}"
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
        echo "VISIT_OPTION_DEFAULT(VISIT_LLVM_DIR \${VISITHOME}/llvm/$LLVM_VERSION/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_llvm_initialize_vars
{
    export VISIT_LLVM_DIR=${VISIT_LLVM_DIR:-"$VISITDIR/llvm/${LLVM_VERSION}/${VISITARCH}"}
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
            check_installed_or_have_src "llvm" $LLVM_VERSION $LLVM_BUILD_DIR $LLVM_FILE $LLVM_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi

            # Download clang
            ALREADY_INSTALLED="NO"
            HAVE_TARBALL="NO"
            if [[ -e ${CLANG_FILE} ]] ; then
                HAVE_TARBALL="YES"
            fi

            if [[ "$ALREADY_INSTALLED" == "NO" && "$HAVE_TARBALL" == "NO" ]] ; then
                download_file ${CLANG_FILE} ${CLANG_URL}
                if [[ $? != 0 ]] ; then
                    warn "Error: Cannot obtain source for ${CLANG_FILE}."
                    return 1
                fi
            fi
        fi
    fi
}

function bv_llvm_dry_run
{
    if [[ "$DO_LLVM" == "yes" ]] ; then
        echo "Dry run option not set for llvm."
    fi
}

function apply_llvm_patch
{
    cd ${LLVM_SRC_DIR} || error "Can't cd to LLVM source dir."

#    info "Currently no patches for llvm"

    cd "$START_DIR"

    return 0
}

function build_llvm
{
    #
    # Uncompress the Clang source dir
    #
    uncompress_src_file $CLANG_SRC_DIR $CLANG_FILE
    untarred_clang=$?
    if [[ $untarred_clang == -1 ]] ; then
        warn "Unable to uncompress Clang source file. Giving Up!"
        return 1
    else
	# The LLVM build system expects the directory to be named
	# clang. Make a soft link so the source version is known.
	if [[ -e clang ]] ; then
	    rm -rf clang
	fi
	ln -s ${CLANG_SRC_DIR} clang
    fi

    #
    # Uncompress the LLVM source dir
    #
    uncompress_src_file $LLVM_SRC_DIR $LLVM_FILE
    untarred_llvm=$?
    if [[ $untarred_llvm == -1 ]] ; then
        warn "Unable to uncompress LLVM source file. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching LLVM . . ."
    apply_llvm_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_llvm == 1 ]] ; then
            warn "Giving up on LLVM build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing. I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Make a build directory for an out-of-source build.
    #
    cd "$START_DIR"
    if [[ ! -d $LLVM_BUILD_DIR ]] ; then
        echo "Making build directory $LLVM_BUILD_DIR"
        mkdir $LLVM_BUILD_DIR
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf ${LLVM_BUILD_DIR}/CMakeCache.txt ${LLVM_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd ${LLVM_BUILD_DIR}

    #
    # Call configure
    #
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

    #
    # Several platforms have had problems with the LLVM cmake configure command
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

    echo "\"${CMAKE_BIN}\"" ${llvm_opts} ../${LLVM_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "Llvm configure failed. Giving up"
        return 1
    fi

    #
    # Build Llvm
    #
    info "Building LLVM . . . (~60 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "LLVM build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing LLVM"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "LLVM install failed. Giving up"
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
    check_if_installed "llvm" $LLVM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_llvm_build
{
    cd "$START_DIR"

    if [[ "$DO_LLVM" == "yes" ]] ; then
        check_if_installed "LLVM" $LLVM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping LLVM build. LLVM is already installed."
        else
            info "Building Llvm (~60 minutes)"
            build_llvm
            if [[ $? != 0 ]] ; then
                error "Unable to build or install LLVM. Bailing out."
            fi
            info "Done building LLVM"
        fi
    fi
}
