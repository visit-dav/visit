function bv_vtk_initialize
{
    export DO_VTK="yes"
    export ON_VTK="on"
    export FORCE_VTK="no"
    export USE_SYSTEM_VTK="no"
    add_extra_commandline_args "vtk" "alt-vtk-dir" 1 "Use alternate VTK (exp)"
}

function bv_vtk_enable
{
DO_VTK="yes"
ON_VTK="on"
FORCE_VTK="yes"
}

function bv_vtk_disable
{
DO_VTK="no"
ON_VTK="off"
FORCE_VTK="no"
}

function bv_vtk_alt_vtk_dir
{
    bv_vtk_enable
    USE_SYSTEM_VTK="yes"
    SYSTEM_VTK_DIR="$1"
    info "Using Alternate VTK: $SYSTEM_VTK_DIR"
}

function bv_vtk_depends_on
{
 depends_on="cmake python"

 if [[ "$DO_MESA" == "yes" ]]; then
      depends_on="${depends_on} mesa"
 fi

 if [[ "$DO_R" == "yes" ]]; then
      depends_on="${depends_on} R"
 fi

 echo ${depends_on}
}

function bv_vtk_force
{
  if [[ "$FORCE_VTK" == "yes" ]]; then
     return 0;
  fi
  return 1;
}

function bv_vtk_info
{
export VTK_FILE=${VTK_FILE:-"visit-vtk-5.8.0.a.tar.gz"}
export VTK_VERSION=${VTK_VERSION:-"5.8.0.a"}
export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"visit-vtk-5.8.0.a"}
export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
export VTK_MD5_CHECKSUM="6964a8d1e4e50d3a4a2d4fb39b900b05"
export VTK_SHA256_CHECKSUM=""
}

function bv_vtk_print
{
printf "%s%s\n" "VTK_FILE=" "${VTK_FILE}"
printf "%s%s\n" "VTK_VERSION=" "${VTK_VERSION}"
printf "%s%s\n" "VTK_BUILD_DIR=" "${VTK_BUILD_DIR}"
}

function bv_vtk_print_usage
{
printf "\t\t%15s\n" "NOTE: not available for download from web"
printf "%-15s %s [%s]\n" "--vtk" "Build VTK" "built by default unless --no-thirdparty flag is used"
}

function bv_vtk_host_profile
{
    echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## VTK" >> $HOSTCONF
    echo "##" >> $HOSTCONF

    if [[ "$USE_SYSTEM_VTK" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR $SYSTEM_VTK_DIR)" >> $HOSTCONF
    else
        if [[ $DO_MANGLED_LIBRARIES == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_MTK_DIR \${VISITHOME}/${VTK_INSTALL_DIR}/$VTK_VERSION/\${VISITARCH})" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR \${VISITHOME}/${VTK_INSTALL_DIR}/$VTK_VERSION/\${VISITARCH})" >> $HOSTCONF
        fi
    fi
}

function bv_vtk_initialize_vars
{
    #if [[ "$USE_SYSTEM_VTK" == "yes" ]]; then
    #    if [[ "$DO_MESA" == "yes" || "$parallel" == "yes" ]]; then
    #        error "System VTK with Custom MESA is not supported"
    #    fi
    #fi
    info "initalizing vtk vars"
    if [[ $DO_R == "yes" ]]; then
        VTK_INSTALL_DIR="vtk-r"
    fi
    if [[ $DO_MANGLED_LIBRARIES == "yes" ]]; then
        VTK_INSTALL_DIR="mangled-$VTK_INSTALL_DIR"
    fi

    if [[ "$DO_MESA" == "no" ]] ; then
        VTK_VERSION="${VTK_VERSION}.no.mesa"
    fi
}

function bv_vtk_ensure
{
    if [[ "$DO_VTK" == "yes" && "$USE_SYSTEM_VTK" == "no" ]] ; then
        ensure_built_or_ready $VTK_INSTALL_DIR $VTK_VERSION $VTK_BUILD_DIR $VTK_FILE
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}

function bv_vtk_dry_run
{
    if [[ "$DO_VTK" == "yes" ]] ; then
        echo "Dry run option not set for vtk"
    fi
}

# *************************************************************************** #
#                            Function 6, build_vtk                            #
# *************************************************************************** #

function apply_vtk_580a_patch
{
    # As of 11/4/2012 all patches were rolled into 5.8.0.a.
    return 0
}

function apply_vtk_patch
{
    if [[ ${VTK_VERSION} == 5.8.0.a ]] ; then
        apply_vtk_580a_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_vtk
{
    #
    # CMake is the build system for VTK.  Call another script that will build
    # that program.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "VTK: CMake found"
    else
        build_cmake
        if [[ $? != 0 ]] ; then
            warn "Unable to build cmake.  Giving up"
            return 1
        fi
    fi

    #
    # We need python to build the vtk python bindings.
    #
    PYTHON_INSTALL="${VISIT_PYTHON_DIR}/bin"
    if [[ -e ${PYTHON_INSTALL}/python ]] ; then
        info "VTK: Python found"
    else
        if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
            build_python
            if [[ $? != 0 ]] ; then
                warn "Unable to build python. Giving up"
                return 1
            fi
        fi
    fi

    # Extract the sources
    if [[ -d $VTK_BUILD_DIR ]] ; then
        if [[ ! -f $VTK_FILE ]] ; then
            warn "The directory VTK exists, deleting before uncompressing"
            rm -Rf $VTK_BUILD_DIR
            ensure_built_or_ready $VTK_INSTALL_DIR    $VTK_VERSION    $VTK_BUILD_DIR    $VTK_FILE
        fi
    fi
    prepare_build_dir $VTK_BUILD_DIR $VTK_FILE
    untarred_vtk=$?
    if [[ $untarred_vtk == -1 ]] ; then
        warn "Unable to prepare VTK build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching VTK . . ."
    apply_vtk_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_vtk == 1 ]] ; then
            warn "Giving up on VTK build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory which had " \
                 "already been patched ... that is, that the patch is " \
                 "failing harmlessly on a second application."
        fi
    fi

    info "Configuring VTK . . ."
    VTK_PREFIX="VTK"
    if [[ $DO_MANGLED_LIBRARIES == "yes" ]]; then
        mangle_libraries $VTK_BUILD_DIR "mangled_$VTK_BUILD_DIR"
        if [[ $? == 0 ]]; then
            VTK_BUILD_DIR="mangled_$VTK_BUILD_DIR"
            VTK_PREFIX="MTK" #TODO: Change this to look up a variable..
        else
            warn "VTK Mangling failed"
            exit 0
        fi
    fi

    # Make a build directory for an out-of-source build.. Change the
    # VISIT_BUILD_DIR variable to represent the out-of-source build directory.
    VTK_SRC_DIR=$VTK_BUILD_DIR
    VTK_BUILD_DIR="${VTK_SRC_DIR}-build"
    if [[ ! -d $VTK_BUILD_DIR ]] ; then
        echo "Making build directory $VTK_BUILD_DIR"
        mkdir $VTK_BUILD_DIR
    fi

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf ${VTK_BUILD_DIR}/CMakeCache.txt ${VTK_BUILD_DIR}/*/CMakeCache.txt

    #
    # Setup paths and libs for python for the VTK build.
    #

    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "${VISIT_PYTHON_DIR}/lib" != "/usr/lib" ]]; then
            export DYLD_LIBRARY_PATH="${VISIT_PYTHON_DIR}/lib/:$DYLD_LIBRARY_PATH"
        fi
    else
        export LD_LIBRARY_PATH="${VISIT_PYTHON_DIR}/lib/:$LD_LIBRARY_PATH"
    fi

    export VTK_PY_LIBS="-lpthread"
    if [[ "$OPSYS" == "Linux" ]]; then
        export VTK_PY_LIBS="$VTK_PY_LIBS -ldl -lutil -lm"
    fi

    vopts=""
    vopts="${vopts} -DCMAKE_BUILD_TYPE:STRING=Release"
    vopts="${vopts} -D${VTK_PREFIX}_DEBUG_LEAKS:BOOL=OFF"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    vtk_inst_path="${VISITDIR}/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}"
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${vtk_inst_path}"
    vopts="${vopts} -D${VTK_PREFIX}_INSTALL_INCLUDE_DIR:PATH=/include/"
    vopts="${vopts} -D${VTK_PREFIX}_INSTALL_LIB_DIR:PATH=/lib/"
    vopts="${vopts} -DBUILD_TESTING:BOOL=OFF"
    vopts="${vopts} -DBUILD_DOCUMENTATION:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_NETCDF:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_EXODUS:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_TK:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_64BIT_IDS:BOOL=ON"
    vopts="${vopts} -D${VTK_PREFIX}_USE_INFOVIS:BOOL=ON"
    if test "$DO_R" = "yes" ; then
        vopts="${vopts} -D${VTK_PREFIX}_USE_GNU_R:BOOL=ON"
        vopts="${vopts} -DR_COMMAND:PATH=${R_INSTALL_DIR}/bin/R"
        vopts="${vopts} -D${VTK_PREFIX}_R_HOME:PATH=${R_INSTALL_DIR}/lib/R"
        vopts="${vopts} -DR_INCLUDE_DIR:PATH=${R_INSTALL_DIR}/lib/R/include"
        vopts="${vopts} -DR_LIBRARY_BASE:PATH=${R_INSTALL_DIR}/lib/R/lib/libR.${SO_EXT}"
        vopts="${vopts} -DR_LIBRARY_LAPACK:PATH=${R_INSTALL_DIR}/lib/R/lib/libRlapack.${SO_EXT}"
        vopts="${vopts} -DR_LIBRARY_BLAS:PATH=${R_INSTALL_DIR}/lib/R/lib/libRblas.${SO_EXT}"
    else
        vopts="${vopts} -D${VTK_PREFIX}_USE_CHARTS:BOOL=OFF"
    fi
    vopts="${vopts} -D${VTK_PREFIX}_USE_METAIO:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_PARALLEL:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_LEGACY_REMOVE:BOOL=ON"
    vopts="${vopts} -D${VTK_PREFIX}_USE_SYSTEM_JPEG:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_SYSTEM_PNG:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_SYSTEM_TIFF:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_SYSTEM_ZLIB:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_SYSTEM_HDF5:BOOL=ON"
    vopts="${vopts} -DHDF5_C_INCLUDE_DIR:PATH=${VISIT_HDF5_DIR}/include"
    vopts="${vopts} -DHDF5_INCLUDE_DIR:PATH=${VISIT_HDF5_DIR}/include"
    h5diff="${VISIT_HDF5_DIR}/bin/hdf5diff"
    vopts="${vopts} -DHDF5_DIFF_EXECUTABLE:FILEPATH=${h5diff}"
    h5lib="${VISIT_HDF5_DIR}/lib/libhdf5"
    for ext in a so dylib ; do
        if test -f ${h5lib}.${ext} ; then h5lib="${h5lib}.${ext}" ; fi
    done
    h5hl="${VISIT_HDF5_DIR}/lib/libhdf5_hl"
    for ext in a so dylib ; do
        if test -f ${h5hl}.${ext} ; then h5hl="${h5hl}.${ext}" ; fi
    done
    vopts="${vopts} -DHDF5_LIBRARY=${h5lib}"
    vopts="${vopts} -DHDF5_hdf5_LIBRARY:FILEPATH=${h5lib}"
    vopts="${vopts} -DHDF5_hdf5_LIBRARY_RELEASE:FILEPATH=${h5lib}"
    vopts="${vopts} -DHDF5_hdf5_hl_LIBRARY:FILEPATH=${h5hl}"
    vopts="${vopts} -DHDF5_hdf5_hl_LIBRARY_RELEASE:FILEPATH=${h5hl}"
    vopts="${vopts} -D${VTK_PREFIX}_USE_CARBON:BOOL=OFF"
    vopts="${vopts} -D${VTK_PREFIX}_USE_ANSI_STD_LIB:BOOL=ON"

    if test "${OPSYS}" = "Darwin" ; then
        lo="-Wl,-headerpad_max_install_names"
        lo="${lo},-compatibility_version,5.7"
        lo="${lo},-current_version,5.7.0"
        vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lo}"
        vopts="${vopts} -D${VTK_PREFIX}_USE_COCOA:BOOL=ON"
    fi

    lf=""
    if test "${OPSYS}" = "IRIX64" ; then
        CFLAGS="${CFLAGS} -LANG:std -64"
        lf="${lf} -LANG:std -64"
    fi
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=${CFLAGS}"
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=${CXXFLAGS}"
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"

    if [[ "$DO_MESA" == "yes" && "$DO_DBIO_ONLY" != "yes" ]]; then
        # mesa
        mm="${VISITDIR}/mesa/${MESA_VERSION}/${VISITARCH}"
        vopts="${vopts} -D${VTK_PREFIX}_USE_MANGLED_MESA:BOOL=OFF"
        vopts="${vopts} -D${VTK_PREFIX}_OPENGL_HAS_OSMESA:BOOL=ON"
        vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${mm}/include"
        osm_lib="${mm}/lib/libOSMesa"
        mm_lib="${mm}/lib/libMesaGL"
        for ext in a so dylib ; do
            if test -f ${osm_lib}.${ext} ; then osm_lib="${osm_lib}.${ext}" ; fi
            if test -f ${mm_lib}.${ext} ; then mm_lib="${mm_lib}.${ext}" ; fi
        done
        vopts="${vopts} -DOSMESA_LIBRARY:FILEPATH=${osm_lib}"
    fi

    if test "x${DO_DBIO_ONLY}" != "xyes" ; then
        # python... but static libs and python filters are incompatible.
        if test "x${DO_STATIC_BUILD}" != "xyes" ; then
            if [[ "$DO_MANGLED_LIBRARIES" != "yes" ]]; then
                vopts="${vopts} -D${VTK_PREFIX}_WRAP_PYTHON:BOOL=ON"
            else
                vopts="${vopts} -D${VTK_PREFIX}_WRAP_PYTHON:BOOL=OFF"
            fi

            py="${PYTHON_COMMAND}"
            vopts="${vopts} -DPYTHON_EXECUTABLE:FILEPATH=${py}"
            pyinc="${PYTHON_INCLUDE_DIR}"
            vopts="${vopts} -DPYTHON_INCLUDE_DIR:PATH=${pyinc}"
            pylib="${PYTHON_LIBRARY}"
            vopts="${vopts} -DPYTHON_LIBRARY:FILEPATH=${pylib}"
            vopts="${vopts} -DPYTHON_EXTRA_LIBS:STRING=${VTK_PY_LIBS}"
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    cd ${VTK_BUILD_DIR}
    issue_command "${CMAKE_BIN}" \
      ${vopts} \
      ../${VTK_SRC_DIR} || error "VTK configuration failed."

    #
    # Now build VTK.
    #
    info "Building VTK . . . (~20 minutes)"
    env DYLD_LIBRARY_PATH=`pwd`/bin $MAKE $MAKE_OPT_FLAGS || \
      error "VTK did not build correctly.  Giving up."

    info "Installing VTK . . . "
    $MAKE install || error "VTK did not install correctly."

    #
    # If on Darwin, fix install names
    #
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        INSTALLNAMEPATH="$VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/$VISITARCH/lib"

        # fix the internal name with in the libraries
        #
        # first change the libraries name and identification by executing the
        # following bourne shell script
        if [[ "$DO_MANGLED_LIBRARIES" == "yes" ]]; then
            #remove python since mangle vtk libraries does not support python (yet:TODO:Fix this)
            VTK_LIB_NAMES="libMapReduceMPI libmpistubs libmtkCommon libmtkDICOMParser libmtkFiltering libmtkGenericFiltering libmtkGeovis libmtkGraphics libmtkHybrid libmtkInfovis libmtkIO libmtkImaging libmtkRendering libmtkViews libmtkVolumeRendering libmtkWidgets libmtkalglib libmtkexpat libmtkfreetype libmtkftgl libmtkjpeg libmtklibxml2 libmtkpng libmtkproj4 libmtksqlite libmtksys libmtktiff libmtkverdict libmtkzlib"
        else
            VTK_LIB_NAMES="libMapReduceMPI libmpistubs libvtkCommon libvtkCommonPythonD libvtkDICOMParser libvtkFiltering libvtkFilteringPythonD libvtkGenericFiltering libvtkGenericFilteringPythonD libvtkGeovis libvtkGeovisPythonD libvtkGraphics libvtkGraphicsPythonD libvtkHybrid libvtkHybridPythonD libvtkInfovis libvtkInfovisPythonD libvtkIO libvtkIOPythonD libvtkImaging libvtkImagingPythonD libvtkPythonCore libvtkRendering libvtkRenderingPythonD libvtkViews libvtkViewsPythonD libvtkVolumeRendering libvtkVolumeRenderingPythonD libvtkWidgets libvtkWidgetsPythonD libvtkalglib libvtkexpat libvtkfreetype libvtkftgl libvtkjpeg libvtklibxml2 libvtkpng libvtkproj4 libvtksqlite libvtksys libvtktiff libvtkverdict libvtkzlib"
        fi
        for i in $VTK_LIB_NAMES
        do
            install_name_tool -id \
                $INSTALLNAMEPATH/$i.$SO_EXT \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
        done

        #
        # Next change the dependent libraries names and paths
        for i in $VTK_LIB_NAMES
        do
            for j in $VTK_LIB_NAMES
            do
                install_name_tool -change \
                    $j.5.8.$SO_EXT \
                    $INSTALLNAMEPATH/$j.$SO_EXT \
                    $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
            done
        if [[ "$DO_MANGLED_LIBRARIES" == "yes" ]]; then
            install_name_tool -change \
                libmtkNetCDF_cxx.dylib $INSTALLNAMEPATH/libmtkNetCDF_cxx.dylib \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
        else
            install_name_tool -change \
                libvtkNetCDF_cxx.dylib $INSTALLNAMEPATH/libvtkNetCDF_cxx.dylib \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
        fi
        if [[ "$DO_R" == "yes" ]]; then
            install_name_tool -change \
                libR.dylib $VISITDIR/R/${R_VERSION}/${VISITARCH}/lib/R/lib/libR.dylib \
            install_name_tool -change \
                libRblas.dylib $VISITDIR/R/${R_VERSION}/${VISITARCH}/lib/R/lib/libRblas.dylib \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
            install_name_tool -change \
                libRlapack.dylib $VISITDIR/R/${R_VERSION}/${VISITARCH}/lib/R/lib/libRlapack.dylib \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
        fi
        done

        if [[ "$DO_MANGLED_LIBRARIES" == "yes" ]]; then
            VTK_PYTHON_MOD_LIB_NAMES="mtkCommonPython.so mtkFilteringPython.so mtkGenericFilteringPython.so mtkGeovisPython.so mtkGraphicsPython.so mtkHybridPython.so mtkInfovisPython.so mtkIOPython.so mtkImagingPython.so mtkRenderingPython.so mtkViewsPython.so mtkVolumeRenderingPython.so"
        else
            VTK_PYTHON_MOD_LIB_NAMES="vtkCommonPython.so vtkFilteringPython.so vtkGenericFilteringPython.so vtkGeovisPython.so vtkGraphicsPython.so vtkHybridPython.so vtkInfovisPython.so vtkIOPython.so vtkImagingPython.so vtkRenderingPython.so vtkViewsPython.so vtkVolumeRenderingPython.so"
        fi
        #
        # Fix vtk python wrapper module intall names.
        #
        for i in $VTK_PYTHON_MOD_LIB_NAMES
        do
            install_name_tool -id \
                $INSTALLNAMEPATH/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/${VTK_INSTALL_DIR}/$i \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/${VTK_INSTALL_DIR}/$i
        done

        #
        # The vtk python module libs depend on the main vtk libs,
        # resolve these install names.
        #

        # The vtk python libs have install names that point to an abs path
        # below VTK_BUILD_DIR.
        # We should be in ${VTK_BUILD_DIR}, we just need its abs path
        VTK_BUILD_DIR_ABS=`pwd`
        info "VTK build directory absolute path: $VTK_BUILD_DIR_ABS"

        for i in $VTK_PYTHON_MOD_LIB_NAMES
        do
            for j in $VTK_LIB_NAMES
            do
                install_name_tool -change \
                    $VTK_BUILD_DIR_ABS/bin/$j.5.8.$SO_EXT \
                    $INSTALLNAMEPATH/$j.$SO_EXT \
                    $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/${VTK_INSTALL_DIR}/$i
            done
        done
    fi

    chmod -R ug+w,a+rX ${VISITDIR}/${VTK_INSTALL_DIR}
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chgrp -R ${GROUP} "$VISITDIR/${VTK_INSTALL_DIR}"
    fi
    cd "$START_DIR"
    info "Done with VTK"
    return 0
}

function bv_vtk_is_enabled
{
    if [[ $DO_VTK == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_vtk_is_installed
{
    if [[ "$USE_SYSTEM_VTK" == "yes" ]]; then
        return 1
    fi

    check_if_installed "$VTK_INSTALL_DIR" $VTK_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_vtk_build
{
    #
    # Build VTK
    #
    cd "$START_DIR"
    if [[ "$DO_VTK" == "yes" && "$USE_SYSTEM_VTK" == "no" ]] ; then
        check_if_installed $VTK_INSTALL_DIR $VTK_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping VTK build.  VTK is already installed."
        else
            info "Building VTK (~20 minutes)"
            build_vtk
            if [[ $? != 0 ]] ; then
                error "Unable to build or install VTK.  Bailing out."
            fi
        fi
        info "Done building VTK"
    fi
}

