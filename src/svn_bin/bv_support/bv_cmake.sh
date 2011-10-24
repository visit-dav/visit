function bv_cmake_initialize
{
    export DO_CMAKE="no"
    export ON_CMAKE="off"
}

function bv_cmake_enable
{
DO_CMAKE="yes"
ON_CMAKE="on"
}

function bv_cmake_disable
{
DO_CMAKE="no"
ON_CMAKE="off"
}

function bv_cmake_depends_on
{
return ""
}

function bv_cmake_info
{
export CMAKE_FILE=${CMAKE_FILE:-"cmake-2.8.3.tar.gz"}
export CMAKE_VERSION=${CMAKE_VERSION:-"2.8.3"}
export CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:-"cmake-2.8.3"}
}

function bv_cmake_print
{
 printf "%s%s\n" "CMAKE_FILE=" "${CMAKE_FILE}"
 printf "%s%s\n" "CMAKE_VERSION=" "${CMAKE_VERSION}"
 printf "%s%s\n" "CMAKE_BUILD_DIR=" "${CMAKE_BUILD_DIR}"
}

function bv_cmake_print_usage
{
printf "%-15s %s [%s]\n" "--cmake"   "Build CMake" "built by default unless --no-thirdparty flag is used"
}

function bv_cmake_host_profile
{
#nothing to be done for cmake in cmake host profile..
echo "##" >> $HOSTCONF
}
function bv_cmake_ensure
{
    if [[ "$CMAKE_INSTALL" == "" ]]; then 
        if [[ "$DO_CMAKE" == "yes" || "$DO_VTK" == "yes" ]] ; then
            ensure_built_or_ready "cmake"  $CMAKE_VERSION  $CMAKE_BUILD_DIR  $CMAKE_FILE
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

function bv_cmake_dry_run
{
  if [[ "$DO_CMAKE" == "yes" ]] ; then
    echo "Dry run option not set for cmake."
  fi
}

# *************************************************************************** #
#                          Function 5, build_cmake                            #
# *************************************************************************** #

function apply_cmake_patch_1
{
   patch -p0 <<\EOF
diff -c a/Modules/Platform/UnixPaths.cmake cmake-2.8.3/Modules/Platform/UnixPaths.cmake
*** a/Modules/Platform/UnixPaths.cmake
--- cmake-2.8.3/Modules/Platform/UnixPaths.cmake
***************
*** 67,72 ****
--- 67,75 ----
    /usr/pkg/lib
    /opt/csw/lib /opt/lib
    /usr/openwin/lib
+
+   # Ubuntu 11.04
+   /usr/lib/x86_64-linux-gnu
    )

  LIST(APPEND CMAKE_SYSTEM_PROGRAM_PATH
***************
*** 75,80 ****
--- 78,86 ----

  LIST(APPEND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    /lib /usr/lib /usr/lib32 /usr/lib64
+
+   # Ubuntu 11.04
+   /usr/lib/x86_64-linux-gnu
    )

  LIST(APPEND CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 1 to cmake."
        return 1
   else
        return 0
   fi
}


function apply_cmake_patch
{
   info "Patching CMake . . ."

   apply_cmake_patch_1
   if [[ $? != 0 ]] ; then
      return 1
   fi

   return 0
}


function build_cmake
{
    #
    # Prepare cmake build directory
    #
    prepare_build_dir $CMAKE_BUILD_DIR $CMAKE_FILE
    untarred_cmake=$?
    if [[ $untarred_cmake == -1 ]] ; then
       warn "Unable to prepare CMake build directory. Giving Up!"
       return 1
    fi

    #
    # Patch cmake
    #
    apply_cmake_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_cmake == 1 ]] ; then
          warn "Giving up on CMake build because the patch failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script\n"\
               "tried to apply a patch to an existing directory which had\n"\
               "already been patched ... that is, that the patch is\n"\
               "failing harmlessly on a second application."
       fi
    fi

    #
    # Issue "bootstrap", which takes the place of configure for CMake.
    #
    info "Bootstrapping CMake . . ."
    cd $CMAKE_BUILD_DIR || error "Can't cd to CMake build dir."
    if [[ "$OPSYS" == "AIX" ]]; then
        env CXX=xlC CC=xlc CXXFLAGS="" CFLAGS="" ./bootstrap --prefix="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH"
    elif [[ "$OPSYS" == "Linux" && "$C_COMPILER" == "xlc" ]]; then
        env CXX=xlC CC=xlc CXXFLAGS="" CFLAGS="" ./bootstrap --prefix="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH"
    else
        env CC=${C_COMPILER} CXX=${CXX_COMPILER} CXXFLAGS="" CFLAGS="" ./bootstrap --prefix="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH"
    fi
    if [[ $? != 0 ]] ; then
       warn "Bootstrap for cmake failed, giving up."
       return 1
    fi

    #
    # Build the CMake program.
    #
    info "Building CMake . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Cannot build cmake, giving up."
       return 1
    fi

    info "Installing CMake . . ."
    $MAKE install
    info "Successfully built CMake"
    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/cmake"
       chgrp -R ${GROUP} "$VISITDIR/cmake"
    fi
    cd "$START_DIR"
    info "Done with CMake"
}

function bv_cmake_build
{
#
# Build CMake
#
cd "$START_DIR"
if [[ "$DO_CMAKE" == "yes" ]]; then
    check_if_installed "cmake" $CMAKE_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping CMake build.  CMake is already installed."
    else
        info "Building CMake (~2 minutes)"
        build_cmake
        if [[ $? != 0 ]] ; then
            error "Unable to build or install CMake.  Bailing out."
        fi
        info "Done building CMake"
    fi
fi
}

