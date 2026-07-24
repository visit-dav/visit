function bv_netcdf_initialize
{
    export DO_NETCDF="no"
    export USE_SYSTEM_NETCDF="no"
    add_extra_commandline_args "netcdf" "alt-netcdf-dir" 1 "Use alternative directory for netcdf"
}

function bv_netcdf_enable
{
    DO_NETCDF="yes"
}

function bv_netcdf_disable
{
    DO_NETCDF="no"
}

function bv_netcdf_alt_netcdf_dir
{
    bv_netcdf_enable
    USE_SYSTEM_NETCDF="yes"
    NETCDF_INSTALL_DIR="$1"
}

function bv_netcdf_depends_on
{
    if [[ "$USE_SYSTEM_NETCDF" == "yes" ]]; then
        echo ""
    else
        local depends_on="zlib"
        if [[ "$DO_HDF5" == "yes" ]] ; then
            depends_on="hdf5"        
        fi
        echo ${depends_on}
    fi
}

function bv_netcdf_initialize_vars
{
    if [[ "$USE_SYSTEM_NETCDF" == "no" ]]; then
        NETCDF_INSTALL_DIR="${VISITDIR}/netcdf/$NETCDF_VERSION/${VISITARCH}"
    fi
}

function bv_netcdf_info
{
    export NETCDF_VERSION=${NETCDF_VERSION-"4.9.3"}
    export NETCDF_FILE=${NETCDF_FILE-"netcdf-c-${NETCDF_VERSION}.tar.gz"}
    export NETCDF_COMPATIBILITY_VERSION=${NETCDF_COMPATIBILITY_VERSION-"4.9"}
    export NETCDF_BUILD_DIR=${NETCDF_BUILD_DIR-"netcdf-c-4.9.3"}
    export NETCDF_SHA256_CHECKSUM="a474149844e6144566673facf097fea253dc843c37bc0a7d3de047dc8adda5dd"
}

function bv_netcdf_print
{
    printf "%s%s\n" "NETCDF_FILE=" "${NETCDF_FILE}"
    printf "%s%s\n" "NETCDF_VERSION=" "${NETCDF_VERSION}"
    printf "%s%s\n" "NETCDF_COMPATIBILITY_VERSION=" "${NETCDF_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "NETCDF_BUILD_DIR=" "${NETCDF_BUILD_DIR}"
}

function bv_netcdf_print_usage
{
    printf "%-20s %s [%s]\n" "--netcdf" "Build NetCDF" "${DO_NETCDF}"
    printf "%-20s %s [%s]\n" "--alt-netcdf-dir" "Use NetCDF from an alternative directory"
}

function bv_netcdf_host_profile
{
    if [[ "$DO_NETCDF" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## NetCDF" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_NETCDF" == "yes" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR $NETCDF_INSTALL_DIR)" \
                >> $HOSTCONF
        else
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_NETCDF_DIR \${VISITHOME}/netcdf/$NETCDF_VERSION/\${VISITARCH})" \
                >> $HOSTCONF
            if [[ "$DO_HDF5" == "yes" ]] ; then
                echo \
                    "VISIT_OPTION_DEFAULT(VISIT_NETCDF_LIBDEP HDF5_HL_LIB TYPE STRING)" \
                    >> $HOSTCONF
            fi
        fi
    fi
}

function bv_netcdf_ensure
{
    if [[ "$DO_NETCDF" == "yes" && "$USE_SYSTEM_NETCDF" == "no" ]] ; then
        ensure_built_or_ready "netcdf" $NETCDF_VERSION $NETCDF_BUILD_DIR \
                              $NETCDF_FILE \
                              http://www.unidata.ucar.edu/downloads/netcdf/ftp/
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_NETCDF="no"
            error "Unable to build NetCDF.  ${NETCDF_FILE} not found."
        fi
    fi
}

function apply_netcdfc493_patches_for_hdf20
{
    info "Patching netcdf-c 4.9.3 for HDF5-2.0"
    patch -p0 << \EOF
diff -r -u netcdf-c-4.9.3.orig/libhdf5/H5FDhttp.c netcdf-c-4.9.3/libhdf5/H5FDhttp.c
--- netcdf-c-4.9.3.orig/libhdf5/H5FDhttp.c	2025-02-07 13:40:00.000000000 -0800
+++ netcdf-c-4.9.3/libhdf5/H5FDhttp.c	2026-01-31 16:12:50.659720000 -0800
@@ -49,6 +49,7 @@
 Define a simple #ifdef test for the version of H5FD_class_t we are using 
 */
 
+#if 0
 #if H5_VERS_MAJOR == 1
 #if H5_VERS_MINOR < 10
 #define H5FDCLASS1 1
@@ -56,6 +57,7 @@
 #else
 #error "Cannot determine version of H5FD_class_t"
 #endif
+#endif
 
 #ifdef H5_HAVE_WIN32_API
 /* The following two defines must be before any windows headers are included */
diff -r -u netcdf-c-4.9.3.orig/libhdf5/H5FDhttp.h netcdf-c-4.9.3/libhdf5/H5FDhttp.h
--- netcdf-c-4.9.3.orig/libhdf5/H5FDhttp.h	2025-02-07 13:40:00.000000000 -0800
+++ netcdf-c-4.9.3/libhdf5/H5FDhttp.h	2026-01-31 16:22:30.395734000 -0800
@@ -31,7 +31,7 @@
 
 #if H5_VERSION_GE(1,13,2)
 #define H5_VFD_HTTP     ((H5FD_class_value_t)(514))
-#define H5FD_HTTP	(H5FDperform_init(H5FD_http_init))
+#define H5FD_HTTP	(H5OPEN H5FD_HTTP_g)
 #else
 #define H5FD_HTTP	(H5FD_http_init())
 #endif
EOF
    if [[ $? != 0 ]] ; then
      warn "netcdf-c 4.9.3 patch for HDF5-2.0 failed."
      return 1
    fi
    return 0
}

function apply_netcdf_patch
{
    if [[ "$DO_HDF5" == "yes" ]] ; then
        apply_netcdfc493_patches_for_hdf20
        if [[ $? != 0 ]] ; then
           return 1
        fi
    fi
    return 0
}

# *************************************************************************** #
#                         Function 8.4, build_netcdf                          #
#                                                                             #
# Mark C. Miller, Wed Oct 27 19:25:09 PDT 2010                                #
# Added patch for exodusII. This way, a single netcdf installation should     #
# work for 'normal' netcdf operations as well as for ExodusII.                #
#                                                                             #
# Kevin Griffin, Mon Nov 17 11:31:52 PST 2014                                 #
# Added patch for OS X 10.9 Mavericks. HAVE_STRLCAT is not getting defined    #
# in this version so its trying to add a duplicate strlcat definition. This   #
# patch comments out the duplicate strlcat definition.                        #
# *************************************************************************** #
function build_netcdf
{
    # Prepare build dir
    #
    prepare_build_dir $NETCDF_BUILD_DIR $NETCDF_FILE SHA256 $NETCDF_SHA256_CHECKSUM
    untarred_netcdf=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_netcdf == -1 ]] ; then
        warn "Unable to prepare NetCDF Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching NetCDF . . ."
    apply_netcdf_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_netcdf == 1 ]] ; then
            warn "Giving up on NetCDF build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # CMake NetCDF
    #
    cmake_opts="\
        -DCMAKE_INSTALL_PREFIX:PATH=\"${VISITDIR}/netcdf/${NETCDF_VERSION}/${VISITARCH}\" \
        -DNETCDF_BUILD_UTILITIES:BOOL=OFF \
        -DNETCDF_ENABLE_EXAMPLES:BOOL=OFF \
        -DNETCDF_ENABLE_FILTER_BLOSC:BOOL=OFF \
        -DNETCDF_ENABLE_FILTER_BZ2:BOOL=OFF \
        -DNETCDF_ENABLE_FILTER_SZIP:BOOL=OFF \
        -DNETCDF_ENABLE_FILTER_TESTING:BOOL=OFF \
        -DNETCDF_ENABLE_FILTER_ZSTD:BOOL=OFF \
        -DNETCDF_ENABLE_REMOTE_FUNCTIONALITY:BOOL=OFF \
        -DNETCDF_ENABLE_TESTS:BOOL=OFF \
        -DNETCDF_ENABLE_V2_API:BOOL=ON \
        -DBUILD_TESTING:BOOL=OFF"

    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        cmake_opts="${cmake_opts} -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
    else
        cmake_opts="${cmake_opts} -DCMAKE_BUILD_TYPE:STRING=Release"
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cmake_opts="${cmake_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        cmake_opts="${cmake_opts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        cmake_opts="${cmake_opts} \
            -DNETCDF_ENABLE_HDF5:BOOL=ON \
            -DHDF5_ROOT:PATH=\"${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}\""
    else
        cmake_opts="${cmake_opts} -DNETCDF_ENABLE_HDF5:BOOL=OFF"
    fi

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        cmake_opts="${cmake_opts} -DZLIB_ROOT:PATH=\"${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}\""
    fi 

    # netcdf needs to find mpi if hdf5 was built with mpi support
    if [[ x"$PAR_COMPILER" != x ]] ; then
        cmake_opts="${cmake_opts} -DMPI_C_COMPILER:STRING=${PAR_COMPILER}"
        cmake_opts="${cmake_opts} -DMPI_CXX_COMPILER:STRING=${PAR_COMPILER_CXX}"
    fi

    if [[ x"$PAR_INCLUDE" != x ]] ; then
        cmake_opts="${cmake_opts} -DMPI_C_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
        cmake_opts="${cmake_opts} -DMPI_CXX_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
    fi

    if [[ x"$PAR_LIBS" != x ]] ; then
        cmake_opts="${cmake_opts} -DMPI_C_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cmake_opts="${cmake_opts} -DMPI_C_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
        cmake_opts="${cmake_opts} -DMPI_CXX_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
        cmake_opts="${cmake_opts} -DMPI_CXX_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
    fi


    info "CMakeing NetCDF . . ."
    cd $NETCDF_BUILD_DIR || error "Can't cd to netcdf build dir."

    # their FindZLIB module interfere's with cmake's, and we want cmake's
    rm cmake/modules/FindZLIB.cmake

    info "Invoking command to cmake NetCDF"

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    rm -f bv_run_cmake.sh
    echo "\"${CMAKE_BIN}\"" ${cmake_opts} ../netcdf-c-${NETCDF_VERSION} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "NetCDF cmake failed.  Giving up"
        return 1
    fi

    #
    # Build NetCDF
    #
    info "Building NetCDF . . . (~2 minutes)"
    $MAKE
    if [[ $? != 0 ]] ; then
        warn "NetCDF build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing NetCDF . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "NetCDF install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/netcdf"
        chgrp -R ${GROUP} "$VISITDIR/netcdf"
    fi
    cd "$START_DIR"
    info "Done with NetCDF"
    return 0
}

function bv_netcdf_is_enabled
{
    if [[ $DO_NETCDF == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_netcdf_is_installed
{
    if [[ "$USE_SYSTEM_NETCDF" == "yes" ]]; then
        return 1
    fi

    check_if_installed "netcdf" $NETCDF_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_netcdf_build
{
    cd "$START_DIR"
    if [[ "$DO_NETCDF" == "yes" && "$USE_SYSTEM_NETCDF" == "no" ]] ; then
        check_if_installed "netcdf" $NETCDF_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping NetCDF build.  NetCDF is already installed."
        else
            info "Building NetCDF (~5 minutes)"
            build_netcdf
            if [[ $? != 0 ]] ; then
                error "Unable to build or install NetCDF.  Bailing out."
            fi
            info "Done building NetCDF"
        fi
    fi
}
