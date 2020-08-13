function bv_vtk_initialize
{
    export DO_VTK="yes"
    export FORCE_VTK="no"
    export USE_SYSTEM_VTK="no"
    add_extra_commandline_args "vtk" "system-vtk" 0 "Using system VTK (exp)"
    add_extra_commandline_args "vtk" "alt-vtk-dir" 1 "Use alternate VTK (exp)"
}

function bv_vtk_enable
{
    DO_VTK="yes"
    FORCE_VTK="yes"
}

function bv_vtk_disable
{
    DO_VTK="no"
    FORCE_VTK="no"
}

function bv_vtk_system_vtk
{
    TEST=`which vtk-config`
    [ $? != 0 ] && error "System vtk-config not found, cannot configure vtk"

    bv_vtk_enable
    USE_SYSTEM_VTK="yes"
    SYSTEM_VTK_DIR="$1"
    info "Using System VTK: $SYSTEM_VTK_DIR"
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
    depends_on="cmake zlib"

    if [[ "$DO_PYTHON" == "yes" ]]; then
        depends_on="${depends_on} python"
    fi

    if [[ "$DO_MESAGL" == "yes" ]]; then
        depends_on="${depends_on} mesagl glu"
    fi

    if [[ "$DO_OSPRAY" == "yes" ]]; then
        depends_on="${depends_on} ospray"
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
    export VTK_FILE=${VTK_FILE:-"VTK-9.0.0.tar.gz"}
    export VTK_VERSION=${VTK_VERSION:-"9.0.0"}
    export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"9.0"}
    export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
    export VTK_URL=${VTK_URL:-"https://www.vtk.org/files/release/${VTK_SHORT_VERSION}"}
    export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK-9.0.0"}
    export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
    export VTK_MD5_CHECKSUM="fa61cd36491d89a17edab18522bdda49"
    #export VTK_SHA256_CHECKSUM="6e269f07b64fb13774f5925161fb4e1f379f4e6a0131c8408c555f6b58ef3cb7"
}

function bv_vtk_print
{
    printf "%s%s\n" "VTK_FILE=" "${VTK_FILE}"
    printf "%s%s\n" "VTK_VERSION=" "${VTK_VERSION}"
    printf "%s%s\n" "VTK_BUILD_DIR=" "${VTK_BUILD_DIR}"
}

function bv_vtk_print_usage
{
    printf "%-20s %s\n" "--vtk" "Build VTK"
    printf "%-20s %s [%s]\n" "--system-vtk" "Use the system installed VTK"
    printf "%-20s %s [%s]\n" "--alt-vtk-dir" "Use VTK from an alternative directory"
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
        # vtk's target system should take care of this, so does VisIt need to know?
        echo "VISIT_OPTION_DEFAULT(VISIT_VTK_INCDEP ZLIB_INCLUDE_DIR)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_VTK_LIBDEP ZLIB_LIBRARY)" >> $HOSTCONF
    fi
}

function bv_vtk_initialize_vars
{
    info "initalizing vtk vars"
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

function apply_vtkopengloptions_patch
{
  # patch vtk's vtkOpenGLOptions.cmake to allow specifying VTK_OPENGL_HAS_OSMESA
  # and VTK_USE_X, otherwise vtkOSOpenGLRenderWindow isn't built and vtkglew
  # doesn't link with OSMESA.

   patch -p0 << \EOF
*** CMake/vtkOpenGLOptions.cmake.orig
--- CMake/vtkOpenGLOptions.cmake
***************
*** 83,96 ****
    set(VTK_CAN_DO_HEADLESS FALSE)
  endif()

! if (VTK_OPENGL_HAS_OSMESA AND VTK_CAN_DO_ONSCREEN)
!   message(FATAL_ERROR
!     "The `VTK_OPENGL_HAS_OSMESA` is ignored if any of the following is true: "
!     "the target platform is Windows, `VTK_USE_COCOA` is `ON`, or `VTK_USE_X` "
!     "is `ON`. OSMesa does not support on-screen rendering and VTK's OpenGL "
!     "selection is at build time, so the current build configuration is not "
!     "satisfiable.")
! endif ()

  cmake_dependent_option(
    VTK_USE_OPENGL_DELAYED_LOAD
--- 83,96 ----
    set(VTK_CAN_DO_HEADLESS FALSE)
  endif()

! #if (VTK_OPENGL_HAS_OSMESA AND VTK_CAN_DO_ONSCREEN)
! #  message(FATAL_ERROR
! #    "The `VTK_OPENGL_HAS_OSMESA` is ignored if any of the following is true: "
! #    "the target platform is Windows, `VTK_USE_COCOA` is `ON`, or `VTK_USE_X` "
! #    "is `ON`. OSMesa does not support on-screen rendering and VTK's OpenGL "
! #    "selection is at build time, so the current build configuration is not "
! #    "satisfiable.")
! #endif ()

  cmake_dependent_option(
    VTK_USE_OPENGL_DELAYED_LOAD

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOpenGLOptions failed."
      return 1
    fi
    return 0;
}

function apply_vtkopenfoamreader_header_patch
{
  # patch vtk's OpenFOAMReader to provide more meta data information
  # useful for VisIt's plugin.

   patch -p0 << \EOF
*** IO/Geometry/vtkOpenFOAMReader.h.orig
--- IO/Geometry/vtkOpenFOAMReader.h
***************
*** 48,53 ****
--- 48,58 ----
  #include "vtkIOGeometryModule.h" // For export macro
  #include "vtkMultiBlockDataSetAlgorithm.h"
  
+ //added by LLNL
+ #include <map>
+ #include <vector>
+ //end added by LLNL
+ 
  class vtkCollection;
  class vtkCharArray;
  class vtkDataArraySelection;
***************
*** 348,353 ****
--- 353,380 ----
  
    friend class vtkOpenFOAMReaderPrivate;
  
+   // Added by LLNL
+   vtkStdString GetCellArrayClassName(const char *name);
+   vtkStdString GetPointArrayClassName(const char *name);
+   vtkStdString GetLagrangianArrayClassName(const char *name);
+ 
+   int GetNumberOfCellZones() 
+     { return static_cast<int>(this->CellZones.size()); }
+   int GetNumberOfFaceZones()
+     { return static_cast<int>(this->FaceZones.size()); }
+   int GetNumberOfPointZones()
+     { return static_cast<int>(this->PointZones.size()); }
+ 
+   vtkStdString GetCellZoneName(int);
+   vtkStdString GetFaceZoneName(int);
+   vtkStdString GetPointZoneName(int);
+ 
+   int GetCellArrayExists(const char *name);
+   int GetPointArrayExists(const char *name);
+   int GetLagrangianArrayExists(const char *name);
+ 
+   // end Added by LLNL
+ 
  protected:
    // refresh flag
    bool Refresh;
***************
*** 423,428 ****
--- 450,465 ----
    // index of the active reader
    int CurrentReaderIndex;
  
+   // added by LLNL
+   std::vector<vtkStdString> CellZones;
+   std::vector<vtkStdString> FaceZones;
+   std::vector<vtkStdString> PointZones;
+ 
+   std::map<vtkStdString, vtkStdString> CellArrayClassName;
+   std::map<vtkStdString, vtkStdString> PointArrayClassName;
+   std::map<vtkStdString, vtkStdString> LagrangianArrayClassName;
+   // end added by LLNL
+ 
    vtkOpenFOAMReader();
    ~vtkOpenFOAMReader() override;
    int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOpenFOAMReader.h failed."
      return 1
    fi
    return 0;
}

function apply_vtkopenfoamreader_source_patch
{
  # patch vtk's OpenFOAMReader to provide more meta data information
  # useful for VisIt's plugin.

   patch -p0 << \EOF
*** IO/Geometry/vtkOpenFOAMReader.cxx.orig
--- IO/Geometry/vtkOpenFOAMReader.cxx
***************
*** 4419,4424 ****
--- 4419,4427 ----
              this->LagrangianFieldFiles->InsertNextValue(fieldFile);
              // object name
              pointObjectNames->InsertNextValue(io.GetObjectName());
+             // added by LLNL
+             this->Parent->LagrangianArrayClassName[io.GetObjectName()] = cn;
+             // end added by LLNL
            }
          }
          else
***************
*** 4434,4444 ****
--- 4437,4453 ----
                this->VolFieldFiles->InsertNextValue(fieldFile);
                // object name
                cellObjectNames->InsertNextValue(io.GetObjectName());
+               // added by LLNL
+               this->Parent->CellArrayClassName[io.GetObjectName()] = cn;
+               // end added by LLNL
              }
              else
              {
                this->PointFieldFiles->InsertNextValue(fieldFile);
                pointObjectNames->InsertNextValue(io.GetObjectName());
+               // added by LLNL
+               this->Parent->PointArrayClassName[io.GetObjectName()] = cn;
+               // end added by LLNL
              }
            }
          }
***************
*** 4644,4649 ****
--- 4653,4705 ----
  
        delete boundaryDict;
      }
+ 
+     // added by LLNL
+     if (this->Parent->GetReadZones())
+     {
+       vtkFoamDict *dictPtr= this->GatherBlocks("pointZones", true);
+       if (dictPtr!= NULL)
+       {
+         this->Parent->PointZones.clear();
+         vtkFoamDict &pointZoneDict = *dictPtr;
+         int nPointZones = static_cast<int>(pointZoneDict.size());
+ 
+         for (int i = 0; i < nPointZones; i++)
+         {
+           this->Parent->PointZones.push_back(pointZoneDict[i]->GetKeyword().c_str());
+         }
+         delete dictPtr;
+       }
+ 
+       dictPtr= this->GatherBlocks("faceZones", true);
+       if (dictPtr!= NULL)
+       {
+         this->Parent->FaceZones.clear();
+         vtkFoamDict &faceZoneDict = *dictPtr;
+         int nFaceZones = static_cast<int>(faceZoneDict.size());
+ 
+         for (int i = 0; i < nFaceZones; i++)
+         {
+           this->Parent->FaceZones.push_back(faceZoneDict[i]->GetKeyword().c_str());
+         }
+         delete dictPtr;
+       }
+ 
+       dictPtr= this->GatherBlocks("cellZones", true);
+       if (dictPtr!= NULL)
+       {
+         this->Parent->CellZones.clear();
+         vtkFoamDict &cellZoneDict = *dictPtr;
+         int nCellZones = static_cast<int>(cellZoneDict.size());
+ 
+         for (int i = 0; i < nCellZones; i++)
+         {
+           this->Parent->CellZones.push_back(cellZoneDict[i]->GetKeyword().c_str());
+         }
+         delete dictPtr;
+       }
+     }
+     // end added by LLNL
    }
  
    // Add scalars and vectors to metadata
***************
*** 9344,9346 ****
--- 9400,9519 ----
      (static_cast<double>(this->Parent->CurrentReaderIndex) + amount) /
      static_cast<double>(this->Parent->NumberOfReaders));
  }
+ 
+ 
+ // added by LLNL
+ 
+ //-----------------------------------------------------------------------------
+ vtkStdString vtkOpenFOAMReader::GetCellArrayClassName(const char *name)
+ {
+   vtkStdString ret;
+   if (name == NULL || name[0] == '\0')
+   {
+         return ret;
+   }
+   std::map<vtkStdString, vtkStdString>::const_iterator itr =
+     this->CellArrayClassName.find(vtkStdString(name));
+   if (itr != this->CellArrayClassName.end())
+   {
+     return itr->second;
+   }
+   else 
+   {
+     return ret;
+   }
+ }
+ 
+ //-----------------------------------------------------------------------------
+ vtkStdString vtkOpenFOAMReader::GetPointArrayClassName(const char *name)
+ {
+   vtkStdString ret;
+   if (name == NULL || name[0] == '\0')
+   {
+         return ret;
+   }
+   std::map<vtkStdString, vtkStdString>::const_iterator itr =
+     this->PointArrayClassName.find(vtkStdString(name));
+   if (itr != this->PointArrayClassName.end())
+   {
+     return itr->second;
+   }
+   else 
+   {
+     return ret;
+   }
+ }
+ 
+ //-----------------------------------------------------------------------------
+ vtkStdString vtkOpenFOAMReader::GetLagrangianArrayClassName(const char *name)
+ {
+   vtkStdString ret;
+   if (name == NULL || name[0] == '\0')
+   {
+         return ret;
+   }
+   std::map<vtkStdString, vtkStdString>::const_iterator itr =
+     this->LagrangianArrayClassName.find(vtkStdString(name));
+   if (itr != this->LagrangianArrayClassName.end())
+   {
+     return itr->second;
+   }
+   else 
+   {
+     return ret;
+   }
+ }
+ 
+ //-----------------------------------------------------------------------------
+ int vtkOpenFOAMReader::GetCellArrayExists(const char *name)
+ {
+   return this->CellDataArraySelection->ArrayExists(name);
+ }
+ 
+ //-----------------------------------------------------------------------------
+ int vtkOpenFOAMReader::GetPointArrayExists(const char *name)
+ {
+   return this->PointDataArraySelection->ArrayExists(name);
+ }
+ 
+ //-----------------------------------------------------------------------------
+ int vtkOpenFOAMReader::GetLagrangianArrayExists(const char *name)
+ {
+   return this->LagrangianDataArraySelection->ArrayExists(name);
+ }
+ 
+ //-----------------------------------------------------------------------------
+ vtkStdString vtkOpenFOAMReader::GetCellZoneName(const int index)
+ {
+   vtkStdString ret;
+   if (index >= 0 && index < this->GetNumberOfCellZones())
+   {
+     ret = this->CellZones[index];
+   }
+   return ret;
+ }
+ 
+ //-----------------------------------------------------------------------------
+ vtkStdString vtkOpenFOAMReader::GetFaceZoneName(const int index)
+ {
+   vtkStdString ret;
+   if (index >= 0 && index < this->GetNumberOfFaceZones())
+   {
+     ret = this->FaceZones[index];
+   }
+   return ret;
+ }
+ 
+ //-----------------------------------------------------------------------------
+ vtkStdString vtkOpenFOAMReader::GetPointZoneName(const int index)
+ {
+   vtkStdString ret;
+   if (index >= 0 && index < this->GetNumberOfPointZones())
+   {
+     ret = this->PointZones[index];
+   }
+   return ret;
+ }
+ 
+ // end added by LLNL
+ 

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOpenFOAMReader.cxx failed."
      return 1
    fi
    return 0;
}

function apply_vtkopenfoamreader_patch
{
    apply_vtkopenfoamreader_header_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi
    apply_vtkopenfoamreader_source_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_vtkprobeopenglversion_patch
{
  # patch vtk's Rendering/OpenGL2/CMakeLists.txt to fix a compile problem
  # with vtkProbeOpenGLVersion when OSMesa is turned on.

   patch -p0 << \EOF
*** Rendering/OpenGL2/CMakeLists.txt.orig
--- Rendering/OpenGL2/CMakeLists.txt
***************
*** 351,372 ****
    vtk_module_link(VTK::RenderingOpenGL2 PUBLIC "-framework UIKit")
  endif ()

! if (NOT ANDROID AND
!     NOT APPLE_IOS AND
!     NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten" AND
!     NOT VTK_OPENGL_USE_GLES)
!   set(probe_no_install)
!   if (NOT _vtk_build_INSTALL_HEADERS)
!     set(probe_no_install
!       NO_INSTALL)
!   endif ()
!   vtk_module_add_executable(vtkProbeOpenGLVersion
!     ${probe_no_install}
!     vtkProbeOpenGLVersion.cxx)
!   target_link_libraries(vtkProbeOpenGLVersion
!     PRIVATE
!       VTK::RenderingOpenGL2)
!   vtk_module_autoinit(
!     TARGETS vtkProbeOpenGLVersion
!     MODULES VTK::RenderingOpenGL2)
! endif ()
--- 351,372 ----
    vtk_module_link(VTK::RenderingOpenGL2 PUBLIC "-framework UIKit")
  endif ()

! #if (NOT ANDROID AND
! #    NOT APPLE_IOS AND
! #    NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten" AND
! #    NOT VTK_OPENGL_USE_GLES)
! #  set(probe_no_install)
! #  if (NOT _vtk_build_INSTALL_HEADERS)
! #    set(probe_no_install
! #      NO_INSTALL)
! #  endif ()
! #  vtk_module_add_executable(vtkProbeOpenGLVersion
! #    ${probe_no_install}
! #    vtkProbeOpenGLVersion.cxx)
! #  target_link_libraries(vtkProbeOpenGLVersion
! #    PRIVATE
! #      VTK::RenderingOpenGL2)
! #  vtk_module_autoinit(
! #    TARGETS vtkProbeOpenGLVersion
! #    MODULES VTK::RenderingOpenGL2)
! #endif ()
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkProbeOpenGLVersion failed."
      return 1
    fi
    return 0;

}

function apply_vtkopenglspheremapper_h_patch
{
##NEED TO UPDATE THIS
  # patch vtk's vtkOpenGLSphereMapper.h to fix bug evidenced when
  # points are double precision

   patch -p0 << \EOF
*** Rendering/OpenGL2/vtkOpenGLSphereMapper.h.orig  2019-06-05 11:49:48.675659000 -0700
--- Rendering/OpenGL2/vtkOpenGLSphereMapper.h  2019-06-05 10:25:08.000000000 -0700
***************
*** 94,105 ****
  
    void RenderPieceDraw(vtkRenderer *ren, vtkActor *act) override;
  
-   virtual void CreateVBO(
-     float * points, vtkIdType numPts,
-     unsigned char *colors, int colorComponents,
-     vtkIdType nc,
-     float *sizes, vtkIdType ns, vtkRenderer *ren);
- 
    // used for transparency
    bool Invert;
    float Radius;
--- 94,99 ----
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOpenGLSphereMapper.h failed."
      return 1
    fi
    return 0;
}

function apply_vtkopenglspheremapper_patch
{
  # patch vtk's vtkOpenGLSphereMapper to fix bug that ignores opacity when
  # specifying single color for the sphere imposters, and another bug
  # evidenced when points are double precision

   patch -p0 << \EOF
*** Rendering/OpenGL2/vtkOpenGLSphereMapper.cxx.orig 2017-12-22 08:33:25.000000000 -0800
--- Rendering/OpenGL2/vtkOpenGLSphereMapper.cxx 2019-06-06 12:21:13.735291000 -0700
***************
*** 15,20 ****
--- 15,21 ----
  
  #include "vtkOpenGLHelper.h"
  
+ #include "vtkDataArrayAccessor.h"
  #include "vtkFloatArray.h"
  #include "vtkMath.h"
  #include "vtkMatrix4x4.h"
***************
*** 211,253 ****
    os << indent << "Radius: " << this->Radius << "\n";
  }
  
! // internal function called by CreateVBO
! void vtkOpenGLSphereMapper::CreateVBO(
!   float * points, vtkIdType numPts,
!   unsigned char *colors, int colorComponents,
!   vtkIdType nc,
!   float *sizes, vtkIdType ns, vtkRenderer *ren)
  {
!   vtkFloatArray *verts = vtkFloatArray::New();
!   verts->SetNumberOfComponents(3);
!   verts->SetNumberOfTuples(numPts*3);
!   float *vPtr = static_cast<float *>(verts->GetVoidPointer(0));
  
!   vtkFloatArray *offsets = vtkFloatArray::New();
!   offsets->SetNumberOfComponents(2);
!   offsets->SetNumberOfTuples(numPts*3);
    float *oPtr = static_cast<float *>(offsets->GetVoidPointer(0));
- 
-   vtkUnsignedCharArray *ucolors = vtkUnsignedCharArray::New();
-   ucolors->SetNumberOfComponents(4);
-   ucolors->SetNumberOfTuples(numPts*3);
    unsigned char *cPtr = static_cast<unsigned char *>(ucolors->GetVoidPointer(0));
  
!   float *pointPtr;
!   unsigned char *colorPtr;
  
    float cos30 = cos(vtkMath::RadiansFromDegrees(30.0));
  
    for (vtkIdType i = 0; i < numPts; ++i)
    {
!     pointPtr = points + i*3;
!     colorPtr = (nc == numPts ? colors + i*colorComponents : colors);
!     float radius = (ns == numPts ? sizes[i] : sizes[0]);
  
      // Vertices
!     *(vPtr++) = pointPtr[0];
!     *(vPtr++) = pointPtr[1];
!     *(vPtr++) = pointPtr[2];
      *(cPtr++) = colorPtr[0];
      *(cPtr++) = colorPtr[1];
      *(cPtr++) = colorPtr[2];
--- 212,250 ----
    os << indent << "Radius: " << this->Radius << "\n";
  }
  
! // internal function called by BuildBufferObjects
! template <typename PtsArray, typename SizesArray>
! void vtkOpenGLSphereMapper_PrepareVBO(
!   PtsArray *points, unsigned char *colors, int colorComponents,
!   vtkIdType nc, SizesArray *sizesA, vtkIdType ns,
!   vtkFloatArray *verts, vtkFloatArray *offsets, vtkUnsignedCharArray *ucolors)
  {
!   vtkIdType numPts = points->GetNumberOfTuples();
  
!   float *vPtr = static_cast<float *>(verts->GetVoidPointer(0));
    float *oPtr = static_cast<float *>(offsets->GetVoidPointer(0));
    unsigned char *cPtr = static_cast<unsigned char *>(ucolors->GetVoidPointer(0));
  
!   vtkDataArrayAccessor<PtsArray> pointPtr(points);
!   vtkDataArrayAccessor<SizesArray> sizes(sizesA);
! 
!   float radius = sizes.Get(0, 0);
!   
!   unsigned char *colorPtr = colors;
  
    float cos30 = cos(vtkMath::RadiansFromDegrees(30.0));
  
    for (vtkIdType i = 0; i < numPts; ++i)
    {
!     if (nc == numPts)
!         colorPtr = colors + i*colorComponents;
!     if (ns == numPts)
!         radius = sizes.Get(i, 0);
  
      // Vertices
!     *(vPtr++) = pointPtr.Get(i, 0);
!     *(vPtr++) = pointPtr.Get(i, 1);
!     *(vPtr++) = pointPtr.Get(i, 2);
      *(cPtr++) = colorPtr[0];
      *(cPtr++) = colorPtr[1];
      *(cPtr++) = colorPtr[2];
***************
*** 255,263 ****
      *(oPtr++) = -2.0f*radius*cos30;
      *(oPtr++) = -radius;
  
!     *(vPtr++) = pointPtr[0];
!     *(vPtr++) = pointPtr[1];
!     *(vPtr++) = pointPtr[2];
      *(cPtr++) = colorPtr[0];
      *(cPtr++) = colorPtr[1];
      *(cPtr++) = colorPtr[2];
--- 252,260 ----
      *(oPtr++) = -2.0f*radius*cos30;
      *(oPtr++) = -radius;
  
!     *(vPtr++) = pointPtr.Get(i, 0);
!     *(vPtr++) = pointPtr.Get(i, 1);
!     *(vPtr++) = pointPtr.Get(i, 2);
      *(cPtr++) = colorPtr[0];
      *(cPtr++) = colorPtr[1];
      *(cPtr++) = colorPtr[2];
***************
*** 265,273 ****
      *(oPtr++) = 2.0f*radius*cos30;
      *(oPtr++) = -radius;
  
!     *(vPtr++) = pointPtr[0];
!     *(vPtr++) = pointPtr[1];
!     *(vPtr++) = pointPtr[2];
      *(cPtr++) = colorPtr[0];
      *(cPtr++) = colorPtr[1];
      *(cPtr++) = colorPtr[2];
--- 262,270 ----
      *(oPtr++) = 2.0f*radius*cos30;
      *(oPtr++) = -radius;
  
!     *(vPtr++) = pointPtr.Get(i, 0);
!     *(vPtr++) = pointPtr.Get(i, 1);
!     *(vPtr++) = pointPtr.Get(i, 2);
      *(cPtr++) = colorPtr[0];
      *(cPtr++) = colorPtr[1];
      *(cPtr++) = colorPtr[2];
***************
*** 275,288 ****
      *(oPtr++) = 0.0f;
      *(oPtr++) = 2.0f*radius;
    }
- 
-   this->VBOs->CacheDataArray("vertexMC", verts, ren, VTK_FLOAT);
-   verts->Delete();
-   this->VBOs->CacheDataArray("offsetMC", offsets, ren, VTK_FLOAT);
-   offsets->Delete();
-   this->VBOs->CacheDataArray("scalarColor", ucolors, ren, VTK_UNSIGNED_CHAR);
-   ucolors->Delete();
-   VBOs->BuildAllVBOs(ren);
  }
  
  //-------------------------------------------------------------------------
--- 272,277 ----
***************
*** 320,326 ****
    // then the scalars do not have to be regenerted.
    this->MapScalars(1.0);
  
!   vtkIdType numPts = poly->GetPoints()->GetNumberOfPoints();
    unsigned char *c;
    int cc;
    vtkIdType nc;
--- 309,317 ----
    // then the scalars do not have to be regenerted.
    this->MapScalars(1.0);
  
!   vtkPoints *pts = poly->GetPoints();
! 
!   vtkIdType numPts = pts->GetNumberOfPoints();
    unsigned char *c;
    int cc;
    vtkIdType nc;
***************
*** 333,373 ****
    else
    {
      double *ac = act->GetProperty()->GetColor();
!     c = new unsigned char[3];
      c[0] = (unsigned char) (ac[0] *255.0);
      c[1] = (unsigned char) (ac[1] *255.0);
      c[2] = (unsigned char) (ac[2] *255.0);
      nc = 1;
!     cc = 3;
    }
  
!   float *scales;
    vtkIdType ns = poly->GetPoints()->GetNumberOfPoints();
    if (this->ScaleArray != nullptr &&
        poly->GetPointData()->HasArray(this->ScaleArray))
    {
!     scales = static_cast<float*>(poly->GetPointData()->GetArray(this->ScaleArray)->GetVoidPointer(0));
      ns = numPts;
    }
    else
    {
!     scales = &this->Radius;
      ns = 1;
    }
  
!   // Iterate through all of the different types in the polydata, building OpenGLs
!   // and IBOs as appropriate for each type.
!   this->CreateVBO(
!     static_cast<float *>(poly->GetPoints()->GetVoidPointer(0)),
!     numPts,
!     c, cc, nc,
!     scales, ns,
!     ren);
  
    if (!this->Colors)
    {
      delete [] c;
    }
  
    // create the IBO
    this->Primitives[PrimitivePoints].IBO->IndexCount = 0;
--- 324,386 ----
    else
    {
      double *ac = act->GetProperty()->GetColor();
!     double opac = act->GetProperty()->GetOpacity();
!     c = new unsigned char[4];
      c[0] = (unsigned char) (ac[0] *255.0);
      c[1] = (unsigned char) (ac[1] *255.0);
      c[2] = (unsigned char) (ac[2] *255.0);
+     c[3] = (unsigned char) (opac  *255.0);
      nc = 1;
!     cc = 4;
    }
  
!   vtkDataArray *scales = NULL;
    vtkIdType ns = poly->GetPoints()->GetNumberOfPoints();
    if (this->ScaleArray != nullptr &&
        poly->GetPointData()->HasArray(this->ScaleArray))
    {
!     scales = poly->GetPointData()->GetArray(this->ScaleArray);
      ns = numPts;
    }
    else
    {
!     scales = vtkFloatArray::New();
!     scales->SetNumberOfTuples(1);
!     scales->SetTuple1(0, this->Radius);
      ns = 1;
    }
  
!   vtkFloatArray *verts = vtkFloatArray::New();
!   verts->SetNumberOfComponents(3);
!   verts->SetNumberOfTuples(numPts*3);
! 
!   vtkFloatArray *offsets = vtkFloatArray::New();
!   offsets->SetNumberOfComponents(2);
!   offsets->SetNumberOfTuples(numPts*3);
! 
!   vtkUnsignedCharArray *ucolors = vtkUnsignedCharArray::New();
!   ucolors->SetNumberOfComponents(4);
!   ucolors->SetNumberOfTuples(numPts*3);
! 
!   vtkOpenGLSphereMapper_PrepareVBO(pts->GetData(), c, cc, nc, scales, ns,
!                                    verts, offsets, ucolors);
! 
!   this->VBOs->CacheDataArray("vertexMC", verts, ren, VTK_FLOAT);
!   verts->Delete();
!   this->VBOs->CacheDataArray("offsetMC", offsets, ren, VTK_FLOAT);
!   offsets->Delete();
!   this->VBOs->CacheDataArray("scalarColor", ucolors, ren, VTK_UNSIGNED_CHAR);
!   ucolors->Delete();
!   this->VBOs->BuildAllVBOs(ren);
  
    if (!this->Colors)
    {
      delete [] c;
    }
+   if (ns != numPts)
+   {
+     scales->Delete();
+   }
  
    // create the IBO
    this->Primitives[PrimitivePoints].IBO->IndexCount = 0;
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOpenGLSphereMapper.cxx failed."
      return 1
    fi
    return 0;
}


function apply_vtkospray_patches
{
	count_patches=3
    # patch vtkOSPRay files:

    # 1) expose vtkViewNodeFactory via vtkOSPRayPass
	current_patch=1
    patch -p0 << \EOF
*** Rendering/OSPRay/vtkOSPRayPass.h.original     2018-04-23 19:23:29.000000000 -0700
--- Rendering/OSPRay/vtkOSPRayPass.h  2018-04-30 21:31:49.911508591 -0700
***************
*** 48,53 ****
--- 48,54 ----
  class vtkRenderPassCollection;
  class vtkSequencePass;
  class vtkVolumetricPass;
+ class vtkViewNodeFactory;

  class VTKRENDERINGOSPRAY_EXPORT vtkOSPRayPass : public vtkRenderPass
  {
***************
*** 74,79 ****
--- 75,82 ----
     */
    virtual void RenderInternal(const vtkRenderState *s);

+   virtual vtkViewNodeFactory* GetViewNodeFactory();
+
   protected:
    /**
     * Default constructor.
*** Rendering/OSPRay/vtkOSPRayPass.cxx.original	2018-04-23 19:23:29.000000000 -0700
--- Rendering/OSPRay/vtkOSPRayPass.cxx	2018-04-30 21:31:49.907508611 -0700
***************
*** 273,275 ****
--- 273,280 ----
      }
    }
  }
+
+ vtkViewNodeFactory* vtkOSPRayPass::GetViewNodeFactory()
+ {
+   return this->Internal->Factory;
+ }
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch ${current_patch}/${count_patches} for vtkOSPRayPass failed."
        return 1
    fi

	# 2) enable vtkOSPRayFollowerNode
	((current_patch++))
	patch -p0 << \EOF
*** Rendering/OSPRay/vtkOSPRayViewNodeFactory.cxx.original	2018-04-23 19:23:29.000000000 -0700
--- Rendering/OSPRay/vtkOSPRayViewNodeFactory.cxx	2018-05-07 19:43:23.902077745 -0700
***************
*** 19,24 ****
--- 19,25 ----
  #include "vtkOSPRayAMRVolumeMapperNode.h"
  #include "vtkOSPRayCameraNode.h"
  #include "vtkOSPRayCompositePolyDataMapper2Node.h"
+ #include "vtkOSPRayFollowerNode.h"
  #include "vtkOSPRayLightNode.h"
  #include "vtkOSPRayRendererNode.h"
  #include "vtkOSPRayPolyDataMapperNode.h"
***************
*** 44,49 ****
--- 45,56 ----
    return vn;
  }

+ vtkViewNode *fol_maker()
+ {
+   vtkOSPRayFollowerNode *vn = vtkOSPRayFollowerNode::New();
+   return vn;
+ }
+
  vtkViewNode *vol_maker()
  {
    return vtkOSPRayVolumeNode::New();
***************
*** 96,101 ****
--- 103,109 ----
    this->RegisterOverride("vtkOpenGLRenderer", ren_maker);
    this->RegisterOverride("vtkOpenGLActor", act_maker);
    this->RegisterOverride("vtkPVLODActor", act_maker);
+   this->RegisterOverride("vtkFollower", fol_maker);
    this->RegisterOverride("vtkPVLODVolume", vol_maker);
    this->RegisterOverride("vtkVolume", vol_maker);
    this->RegisterOverride("vtkOpenGLCamera", cam_maker);
*** Rendering/OSPRay/CMakeLists.txt.original	2018-04-23 19:23:28.000000000 -0700
--- Rendering/OSPRay/CMakeLists.txt	2018-04-23 21:07:41.269154859 -0700
***************
*** 7,12 ****
--- 7,13 ----
    vtkOSPRayVolumeNode.cxx
    vtkOSPRayCameraNode.cxx
    vtkOSPRayCompositePolyDataMapper2Node.cxx
+   vtkOSPRayFollowerNode.cxx
    vtkOSPRayLightNode.cxx
    vtkOSPRayMaterialHelpers.cxx
    vtkOSPRayMaterialLibrary.cxx
*** Rendering/OSPRay/vtkOSPRayFollowerNode.h.original	1969-12-31 16:00:00.000000000 -0800
--- Rendering/OSPRay/vtkOSPRayFollowerNode.h	2018-04-23 21:07:41.269154859 -0700
***************
*** 0 ****
--- 1,49 ----
+ /*=========================================================================
+
+   Program:   Visualization Toolkit
+   Module:    vtkOSPRayFollowerNode.h
+
+   Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
+   All rights reserved.
+   See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
+
+      This software is distributed WITHOUT ANY WARRANTY; without even
+      the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
+      PURPOSE.  See the above copyright notice for more information.
+
+ =========================================================================*/
+ /**
+  * @class   vtkOSPRayFollowerNode
+  * @brief   links vtkFollower to OSPRay
+  *
+  * Translates vtkFollower state into OSPRay rendering calls
+ */
+
+ #ifndef vtkOSPRayFollowerNode_h
+ #define vtkOSPRayFollowerNode_h
+
+ #include "vtkOSPRayActorNode.h"
+
+ class VTKRENDERINGOSPRAY_EXPORT vtkOSPRayFollowerNode :
+   public vtkOSPRayActorNode
+ {
+ public:
+   static vtkOSPRayFollowerNode* New();
+   vtkTypeMacro(vtkOSPRayFollowerNode, vtkOSPRayActorNode);
+   void PrintSelf(ostream& os, vtkIndent indent) override;
+
+   /**
+    * Overridden to take into account my renderables time, including
+    * its associated camera
+    */
+   virtual vtkMTimeType GetMTime() override;
+
+ protected:
+   vtkOSPRayFollowerNode();
+   ~vtkOSPRayFollowerNode();
+
+ private:
+   vtkOSPRayFollowerNode(const vtkOSPRayFollowerNode&) = delete;
+   void operator=(const vtkOSPRayFollowerNode&) = delete;
+ };
+ #endif
*** Rendering/OSPRay/vtkOSPRayFollowerNode.cxx.original	1969-12-31 16:00:00.000000000 -0800
--- Rendering/OSPRay/vtkOSPRayFollowerNode.cxx	2018-04-27 18:41:41.770557480 -0700
***************
*** 0 ****
--- 1,51 ----
+ /*=========================================================================
+
+   Program:   Visualization Toolkit
+   Module:    vtkOSPRayFollowerNode.cxx
+
+   Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
+   All rights reserved.
+   See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
+
+      This software is distributed WITHOUT ANY WARRANTY; without even
+      the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
+      PURPOSE.  See the above copyright notice for more information.
+
+ =========================================================================*/
+ #include "vtkOSPRayFollowerNode.h"
+ #include "vtkCamera.h"
+ #include "vtkFollower.h"
+ #include "vtkObjectFactory.h"
+
+ //============================================================================
+ vtkStandardNewMacro(vtkOSPRayFollowerNode);
+
+ //----------------------------------------------------------------------------
+ vtkOSPRayFollowerNode::vtkOSPRayFollowerNode()
+ {
+ }
+
+ //----------------------------------------------------------------------------
+ vtkOSPRayFollowerNode::~vtkOSPRayFollowerNode()
+ {
+ }
+
+ //----------------------------------------------------------------------------
+ void vtkOSPRayFollowerNode::PrintSelf(ostream& os, vtkIndent indent)
+ {
+   this->Superclass::PrintSelf(os, indent);
+ }
+
+ //----------------------------------------------------------------------------
+ vtkMTimeType vtkOSPRayFollowerNode::GetMTime()
+ {
+   vtkMTimeType mtime = this->Superclass::GetMTime();
+   vtkCamera *cam = ((vtkFollower*)this->GetRenderable())->GetCamera();
+
+   if (cam->GetMTime() > mtime)
+   {
+     mtime = cam->GetMTime();
+   }
+
+   return mtime;
+ }
*** Rendering/Core/vtkFollower.cxx.original	2017-12-22 10:33:25.000000000 -0600
--- Rendering/Core/vtkFollower.cxx	2018-06-14 13:35:08.481815058 -0500
***************
*** 156,161 ****
--- 156,165 ----
      this->Transform->GetMatrix(this->Matrix);
      this->MatrixMTime.Modified();
      this->Transform->Pop();
+
+     // if we get to here it's pretty safe to assume
+     // that our transform isn't an identity matrix
+     this->IsIdentity = 0;
    }
  }
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch $current_patch/$count_patches for vtkOSPRayFollowerNode failed."
        return 1
    fi

	((current_patch++))
    patch -p0 << \EOF
*** Rendering/OSPRay/vtkOSPRayVolumeMapper.cxx.original	2018-04-23 15:32:58.538749914 -0400
--- Rendering/OSPRay/vtkOSPRayVolumeMapper.cxx	2018-04-23 15:34:58.399824907 -0400
***************
*** 72,77 ****
--- 72,79 ----
    {
      this->Init();
    }
+   vtkOSPRayRendererNode::SetSamplesPerPixel(vtkOSPRayRendererNode::GetSamplesPerPixel(ren), this->InternalRenderer);
+   vtkOSPRayRendererNode::SetAmbientSamples(vtkOSPRayRendererNode::GetAmbientSamples(ren), this->InternalRenderer);
    this->InternalRenderer->SetRenderWindow(ren->GetRenderWindow());
    this->InternalRenderer->SetActiveCamera(ren->GetActiveCamera());
    this->InternalRenderer->SetBackground(ren->GetBackground());
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch $current_patch/$count_patches for vtkOSPRayVolumeMapper failed."
        return 1
    fi
}

function apply_vtkospraypolydatamappernode_patch
{
    # patch vtk's vtkOSPRayPolyDataMapperNode to handle vtkDataSets in
    # addition to vtkPolyData.

    patch -p0 << \EOF
diff -c Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.h.original Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.h
*** Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.h.original     Fri Dec 22 08:33:25 2017
--- Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.h      Fri Dec 28 15:56:33 2018
***************
*** 25,30 ****
--- 25,31 ----
  #include "vtkRenderingOSPRayModule.h" // For export macro
  #include "vtkPolyDataMapperNode.h"
  
+ class vtkDataSetSurfaceFilter;
  class vtkOSPRayActorNode;
  class vtkPolyData;
  
***************
*** 61,66 ****
--- 62,69 ----
    void CreateNewMeshes();
    void AddMeshesToModel(void *arg);
  
+   vtkDataSetSurfaceFilter *GeometryExtractor;
+ 
  private:
    vtkOSPRayPolyDataMapperNode(const vtkOSPRayPolyDataMapperNode&) = delete;
    void operator=(const vtkOSPRayPolyDataMapperNode&) = delete;
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOSPRayPolyDataMapperNode failed."
      return 1
    fi

    patch -p0 << \EOF
diff -c Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.cxx.original Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.cxx
*** Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.cxx.original   Fri Dec 22 08:33:25 2017
--- Rendering/OSPRay/vtkOSPRayPolyDataMapperNode.cxx    Fri Dec 28 16:32:06 2018
***************
*** 19,24 ****
--- 19,25 ----
  #include "vtkOSPRayMaterialHelpers.h"
  #include "vtkOSPRayRendererNode.h"
  #include "vtkDataArray.h"
+ #include "vtkDataSetSurfaceFilter.h"
  #include "vtkFloatArray.h"
  #include "vtkImageData.h"
  #include "vtkInformation.h"
***************
*** 738,749 ****
--- 739,755 ----
  vtkOSPRayPolyDataMapperNode::vtkOSPRayPolyDataMapperNode()
  {
    this->OSPMeshes = nullptr;
+   this->GeometryExtractor = nullptr;
  }
  
  //----------------------------------------------------------------------------
  vtkOSPRayPolyDataMapperNode::~vtkOSPRayPolyDataMapperNode()
  {
    delete (vtkosp::MyGeom*)this->OSPMeshes;
+   if ( this->GeometryExtractor )
+   {
+     this->GeometryExtractor->Delete();
+   }
  }
  
  //----------------------------------------------------------------------------
***************
*** 1318,1324 ****
      vtkMapper *mapper = act->GetMapper();
      if (mapper)
      {
!       poly = (vtkPolyData*)(mapper->GetInput());
      }
      if (poly)
      {
--- 1324,1343 ----
      vtkMapper *mapper = act->GetMapper();
      if (mapper)
      {
!       if (mapper->GetInput()->GetDataObjectType() == VTK_POLY_DATA)
!       {
!         poly = (vtkPolyData*)(mapper->GetInput());
!       }
!       else
!       {
!         if (! this->GeometryExtractor)
!         {
!           this->GeometryExtractor = vtkDataSetSurfaceFilter::New();
!         }
!         this->GeometryExtractor->SetInputData(mapper->GetInput());
!         this->GeometryExtractor->Update();
!         poly = (vtkPolyData*)this->GeometryExtractor->GetOutput();
!       }
      }
      if (poly)
      {
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOSPRayPolyDataMapperNode failed."
      return 1
    fi

    return 0;
}


function apply_vtkospray_linking_patch
{
    # fix from kevin griffin
    # patch to vtk linking issue noticed on macOS
    patch -p0 << \EOF
    diff -c Rendering/OSPRay/CMakeLists.txt.orig  Rendering/OSPRay/CMakeLists.txt
    *** Rendering/OSPRay/CMakeLists.txt.orig	2019-05-21 15:15:50.000000000 -0700
    --- Rendering/OSPRay/CMakeLists.txt	2019-05-21 15:16:07.000000000 -0700
    ***************
    *** 37,42 ****
    --- 37,45 ----
      vtk_module_library(vtkRenderingOSPRay ${Module_SRCS})
  
      target_link_libraries(${vtk-module} LINK_PUBLIC ${OSPRAY_LIBRARIES})
    + # patch to solve linking issue noticed on macOS
    + target_link_libraries(${vtk-module} LINK_PUBLIC vtkFiltersGeometry)
    + 
  
      # OSPRay_Core uses MMTime which is in it's own special library.
      if(WIN32)
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for ospray linking failed."
      return 1
    fi

    return 0;
}

function apply_vtk_python3_python_args_patch
{
    # in python 3.7.5:
    #  PyUnicode_AsUTF8 returns a const char *, which you cannot assign
    #  to a char *. 
    # Add cast to allow us to compile.
    patch -p0 << \EOF
    diff -c Wrapping/PythonCore/vtkPythonArgs.orig.cxx Wrapping/PythonCore/vtkPythonArgs.cxx
    *** Wrapping/PythonCore/vtkPythonArgs.orig.cxx	2020-03-24 14:29:39.000000000 -0700
    --- Wrapping/PythonCore/vtkPythonArgs.cxx	2020-03-24 14:29:52.000000000 -0700
    ***************
    *** 102,108 ****
        else if (PyUnicode_Check(o))
        {
      #if PY_VERSION_HEX >= 0x03030000
    !     a = PyUnicode_AsUTF8(o);
          return true;
      #else
          PyObject *s = _PyUnicode_AsDefaultEncodedString(o, nullptr);
    --- 102,108 ----
        else if (PyUnicode_Check(o))
        {
      #if PY_VERSION_HEX >= 0x03030000
    !     a = (char*)PyUnicode_AsUTF8(o);
          return true;
      #else
          PyObject *s = _PyUnicode_AsDefaultEncodedString(o, nullptr);
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for python3 vtkPythonArgs const issue failed."
      return 1
    fi

    return 0;
}




function apply_vtk_patch
{
    apply_vtkprobeopenglversion_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtkopengloptions_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtkopenfoamreader_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    # will have to test if these or versions thereof are still required
    #apply_vtkopenglspheremapper_h_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    #apply_vtkopenglspheremapper_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    # Note: don't guard ospray patches by if ospray is selected 
    # b/c subsequent calls to build_visit won't get a chance to patch
    # given the if test logic used above
    #apply_vtkospraypolydatamappernode_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    #apply_vtkospray_patches
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    #apply_vtkospray_linking_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    apply_vtk_python3_python_args_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}

function build_vtk
{
    # Extract the sources
    if [[ -d $VTK_BUILD_DIR ]] ; then
        if [[ ! -f $VTK_FILE ]] ; then
            warn "The directory VTK exists, deleting before uncompressing"
            rm -Rf $VTK_BUILD_DIR
            ensure_built_or_ready $VTK_INSTALL_DIR    $VTK_VERSION    $VTK_BUILD_DIR    $VTK_FILE
        fi
    fi

    #
    # Prepare the build dir using src file.
    #
    prepare_build_dir $VTK_BUILD_DIR $VTK_FILE
    untarred_vtk=$?
    # 0, already exists, 1 untarred src, 2 error

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
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    # move back up to the start dir
    cd "$START_DIR"

    #
    # Configure VTK
    #
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

    # Use Mesa as GL?
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
        vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING=\"${MESAGL_OPENGL_LIB};${LLVM_LIB}\""
        vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH=${MESAGL_GLU_LIB}"
        vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
        vopts="${vopts} -DOSMESA_LIBRARY:STRING=\"${MESAGL_OSMESA_LIB};${LLVM_LIB}\""
        vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"

        if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
                vopts="${vopts} -DVTK_USE_X:BOOL=OFF"
            fi
        fi
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
    vopts="${vopts} -DVTK_LEGACY_REMOVE:BOOL=true"
    vopts="${vopts} -DVTK_BUILD_TESTING:BOOL=false"
    vopts="${vopts} -DBUILD_TESTING:BOOL=false"
    vopts="${vopts} -DVTK_BUILD_DOCUMENTATION:BOOL=false"
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"
    # setting this to true causes errors when building debug versions of visit.
    vopts="${vopts} -DVTK_REPORT_OPENGL_ERRORS:BOOL=false"
    if test "${OPSYS}" = "Darwin" ; then
        vopts="${vopts} -DVTK_USE_COCOA:BOOL=ON"
        vopts="${vopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${vtk_inst_path}/lib"
        if test "${MACOSX_DEPLOYMENT_TARGET}" = "10.10"; then
            # If building on 10.10 (Yosemite) check if we are building with Xcode 7 ...
            XCODE_VER=$(xcodebuild -version | head -n 1 | awk '{print $2}')
            if test ${XCODE_VER%.*} == 7; then
                # Workaround for Xcode 7 not having a 10.10 SDK: Prevent CMake from linking to 10.11 SDK
                # by using Frameworks installed in root directory.
                echo "Xcode 7 on MacOS 10.10 detected: Enabling CMake workaround"
                vopts="${vopts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=\"\" -DCMAKE_OSX_SYSROOT:STRING=/"
            fi
        elif test "${MACOSX_DEPLOYMENT_TARGET}" = "10.12"; then
            # If building on 10.12 (Sierra) check if we are building with Xcode 9 ...
            XCODE_VER=$(xcodebuild -version | head -n 1 | awk '{print $2}')
            if test ${XCODE_VER%.*} == 9; then
                # Workaround for Xcode 9 not having a 10.12 SDK: Prevent CMake from linking to 10.13 SDK
                # by using Frameworks installed in root directory.
                echo "Xcode 9 on MacOS 10.12 detected: Enabling CMake workaround"
                vopts="${vopts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=\"\" -DCMAKE_OSX_SYSROOT:STRING=/"
            fi
        fi
    fi

    # allow VisIt to override any of vtk's classes
    vopts="${vopts} -DVTK_ALL_NEW_OBJECT_FACTORY:BOOL=true"
    # disable downloads (also disables testing)
    vopts="${vopts} -DVTK_FORBID_DOWNLOADS:BOOL=true"

    # Turn off module groups
    vopts="${vopts} -DVTK_GROUP_ENABLE_Imaging:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_MPI:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Qt:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_StandAlone:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Views:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Web:STRING=DONT_WANT"

    # Turn on individual modules. dependent modules are turned on automatically
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_CommonCore:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersFlowPaths:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersHybrid:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersModeling:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GeovisCore:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOEnSight:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOGeometry:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOLegacy:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOPLY:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOXML:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_InteractionStyle:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingAnnotation:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingFreeType:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingVolumeOpenGL2:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_libxml2:STRING=YES"

    # Tell VTK where to locate qmake if we're building graphical support. We
    # do not add graphical support for server-only builds.
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then
                vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GUISupportQt:BOOL:STRING=YES"
                vopts="${vopts} -DQt5_DIR:FILEPATH=${QT_INSTALL_DIR}/lib/cmake/Qt5"
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
            vopts="${vopts} -DVTK_PYTHON_VERSION:STRING=2"
            vopts="${vopts} -DPython2_EXECUTABLE:FILEPATH=${py}"
            #vopts="${vopts} -DPYTHON_EXTRA_LIBS:STRING=${VTK_PY_LIBS}"
            vopts="${vopts} -DPython2_INCLUDE_DIR:PATH=${pyinc}"
            vopts="${vopts} -DPython2_LIBRARY:FILEPATH=${pylib}"
            #            vopts="${vopts} -DPYTHON_UTIL_LIBRARY:FILEPATH="
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi


    # Use OSPRay?
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:BOOL=ON"
        vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}"
        vopts="${vopts} -Dembree_DIR=${EMBREE_INSTALL_DIR}"
    fi

    # zlib support, use the one we build
    vopts="${vopts} -DVTK_MODULE_USE_EXTERNAL_VTK_zlib:BOOL=ON"
    vopts="${vopts} -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}"
    if [[ "$VISIT_BUILD_MODE" == "Release" ]] ; then
        vopts="${vopts} -DZLIB_LIBRARY_RELEASE:FILEPATH=${ZLIB_LIBRARY}"
    else
        vopts="${vopts} -DZLIB_LIBRARY_DEBUG:FILEPATH=${ZLIB_LIBRARY}"
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
