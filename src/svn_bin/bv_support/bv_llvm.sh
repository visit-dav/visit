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

    echo ${depends_on}
}

function bv_llvm_info
{
    export BV_LLVM_FILE=${BV_LLVM_FILE:-"llvm-4.0.0.src.tar.gz"}
    export BV_LLVM_VERSION=${BV_LLVM_VERSION:-"4.0.0"}
    export BV_LLVM_BUILD_DIR=${BV_LLVM_BUILD_DIR:-"llvm-4.0.0.src"}
    export LLVM_MD5_CHECKSUM="7cbcd974e214d08928d53df90bf57221"
    export LLVM_SHA256_CHECKSUM=""
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
            ensure_built_or_ready "llvm"   $BV_LLVM_VERSION   $BV_LLVM_BUILD_DIR   $BV_LLVM_FILE
            if [[ $? != 0 ]] ; then
                return 1
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
    cd ${BV_LLVM_BUILD_DIR} || error "Couldn't cd to llvm build dir."

    #
    # Remove any CMakeCache.txt files just to be safe.
    #
    rm -f CMakeCache.txt */CMakeCache.txt

    info "Configuring LLVM . . ."
    ${CMAKE_COMMAND} \
        -DCMAKE_INSTALL_PREFIX:PATH="${VISITDIR}/llvm/${BV_LLVM_VERSION}/${VISITARCH}" \
        -DCMAKE_BUILD_TYPE:STRING="${VISIT_BUILD_MODE}" \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON \
        -DBUILD_SHARED_LIBS:BOOL=OFF \
        -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER} \
        -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}" \
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER} \
        -DLLVM_TARGETS_TO_BUILD=X86 \
        -DLLVM_ENABLE_RTTI:BOOL=ON \
        -DLLVM_BUILD_LLVM_DYLIB:BOOL=ON \
        ../${BV_LLVM_SRC_DIR}
    if [[ $? != 0 ]] ; then
        warn "LLVM cmake failed.  Giving up"
        return 1
    fi

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
