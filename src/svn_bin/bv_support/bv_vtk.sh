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
 depends_on="cmake"

 if [[ "$DO_PYTHON" == "yes" ]]; then
      depends_on="${depends_on} python"
 fi

 if [[ "$DO_R" == "yes" ]]; then
      depends_on="${depends_on} R"
 fi

 # Only depend on Qt if we're not doing server-only builds.
 if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
     if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
         if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then 
             depends_on="${depends_on} qt"
         fi
     fi
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
export VTK_FILE=${VTK_FILE:-"vtk-6.0.0.rc3.tar.gz"}  ##vtk-6.0.0.tar.gz"}
export VTK_VERSION=${VTK_VERSION:-"6.0.0"}
export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"6.0"}
export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK6.0.0.rc3"} #vtk-6.0.0"}
export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
export VTK_MD5_CHECKSUM=""
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
    info "initalizing vtk vars"
    if [[ $DO_R == "yes" ]]; then
        VTK_INSTALL_DIR="vtk-r"
    fi
    if [[ $DO_MANGLED_LIBRARIES == "yes" ]]; then
        VTK_INSTALL_DIR="mangled-$VTK_INSTALL_DIR"
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

function apply_vtk_600_patch
{
    return 0
}

function apply_vtk_patch
{
    if [[ ${VTK_VERSION} == 6.0.0 ]] ; then
        apply_vtk_600_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function bv_vtk_fixup
{
    #
    # If on Darwin, fix install names
    #
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        INSTALLNAMEPATH="$VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/$VISITARCH/lib"

        # The list of VTK libraries that our build produces.
        #
        cat >> vtklibnames.txt <<EOF
libvtkCommonComputationalGeometry
libvtkCommonComputationalGeometryPython27D
libvtkCommonCore
libvtkCommonCorePython27D
libvtkCommonDataModel
libvtkCommonDataModelPython27D
libvtkCommonExecutionModel
libvtkCommonExecutionModelPython27D
libvtkCommonMath
libvtkCommonMathPython27D
libvtkCommonMisc
libvtkCommonMiscPython27D
libvtkCommonSystem
libvtkCommonSystemPython27D
libvtkCommonTransforms
libvtkCommonTransformsPython27D
libvtkDICOMParser
libvtkFiltersCore
libvtkFiltersCorePython27D
libvtkFiltersExtraction
libvtkFiltersExtractionPython27D
libvtkFiltersFlowPaths
libvtkFiltersFlowPathsPython27D
libvtkFiltersGeneral
libvtkFiltersGeneralPython27D
libvtkFiltersGeometry
libvtkFiltersGeometryPython27D
libvtkFiltersHybrid
libvtkFiltersHybridPython27D
libvtkFiltersModeling
libvtkFiltersModelingPython27D
libvtkFiltersSources
libvtkFiltersSourcesPython27D
libvtkFiltersStatistics
libvtkFiltersStatisticsPython27D
libvtkGUISupportQt
libvtkGUISupportQtOpenGL
libvtkGeovisCore
libvtkGeovisCorePython27D
libvtkIOCore
libvtkIOCorePython27D
libvtkIOEnSight
libvtkIOEnSightPython27D
libvtkIOGeometry
libvtkIOGeometryPython27D
libvtkIOImage
libvtkIOImagePython27D
libvtkIOLegacy
libvtkIOLegacyPython27D
libvtkIOPLY
libvtkIOPLYPython27D
libvtkIOXML
libvtkIOXMLParser
libvtkIOXMLParserPython27D
libvtkIOXMLPython27D
libvtkImagingColor
libvtkImagingColorPython27D
libvtkImagingCore
libvtkImagingCorePython27D
libvtkImagingFourier
libvtkImagingFourierPython27D
libvtkImagingGeneral
libvtkImagingGeneralPython27D
libvtkImagingHybrid
libvtkImagingHybridPython27D
libvtkImagingSources
libvtkImagingSourcesPython27D
libvtkInfovisCore
libvtkInfovisCorePython27D
libvtkInfovisLayout
libvtkInfovisLayoutPython27D
libvtkInteractionStyle
libvtkInteractionStylePython27D
libvtkInteractionWidgets
libvtkInteractionWidgetsPython27D
libvtkRenderingAnnotation
libvtkRenderingAnnotationPython27D
libvtkRenderingCore
libvtkRenderingCorePython27D
libvtkRenderingFreeType
libvtkRenderingFreeTypeOpenGL
libvtkRenderingFreeTypeOpenGLPython27D
libvtkRenderingFreeTypePython27D
libvtkRenderingOpenGL
libvtkRenderingOpenGLPython27D
libvtkRenderingVolume
libvtkRenderingVolumePython27D
libvtkViewsCore
libvtkViewsCorePython27D
libvtkWrappingPython27Core
libvtkalglib
libvtkexpat
libvtkfreetype
libvtkftgl
libvtkjpeg
libvtkjsoncpp
libvtklibxml2
libvtkmetaio
libvtkpng
libvtkproj4
libvtksys
libvtktiff
libvtkzlib
EOF

        # first change the libraries name and identification by executing the
        # following bourne shell script
        if [[ "$DO_MANGLED_LIBRARIES" == "yes" ]]; then
            #remove python since mangle vtk libraries does not support python (yet:TODO:Fix this)
            # Change "vtk" to "mtk"
            sed "/s/libvtk/libmtk/g" vtklibnames.txt > mtklibnames.txt
            VTK_LIB_NAMES="`cat mtklibnames.txt`"
            rm -f mtklibnames.txt
        else
            VTK_LIB_NAMES="`cat vtklibnames.txt`"
        fi
        rm -f vtklibnames.txt
        for i in $VTK_LIB_NAMES
        do
            install_name_tool -id \
                $INSTALLNAMEPATH/${i}-${VTK_COMPATIBILITY_VERSION}.$SO_EXT \
                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/${i}-${VTK_COMPATIBILITY_VERSION}.$SO_EXT
        done

        #
        # Next change the dependent libraries names and paths
        for i in $VTK_LIB_NAMES
        do
            for j in $VTK_LIB_NAMES
            do
                install_name_tool -change \
                    $j-${VTK_COMPATIBILITY_VERSION}.1.$SO_EXT \
                    $INSTALLNAMEPATH/$j-${VTK_COMPATIBILITY_VERSION}.$SO_EXT \
                    $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/${i}-${VTK_COMPATIBILITY_VERSION}.$SO_EXT
            done

#        if [[ "$DO_MANGLED_LIBRARIES" == "yes" ]]; then
#            install_name_tool -change \
#                libmtkNetCDF_cxx.dylib $INSTALLNAMEPATH/libmtkNetCDF_cxx.dylib \
#                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
#        else
#            install_name_tool -change \
#                libvtkNetCDF_cxx.dylib $INSTALLNAMEPATH/libvtkNetCDF_cxx.dylib \
#                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
#        fi
#        if [[ "$DO_R" == "yes" ]]; then
#            install_name_tool -change \
#                libR.dylib $VISITDIR/R/${R_VERSION}/${VISITARCH}/lib/R/lib/libR.dylib \
#            install_name_tool -change \
#                libRblas.dylib $VISITDIR/R/${R_VERSION}/${VISITARCH}/lib/R/lib/libRblas.dylib \
#                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
#            install_name_tool -change \
#                libRlapack.dylib $VISITDIR/R/${R_VERSION}/${VISITARCH}/lib/R/lib/libRlapack.dylib \
#                $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/$i.$SO_EXT
#        fi
        done

        # Come up with the names of the Python modules
        VTK_PYTHON_MOD_LIB_NAMES=""
        for i in $VTK_LIB_NAMES
        do
            py=`echo $i | grep Python`
            if [[ "x${i}" == "x${py}" ]]; then
                for f in 26D 27D 30D 31D 32D 33D 26 27 30 31 32 33 
                do
                    py=`echo $py | sed "s/$f//g"`
                done

                VTK_PYTHON_MOD_LIB_NAMES="$VTK_PYTHON_MOD_LIB_NAMES $py.so"
            fi
        done

        #
        # Fix vtk python wrapper module install names.
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
                    $VTK_BUILD_DIR_ABS/bin/$j-${VTK_COMPATIBILITY_VERSION}.1.$SO_EXT \
                    $INSTALLNAMEPATH/$j-${VTK_COMPATIBILITY_VERSION}.$SO_EXT \
                    $VISITDIR/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/python${PYTHON_COMPATIBILITY_VERSION}/site-packages/${VTK_INSTALL_DIR}/$i
            done
        done
    fi
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
    if [[ "$DO_PYTHON" == "yes" ]] ; then
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
    fi

    #
    # We need Qt to build the vtk Qt support. Only do Qt if we're not doing
    # a server-only build.
    #
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then
                if [[ -e ${QT_BIN_DIR}/qmake ]] ; then
                    info "VTK: Qt found"
                else
                    build_qt
                    if [[ $? != 0 ]] ; then
                        warn "Unable to build Qt. Giving up"
                        return 1
                    fi
                fi
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
    vtk_build_mode="${VISIT_BUILD_MODE}"
    vtk_inst_path="${VISITDIR}/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}"
    vtk_debug_leaks="false"

    # Some linker flags.
    lf=""
    if test "${OPSYS}" = "Darwin" ; then
        lf="-Wl,-headerpad_max_install_names"
        lf="${lf},-compatibility_version,${VTK_COMPATIBILITY_VERSION}"
        lf="${lf},-current_version,${VTK_VERSION}"
    fi

    # Add some extra arguments to the VTK cmake command line via the 
    # VTK_EXTRA_OPTIONS environment variable.
    if test -n "$VTK_EXTRA_OPTIONS" ; then
        vopts="${vopts} $VTK_EXTRA_OPTIONS"
    fi

    # normal stuff
    vopts="${vopts} -DCMAKE_BUILD_TYPE:STRING=${vtk_build_mode}"
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${vtk_inst_path}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    vopts="${vopts} -D${VTK_PREFIX}_DEBUG_LEAKS:BOOL=${vtk_debug_leaks}"
    vopts="${vopts} -DBUILD_TESTING:BOOL=false"
    vopts="${vopts} -DBUILD_DOCUMENTATION:BOOL=false"
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"
    if test "${OPSYS}" = "Darwin" ; then
        vopts="${vopts} -D${VTK_PREFIX}_USE_COCOA:BOOL=ON"
    fi

    # allow VisIt to override any of vtk's classes
    vopts="${vopts} -D${VTK_PREFIX}_ALL_NEW_OBJECT_FACTORY:BOOL=true"

    # Turn off module groups
    vopts="${vopts} -D${VTK_PREFIX}_Group_Imaging:BOOL=false"
    vopts="${vopts} -D${VTK_PREFIX}_Group_MPI:BOOL=false"
    vopts="${vopts} -D${VTK_PREFIX}_Group_Qt:BOOL=false"
    vopts="${vopts} -D${VTK_PREFIX}_Group_Rendering:BOOL=false"
    vopts="${vopts} -D${VTK_PREFIX}_Group_StandAlone:BOOL=false"
    vopts="${vopts} -D${VTK_PREFIX}_Group_Tk:BOOL=false"
    vopts="${vopts} -D${VTK_PREFIX}_Group_Views:BOOL=false"

    # Turn on individual modules. dependent modules are turned on automatically
    vopts="${vopts} -DModule_vtkCommonCore:BOOL=true"
    vopts="${vopts} -DModule_vtkFiltersFlowPaths:BOOL=true"
    vopts="${vopts} -DModule_vtkFiltersHybrid:BOOL=true"
    vopts="${vopts} -DModule_vtkFiltersModeling:BOOL=true"
    vopts="${vopts} -DModule_vtkGeovisCore:BOOL=true"
    vopts="${vopts} -DModule_vtkIOEnSight:BOOL=true"
    vopts="${vopts} -DModule_vtkIOGeometry:BOOL=true"
    vopts="${vopts} -DModule_vtkIOLegacy:BOOL=true"
    vopts="${vopts} -DModule_vtkIOPLY:BOOL=true"
    vopts="${vopts} -DModule_vtkIOXML:BOOL=true"
    vopts="${vopts} -DModule_vtkInteractionStyle:BOOL=true"
    vopts="${vopts} -DModule_vtkRenderingAnnotation:BOOL=true"
    vopts="${vopts} -DModule_vtkRenderingFreeType:BOOL=true"
    vopts="${vopts} -DModule_vtkRenderingFreeTypeOpenGL:BOOL=true"
    vopts="${vopts} -DModule_vtkRenderingOpenGL:BOOL=true"
    vopts="${vopts} -DModule_vtklibxml2:BOOL=true"

    # Tell VTK where to locate qmake if we're building graphical support. We
    # do not add graphical support for server-only builds.
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then
                vopts="${vopts} -DModule_vtkGUISupportQtOpenGL:BOOL=true"
                vopts="${vopts} -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_BIN_DIR}/qmake"
            fi
        fi
    fi

    # Add python wrapping
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        # python... but static libs and python filters are incompatible.
        if [[ "$DO_STATIC_BUILD" != "yes" ]]; then
            py="${PYTHON_COMMAND}"
            pyinc="${PYTHON_INCLUDE_DIR}"
            pylib="${PYTHON_LIBRARY}"

            vopts="${vopts} -D${VTK_PREFIX}_WRAP_PYTHON:BOOL=true"
            vopts="${vopts} -DPYTHON_EXECUTABLE:FILEPATH=${py}"
            vopts="${vopts} -DPYTHON_EXTRA_LIBS:STRING=${VTK_PY_LIBS}"
            vopts="${vopts} -DPYTHON_INCLUDE_DIR:PATH=${pyinc}"
            vopts="${vopts} -DPYTHON_LIBRARY:FILEPATH=${pylib}"
#            vopts="${vopts} -DPYTHON_UTIL_LIBRARY:FILEPATH="
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi

    # Add R support
    if test "$DO_R" = "yes" ; then
        vopts="${vopts} -DModule_vtkFiltersStatisticsGnuR:BOOL=true"
        vopts="${vopts} -DR_COMMAND:PATH=${R_INSTALL_DIR}/bin/R"
        vopts="${vopts} -D${VTK_PREFIX}_R_HOME:PATH=${R_INSTALL_DIR}/lib/R"
        vopts="${vopts} -DR_INCLUDE_DIR:PATH=${R_INSTALL_DIR}/lib/R/include"
        vopts="${vopts} -DR_LIBRARY_BASE:PATH=${R_INSTALL_DIR}/lib/R/lib/libR.${SO_EXT}"
        vopts="${vopts} -DR_LIBRARY_LAPACK:PATH=${R_INSTALL_DIR}/lib/R/lib/libRlapack.${SO_EXT}"
        vopts="${vopts} -DR_LIBRARY_BLAS:PATH=${R_INSTALL_DIR}/lib/R/lib/libRblas.${SO_EXT}"
    fi

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    cd ${VTK_BUILD_DIR}

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command".  This was first discovered on 
    # BGQ and then showed up in random cases for both OSX and Linux machines. 
    # Brad resolved this on BGQ  with a simple work around - we write a simple 
    # script that we invoke with bash which calls cmake with all of the properly
    # arguments. We are now using this strategy for all platforms.
    #

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "${CMAKE_BIN}" ${vopts} ../${VTK_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "VTK configuration failed."


    #
    # Now build VTK.
    #
    info "Building VTK . . . (~20 minutes)"
    env DYLD_LIBRARY_PATH=`pwd`/bin $MAKE $MAKE_OPT_FLAGS || \
      error "VTK did not build correctly.  Giving up."

    info "Installing VTK . . . "
    $MAKE install || error "VTK did not install correctly."

    # Filter out an include that references the user's VTK build directory
    configdir="${vtk_inst_path}/lib/cmake/vtk-${VTK_SHORT_VERSION}"
    cat ${configdir}/VTKConfig.cmake | grep -v "vtkTestingMacros" > ${configdir}/VTKConfig.cmake.new
    mv ${configdir}/VTKConfig.cmake.new ${configdir}/VTKConfig.cmake

    # Fix up the libraries for MacOS X.
    bv_vtk_fixup

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

