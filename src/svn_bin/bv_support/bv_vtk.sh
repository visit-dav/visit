function bv_vtk_initialize
{
export DO_VTK="no"
export ON_VTK="off"
}

function bv_vtk_enable
{
DO_VTK="yes"
ON_VTK="on"
}

function bv_vtk_disable
{
DO_VTK="no"
ON_VTK="off"
}

function bv_vtk_depends_on
{
return ""
}

function bv_vtk_info
{
export VTK_FILE=${VTK_FILE:-"visit-vtk-5.8.tar.gz"}
export VTK_VERSION=${VTK_VERSION:-"5.8.0"}
export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"visit-vtk-5.8"}
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
echo \
"## Specify the location of the vtk." >> $HOSTCONF
echo "##" >> $HOSTCONF
echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR \${VISITHOME}/vtk/$VTK_VERSION/\${VISITARCH})" >> $HOSTCONF
echo >> $HOSTCONF
    
}

function bv_vtk_ensure
{
    if [[ "$DO_VTK" == "yes" ]] ; then
        ensure_built_or_ready "vtk" $VTK_VERSION $VTK_BUILD_DIR $VTK_FILE
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
function issue_command
{
    echo $@
    $@
    return $?
}

function apply_vtk_580_patch_1
{
   patch -p0 <<\EOF
diff -c a/IO/CMakeLists.txt visit-vtk-5.8/IO/CMakeLists.txt
*** a/IO/CMakeLists.txt
--- visit-vtk-5.8/IO/CMakeLists.txt
***************
*** 92,98 ****
  vtkMoleculeReaderBase.cxx
  vtkOBJReader.cxx
  ${_VTK_OGGTHEORA_SOURCES}
- vtkOpenFOAMReader.cxx
  vtkOutputStream.cxx
  vtkPDBReader.cxx
  vtkPLOT3DReader.cxx
--- 92,97 ----
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 1 to VTK 5.8.0"
        return 1
   else
        return 0
   fi
}

function apply_vtk_580_patch
{
   apply_vtk_580_patch_1
   if [[ $? != 0 ]] ; then
       return 1
   fi
}

function apply_vtk_patch
{
   if [[ ${VTK_VERSION} == 5.8.0 ]] ; then
       apply_vtk_580_patch
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

    #
    # We need Mesa to build vtk with mangled Mesa.
    #
    MESA_INSTALL="$VISITDIR/mesa/${MESA_VERSION}/$VISITARCH/lib"
    if [[ -e ${MESA_INSTALL} ]] ; then
       info "VTK: Mesa found"
    else
       if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
          build_mesa
          if [[ $? != 0 ]] ; then
             warn "Unable to build Mesa. Giving up"
             return 1
          fi
       fi
    fi

    # Extract the sources
    if [[ -d $VTK_BUILD_DIR ]] ; then
        if [[ ! -f $VTK_FILE ]] ; then
            warn "The directory VTK exists, deleting before uncompressing"
            rm -Rf $VTK_BUILD_DIR
            ensure_built_or_ready "vtk"    $VTK_VERSION    $VTK_BUILD_DIR    $VTK_FILE
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
    vopts="${vopts} -DVTK_DEBUG_LEAKS:BOOL=OFF"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
      vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
      vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    vtk_inst_path="${VISITDIR}/vtk/${VTK_VERSION}/${VISITARCH}"
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${vtk_inst_path}"
    vopts="${vopts} -DVTK_INSTALL_INCLUDE_DIR:PATH=/include/"
    vopts="${vopts} -DVTK_INSTALL_LIB_DIR:PATH=/lib/"
    vopts="${vopts} -DBUILD_TESTING:BOOL=OFF"
    vopts="${vopts} -DBUILD_DOCUMENTATION:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_NETCDF:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_EXODUS:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_TK:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_64BIT_IDS:BOOL=ON"
    vopts="${vopts} -DVTK_USE_INFOVIS:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_METAIO:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_PARALLEL:BOOL=OFF"
    vopts="${vopts} -DVTK_LEGACY_REMOVE:BOOL=ON"
    vopts="${vopts} -DVTK_USE_SYSTEM_JPEG:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_SYSTEM_PNG:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_SYSTEM_TIFF:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_SYSTEM_ZLIB:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_SYSTEM_HDF5:BOOL=ON"
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
    vopts="${vopts} -DVTK_USE_CARBON:BOOL=OFF"
    vopts="${vopts} -DVTK_USE_ANSI_STD_LIB:BOOL=ON"

    if test "${OPSYS}" = "Darwin" ; then
      lo="-Wl,-headerpad_max_install_names"
      lo="${lo},-compatibility_version,5.7"
      lo="${lo},-current_version,5.7.0"
      vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lo}"
      vopts="${vopts} -DVTK_USE_COCOA:BOOL=ON"
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

    if test "x${DO_DBIO_ONLY}" != "xyes" ; then
      # mesa
      mm="${VISITDIR}/mesa/${MESA_VERSION}/${VISITARCH}"
      vopts="${vopts} -DVTK_USE_MANGLED_MESA:BOOL=OFF"
      vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
      vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${mm}/include"
      osm_lib="${mm}/lib/libOSMesa"
      mm_lib="${mm}/lib/libMesaGL"
      for ext in a so dylib ; do
        if test -f ${osm_lib}.${ext} ; then osm_lib="${osm_lib}.${ext}" ; fi
        if test -f ${mm_lib}.${ext} ; then mm_lib="${mm_lib}.${ext}" ; fi
      done
      vopts="${vopts} -DOSMESA_LIBRARY:FILEPATH=${osm_lib}"

      # python... but static libs and python filters are incompatible.
      if test "x${DO_STATIC_BUILD}" != "xyes" ; then
        vopts="${vopts} -DVTK_WRAP_PYTHON:BOOL=ON"
        py="${VISIT_PYTHON_DIR}/bin/python"
        pycompat="${PYTHON_COMPATIBILITY_VERSION}"
        vopts="${vopts} -DPYTHON_EXECUTABLE:FILEPATH=${py}"
        pyinc="${VISIT_PYTHON_DIR}/include/python${pycompat}"
        vopts="${vopts} -DPYTHON_INCLUDE_DIR:PATH=${pyinc}"
        pylib="${VISIT_PYTHON_DIR}/lib/libpython${pycompat}"
        for ext in so a dylib ; do
          if test -f ${pylib}.${ext} ; then pylib="${pylib}.${ext}" ; fi
        done
        vopts="${vopts} -DPYTHON_LIBRARY:FILEPATH=${pylib}"
        vopts="${vopts} -DPYTHON_EXTRA_LIBS:STRING=${VTK_PY_LIBS}"
      else
        warn "Forgetting python filters because we are doing a static build."
      fi
    fi

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    cd ${VTK_BUILD_DIR}
    issue_command ${CMAKE_BIN} \
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
        INSTALLNAMEPATH="$VISITDIR/vtk/${VTK_VERSION}/$VISITARCH/lib"

        # fix the internal name with in the libraries
        #
        # first change the libraries name and identification by executing the
        # following bourne shell script
        VTK_LIB_NAMES="libMapReduceMPI libmpistubs libvtkCommon libvtkCommonPythonD libvtkDICOMParser libvtkFiltering libvtkFilteringPythonD libvtkGenericFiltering libvtkGenericFilteringPythonD libvtkGraphics libvtkGraphicsPythonD libvtkHybrid libvtkHybridPythonD libvtkIO libvtkIOPythonD libvtkImaging libvtkImagingPythonD libvtkPythonCore libvtkRendering libvtkRenderingPythonD libvtkVolumeRendering libvtkVolumeRenderingPythonD libvtkWidgets libvtkWidgetsPythonD libvtkalglib libvtkexpat libvtkfreetype libvtkftgl libvtkjpeg libvtklibxml2 libvtkpng libvtkproj4 libvtksqlite libvtksys libvtktiff libvtkverdict libvtkzlib"
        for i in $VTK_LIB_NAMES
        do
           install_name_tool -id \
             $INSTALLNAMEPATH/$i.$SO_EXT \
             $VISITDIR/vtk/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
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
                $VISITDIR/vtk/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
          done
          install_name_tool -change \
                libvtkNetCDF_cxx.dylib $INSTALLNAMEPATH/libvtkNetCDF_cxx.dylib \
                $VISITDIR/vtk/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
        done

        VTK_PYTHON_MOD_LIB_NAMES="vtkCommonPython.so vtkFilteringPython.so vtkGenericFilteringPython.so vtkGraphicsPython.so vtkHybridPython.so vtkIOPython.so vtkImagingPython.so vtkRenderingPython.so vtkVolumeRenderingPython.so"
        #
        # Fix vtk python wrapper module intall names.
        #
        for i in $VTK_PYTHON_MOD_LIB_NAMES
        do
            install_name_tool -id \
             $INSTALLNAMEPATH/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/vtk/$i \
             $VISITDIR/vtk/${VTK_VERSION}/${VISITARCH}/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/vtk/$i
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
                $VISITDIR/vtk/${VTK_VERSION}/${VISITARCH}/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/vtk/$i
          done
        done
    fi

    chmod -R ug+w,a+rX ${VISITDIR}/vtk
    if [[ "$DO_GROUP" == "yes" ]] ; then
       chgrp -R ${GROUP} "$VISITDIR/vtk"
    fi
    cd "$START_DIR"
    info "Done with VTK"
    return 0
}

function bv_vtk_build
{
#
# Build VTK
#
cd "$START_DIR"
if [[ "$DO_VTK" == "yes" ]] ; then
    check_if_installed "vtk" $VTK_VERSION
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

