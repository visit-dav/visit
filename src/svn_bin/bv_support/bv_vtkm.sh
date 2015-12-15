
function bv_vtkm_initialize
{
    export DO_VTKM="no"
    export ON_VTKM="off"
    export USE_SYSTEM_VTKM="no"
    add_extra_commandline_args "vtkm" "alt-vtkm-dir" 1 "Use alternative directory for vtkm"
}

function bv_vtkm_enable
{
    DO_VTKM="yes"
    ON_VTKM="on"
}

function bv_vtkm_disable
{
    DO_VTKM="no"
    ON_VTKM="off"
}

function bv_vtkm_alt_vtkm_dir
{
    bv_vtkm_enable
    USE_SYSTEM_VTKM="yes"
    VTKM_INSTALL_DIR="$1"
}

function bv_vtkm_depends_on
{
    if [[ "$USE_SYSTEM_VTKM" == "yes" ]]; then
        echo ""
    else
        local depends_on=""

        echo $depends_on
    fi
}

function bv_vtkm_initialize_vars
{
    if [[ "$USE_SYSTEM_VTKM" == "no" ]]; then
        VTKM_INSTALL_DIR="\${VISITHOME}/vtkm/$VTKM_VERSION/\${VISITARCH}"
    fi
}

function bv_vtkm_info
{
    export VTKM_VERSION=${VTKM_VERSION:-"763de94"}
    export VTKM_FILE=${VTKM_FILE:-"vtkm-${VTKM_VERSION}.tar.gz"}
    export VTKM_BUILD_DIR=${VTKM_BUILD_DIR:-"vtkm-${VTKM_VERSION}"}
    export VTKM_MD5_CHECKSUM="c3ed3228bf6382b4cd88de335d63ca4a"
    export VTKM_SHA256_CHECKSUM="05c4dd03ebaa2f5be38b2e2c1d9415740b93b385fee46ca6e5df42c7fd419c25"
}

function bv_vtkm_print
{
    printf "%s%s\n" "VTKM_FILE=" "${VTKM_FILE}"
    printf "%s%s\n" "VTKM_VERSION=" "${VTKM_VERSION}"
    printf "%s%s\n" "VTKM_BUILD_DIR=" "${VTKM_BUILD_DIR}"
}

function bv_vtkm_print_usage
{
    printf "%-15s %s [%s]\n" "--vtkm" "Build vtkm support" "$DO_VTKM"
}

function bv_vtkm_graphical
{
    local graphical_out="vtkm     $VTKM_VERSION($VTKM_FILE)      $ON_VTKM"
    echo "$graphical_out"
}

function bv_vtkm_host_profile
{
    if [[ "$DO_VTKM" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VTKM" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_VTKM_DIR ${VTKM_INSTALL_DIR})" \
        >> $HOSTCONF
    fi
}

function bv_vtkm_ensure
{
    if [[ "$DO_VTKM" == "yes" && "$USE_SYSTEM_VKTM" == "no" ]] ; then
        ensure_built_or_ready "vtkm" $VTKM_VERSION $VTKM_BUILD_DIR $VTKM_FILE $VTKM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_VTKM="no"
            error "Unable to build vtkm${VTKM_FILE} not found."
        fi
    fi
}

function bv_vtkm_dry_run
{
    if [[ "$DO_VTKM" == "yes" ]] ; then
        echo "Dry run option not set for vtkm"
    fi
}

# *************************************************************************** #
#                            Function 8, build_vtkm
#
#
# *************************************************************************** #

function apply_vtkm_763de94_patch_1
{
    patch -p0 << \EOF
diff -c vtkm/exec/cuda/internal/CMakeLists.txt.orig vtkm/exec/cuda/internal/CMakeLists.txt
*** vtkm/exec/cuda/internal/CMakeLists.txt.orig
--- vtkm/exec/cuda/internal/CMakeLists.txt
***************
*** 23,28 ****
--- 23,29 ----
    ExecutionPolicy.h
    IteratorFromArrayPortal.h
    WrappedOperators.h
+   ThrustPatches.h
    )
  
  #-----------------------------------------------------------------------------
EOF
    if [[ $? != 0 ]] ; then
      warn "vtkm patch 1 failed."
      return 1
    fi

    return 0;
}

function apply_vtkm_763de94_patch_2
{
    patch -p0 << \EOF
diff -c vtkm/internal/Configure.h.in.orig vtkm/internal/Configure.h.in
--- vtkm/internal/Configure.h.in.orig
+++ vtkm/internal/Configure.h.in
@@ -137,12 +137,22 @@
 #define VTK_M_THIRDPARTY_CLANG_WARNING_PRAGMAS
 #endif

+// Older versions of GCC don't support the push/pop pragmas. Right now we are
+// not checking for GCC 3 or earlier. I'm not sure we have a use case for that.
+#if defined(VTKM_GCC) && (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
+#define VTK_M_THIRDPARTY_WARNINGS_PUSH
+#define VTK_M_THRIDPARTY_WARNINGS_POP
+#else
+#define VTK_M_THIRDPARTY_WARNINGS_PUSH _Pragma("GCC diagnostic push")
+#define VTK_M_THRIDPARTY_WARNINGS_POP  _Pragma("GCC diagnostic pop")
+#endif
+
 #define VTKM_THIRDPARTY_PRE_INCLUDE \
-  _Pragma("GCC diagnostic push") \
+  VTK_M_THIRDPARTY_WARNINGS_PUSH \
   VTK_M_THIRDPARTY_GCC_WARNING_PRAGMAS \
   VTK_M_THIRDPARTY_CLANG_WARNING_PRAGMAS
 #define VTKM_THIRDPARTY_POST_INCLUDE \
-  _Pragma("GCC diagnostic pop")
+  VTK_M_THRIDPARTY_WARNINGS_POP

 #else
 #define VTKM_THIRDPARTY_PRE_INCLUDE
EOF
    if [[ $? != 0 ]] ; then
      warn "vtkm patch 2 failed."
      return 1
    fi

    return 0;
}

function apply_vtkm_763de94_patch_3
{
    patch -p0 << \EOF
diff -c vtkm/ListTag.h.orig vtkm/ListTag.h
*** vtkm/ListTag.h.orig
--- vtkm/ListTag.h
***************
*** 49,55 ****
  ///
  #define VTKM_IS_LIST_TAG(tag) \
    VTKM_STATIC_ASSERT_MSG( \
!     ::vtkm::internal::ListTagCheck<tag>::Valid, \
      "Provided type is not a valid VTK-m list tag.")
  
  namespace detail {
--- 49,55 ----
  ///
  #define VTKM_IS_LIST_TAG(tag) \
    VTKM_STATIC_ASSERT_MSG( \
!     (::vtkm::internal::ListTagCheck<tag>::Valid), \
      "Provided type is not a valid VTK-m list tag.")
  
  namespace detail {
EOF
    if [[ $? != 0 ]] ; then
      warn "vtkm patch 3 failed."
      return 1
    fi

    return 0;
}

function apply_vtkm_patch
{
    info "Patching vtkm . . ."

    if [[ "${VTKM_VERSION}" == "763de94" ]] ; then
        apply_vtkm_763de94_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtkm_763de94_patch_2
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtkm_763de94_patch_3
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_vtkm
{
    #
    # CMake is the build system for VTK.  Call another script that will build
    # that program.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "vtkm: cmake found"
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
    prepare_build_dir $VTKM_BUILD_DIR $VTKM_FILE
    untarred_vtkm=$?
    if [[ $untarred_vtkm == -1 ]] ; then
       warn "Unable to prepare vtkm build directory. Giving Up!"
       return 1
    fi
    
    #
    # Apply patches
    #
    cd $VTKM_BUILD_DIR || error "Can't cd to vtkm build dir."
    apply_vtkm_patch
    if [[ $? != 0 ]] ; then
        warn "Patch failed, but continuing."
    fi
    # move back up to the start dir
    cd "$START_DIR"

    #
    # Call cmake
    #
    info "Configuring vtkm . . ."
    
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    # Make a build directory for an out-of-source build.. Change the
    # VTKM_BUILD_DIR variable to represent the out-of-source build directory.
    VTKM_SRC_DIR=$VTKM_BUILD_DIR
    VTKM_BUILD_DIR="${VTKM_SRC_DIR}-build"
    if [[ ! -d $VTKM_BUILD_DIR ]] ; then
        echo "Making build directory $VTKM_BUILD_DIR"
        mkdir $VTKM_BUILD_DIR
    fi

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf ${VTKM_BUILD_DIR}/CMakeCache.txt ${VTKM_BUILD_DIR}/*/CMakeCache.txt

    cd $VTKM_BUILD_DIR || error "Can't cd to vtkm build dir."

    vopts=""
    vopts="${vopts} -DBOOST_INCLUDEDIR=${VISITDIR}/boost/${BOOST_VERSION}/${VISITARCH}/include"
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/vtkm/${VTKM_VERSION}/${VISITARCH}"
    vopts="${vopts} -DVTKm_ENABLE_TESTING=OFF"
    vopts="${vopts} -DVTKm_ENABLE_CUDA=ON"
    #
    # Several platforms have had problems with the VTK cmake configure
    # command issued simply via "issue_command".  This was first discovered
    # on BGQ and then showed up in random cases for both OSX and Linux
    # machines. Brad resolved this on BGQ  with a simple work around - we
    # write a simple script that we invoke with bash which calls cmake with
    # all of the properly arguments. We are now using this strategy for all
    # platforms.
    #
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${vopts} ../${VTKM_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "VTKM configuration failed."

    #
    # Build vtkm
    #
    info "Building vtkm . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS || error "vtkm did not build correctly. Giving up."

    info "Installing vtkm . . . (~2 minutes)"
    $MAKE install || error "vtkm did not install correctly."

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/vtkm"
       chgrp -R ${GROUP} "$VISITDIR/vtkm"
    fi
    cd "$START_DIR"
    info "Done with vtkm"
    return 0
}

function bv_vtkm_is_enabled
{
    if [[ $DO_VTKM == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_vtkm_is_installed
{
    if [[ "$USE_SYSTEM_VTKM" == "yes" ]]; then
        return 1
    fi

    check_if_installed "vtkm" $VTKM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_vtkm_build
{
    cd "$START_DIR"
    if [[ "$DO_VTKM" == "yes" && "$USE_SYSTEM_VTKM" == "no" ]] ; then
        check_if_installed "vtkm" $VTKM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping vtkm build.  vtkm is already installed."
        else
            info "Building vtkm (~2 minutes)"
            build_vtkm
            if [[ $? != 0 ]] ; then
                error "Unable to build or install vtkm.  Bailing out."
            fi
            info "Done building vtkm"
        fi
    fi
}

