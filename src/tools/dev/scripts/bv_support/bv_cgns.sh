function bv_cgns_initialize
{
    export DO_CGNS="no"
}

function bv_cgns_enable
{
    DO_CGNS="yes"
}

function bv_cgns_disable
{
    DO_CGNS="no"
}

function bv_cgns_depends_on
{
    local depends="cmake"
    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends="$depends hdf5"
        if [[ "$DO_ZLIB" == "yes" ]] ; then
            depends="$depends zlib"
        fi
    fi
    echo $depends
}

function bv_cgns_info
{
    export CGNS_FILE=${CGNS_FILE:-"CGNS-4.1.0.tar.gz"}
    export CGNS_VERSION=${CGNS_VERSION:-"4.1.0"}
    export CGNS_COMPATIBILITY_VERSION=${CGNS_COMPATIBILITY_VERSION:-"4.1"}
    export CGNS_BUILD_DIR=${CGNS_BUILD_DIR:-"CGNS-4.1.0"}
    export CGNS_SHA256_CHECKSUM="b4584e4d0fa52c737a0fb4738157a88581df251c8c5886175ee287e1777e99fd"
}

function bv_cgns_print
{
    printf "%s%s\n" "CGNS_FILE=" "${CGNS_FILE}"
    printf "%s%s\n" "CGNS_VERSION=" "${CGNS_VERSION}"
    printf "%s%s\n" "CGNS_COMPATIBILITY_VERSION=" "${CGNS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CGNS_BUILD_DIR=" "${CGNS_BUILD_DIR}"
}

function bv_cgns_print_usage
{
    printf "%-20s %s [%s]\n" "--cgns"    "Build CGNS" "$DO_CGNS" 
}

function bv_cgns_host_profile
{
    if [[ "$DO_CGNS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CGNS" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR \${VISITHOME}/cgns/$CGNS_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        if [[ "$DO_HDF5" == "yes" ]] ; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIB TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi

}

function bv_cgns_ensure
{
    if [[ "$DO_CGNS" == "yes" ]] ; then
        ensure_built_or_ready "cgns" $CGNS_VERSION $CGNS_BUILD_DIR $CGNS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CGNS="no"
            error "Unable to build CGNS.  ${CGNS_FILE} not found."
        fi
    fi
}


function apply_cgns_410_cmake_patch
{
    info "Patching CGNS 4.1.0 CMakeLists.txt to remove h5dump and adjust HDF5 vars"
    patch -p0 << \EOF
--- CMakeLists.txt	2021-03-02 12:17:56.000000000 -0800
+++ CMakeLists.txt	2026-03-05 16:06:45.175797000 -0800
@@ -202,9 +202,15 @@
   set (SEARCH_PACKAGE_NAME "hdf5")
 
   find_package (HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS ${FIND_HDF_COMPONENTS})
+
+  if(HDF5_FOUND)
+      set(HDF5_static_C_FOUND ${HDF5_PROVIDES_STATIC_LIBS})
+      set(HDF5_shared_C_FOUND ${HDF5_PROVIDES_SHARED_LIBS})
+  endif()
+
   message (STATUS "HDF5 C libs:${HDF5_FOUND} static:${HDF5_static_C_FOUND} and shared:${HDF5_shared_C_FOUND}")
   if (HDF5_FOUND)
-    add_executable (h5dump IMPORTED)
+    #add_executable (h5dump IMPORTED)
     if (NOT HDF5_static_C_FOUND AND NOT HDF5_shared_C_FOUND)
       set (FIND_HDF_COMPONENTS C)
 
@@ -217,19 +223,19 @@
         add_definitions (-DH5_BUILT_AS_STATIC_LIB)
       endif (HDF5_BUILD_SHARED_LIBS)
       if (BUILD_SHARED_LIBS AND WIN32)
-        set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dumpdll")
+        #set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dumpdll")
       else (BUILD_SHARED_LIBS AND WIN32)
-        set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
+        #set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
       endif (BUILD_SHARED_LIBS AND WIN32)
     else (NOT HDF5_static_C_FOUND AND NOT HDF5_shared_C_FOUND)
       if (BUILD_SHARED_LIBS AND HDF5_shared_C_FOUND)
-        set (LINK_LIBS ${LINK_LIBS} ${HDF5_C_SHARED_LIBRARY})
+        set (LINK_LIBS ${LINK_LIBS} hdf5-shared )
       else (HDF5_static_C_FOUND)
-        set (LINK_LIBS ${LINK_LIBS} ${HDF5_C_STATIC_LIBRARY})
+        set (LINK_LIBS ${LINK_LIBS} hdf5-static)
       endif (BUILD_SHARED_LIBS AND HDF5_shared_C_FOUND)
-      set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
+      #set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
     endif (NOT HDF5_static_C_FOUND AND NOT HDF5_shared_C_FOUND)
-    set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:h5dump>)
+    #set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:h5dump>)
 
     set (HDF5_HAVE_H5PUBCONF_H 1)
     set (HDF5_HAVE_HDF5 1)
@@ -250,9 +256,9 @@
     endif ()
     set (LINK_LIBS ${LINK_LIBS} ${HDF5_LIBRARIES})
 
-    add_executable (h5dump IMPORTED)
-    set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
-    set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:h5dump>)
+    #add_executable (h5dump IMPORTED)
+    #set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
+    #set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:h5dump>)
   endif (HDF5_FOUND)
   set (HDF5_PACKAGE_NAME ${SEARCH_PACKAGE_NAME})
 
@@ -333,7 +339,10 @@
   set(CGNS_ENABLE_PARALLEL "OFF" CACHE BOOL "Enable or disable parallel interface ?")
   mark_as_advanced(CLEAR CGNS_ENABLE_PARALLEL)
   # Check that HDF5 has parallel support
-  if (NOT (HDF5_IS_PARALLEL OR HDF5_ENABLE_PARALLEL))
+  # HDF5 2.0 changed the cmake var that indicates MPI support to HDF5_PROVIDES_PARALLEL, see:
+  # https://www.hdfgroup.org/2025/11/10/release-of-hdf5-2-0-0-newsletter-207/
+  # https://github.com/HDFGroup/hdf5/pull/5716
+  if (NOT (HDF5_IS_PARALLEL OR HDF5_ENABLE_PARALLEL OR HDF5_PROVIDES_PARALLEL))
       message(FATAL_ERROR "HDF5 has been found, but is missing parallel support.")
   endif()
 else (CGNS_ENABLE_HDF5 AND HDF5_NEED_MPI)
EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}


function apply_cgns_patch
{
    if [[ ${CGNS_VERSION} == "4.1.0" ]] ; then
        apply_cgns_410_cmake_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.5, build_cgns                            #
#                                                                             #
# Kevin Griffin, Tue Dec 30 11:39:02 PST 2014                                 #
# Added a patch for the configure script to correctly locate and use the      #
# the dylib for hdf5 and szip. Added the correct linker options to the        #
# dynamic library creation.                                                   #
#                                                                             #
# Kevin Griffin, Fri Jan 16 10:28:21 PST 2015                                 #
# Fixed the --with-szip and --with-zlib to specify the full path to the       #
# library for both OSX and linux                                              #
#                                                                             #
# Kevin Griffin, Mon Apr 27 15:20:43 PDT 2015                                 #
# Patched the configure file to use the zlib library specified in the         #
# --with-zlib option.                                                         #
#                                                                             #
# *************************************************************************** #

function build_cgns
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CGNS_BUILD_DIR $CGNS_FILE SHA256 $CGNS_SHA256_CHECKSUM
    untarred_cgns=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_cgns == -1 ]] ; then
        warn "Unable to prepare CGNS Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    cd $CGNS_BUILD_DIR
    apply_cgns_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_cgns == 1 ]] ; then
            warn "Giving up on CGNS build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure CGNS (CMake)
    #

    info "Configuring CGNS . . ."

    cd ../
    CGNS_SRC_DIR="${CGNS_BUILD_DIR}"
    CGNS_BUILD_SUBDIR="${CGNS_SRC_DIR}-build"
    CGNS_INSTALL_DIR="${VISITDIR}/cgns/${CGNS_VERSION}/${VISITARCH}"

    if [[ ! -d "${CGNS_BUILD_SUBDIR}" ]] ; then
        mkdir "${CGNS_BUILD_SUBDIR}" || error "Unable to create CGNS build directory."
    fi

    cd "${CGNS_BUILD_SUBDIR}" || error "Can't cd to CGNS build dir."

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cgns_cmake_build_type="Debug"
    else
        cgns_cmake_build_type="Release"
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cgns_build_shared="OFF"
    else
        cgns_build_shared="ON"
    fi

    # Help CMake find enabled deps in VisIt's prefix.

    cmake_opts=""
    cmake_opts="${cmake_opts} -DCMAKE_INSTALL_PREFIX:PATH=${CGNS_INSTALL_DIR}"
    cmake_opts="${cmake_opts} -DCMAKE_BUILD_TYPE:STRING=${cgns_cmake_build_type}"
    cmake_opts="${cmake_opts} -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON"
    cmake_opts="${cmake_opts} -DCMAKE_C_FLAGS:STRING=\"${CFLAGS} ${C_OPT_FLAGS}\""
    cmake_opts="${cmake_opts} -DBUILD_SHARED_LIBS:BOOL=${cgns_build_shared}"

    # CGNS-specific options.
    cmake_opts="${cmake_opts} -DCGNS_BUILD_SHARED:BOOL=${cgns_build_shared}"
    cmake_opts="${cmake_opts} -DCGNS_ENABLE_FORTRAN:BOOL=OFF"
    cmake_opts="${cmake_opts} -DCGNS_BUILD_CGNSTOOLS:BOOL=OFF"
    cmake_opts="${cmake_opts} -DCGNS_ENABLE_TESTS:BOOL=OFF"
    cmake_opts="${cmake_opts} -DCGNS_ENABLE_PARALLEL:BOOL=OFF"

    if [[ "$DO_HDF5" == "yes" ]] ; then
        cmake_opts="${cmake_opts} -DCGNS_ENABLE_HDF5:BOOL=ON"
        cmake_opts="${cmake_opts} -DHDF5_DIR:PATH=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/cmake"
        if [[ "$DO_ZLIB" == "yes" ]] ; then
            cmake_opts="${cmake_opts} -DHDF5_NEED_ZLIB:BOOL=ON"
            # allow using ZLIB_ROOT
            cmake_opts="${cmake_opts} -DCMAKE_POLICY_DEFAULT_CMP0074=NEW"
            cmake_opts="${cmake_opts} -DZLIB_ROOT:PATH=${VISIT_ZLIB_DIR}"
        fi
        # if HDF5 built with parallel, then need to set MPI_HOME
        if [[ "$PAR_COMPILER" != "" ]] ; then
            cmake_opts="${cmake_opts} -DHDF5_NEED_MPI:BOOL=ON"
            cmake_opts="${cmake_opts} -DMPI_HOME:STRING=${PAR_HOME}"
        fi
    else
        cmake_opts="${cmake_opts} -DCGNS_ENABLE_HDF5:BOOL=OFF"
    fi

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_COMMAND}\"" ${cmake_opts} "../${CGNS_SRC_DIR}" >> bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "CGNS configuration failed."

    #
    # Build CGNS
    #
    info "Building CGNS . . . (~2 minutes)"

    ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS || error "CGNS did not build correctly. Giving up."

    #
    # Install into the VisIt third party location.
    #
    info "Installing CGNS . . ."

    ${CMAKE_COMMAND} --install . || error "CGNS did not install correctly. Giving up."

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/cgns"
        chgrp -R ${GROUP} "$VISITDIR/cgns"
    fi
    cd "$START_DIR"
    info "Done with CGNS"
    return 0
}

function bv_cgns_is_enabled
{
    if [[ $DO_CGNS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cgns_is_installed
{
    check_if_installed "cgns" $CGNS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cgns_build
{
    cd "$START_DIR"
    if [[ "$DO_CGNS" == "yes" ]] ; then
        check_if_installed "cgns" $CGNS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CGNS build.  CGNS is already installed."
        else
            info "Building CGNS (~2 minutes)"
            build_cgns
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CGNS.  Bailing out."
            fi
            info "Done building CGNS"
        fi
    fi
}
