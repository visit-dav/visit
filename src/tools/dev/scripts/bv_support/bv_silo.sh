function bv_silo_initialize
{
    export DO_SILO="no"
    export DO_SILEX="no"
    add_extra_commandline_args "silo" "silex" 0 "Enable silex when building Silo"
}

function bv_silo_enable
{
    DO_SILO="yes"
}

function bv_silo_disable
{
    DO_SILO="no"
}

function bv_silo_silex
{
    info "Enabling silex in Silo build"
    DO_SILEX="yes"
    bv_silo_enable
}

function bv_silo_depends_on
{
    local depends_on=""

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        depends_on="zlib"
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="$depends_on hdf5"
    fi
    
    echo $depends_on
}

function bv_silo_info
{
    export SILO_VERSION=${SILO_VERSION:-"4.12.0"}
    export SILO_FILE=${SILO_FILE:-"Silo-${SILO_VERSION}.tar.xz"}
    export SILO_COMPATIBILITY_VERSION=${SILO_COMPATIBILITY_VERSION:-"4.12.0"}
    export SILO_BUILD_DIR=${SILO_BUILD_DIR:-"Silo-${SILO_VERSION}-build"}
    export SILO_SHA256_CHECKSUM="bde1685e4547d5dd7416bd6215b41f837efef0e4934d938ba776957afbebdff0"
}

function bv_silo_print
{
    printf "%s%s\n" "SILO_FILE=" "${SILO_FILE}"
    printf "%s%s\n" "SILO_VERSION=" "${SILO_VERSION}"
    printf "%s%s\n" "SILO_COMPATIBILITY_VERSION=" "${SILO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "SILO_BUILD_DIR=" "${SILO_BUILD_DIR}"
}

function bv_silo_print_usage
{
    printf "%-20s %s [%s]\n" "--silo" "Build Silo support" "$DO_SILO"
    printf "%-20s %s [%s]\n" "--silex" "Enable silex when building Silo" "$DO_SILEX"
}

function bv_silo_host_profile
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Silo" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_SILO_DIR \${VISITHOME}/silo/$SILO_VERSION/\${VISITARCH})" \
            >> $HOSTCONF

        libdep=""
        if [[ "$DO_HDF5" == "yes" ]] ; then
            libdep="HDF5_LIB"
        fi
        libdep="$libdep ZLIB_LIB"
        if [[ -n "$libdep" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP $libdep TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_silo_ensure
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        ensure_built_or_ready "silo" $SILO_VERSION $SILO_BUILD_DIR $SILO_FILE $SILO_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_SILO="no"
            error "Unable to build Silo.  ${SILO_FILE} not found."
        fi
    fi
}

function apply_silo_4120_const_ns_patch
{
    info "Patching Silo 4.12.0 for constant namescheme issue"
    patch -p1 << \EOF
From 43a52d788a3c15bee3b9391906e8ed276c5a456c Mon Sep 17 00:00:00 2001
From: "Mark C. Miller" <5720676+markcmiller86@users.noreply.github.com>
Date: Fri, 23 Jan 2026 19:03:18 -0800
Subject: [PATCH] fix const nameschemes

---
 src/silo/silo_ns.c | 25 ++++++++++++++++++-------
 1 file changed, 18 insertions(+), 7 deletions(-)

diff --git a/Silo-4.12.0/src/silo/silo_ns.c b/Silo-4.12.0/src/silo/silo_ns.c
index 645077dd..e17a57d4 100644
--- a/Silo-4.12.0/src/silo/silo_ns.c
+++ b/Silo-4.12.0/src/silo/silo_ns.c
@@ -424,22 +424,33 @@ DBMakeNamescheme(char const *fmt, ...)
     */
     if (rv->ncspecs == 0)
     {
-        int rm_unnecessary_delim = 0;
+        free(rv->fmt);
 
-        if (n > 2 && fmt[0] == fmt[n-1])
-            rm_unnecessary_delim = !db_VariableNameValid(fmt);
+        /* If whole string is valid, take all of it. */
+        if (db_VariableNameValid(fmt))
+        {
+            rv->fmt = STRNDUP(&fmt[0],n);
+            rv->fmtlen = n;
+            return rv;
+        }
 
-        free(rv->fmt);
+        /* If whole string but first char is valid, take all but first char */
+        if (db_VariableNameValid(&fmt[1]))
+        {
+            rv->fmt = STRNDUP(&fmt[1],n-1);
+            rv->fmtlen = n-1;
+            return rv;
+        }
 
-        if (rm_unnecessary_delim)
+        if (fmt[0] == fmt[n-1])
         {
             rv->fmt = STRNDUP(&fmt[1],n-2);
             rv->fmtlen = n-2;
         }
         else
         {
-            rv->fmt = STRNDUP(&fmt[0],n);
-            rv->fmtlen = n;
+            rv->fmt = STRNDUP(&fmt[0],n-1);
+            rv->fmtlen = n-1;
         }
 
         return rv;
-- 
2.50.1 (Apple Git-155)
EOF

    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_silo_4120_mpio_vfd_patch
{
    info "Patching Silo 4.12.0 for mpio vfd issue"
    patch -p1 << \EOF
From 7cb7983ed1c369b3ad47d1b5852fa8651bb7bdb2 Mon Sep 17 00:00:00 2001
From: "Mark C. Miller" <5720676+markcmiller86@users.noreply.github.com>
Date: Tue, 10 Feb 2026 18:54:11 -0800
Subject: [PATCH] rm mpio vfd from dflt list

---
 Silo-4.12.0/src/silo/silo.c | 2 --
 1 file changed, 2 deletions(-)

diff --git a/Silo-4.12.0/src/silo/silo.c b/Silo-4.12.0/src/silo/silo.c
index f34653cf..893137da 100644
--- a/Silo-4.12.0/src/silo/silo.c
+++ b/Silo-4.12.0/src/silo/silo.c
@@ -2837,8 +2837,6 @@ const int* db_get_used_file_options_sets_ids()
     used_slots[n++] = DB_FILE_OPTS_H5_DEFAULT_SPLIT;
     used_slots[n++] = DB_FILE_OPTS_H5_DEFAULT_DIRECT;
     used_slots[n++] = DB_FILE_OPTS_H5_DEFAULT_FAMILY;
-    used_slots[n++] = DB_FILE_OPTS_H5_DEFAULT_MPIO;
-    used_slots[n++] = DB_FILE_OPTS_H5_DEFAULT_MPIP;
     for (i = n; i < MAX_FILE_OPTIONS_SETS+NUM_DEFAULT_FILE_OPTIONS_SETS+1; i++)
         used_slots[i] = -1;

--
2.50.1 (Apple Git-155)
EOF

    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_silo_4120_lib_vs_lib64_patch
{
    info "Patching Silo 4.12.0 for install lib/lib64 issue"
    patch -p1 << \EOF
--- a/Silo-4.12.0/CMake/SiloConfig.cmake.in	2025-11-20 21:30:59.000000000 -0800
+++ b/Silo-4.12.0/CMake/SiloConfig.cmake.in	2026-02-20 16:39:18.900700000 -0800
@@ -99,7 +99,7 @@
 # project which has already built SILO as a subproject
 #-----------------------------------------------------------------------------
 if(NOT TARGET @SILO_NAME@)
-  include (${PACKAGE_PREFIX_DIR}/lib/cmake/Silo/@silo_targets_name@.cmake)
+  include (${CMAKE_CURRENT_LIST_DIR}/@silo_targets_name@.cmake)
 
   if(SILO_ENABLE_HDF5)
       include(CMakeFindDependencyMacro)
EOF

    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_silo_4120_hzip_needs_zlib_patch
{
    info "Patching Silo 4.12.0 for hzip needs zlib issue"
    patch -p0 << \EOF
--- Silo-4.12.0/CMakeLists.txt.orig	2026-03-10 12:08:28.256403000 -0700
+++ Silo-4.12.0/CMakeLists.txt	2026-03-10 12:08:15.604238000 -0700
@@ -617,9 +617,7 @@
             ${Silo_SOURCE_DIR}/src/hzip)
     endif()
 
-    if(ZLIB_FOUND)
-        list(APPEND silo_library_include_dirs ${ZLIB_INCLUDE_DIR})
-    else()
+    if(NOT ZLIB_FOUND)
         list(APPEND SILO_COMPILE_DEFINES WITHOUT_ZLIB)
     endif()
 endif()
@@ -627,6 +625,10 @@
 add_library(silo ${silo_library_sources})
 set_target_properties(silo PROPERTIES VERSION ${SILO_VERSION} SOVERSION ${SILO_SOVERSION})
 
+if(ZLIB_FOUND AND SILO_ENABLE_HZIP)
+    target_link_libraries(silo  ZLIB::ZLIB)
+endif()
+
 target_compile_definitions(silo PRIVATE ${SILO_COMPILE_DEFINES})
 add_dependencies(silo pdb_detect)
 target_include_directories(silo PRIVATE ${silo_library_include_dirs})
EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_silo_4120_extents_calc
{
    info "Patching Silo 4.12.0 for extents calc fix"
    patch -p1 << \EOF
From: "Mark C. Miller" <5720676+markcmiller86@users.noreply.github.com>
Date: Mon, 30 Mar 2026 08:51:03 -0700
Subject: [PATCH 33/33] Fix extents calc

Fix include_point used in extents calc to condition index tests on ndims
---
 Silo-4.12.0/src/silo/silo.c | 12 ++++++------
 1 file changed, 6 insertions(+), 6 deletions(-)

diff --git a/Silo-4.12.0/src/silo/silo.c b/Silo-4.12.0/src/silo/silo.c
index bc963bd7..e828de08 100644
--- a/Silo-4.12.0/src/silo/silo.c
+++ b/Silo-4.12.0/src/silo/silo.c
@@ -11385,12 +11385,12 @@ include_point(int ptidx, int ndims, int const *dims,
     int j = ndims>1 ? (ptidx/dims[0])           % dims[1] : 0;
     int k = ndims>2 ? (ptidx/(dims[1]*dims[0])) % dims[2] : 0;

-    if (i < minidx[0]) return 0;
-    if (i > maxidx[0]) return 0;
-    if (j < minidx[1]) return 0;
-    if (j > maxidx[1]) return 0;
-    if (k < minidx[2]) return 0;
-    if (k > maxidx[2]) return 0;
+    if (ndims > 0 && i < minidx[0]) return 0;
+    if (ndims > 0 && i > maxidx[0]) return 0;
+    if (ndims > 1 && j < minidx[1]) return 0;
+    if (ndims > 1 && j > maxidx[1]) return 0;
+    if (ndims > 2 && k < minidx[2]) return 0;
+    if (ndims > 2 && k > maxidx[2]) return 0;

     return 1;
 }
--
2.50.1 (Apple Git-155)
EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_silo_patch
{
    info "Patching silo . . ."

    compare_version_strings $SILO_VERSION 4.12.0 -eq
    if [[ $? -eq 0 ]]; then
        apply_silo_4120_const_ns_patch
        if [[ $? != 0 ]] ; then
            warn "Giving up on Silo build because the patch failed."
            return 1
        fi
        apply_silo_4120_mpio_vfd_patch
        if [[ $? != 0 ]] ; then
            warn "Giving up on Silo build because the patch failed."
            return 1
        fi
        apply_silo_4120_lib_vs_lib64_patch
        if [[ $? != 0 ]] ; then
            warn "Giving up on Silo build because the patch failed."
            return 1
        fi
        apply_silo_4120_hzip_needs_zlib_patch
        if [[ $? != 0 ]] ; then
            warn "Giving up on Silo build because the patch failed."
            return 1
        fi
        apply_silo_4120_extents_calc
        if [[ $? != 0 ]] ; then
            warn "Giving up on Silo build because the patch failed."
            return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                            Function 8, build_silo
#
# Modfications:
#   Mark C. Miller, Wed Feb 18 22:57:25 PST 2009
#   Added logic to build silex and copy bins on Mac. Removed disablement of
#   browser.
#
#   Mark C. Miller Mon Jan  7 10:31:46 PST 2013
#   PDB/SCORE lite headers are now handled in Silo and require additional
#   configure option to ensure they are installed.
#
#   Brad Whitlock, Tue Apr  9 12:20:22 PDT 2013
#   Add support for custom zlib.
#
#   Kathleen Biagas, Tue Jun 10 08:21:33 MST 2014
#   Disable silex for static builds.
#
# *************************************************************************** #

function build_silo
{
    #
    # Prepare build dir
    #
    prepare_build_dir $SILO_BUILD_DIR $SILO_FILE
    untarred_silo=$?
    if [[ $untarred_silo == -1 ]] ; then
        warn "Unable to prepare Silo build directory. Giving Up!"
        return 1
    fi
    apply_silo_patch || return 1
    
    #
    # CMake Silo
    #
    cmake_opts="\
        -DCMAKE_INSTALL_PREFIX:PATH=\"${VISITDIR}/silo/${SILO_VERSION}/${VISITARCH}\" \
        -DBUILD_TESTING:BOOL=OFF \
        -DSILO_ENABLE_INSTALL_LITE_HEADERS:BOOL=ON"

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cmake_opts="${cmake_opts} -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
    else
        cmake_opts="${cmake_opts} -DCMAKE_BUILD_TYPE:STRING=Release"
    fi

    if [[ "$FC_COMPILER" == "yes" ]] ; then
        cmake_opts="${cmake_opts} -DSILO_ENABLE_FORTRAN:BOOL=ON"
    else
        cmake_opts="${cmake_opts} -DSILO_ENABLE_FORTRAN:BOOL=OFF"
    fi
       
    if [[ "$DO_PYTHON" == "yes" ]] ; then
        cmake_opts="${cmake_opts} \
            -DSILO_ENABLE_PYTHON_MODULE:BOOL=ON \
            -DSILO_PYTHON_DIR:PATH=\"${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}\""
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cmake_opts="${cmake_opts} -DSILO_ENABLE_SHARED:BOOL=OFF"
    else
        cmake_opts="${cmake_opts} -DSILO_ENABLE_SHARED:BOOL=ON"
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        cmake_opts="${cmake_opts} \
            -DSILO_ENABLE_HDF5:BOOL=ON \
            -DSILO_HDF5_DIR:PATH=\"${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}\""
    else
        cmake_opts="${cmake_opts} -DSILO_ENABLE_HDF5:BOOL=OFF"
    fi

    if [[ "$DO_ZLIB" == "yes" ]]; then
        cmake_opts="${cmake_opts} \
            -DZLIB_ROOT:PATH=${VISIT_ZLIB_DIR} \
            -DSILO_BUILD_FOR_BSD_LICENSE:BOOL=OFF \
            -DSILO_ENABLE_HZIP:BOOL=ON \
            -DSILO_ENABLE_FPZIP:BOOL=ON \
            -DSILO_ZLIB_DIR:PATH=${VISIT_ZLIB_DIR}"
    fi

    if [[ "$DO_SILEX" == "yes" ]]; then
        cmake_opts="${cmake_opts} \
            -DSILO_ENABLE_SILEX:BOOL=ON \
            -DSILO_QT6_DIR:PATH=\"${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}\""
    fi

    # silo needs to find mpi if hdf5 was built with mpi support
    if [[ "$PAR_COMPILER" != "" ]] ; then
        cmake_opts="${cmake_opts} -DMPI_C_COMPILER:STRING=${PAR_COMPILER}"
        cmake_opts="${cmake_opts} -DMPI_CXX_COMPILER:STRING=${PAR_COMPILER_CXX}"
    fi

    if [[ "$PAR_INCLUDE" != "" ]] ; then
        cmake_opts="${cmake_opts} -DMPI_C_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
        cmake_opts="${cmake_opts} -DMPI_CXX_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
    fi

    if [[ "$PAR_LIBS" != "" ]] ; then
        cmake_opts="${cmake_opts} -DMPI_C_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cmake_opts="${cmake_opts} -DMPI_C_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
        cmake_opts="${cmake_opts} -DMPI_CXX_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cmake_opts="${cmake_optss} -DMPI_CXX_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
    fi

    info "CMake'ing Silo"
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    mkdir -p ${SILO_BUILD_DIR}
    cd ${SILO_BUILD_DIR} || error "Can't cd to Silo build dir."

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command".  This was first discovered on 
    # BGQ and then showed up in random cases for both OSX and Linux machines. 
    # Brad resolved this on BGQ  with a simple work around - we write a simple 
    # script that we invoke with bash which calls cmake with all of the proper
    # arguments. We are now using this strategy for all platforms.
    #

    rm -f bv_run_cmake.sh
    echo "\"${CMAKE_BIN}\"" ${cmake_opts} ../Silo-${SILO_VERSION} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "Silo cmake failed. Giving up"
        return 1
    fi

    #
    # Build Silo
    #
    info "Building Silo. . . (~5 minutes)"
    ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Silo build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing Silo"
    ${CMAKE_COMMAND} --install .
    if [[ $? != 0 ]] ; then
        warn "Silo install failed. Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/silo"
        chgrp -R ${GROUP} "$VISITDIR/silo"
    fi
    cd "$START_DIR"
    info "Done with Silo"
    return 0
}

function bv_silo_is_enabled
{
    if [[ $DO_SILO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_silo_is_installed
{
    check_if_installed "silo" $SILO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_silo_build
{
    cd "$START_DIR"
    if [[ "$DO_SILO" == "yes" ]] ; then
        check_if_installed "silo" $SILO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Silo build.  Silo is already installed."
        else
            info "Building Silo (~2 minutes)"
            build_silo
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Silo.  Bailing out."
            fi
            info "Done building Silo"
        fi
    fi
}
