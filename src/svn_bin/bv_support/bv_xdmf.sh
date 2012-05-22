function bv_xdmf_initialize
{
export DO_XDMF="no"
export ON_XDMF="off"
}

function bv_xdmf_enable
{
DO_XDMF="yes"
ON_XDMF="on"

#xdmf is dependent on HDF5
DO_HDF5="yes"
ON_HDF5="on"
}

function bv_xdmf_disable
{
DO_XDMF="no"
ON_XDMF="off"
}

function bv_xdmf_depends_on
{
echo "cmake vtk hdf5"
}

function bv_xdmf_info
{
export XDMF_FILE=${XDMF_FILE:-"Xdmf-2.1.1.tar.gz"}
export XDMF_VERSION=${XDMF_VERSION:-"2.1.1"}
export XDMF_COMPATIBILITY_VERSION=${XDMF_COMPATIBILITY_VERSION:-"2.1.1"}
export XDMF_BUILD_DIR=${XDMF_BUILD_DIR:-"Xdmf"}
export XDMF_MD5_CHECKSUM="09e2afd3a1b7b3e7d650b860212a95d1"
export XDMF_SHA256_CHECKSUM=""
}

function bv_xdmf_print
{
  printf "%s%s\n" "XDMF_FILE=" "${XDMF_FILE}"
  printf "%s%s\n" "XDMF_VERSION=" "${XDMF_VERSION}"
  printf "%s%s\n" "XDMF_COMPATIBILITY_VERSION=" "${XDMF_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "XDMF_BUILD_DIR=" "${XDMF_BUILD_DIR}"
}

function bv_xdmf_print_usage
{
printf "\t\t%15s\n" "NOTE: not available for download from web" 
printf "%-15s %s [%s]\n" "--xdmf" "Build Xdmf" "$DO_XDMF"
}

function bv_xdmf_graphical
{
local graphical_out="Xdmf     $XDMF_VERSION($XDMF_FILE)    $ON_XDMF"
echo "$graphical_out"
}

function bv_xdmf_host_profile
{
    if [[ "$DO_XDMF" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Xdmf" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR \${VISITHOME}/Xdmf/$XDMF_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP}  VTK_LIBRARY_DIRS vtklibxml2 ${VISIT_VTK_LIBDEP} TYPE STRING)"\
        >> $HOSTCONF
    fi
}

function bv_xdmf_ensure
{
    if [[ "$DO_XDMF" == "yes" ]] ; then
        ensure_built_or_ready "Xdmf" $XDMF_VERSION $XDMF_BUILD_DIR $XDMF_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XDMF="no"
            error "Unable to build XDMF.  ${XDMF_FILE} not found."
        fi
    fi
}

function bv_xdmf_dry_run
{
  if [[ "$DO_XDMF" == "yes" ]] ; then
    echo "Dry run option not set for xdmf."
  fi
}


# *************************************************************************** #
#                         Function 8.19, build_xdmf                           #
# *************************************************************************** #

function build_xdmf
{
    CMAKE_BIN="${CMAKE_COMMAND}"

    #
    # Prepare build dir
    #
    prepare_build_dir $XDMF_BUILD_DIR $XDMF_FILE
    untarred_XDMF=$?
    if [[ $untarred_XDMF == -1 ]] ; then
       warn "Unable to prepare Xdmf Build Directory. Giving up"
       return 1
    fi

    # We need to patch the CMakeLists.txt so it uses our HDF5 by default
    # when we tell it to use the system versions.
    cd $XDMF_BUILD_DIR || error "Can't cd to Xdmf build dir."
    rm -f CMakeCache.txt #remove any CMakeCache that may have existed 
    # Configure Xdmf
    info "Executing CMake on Xdmf"
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        XDMF_SHARED_LIBS="OFF"
        LIBEXT="a"
    else
        XDMF_SHARED_LIBS="ON"
        LIBEXT="${SO_EXT}"
    fi

    ${CMAKE_BIN} -DCMAKE_INSTALL_PREFIX:PATH="$VISITDIR/Xdmf/${XDMF_VERSION}/${VISITARCH}"\
        -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON \
        -DBUILD_SHARED_LIBS:BOOL=${XDMF_SHARED_LIBS}\
        -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}"\
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}\
        -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}"\
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER}\
        -DBUILD_TESTING:BOOL=OFF \
        -DXDMF_BUILD_MPI:BOOL=OFF \
        -DXDMF_BUILD_VTK:BOOL=OFF \
        -DXDMF_BUILD_UTILS:BOOL=OFF \
        -DXDMF_SYSTEM_HDF5:BOOL=ON \
        -DHDF5_INCLUDE_PATH:PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/include" \
        -DHDF5_LIBRARY:PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib/libhdf5.${SO_EXT}" \
        -DXDMF_SYSTEM_ZLIB:BOOL=ON \
        -DZLIB_INCLUDE_PATH:PATH="$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/include/vtklibz" \
        -DZLIB_LIBRARY:PATH="$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib/libvtkzlib.${SO_EXT}" \
        -DXDMF_SYSTEM_LIBXML2:BOOL=ON \
        -DLIBXML2_INCLUDE_PATH:PATH="$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/include/vtklibxml2" \
        -DLIBXML2_LIBRARY="$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib/libvtklibxml2.${SO_EXT}" \
        .

    if [[ $? != 0 ]] ; then
       warn "Xdmf configure failed.  Giving up"
       return 1
    fi

    #
    # Build Xdmf
    #
    info "Building Xdmf . . . (~3 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
       warn "Xdmf build failed.  Giving up"
       return 1
    fi

    # Install Xdmf
    info "Installing Xdmf"
    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "Xdmf install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" != "yes" && "$OPSYS" == "Darwin" ]]; then
        LIBDIR="$VISITDIR/Xdmf/${XDMF_VERSION}/${VISITARCH}/lib"
        install_name_tool -id $LIBDIR/libXdmf.dylib $LIBDIR/libXdmf.dylib
    fi


    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/Xdmf"
       chgrp -R ${GROUP} "$VISITDIR/Xdmf"
    fi

    cd "$START_DIR"
    info "Done with Xdmf"
    return 0
}

function bv_xdmf_is_enabled
{
    if [[ $DO_XDMF == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_xdmf_is_installed
{
    check_if_installed "Xdmf" $XDMF_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_xdmf_build
{
cd "$START_DIR"
if [[ "$DO_XDMF" == "yes" ]] ; then
    check_if_installed "Xdmf" $XDMF_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Xdmf build.  Xdmf is already installed."
    else
        info "Building Xdmf (~2 minutes)"
        build_xdmf
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Xdmf.  Bailing out."
        fi
        info "Done building Xdmf"
   fi
fi
}
