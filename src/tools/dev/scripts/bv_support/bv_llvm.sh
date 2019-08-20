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
    export BV_LLVM_VERSION=${BV_LLVM_VERSION:-"5.0.0"}
    export BV_LLVM_FILE=${BV_LLVM_FILE:-"llvm-${BV_LLVM_VERSION}.src.tar.xz"}
    export BV_LLVM_URL=${BV_LLVM_URL:-"http://releases.llvm.org/${BV_LLVM_VERSION}/"}
    export BV_LLVM_BUILD_DIR=${BV_LLVM_BUILD_DIR:-"llvm-${BV_LLVM_VERSION}.src"}
    export LLVM_MD5_CHECKSUM="5ce9c5ad55243347ea0fdb4c16754be0"
    export LLVM_SHA256_CHECKSUM="e35dcbae6084adcf4abb32514127c5eabd7d63b733852ccdb31e06f1373136da"
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
            ensure_built_or_ready "llvm"   $BV_LLVM_VERSION   $BV_LLVM_BUILD_DIR   $BV_LLVM_FILE $BV_LLVM_URL
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

function apply_llvm_patch
{
    # fixes a bug in LLVM 5.0.0
    # where if the LLVM_BUILD_LLVM_DYLIB CMake var is set to ON,
    # CMake will fail when checking an internal variable that is empty
    # patch based on https://reviews.llvm.org/D31445

    patch -p0 << \EOF
*** tools/llvm-shlib/CMakeLists.txt.original     2018-06-14 16:16:13.185286160 -0500
--- tools/llvm-shlib/CMakeLists.txt      2018-06-14 16:16:59.773283611 -0500
***************
*** 36,42 ****

  add_llvm_library(LLVM SHARED DISABLE_LLVM_LINK_LLVM_DYLIB SONAME ${SOURCES})

! list(REMOVE_DUPLICATES LIB_NAMES)
  if(("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux") OR (MINGW) OR (HAIKU) OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD") OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "DragonFly")) # FIXME: It should be "GNU ld for elf"
    configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_version_script.map.in
--- 36,44 ----

  add_llvm_library(LLVM SHARED DISABLE_LLVM_LINK_LLVM_DYLIB SONAME ${SOURCES})

! if(LIB_NAMES)
!     list(REMOVE_DUPLICATES LIB_NAMES)
! endif()
  if(("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux") OR (MINGW) OR (HAIKU) OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD") OR ("${CMAKE_SYSTEM_NAME}" STREQUAL "DragonFly")) # FIXME: It should be "GNU ld for elf"
    configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_version_script.map.in
EOF
    if [[ $? != 0 ]] ; then
        warn "llvm patch for tools/llvm-shlib/CMakeLists.txt failed"
        return 1
    fi

    # fixes a bug in LLVM 5.0.0
    # a vector<char> is cast to a vector<unsigned char>. This patch comes
    # from http://lists.busybox.net/pipermail/buildroot/2018-May/221648.html
    # This is presumably fixed in LLVM 6.0.0.

    patch -p0 << \EOF
--- include/llvm/ExecutionEngine/Orc/OrcRemoteTargetClient.h.orig	2019-07-26 13:23:06.588925000 -0700
+++ include/llvm/ExecutionEngine/Orc/OrcRemoteTargetClient.h	2019-07-26 13:23:53.990216000 -0700
@@ -713,8 +713,8 @@
 
   uint32_t getTrampolineSize() const { return RemoteTrampolineSize; }
 
-  Expected<std::vector<char>> readMem(char *Dst, JITTargetAddress Src,
-                                      uint64_t Size) {
+  Expected<std::vector<uint8_t>> readMem(char *Dst, JITTargetAddress Src,
+                                         uint64_t Size) {
     // Check for an 'out-of-band' error, e.g. from an MM destructor.
     if (ExistingError)
       return std::move(ExistingError);
EOF
    if [[ $? != 0 ]] ; then
        warn "llvm patch for include/llvm/ExecutionEngine/Orc/OrcRemoteTargetClient.h failed"
        return 1
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
    if [[ $DO_PYTHON == "yes" ]] ; then
        LLVM_CMAKE_PYTHON="-DPYTHON_EXECUTABLE:FILEPATH=$PYTHON_COMMAND"
    fi

    #
    # Determine the LLVM_TARGET_TO_BUILD.
    #
    if [[ "$(uname -m)" == "ppc64" || "$(uname -m)" == "ppc64le" ]]; then
        LLVM_TARGET="PowerPC"
    else
        LLVM_TARGET="X86"
    fi

    # LLVM documentation states thet BUILD_SHARED_LIBS is not to be used
    # in conjuction with LLVM_BUILD_LLVM_DYLIB, and should only be used
    # by LLVM developers.
    ${CMAKE_COMMAND} \
        -DCMAKE_INSTALL_PREFIX:PATH="${VISIT_LLVM_DIR}" \
        -DCMAKE_BUILD_TYPE:STRING="${VISIT_BUILD_MODE}" \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON \
        -DBUILD_SHARED_LIBS:BOOL=OFF \
        -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER} \
        -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}" \
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER} \
        -DLLVM_TARGETS_TO_BUILD=${LLVM_TARGET} \
        -DLLVM_ENABLE_RTTI:BOOL=ON \
        -DLLVM_BUILD_LLVM_DYLIB:BOOL=ON \
        $LLVM_CMAKE_PYTHON \
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
