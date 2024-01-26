function bv_cmake_initialize
{
    export DO_CMAKE="yes"
    export FORCE_CMAKE="no"
    export USE_SYSTEM_CMAKE="no"
    add_extra_commandline_args "cmake" "system-cmake" 0 "Use cmake found on system"
    add_extra_commandline_args "cmake" "alt-cmake-dir" 1 "Use cmake found in alternative directory"
    add_extra_commandline_args "cmake" "bin-cmake-dir" 1 "Use cmake found in alternative binary directory"
}

function bv_cmake_enable
{
    DO_CMAKE="yes"
    FORCE_CMAKE="yes"
}

function bv_cmake_disable
{
    DO_CMAKE="no"
    FORCE_CMAKE="no"
}

function bv_cmake_depends_on
{
    echo ""
}

function bv_cmake_force
{
    if [[ "$FORCE_CMAKE" == "yes" ]]; then
        return 0;
    fi
    return 1;
}

function cmake_set_vars_helper
{
    CMAKE_VERSION=`"${CMAKE_COMMAND}" --version`
    CMAKE_VERSION=${CMAKE_VERSION/cmake version }
    CMAKE_BUILD_DIR=`"${CMAKE_COMMAND}" --system-information 2>& 1 | grep _CMAKE_INSTALL_DIR | grep -v _CMAKE_INSTALL_DIR:INTERNAL | sed -e s/\"//g -e s/_CMAKE_INSTALL_DIR//g`
    CMAKE_BUILD_DIR=`echo $CMAKE_BUILD_DIR`
    CMAKE_INSTALL="$CMAKE_BUILD_DIR/bin"
    CMAKE_ROOT=`"$CMAKE_COMMAND" --system-information 2>&1 | grep CMAKE_ROOT | grep -v CMAKE_ROOT:INTERNAL | sed -e s/\"//g -e s/CMAKE_ROOT//g` 
    CMAKE_ROOT=`echo "$CMAKE_ROOT"`
    CMAKE_ROOT=`echo $CMAKE_ROOT`

    echo "version: $CMAKE_VERSION build: $CMAKE_BUILD_DIR bin: $CMAKE_INSTALL root: $CMAKE_ROOT"
}

function bv_cmake_system_cmake
{
    echo "using system cmake"

    TEST=`which cmake`
    [ $? != 0 ] && error "System CMake not found"
    
    bv_cmake_enable
    
    USE_SYSTEM_CMAKE="yes"
    
    CMAKE_COMMAND="cmake"
    CMAKE_FILE=""
    cmake_set_vars_helper #set vars..
}

function bv_cmake_alt_cmake_dir
{
    CMAKE_ALT_DIR="$1"
    echo "Using cmake from alternative directory $1"

    [ ! -e "$CMAKE_ALT_DIR/bin/cmake" ] && error "cmake was not found in directory: $1/bin"

    bv_cmake_enable
    USE_SYSTEM_CMAKE="yes"

    CMAKE_COMMAND="$CMAKE_ALT_DIR/bin/cmake"
    CMAKE_FILE=""
    cmake_set_vars_helper #set vars..
}

function bv_cmake_bin_cmake_dir
{
    CMAKE_BIN_DIR="$1"
    echo "Using cmake from bin directory $1"

    [ ! -e "$CMAKE_BIN_DIR/cmake" ] && error "cmake was not found in directory: $1/"

    bv_cmake_enable
    USE_SYSTEM_CMAKE="yes"

    CMAKE_COMMAND="$CMAKE_BIN_DIR/cmake"
    CMAKE_FILE=""
    cmake_set_vars_helper #set vars..
}


function bv_cmake_info
{
    export CMAKE_URL=${CMAKE_URL:-"https://cmake.org/files/v3.24/"}
    export CMAKE_VERSION=${CMAKE_VERSION:-"3.24.3"}
    export CMAKE_FILE=${CMAKE_FILE:-"cmake-${CMAKE_VERSION}.tar.gz"}
    export CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:-"cmake-${CMAKE_VERSION}"}
    export CMAKE_SHA256_CHECKSUM="b53aa10fa82bff84ccdb59065927b72d3bee49f4d86261249fc0984b3b367291"
}

function bv_cmake_print
{
    printf "%s%s\n" "CMAKE_FILE=" "${CMAKE_FILE}"
    printf "%s%s\n" "CMAKE_VERSION=" "${CMAKE_VERSION}"
    printf "%s%s\n" "CMAKE_BUILD_DIR=" "${CMAKE_BUILD_DIR}"
}

function bv_cmake_print_usage
{
    printf "%-20s %s\n" "--cmake" "Build CMake"
    printf "%-20s %s [%s]\n" "--system-cmake"  "Use the system installed CMake"
    printf "%-20s %s [%s]\n" "--alt-cmake-dir" "Use CMake from an alternative directory"
    printf "%-20s %s [%s]\n" "--bin-cmake-dir" "Use CMake from an alternative binary directory"
}

function bv_cmake_host_profile
{
    #nothing to be done for cmake in cmake host profile..
    echo "##" >> $HOSTCONF
}

function bv_cmake_initialize_vars
{
    if [[ "$USE_SYSTEM_CMAKE" != "yes" ]]; then 
        if [[ "$DO_CMAKE" == "yes" || "$DO_VTK" == "yes" ]] ; then
            #initialize variables where cmake should exist..
            CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/${VISITARCH}/bin"}
            CMAKE_ROOT=${CMAKE_ROOT:-"$VISITDIR/cmake/${CMAKE_VERSION}/${VISITARCH}/share/cmake-${CMAKE_VERSION%.*}"}
            CMAKE_COMMAND="${CMAKE_INSTALL}/cmake"
        fi
    fi

}

function bv_cmake_ensure
{
    if [[ "$USE_SYSTEM_CMAKE" != "yes" ]]; then 
        if [[ "$DO_CMAKE" == "yes" || "$DO_VTK" == "yes" ]] ; then
            ensure_built_or_ready "cmake"  $CMAKE_VERSION  $CMAKE_BUILD_DIR  $CMAKE_FILE $CMAKE_URL
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
}

# *************************************************************************** #
#                          Function 5, build_cmake                            #
# *************************************************************************** #

function apply_cmake_patch_5
{
    patch -p0 <<\EOF
*** cmake-3.8.1/Source/kwsys/SystemInformation.cxx	Tue May  2 05:59:43 2017
--- cmake-3.8.1-new/Source/kwsys/SystemInformation.cxx	Thu Jun 29 11:18:38 2017
***************
*** 4805,4811 ****
    std::string lastArg = command.substr(start + 1, command.size() - start - 1);
    args.push_back(lastArg.c_str());
  
!   args.push_back(0);
  
    std::string buffer = this->RunProcess(args);
  
--- 4805,4811 ----
    std::string lastArg = command.substr(start + 1, command.size() - start - 1);
    args.push_back(lastArg.c_str());
  
!   args.push_back(nullptr);
  
    std::string buffer = this->RunProcess(args);
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 5 to cmake."
        return 1
    else
        return 0
    fi
}

function apply_cmake_patch_4
{
    patch -p0 <<\EOF
--- cmake-3.0.2/Source/cmMakefileTargetGenerator.cxx
+++ cmake-3.0.2-new/Source/cmMakefileTargetGenerator.cxx
@@ -306,6 +306,11 @@ std::string cmMakefileTargetGenerator::G
     // Add target-specific flags.
     this->LocalGenerator->AddCompileOptions(flags, this->Target,
                                             lang, this->ConfigName);
+#if 1
+    for(size_t j = 0; j < flags.size(); ++j)
+        if(flags[j] == '"') 
+            flags[j] = ' ';
+#endif
 
     ByLanguageMap::value_type entry(l, flags);
     i = this->FlagsByLanguage.insert(entry).first;
@@ -1773,6 +1778,19 @@ cmMakefileTargetGenerator
     // shell no-op ":".
     if(!cmd->empty() && (*cmd)[0] != ':')
       {
+#if 1
+      // Work around a problem with random quotes being inserted into the link line.
+      std::string::size_type pos = cmd->find("\"");
+      if(pos != std::string::npos)
+      {
+          std::string cp(*cmd);
+          for(size_t j = 0; j < cp.size(); ++j)
+              if(cp[j] == '"')
+                  cp[j] = ' ';
+          linkScriptStream << cp << "\n";
+          continue;
+      }
+#endif
       linkScriptStream << *cmd << "\n";
       }
     }
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 4 ${CMAKE_VERSION} to cmake."
        return 1
    else
        return 0
    fi
}

function apply_cmake_patch_3
{
    patch -p0 <<\EOF
*** cmake-2.8.12.2/Modules/Platform/Darwin.cmake
--- cmake-2.8.12.2/Modules/Platform/Darwin.cmake.patched
***************
*** 201,208 ****
    endif()
  endif()
  
  # Make sure the combination of SDK and Deployment Target are allowed
! if(CMAKE_OSX_DEPLOYMENT_TARGET)
    if("${_CMAKE_OSX_SYSROOT_PATH}" MATCHES "^.*/MacOSX([0-9]+\\.[0-9]+)[^/]*\\.sdk")
      set(_sdk_ver "${CMAKE_MATCH_1}")
    elseif("${_CMAKE_OSX_SYSROOT_ORIG}" MATCHES "^macosx([0-9]+\\.[0-9]+)$")
--- 201,213 ----
    endif()
  endif()
  
+ #
+ # This sanity check fails on OS X 10.8 regardless of how values are
+ # set. It has been disabled for this installation of VisIt by using
+ # 'FALSE' as the triggering condition.
+ #
  # Make sure the combination of SDK and Deployment Target are allowed
! if(FALSE)
    if("${_CMAKE_OSX_SYSROOT_PATH}" MATCHES "^.*/MacOSX([0-9]+\\.[0-9]+)[^/]*\\.sdk")
      set(_sdk_ver "${CMAKE_MATCH_1}")
    elseif("${_CMAKE_OSX_SYSROOT_ORIG}" MATCHES "^macosx([0-9]+\\.[0-9]+)$")
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 3 to cmake."
        return 1
    else
        return 0
    fi
}

function apply_cmake_patch_2
{
    patch -p0 <<\EOF
*** cmake-2.8.10.2/Modules/Platform/Darwin.cmake
--- cmake-2.8.10.2/Modules/Platform/Darwin.cmake.patched
***************
*** 180,187 ****
    endif()
  endif()
  
  # Make sure the combination of SDK and Deployment Target are allowed
! if(CMAKE_OSX_DEPLOYMENT_TARGET)
    if("${_CMAKE_OSX_SYSROOT_PATH}" MATCHES "^.*/MacOSX([0-9]+\\.[0-9]+)[^/]*\\.sdk")
      set(_sdk_ver "${CMAKE_MATCH_1}")
    elseif("${_CMAKE_OSX_SYSROOT_ORIG}" MATCHES "^macosx([0-9]+\\.[0-9]+)$")
--- 180,192 ----
    endif()
  endif()
  
+ #
+ # This sanity check fails on OS X 10.8 regardless of how values are
+ # set. It has been disabled for this installation of VisIt by using
+ # 'FALSE' as the triggering condition.
+ #
  # Make sure the combination of SDK and Deployment Target are allowed
! if(FALSE)
    if("${_CMAKE_OSX_SYSROOT_PATH}" MATCHES "^.*/MacOSX([0-9]+\\.[0-9]+)[^/]*\\.sdk")
      set(_sdk_ver "${CMAKE_MATCH_1}")
    elseif("${_CMAKE_OSX_SYSROOT_ORIG}" MATCHES "^macosx([0-9]+\\.[0-9]+)$")
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 2 to cmake."
        return 1
    else
        return 0
    fi
}

function apply_cmake_patch_1
{
    patch -p0 <<\EOF
diff -c a/Modules/Platform/UnixPaths.cmake cmake-2.8.8/Modules/Platform/UnixPaths.cmake
*** a/Modules/Platform/UnixPaths.cmake
--- cmake-2.8.8/Modules/Platform/UnixPaths.cmake
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

    if [[ "${CMAKE_VERSION}" == "2.8.0" ]]; then
        apply_cmake_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    if [[ "${CMAKE_VERSION}" == "2.8.10.2" ]]; then
        apply_cmake_patch_2
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    if [[ "${CMAKE_VERSION}" == "2.8.12.2" ]]; then
        apply_cmake_patch_3
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    if [[ "${CMAKE_VERSION}" == "3.0.2" && "$BUILD_VISIT_BGQ" == "yes" ]]; then
        apply_cmake_patch_4
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    CXX_COMPILER_BASENAME=$(basename ${CXX_COMPILER})
    if [[ "${CMAKE_VERSION}" == "3.8.1" &&  "${CXX_COMPILER_BASENAME}" == "icpc" ]]; then
        apply_cmake_patch_5
        if [[ $? != 0 ]] ; then
            return 1
        fi
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
    # 0, already exists, 1 untarred src, 2 error

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
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
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

function bv_cmake_is_enabled
{
    if [[ $DO_CMAKE == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cmake_is_installed
{
    if [[ "$USE_SYSTEM_CMAKE" == "yes" ]]; then
        return 1
    fi

    check_if_installed "cmake" $CMAKE_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cmake_build
{
    #
    # Build CMake
    #
    cd "$START_DIR"
    if [[ "$DO_CMAKE" == "yes" && "$USE_SYSTEM_CMAKE" == "no" ]]; then
        check_if_installed "cmake" $CMAKE_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CMake build.  CMake is already installed."
        else
            info "Building CMake (~5 minutes)"
            build_cmake
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CMake.  Bailing out."
            fi
            info "Done building CMake"
        fi
    fi
}
