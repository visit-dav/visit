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
export VTK_FILE=${VTK_FILE:-"VTK-6.1.0.tar.gz"}
export VTK_VERSION=${VTK_VERSION:-"6.1.0"}
export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"6.1"}
export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK-6.1.0"}
export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
export VTK_URL=${VTK_URL:-"http://www.vtk.org/files/release/6.1"}
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

    echo "SETUP_APP_VERSION(VTK $VTK_VERSION)" >> $HOSTCONF
    if [[ "$USE_SYSTEM_VTK" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR $SYSTEM_VTK_DIR)" >> $HOSTCONF
    else
            echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR \${VISITHOME}/${VTK_INSTALL_DIR}/\${VTK_VERSION}/\${VISITARCH})" >> $HOSTCONF
    fi
}

function bv_vtk_initialize_vars
{
    info "initalizing vtk vars"
    if [[ $DO_R == "yes" ]]; then
        VTK_INSTALL_DIR="vtk-r"
    fi
}

function bv_vtk_ensure
{
    if [[ "$DO_VTK" == "yes" && "$USE_SYSTEM_VTK" == "no" ]] ; then
        ensure_built_or_ready $VTK_INSTALL_DIR $VTK_VERSION $VTK_BUILD_DIR $VTK_FILE $VTK_URL
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
# fix for 10.9 -- this fix is already in newer versions of VTK
    info "Patching vtk-6"
    patch -p0 << \EOF
diff -c CMakeLists.txt.orig CMakeLists.txt
*** CMakeLists.txt.orig	2014-05-30 15:54:16.000000000 -0700
--- CMakeLists.txt	2014-05-30 15:54:25.000000000 -0700
***************
*** 4,13 ****
  
  # Objective-C++ compile flags, future CMake versions might make this obsolete
  IF(APPLE)
!   # Being a library, VTK may be linked in either GC (garbage collected)
!   # processes or non-GC processes.  Default to "GC supported" so that both
!   # GC and MRR (manual reference counting) are supported.
!   SET(VTK_OBJCXX_FLAGS_DEFAULT "-fobjc-gc")
    SET(VTK_REQUIRED_OBJCXX_FLAGS ${VTK_OBJCXX_FLAGS_DEFAULT} CACHE STRING "Extra flags for Objective-C++ compilation")
    MARK_AS_ADVANCED(VTK_REQUIRED_OBJCXX_FLAGS)
  ENDIF(APPLE)
--- 4,10 ----
  
  # Objective-C++ compile flags, future CMake versions might make this obsolete
  IF(APPLE)
!   SET(VTK_OBJCXX_FLAGS_DEFAULT "")
    SET(VTK_REQUIRED_OBJCXX_FLAGS ${VTK_OBJCXX_FLAGS_DEFAULT} CACHE STRING "Extra flags for Objective-C++ compilation")
    MARK_AS_ADVANCED(VTK_REQUIRED_OBJCXX_FLAGS)
  ENDIF(APPLE)

EOF
    if [[ $? != 0 ]] ; then
      warn "vtk6 patch failed."
      return 1
    fi

    return 0;
}

function apply_vtk_610_patch
{
   patch -p0 << \EOF
diff -c Rendering/OpenGL/vtkXOpenGLRenderWindow.cxx.orig Rendering/OpenGL/vtkXOpenGLRenderWindow.cxx
*** Rendering/OpenGL/vtkXOpenGLRenderWindow.cxx.orig	2015-01-29 15:59:05.000000000 -0800
--- Rendering/OpenGL/vtkXOpenGLRenderWindow.cxx	2015-01-29 16:00:02.000000000 -0800
***************
*** 27,33 ****
  
  // define GLX_GLXEXT_LEGACY to prevent glx.h to include glxext.h provided by
  // the system
! //#define GLX_GLXEXT_LEGACY
  #include "GL/glx.h"
  
  #include "vtkgl.h"
--- 27,33 ----
  
  // define GLX_GLXEXT_LEGACY to prevent glx.h to include glxext.h provided by
  // the system
! #define GLX_GLXEXT_LEGACY
  #include "GL/glx.h"
  
  #include "vtkgl.h"

EOF
    if [[ $? != 0 ]] ; then
      warn "vtk6 patch failed."
      return 1
    fi

    return 0;
}

function apply_vtk_610_patch_2
{
   patch -p0 << \EOF
diff -c Rendering/Core/vtkMapper.cxx.orig Rendering/Core/vtkMapper.cxx
*** Rendering/Core/vtkMapper.cxx.orig	2015-03-19 18:46:17.000000000 -0700
--- Rendering/Core/vtkMapper.cxx	2015-03-19 18:44:43.000000000 -0700
***************
*** 18,23 ****
--- 18,24 ----
  #include "vtkExecutive.h"
  #include "vtkLookupTable.h"
  #include "vtkFloatArray.h"
+ #include "vtkDoubleArray.h"
  #include "vtkImageData.h"
  #include "vtkPointData.h"
  #include "vtkMath.h"
***************
*** 517,523 ****
  template<class T>
  void vtkMapperCreateColorTextureCoordinates(T* input, float* output,
                                              vtkIdType num, int numComps,
!                                             int component, double* range)
  {
    double tmp, sum;
    double k = 1.0 / (range[1]-range[0]);
--- 518,524 ----
  template<class T>
  void vtkMapperCreateColorTextureCoordinates(T* input, float* output,
                                              vtkIdType num, int numComps,
!                                             int component, double* range, bool isLogScale)
  {
    double tmp, sum;
    double k = 1.0 / (range[1]-range[0]);
***************
*** 529,540 ****
      for (i = 0; i < num; ++i)
        {
        sum = 0;
!       for (j = 0; j < numComps; ++j)
!         {
!         tmp = static_cast<double>(*input);
!         sum += (tmp * tmp);
!         ++input;
!         }
        output[i] = k * (sqrt(sum) - range[0]);
        if (output[i] > 1.0)
          {
--- 530,545 ----
      for (i = 0; i < num; ++i)
        {
        sum = 0;
!       for (j = 0; j < numComps; ++j) {
!           if(!isLogScale) {
!               tmp = static_cast<double>(*input);
!           } else {
!               tmp = static_cast<double>(log10(*input));
!           }
!           
!           sum += (tmp * tmp);
!           ++input;
!       }
        output[i] = k * (sqrt(sum) - range[0]);
        if (output[i] > 1.0)
          {
***************
*** 551,557 ****
      input += component;
      for (i = 0; i < num; ++i)
        {
!       output[i] = k * (static_cast<double>(*input) - range[0]);
        if (output[i] > 1.0)
          {
          output[i] = 1.0;
--- 556,570 ----
      input += component;
      for (i = 0; i < num; ++i)
        {
!           if(!isLogScale) {
!               output[i] = k * (static_cast<double>(*input) - range[0]);
!           } else {
!               if(*input > 0) {
!                   output[i] = k * (static_cast<double>(log10(*input)) - range[0]);
!               } else {
!                   output[i] = 0;
!               }
!           }
        if (output[i] > 1.0)
          {
          output[i] = 1.0;
***************
*** 565,571 ****
      }
  }
  
- 
  #define ColorTextureMapSize 256
  // a side effect of this is that this->ColorCoordinates and
  // this->ColorTexture are set.
--- 578,583 ----
***************
*** 583,588 ****
--- 595,604 ----
      this->Colors = 0;
      }
  
+     double minRange = range[0];
+     double maxRange = range[1];
+     bool isLogScale = this->LookupTable->UsingLogScale() == 1;
+     
    // If the lookup table has changed, the recreate the color texture map.
    // Set a new lookup table changes this->MTime.
    if (this->ColorTextureMap == 0 ||
***************
*** 599,618 ****
      // Get the texture map from the lookup table.
      // Create a dummy ramp of scalars.
      // In the future, we could extend vtkScalarsToColors.
!     double k = (range[1]-range[0]) / (ColorTextureMapSize-1);
!     vtkFloatArray* tmp = vtkFloatArray::New();
      tmp->SetNumberOfTuples(ColorTextureMapSize);
!     float* ptr = tmp->GetPointer(0);
      for (int i = 0; i < ColorTextureMapSize; ++i)
!       {
!       *ptr = range[0] + i * k;
!       ++ptr;
!       }
      this->ColorTextureMap = vtkImageData::New();
      this->ColorTextureMap->SetExtent(0,ColorTextureMapSize-1,
                                       0,0, 0,0);
!     this->ColorTextureMap->GetPointData()->SetScalars(
!          this->LookupTable->MapScalars(tmp, this->ColorMode, 0));
      this->LookupTable->SetAlpha(orig_alpha);
      // Do we need to delete the scalars?
      this->ColorTextureMap->GetPointData()->GetScalars()->Delete();
--- 615,643 ----
      // Get the texture map from the lookup table.
      // Create a dummy ramp of scalars.
      // In the future, we could extend vtkScalarsToColors.
!     if(isLogScale) {
!         double logRange[2];
!         vtkLookupTable::GetLogRange(range, logRange);
!         minRange = logRange[0];
!         maxRange = logRange[1];
!     }
! 
!     double k = (maxRange - minRange) / (double)(ColorTextureMapSize-1);
!     vtkDoubleArray* tmp = vtkDoubleArray::New();
      tmp->SetNumberOfTuples(ColorTextureMapSize);
!     double* ptr = tmp->GetPointer(0);
!     
      for (int i = 0; i < ColorTextureMapSize; ++i)
!     {
!         double tmpVal = minRange + i * k;
!         *ptr = !isLogScale ? tmpVal : (double)pow(10., tmpVal);
!         ++ptr;
!     }
!         
      this->ColorTextureMap = vtkImageData::New();
      this->ColorTextureMap->SetExtent(0,ColorTextureMapSize-1,
                                       0,0, 0,0);
!     this->ColorTextureMap->GetPointData()->SetScalars(this->LookupTable->MapScalars(tmp, this->ColorMode, 0));
      this->LookupTable->SetAlpha(orig_alpha);
      // Do we need to delete the scalars?
      this->ColorTextureMap->GetPointData()->GetScalars()->Delete();
***************
*** 635,641 ****
        this->ColorCoordinates->UnRegister(this);
        this->ColorCoordinates = 0;
        }
! 
      // Now create the color texture coordinates.
      int numComps = scalars->GetNumberOfComponents();
      void* input = scalars->GetVoidPointer(0);
--- 660,671 ----
        this->ColorCoordinates->UnRegister(this);
        this->ColorCoordinates = 0;
        }
!         
!     if(isLogScale) {
!         range[0] = minRange;
!         range[1] = maxRange;
!     }
!         
      // Now create the color texture coordinates.
      int numComps = scalars->GetNumberOfComponents();
      void* input = scalars->GetVoidPointer(0);
***************
*** 660,666 ****
        vtkTemplateMacro(
          vtkMapperCreateColorTextureCoordinates(static_cast<VTK_TT*>(input),
                                                 output, num, numComps,
!                                                scalarComponent, range)
          );
        case VTK_BIT:
          vtkErrorMacro("Cannot color by bit array.");
--- 690,696 ----
        vtkTemplateMacro(
          vtkMapperCreateColorTextureCoordinates(static_cast<VTK_TT*>(input),
                                                 output, num, numComps,
!                                                scalarComponent, range, isLogScale)
          );
        case VTK_BIT:
          vtkErrorMacro("Cannot color by bit array.");

EOF
    if [[ $? != 0 ]] ; then
      warn "vtk610_2 patch failed."
      return 1
    fi

    return 0;
}

function apply_vtk_patch
{
    if [[ ${VTK_VERSION} == 6.0.0 ]] ; then
        apply_vtk_600_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
    
    # also apply objc flag patch to 6.1.0
    
    if [[ ${VTK_VERSION} == 6.1.0 ]] ; then
        apply_vtk_600_patch
        apply_vtk_610_patch_2
        if [[ "$OPSYS" == "Linux" ]] ; then
	   apply_vtk_610_patch
        fi
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
    cd $VTK_BUILD_DIR || error "Can't cd to VTK build dir."
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
    # move back up to the start dir 
    cd "$START_DIR"

    info "Configuring VTK . . ."

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
    vopts="${vopts} -DVTK_DEBUG_LEAKS:BOOL=${vtk_debug_leaks}"
    vopts="${vopts} -DBUILD_TESTING:BOOL=false"
    vopts="${vopts} -DBUILD_DOCUMENTATION:BOOL=false"
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DVTK_REPORT_OPENGL_ERRORS:BOOL=false"
    if test "${OPSYS}" = "Darwin" ; then
        vopts="${vopts} -DVTK_USE_COCOA:BOOL=ON"
        vopts="${vopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${vtk_inst_path}/lib"
    fi

    # allow VisIt to override any of vtk's classes
    vopts="${vopts} -DVTK_ALL_NEW_OBJECT_FACTORY:BOOL=true"

    # Turn off module groups
    vopts="${vopts} -DVTK_Group_Imaging:BOOL=false"
    vopts="${vopts} -DVTK_Group_MPI:BOOL=false"
    vopts="${vopts} -DVTK_Group_Qt:BOOL=false"
    vopts="${vopts} -DVTK_Group_Rendering:BOOL=false"
    vopts="${vopts} -DVTK_Group_StandAlone:BOOL=false"
    vopts="${vopts} -DVTK_Group_Tk:BOOL=false"
    vopts="${vopts} -DVTK_Group_Views:BOOL=false"

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
                if [[ ${IS_QT5} == "yes" ]]; then
                    vopts="${vopts} -DVTK_QT_VERSION=5"
                    vopts="${vopts} -DCMAKE_PREFIX_PATH=${QT_INSTALL_DIR}/lib/cmake"
                fi
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

            vopts="${vopts} -DVTK_WRAP_PYTHON:BOOL=true"
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
        vopts="${vopts} -DVTK_R_HOME:PATH=${R_INSTALL_DIR}/lib/R"
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
    echo "\"${CMAKE_BIN}\"" ${vopts} ../${VTK_SRC_DIR} > bv_run_cmake.sh
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

