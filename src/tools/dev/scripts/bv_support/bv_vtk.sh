function bv_vtk_initialize
{
    info "bv_vtk_initialize"
    export DO_VTK="yes"
    export FORCE_VTK="no"
    export USE_SYSTEM_VTK="no"
    add_extra_commandline_args "vtk" "system-vtk" 0 "Using system VTK (exp)"
    add_extra_commandline_args "vtk" "alt-vtk-dir" 1 "Use alternate VTK (exp)"
}

function bv_vtk_enable
{
    info "bv_vtk_enable"
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
    elif [[ "$DO_OSMESA" == "yes" ]]; then
        depends_on="${depends_on} osmesa"
    fi

    if [[ "$DO_OSPRAY" == "yes" ]]; then
        depends_on="${depends_on} ospray"
    fi

    # Only depend on Qt if we're not doing server-only builds.
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then
                if [[ "$DO_QT6" == "yes" ]]; then
                    depends_on="${depends_on} qt6"
                else
                    depends_on="${depends_on} qt"
                fi
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
    info "bv_vtk_info"
    export VTK_FILE=${VTK_FILE:-"VTK-9.2.6.tar.gz"}
    export VTK_VERSION=${VTK_VERSION:-"9.2.6"}
    export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"9.2"}
    export VTK_SHA256_CHECKSUM="06fc8d49c4e56f498c40fcb38a563ed8d4ec31358d0101e8988f0bb4d539dd12"
    export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
    export VTK_URL=${VTK_URL:-"http://www.vtk.org/files/release/${VTK_SHORT_VERSION}"}
    export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK-${VTK_VERSION}"}
    export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
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

# *************************************************************************** #
#                            Function 6, build_vtk                            #
# *************************************************************************** #
function apply_vtk9_allow_onscreen_and_osmesa_patch
{
  # patch that allows VTK9 to be built with onscreen and osmesa support.
   patch -p0 << \EOF
--- CMake/vtkOpenGLOptions.cmake.orig	2023-10-26 15:53:06.351818000 -0700
+++ CMake/vtkOpenGLOptions.cmake	2023-10-26 16:09:34.984852000 -0700
@@ -112,15 +112,6 @@
     "Please set to `OFF` any of these two.")
 endif ()

-if (VTK_OPENGL_HAS_OSMESA AND VTK_CAN_DO_ONSCREEN)
-  message(FATAL_ERROR
-    "The `VTK_OPENGL_HAS_OSMESA` can't be set to `ON` if any of the following is true: "
-    "the target platform is Windows, `VTK_USE_COCOA` is `ON`, or `VTK_USE_X` "
-    "is `ON` or `VTK_USE_SDL2` is `ON`. OSMesa does not support on-screen "
-    "rendering and VTK's OpenGL selection is at build time, so the current "
-    "build configuration is not satisfiable.")
-endif ()
-
 cmake_dependent_option(
   VTK_USE_OPENGL_DELAYED_LOAD
   "Use delay loading for OpenGL"
EOF

   patch -p0 << \EOF
--- Utilities/OpenGL/CMakeLists.txt.orig	2023-10-26 15:56:37.290225000 -0700
+++ Utilities/OpenGL/CMakeLists.txt	2023-10-26 16:12:18.817101000 -0700
@@ -45,7 +45,7 @@
   list(APPEND opengl_targets OpenGL::EGL)
 endif ()

-if (VTK_OPENGL_HAS_OSMESA AND NOT VTK_CAN_DO_ONSCREEN)
+if (VTK_OPENGL_HAS_OSMESA)
   vtk_module_third_party_external(
     PACKAGE OSMesa
     TARGETS OSMesa::OSMesa)
EOF

   patch -p0 << \EOF
--- Rendering/OpenGL2/CMakeLists.txt.orig	2023-10-26 15:57:15.850399000 -0700
+++ Rendering/OpenGL2/CMakeLists.txt	2023-10-26 15:57:31.520455000 -0700
@@ -350,8 +350,11 @@
     PUBLIC SDL2::SDL2)
 endif ()

+# Not sure if find_package is necessary, should it be vtk_module_find_package?
+find_package(OpenGL REQUIRED)
 if (VTK_USE_X)
   vtk_module_find_package(PACKAGE X11)
+  vtk_module_link(VTK::RenderingOpenGL2 PRIVATE ${OPENGL_gl_LIBRARY})
   vtk_module_link(VTK::RenderingOpenGL2 PUBLIC X11::X11)
   if (TARGET X11::Xcursor)
     vtk_module_link(VTK::RenderingOpenGL2 PRIVATE X11::Xcursor)
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch allowing onscreen and osmesa failed."
      return 1
    fi
    return 0;
}

function apply_vtk9_vtkdatawriter_patch
{
  # patch vtkDataWriter to fix a bug when writing a vtkBitArray
   patch -p0 << \EOF
--- IO/Legacy/vtkDataWriter.cxx.orig      2023-06-28 10:10:35.329318000 -0700
+++ IO/Legacy/vtkDataWriter.cxx   2023-06-28 10:10:55.904372000 -0700
@@ -1136,7 +1136,7 @@
       else
       {
         unsigned char* cptr = static_cast<vtkBitArray*>(data)->GetPointer(0);
-        fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num - 1) / 8 + 1));
+        fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num*numComp - 1) / 8 + 1));
       }
       *fp << "\n";
     }

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkDataWriter.cxx failed."
      return 1
    fi
    return 0;

}

function apply_vtk9_vtkopenfoamreader_header_patch
{
  # patch vtk's OpenFOAMReader to provide more meta data information
  # useful for VisIt's plugin.

   patch -p0 << \EOF
*** IO/Geometry/vtkOpenFOAMReader.h.orig        Mon Apr 19 13:46:49 2021
--- IO/Geometry/vtkOpenFOAMReader.h     Mon Apr 19 13:50:25 2021
***************
*** 50,55 ****
--- 50,60 ----
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
*** 350,355 ****
--- 355,382 ----

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
*** 425,430 ****
--- 452,467 ----
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

function apply_vtk9_vtkopenfoamreader_source_patch
{
  # patch vtk's OpenFOAMReader to provide more meta data information
  # useful for VisIt's plugin.

   patch -p0 << \EOF
*** IO/Geometry/vtkOpenFOAMReader.cxx   Thu Apr  8 05:26:53 2021
--- IO/Geometry/vtkOpenFOAMReader.cxx   Tue Apr 20 16:35:35 2021
***************
*** 5058,5063 ****
--- 5058,5077 ----
      if (io.Open(tempPath + "/" + fieldFile)) // file exists and readable
      {
        this->AddFieldName(fieldFile, io.GetClassName(), isLagrangian);
+       // added by LLNL
+       if(isLagrangian)
+       {
+         this->Parent->LagrangianArrayClassName[io.GetObjectName()] = io.GetClassName();
+       }
+       else
+       {
+         if(io.GetClassName().substr(0,3) == "vol")
+           this->Parent->CellArrayClassName[io.GetObjectName()] = io.GetClassName();
+         else
+           this->Parent->PointArrayClassName[io.GetObjectName()] = io.GetClassName();
+       }
+       // end added by LLNL
+
        io.Close();
      }
    }
***************
*** 5326,5331 ****
--- 5340,5389 ----
      }
    }

+   // added by LLNL
+   if (this->Parent->GetReadZones())
+   {
+     auto dictPtr(this->GatherBlocks("pointZones", true));
+     if (dictPtr != nullptr)
+     {
+       this->Parent->PointZones.clear();
+       vtkFoamDict &pointZoneDict = *dictPtr;
+       int nPointZones = static_cast<int>(pointZoneDict.size());
+
+       for (int i = 0; i < nPointZones; i++)
+       {
+         this->Parent->PointZones.push_back(pointZoneDict[i]->GetKeyword().c_str());
+       }
+     }
+
+     dictPtr= this->GatherBlocks("faceZones", true);
+     if (dictPtr!= NULL)
+     {
+       this->Parent->FaceZones.clear();
+       vtkFoamDict &faceZoneDict = *dictPtr;
+       int nFaceZones = static_cast<int>(faceZoneDict.size());
+
+       for (int i = 0; i < nFaceZones; i++)
+       {
+         this->Parent->FaceZones.push_back(faceZoneDict[i]->GetKeyword().c_str());
+       }
+     }
+
+     dictPtr= this->GatherBlocks("cellZones", true);
+     if (dictPtr!= NULL)
+     {
+       this->Parent->CellZones.clear();
+       vtkFoamDict &cellZoneDict = *dictPtr;
+       int nCellZones = static_cast<int>(cellZoneDict.size());
+
+       for (int i = 0; i < nCellZones; i++)
+       {
+         this->Parent->CellZones.push_back(cellZoneDict[i]->GetKeyword().c_str());
+       }
+     }
+   }
+   // end added by LLNL
+
    // Add scalars and vectors to metadata
    vtkStdString timePath(this->CurrentTimePath());
    // do not do "RemoveAllArrays()" to accumulate array selections
***************
*** 9990,9992 ****
--- 10048,10166 ----
      (static_cast<double>(this->Parent->CurrentReaderIndex) + amount) /
      static_cast<double>(this->Parent->NumberOfReaders));
  }
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

function apply_vtk9_vtkopenfoamreader_patch
{
    apply_vtk9_vtkopenfoamreader_header_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi
    apply_vtk9_vtkopenfoamreader_source_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_vtk9_vtkospray_patches
{
    count_patches=4
    # patch vtkOSPRay files:

    # 1) expose vtkViewNodeFactory via vtkOSPRayPass.h
    current_patch=1
    patch -p0 << \EOF
*** Rendering/RayTracing/vtkOSPRayPass_orig.h   2021-04-29 17:14:23.000000000 -0600
--- Rendering/RayTracing/vtkOSPRayPass.h        2021-04-29 17:16:08.000000000 -0600
*************** class vtkOverlayPass;
*** 50,55 ****
--- 50,56 ----
  class vtkRenderPassCollection;
  class vtkSequencePass;
  class vtkVolumetricPass;
+ class vtkViewNodeFactory;

  class VTKRENDERINGRAYTRACING_EXPORT vtkOSPRayPass : public vtkRenderPass
  {
*************** public:
*** 76,81 ****
--- 77,87 ----
     */
    virtual void RenderInternal(const vtkRenderState* s);

+   /**
+    * Called by VisIt
+    */
+   virtual vtkViewNodeFactory* GetViewNodeFactory();
+
    ///@{
    /**
     * Wrapper around ospray's init and shutdown that protect
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch ${current_patch}/${count_patches} for vtkOSPRayPass.h failed."
        return 1
    fi

    # 2) expose vtkViewNodeFactory via vtkOSPRayPass.cxx
    ((current_patch++))
    patch -p0 << \EOF
*** Rendering/RayTracing/vtkOSPRayPass_orig.cxx 2021-04-29 17:17:02.000000000 -0600
--- Rendering/RayTracing/vtkOSPRayPass.cxx      2021-04-29 17:19:10.000000000 -0600
*************** void vtkOSPRayPass::RenderInternal(const
*** 430,435 ****
--- 430,441 ----
  }

  //------------------------------------------------------------------------------
+ vtkViewNodeFactory* vtkOSPRayPass::GetViewNodeFactory()
+ {
+   return this->Internal->Factory;
+ }
+
+ //------------------------------------------------------------------------------
  bool vtkOSPRayPass::IsSupported()
  {
    static bool detected = false;
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch ${current_patch}/${count_patches} for vtkOSPRayPass.cxx failed."
        return 1
    fi

    # 3) Set the samples in the VolumeMapper
    ((current_patch++))
    patch -p0 << \EOF
*** Rendering/RayTracing/vtkOSPRayVolumeMapper_orig.cxx 2021-05-03 09:40:09.000000000 -0600
--- Rendering/RayTracing/vtkOSPRayVolumeMapper.cxx      2021-05-03 09:41:03.000000000 -0600
*************** void vtkOSPRayVolumeMapper::Render(vtkRe
*** 72,77 ****
--- 72,81 ----
    {
      this->Init();
    }
+   vtkOSPRayRendererNode::SetSamplesPerPixel(
+     vtkOSPRayRendererNode::GetSamplesPerPixel(ren), this->InternalRenderer);
+   vtkOSPRayRendererNode::SetAmbientSamples(
+     vtkOSPRayRendererNode::GetAmbientSamples(ren), this->InternalRenderer);
    this->InternalRenderer->SetRenderWindow(ren->GetRenderWindow());
    this->InternalRenderer->SetActiveCamera(ren->GetActiveCamera());
    this->InternalRenderer->SetBackground(ren->GetBackground());
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch $current_patch/$count_patches for vtkOSPRayVolumeMapper.cxx failed."
        return 1
    fi

    # 4) Bump to OSPRay 3
    ((current_patch++))
    patch -p1 << \EOF
From ba568f70bb36cbe9535268a63f5efc0eddc71c8a Mon Sep 17 00:00:00 2001
From: "David E. DeMarle" <david.demarle@intel.com>
Date: Sat, 21 Oct 2023 17:18:40 -0400
Subject: [PATCH] bump VTK 9.1.0 to OSPRay 3.0.0

---
 Rendering/RayTracing/CMakeLists.txt           |  2 +-
 Rendering/RayTracing/RTWrapper/Backend.h      |  5 ++-
 .../RTWrapper/OSPRay/OSPRayBackend.h          | 25 +++++++++-----
 Rendering/RayTracing/RTWrapper/RTWrapper.h    |  3 ++
 .../RayTracing/vtkOSPRayMaterialHelpers.cxx   |  9 +++--
 .../vtkOSPRayMoleculeMapperNode.cxx           |  8 ++---
 .../vtkOSPRayPolyDataMapperNode.cxx           | 34 ++++++++++---------
 .../vtkOSPRayUnstructuredVolumeMapperNode.cxx |  2 +-
 .../RayTracing/vtkOSPRayVolumeMapperNode.cxx  | 24 +++++++++++--
 .../RayTracing/vtkOSPRayVolumeMapperNode.h    |  1 +
 10 files changed, 74 insertions(+), 39 deletions(-)

diff --git a/Rendering/RayTracing/CMakeLists.txt b/Rendering/RayTracing/CMakeLists.txt
index fdaccd6e56..6b5edc264b 100644
--- a/Rendering/RayTracing/CMakeLists.txt
+++ b/Rendering/RayTracing/CMakeLists.txt
@@ -56,7 +56,7 @@ vtk_module_add_module(VTK::RenderingRayTracing
 if (VTK_ENABLE_OSPRAY)
   vtk_module_find_package(
     PACKAGE ospray
-    VERSION 2.1)
+    VERSION 3.0)

   vtk_module_link(VTK::RenderingRayTracing
     PUBLIC
diff --git a/Rendering/RayTracing/RTWrapper/Backend.h b/Rendering/RayTracing/RTWrapper/Backend.h
index ec0a9779f6..3dbea9350b 100644
--- a/Rendering/RayTracing/RTWrapper/Backend.h
+++ b/Rendering/RayTracing/RTWrapper/Backend.h
@@ -27,7 +27,7 @@ namespace RTW
         virtual RTWGroup NewGroup() = 0;
         virtual RTWTexture NewTexture(const char* type) = 0;
         virtual RTWLight NewLight(const char *light_type) = 0;
-        virtual RTWMaterial NewMaterial(const char *renderer_type, const char *material_type) = 0;
+        virtual RTWMaterial NewMaterial(const char *material_type) = 0;
         virtual RTWVolume NewVolume(const char *type) = 0;
         virtual RTWVolumetricModel NewVolumetricModel(RTWVolume volume) = 0;
         virtual RTWTransferFunction NewTransferFunction(const char *type) = 0;
@@ -46,13 +46,16 @@ namespace RTW
         virtual void SetObjectAsData(RTWObject target, const char *id, RTWDataType type, RTWObject obj) = 0;
         virtual void SetParam(RTWObject, const char *id, RTWDataType type, const void* mem) = 0;
         virtual void SetBool(RTWObject, const char *id, bool x) = 0;
+        virtual void SetBox1f(RTWObject, const char *id, float x, float y) = 0;
         virtual void SetInt(RTWObject, const char *id, int32_t x) = 0;
+        virtual void SetUInt(RTWObject, const char *id, uint32_t x) = 0;
         virtual void SetVec2i(RTWObject, const char *id, int32_t x, int32_t y) = 0;
         virtual void SetFloat(RTWObject, const char *id, float x) = 0;
         virtual void SetVec2f(RTWObject, const char *id, float x, float y) = 0;
         virtual void SetVec3i(RTWObject, const char *id, int x, int y, int z) = 0;
         virtual void SetVec3f(RTWObject, const char *id, float x, float y, float z) = 0;
         virtual void SetVec4f(RTWObject, const char *id, float x, float y, float z, float w) = 0;
+        virtual void SetLinear2f(RTWObject, const char *id, float x, float y, float z, float w) = 0;

         virtual void RemoveParam(RTWObject, const char *id) = 0;

diff --git a/Rendering/RayTracing/RTWrapper/OSPRay/OSPRayBackend.h b/Rendering/RayTracing/RTWrapper/OSPRay/OSPRayBackend.h
index 11e97c06ed..03d2622c0a 100644
--- a/Rendering/RayTracing/RTWrapper/OSPRay/OSPRayBackend.h
+++ b/Rendering/RayTracing/RTWrapper/OSPRay/OSPRayBackend.h
@@ -81,15 +81,9 @@ namespace RTW
         {
           std::runtime_error("OSPRay device could not be fetched!");
         }
-#if OSPRAY_VERSION_MINOR > 1
         ospDeviceSetErrorCallback(device, [](void *, OSPError, const char *errorDetails) {
           std::cerr << "OSPRay ERROR: " << errorDetails << std::endl;
         }, nullptr);
-#else
-        ospDeviceSetErrorFunc(device, [](OSPError, const char *errorDetails) {
-          std::cerr << "OSPRay ERROR: " << errorDetails << std::endl;
-        });
-#endif
         once = true;
       }
       return ret;
@@ -196,9 +190,9 @@ namespace RTW
       return reinterpret_cast<RTWLight>(ospNewLight(light_type));
     }

-    RTWMaterial NewMaterial(const char *renderer_type, const char *material_type) override
+    RTWMaterial NewMaterial(const char *material_type) override
     {
-      return reinterpret_cast<RTWMaterial>(ospNewMaterial(renderer_type, material_type));
+      return reinterpret_cast<RTWMaterial>(ospNewMaterial(material_type));
     }

     RTWVolume NewVolume(const char *type) override
@@ -277,6 +271,11 @@ namespace RTW
       ospSetInt(reinterpret_cast<OSPObject>(object), id, x);
     }

+    void SetUInt(RTWObject object, const char *id, uint32_t x) override
+    {
+      ospSetUInt(reinterpret_cast<OSPObject>(object), id, x);
+    }
+
     void SetBool(RTWObject object, const char *id, bool x) override
     {
       ospSetBool(reinterpret_cast<OSPObject>(object), id, x);
@@ -287,6 +286,16 @@ namespace RTW
       ospSetFloat(reinterpret_cast<OSPObject>(object), id, x);
     }

+    void SetLinear2f(RTWObject object, const char *id, float x, float y, float z, float w) override
+    {
+      ospSetLinear2f(reinterpret_cast<OSPObject>(object), id, x, y, z, w);
+    }
+
+    void SetBox1f(RTWObject object, const char *id, float x, float y) override
+    {
+      ospSetBox1f(reinterpret_cast<OSPObject>(object), id, x, y);
+    }
+
     void SetVec2f(RTWObject object, const char *id, float x, float y) override
     {
       ospSetVec2f(reinterpret_cast<OSPObject>(object), id, x, y);
diff --git a/Rendering/RayTracing/RTWrapper/RTWrapper.h b/Rendering/RayTracing/RTWrapper/RTWrapper.h
index 1310a6b7ec..877fea3e93 100644
--- a/Rendering/RayTracing/RTWrapper/RTWrapper.h
+++ b/Rendering/RayTracing/RTWrapper/RTWrapper.h
@@ -137,7 +137,10 @@ std::set<RTWBackendType> rtwGetAvailableBackends();

 #define ospSetFloat backend->SetFloat
 #define ospSetBool backend->SetBool
+#define ospSetBox1f backend->SetBox1f
 #define ospSetInt backend->SetInt
+#define ospSetLinear2f backend->SetLinear2f
+#define ospSetUInt backend->SetUInt
 #define ospSetVec2i backend->SetVec2i
 #define ospSetVec3i backend->SetVec3i
 #define ospSetVec2f backend->SetVec2f
diff --git a/Rendering/RayTracing/vtkOSPRayMaterialHelpers.cxx b/Rendering/RayTracing/vtkOSPRayMaterialHelpers.cxx
index 5988720713..883a15beca 100644
--- a/Rendering/RayTracing/vtkOSPRayMaterialHelpers.cxx
+++ b/Rendering/RayTracing/vtkOSPRayMaterialHelpers.cxx
@@ -79,10 +79,10 @@ OSPTexture vtkOSPRayMaterialHelpers::NewTexture2D(RTW::Backend* backend, const o
   ospSetObject(texture, "data", data_handle);
   ospRelease(data_handle);

-  ospSetInt(texture, "format", static_cast<int>(type));
+  ospSetUInt(texture, "format", type);
   if (flags & OSP_TEXTURE_FILTER_NEAREST)
   {
-    ospSetInt(texture, "filter", OSP_TEXTURE_FILTER_NEAREST);
+    ospSetUInt(texture, "filter", OSP_TEXTURE_FILTER_NEAREST);
   }
   ospCommit(texture);

@@ -374,14 +374,13 @@ OSPMaterial vtkOSPRayMaterialHelpers::NewMaterial(
     return result;

   (void)oRenderer;
-  const std::string rendererType = vtkOSPRayRendererNode::GetRendererType(orn->GetRenderer());
-  result = ospNewMaterial(rendererType.c_str(), ospMatName.c_str());
+  result = ospNewMaterial(ospMatName.c_str());

   if (!result)
   {
     vtkGenericWarningMacro(
       "OSPRay failed to create material: " << ospMatName << ". Trying obj instead.");
-    result = ospNewMaterial(rendererType.c_str(), "obj");
+    result = ospNewMaterial("obj");
   }

   ospCommit(result);
diff --git a/Rendering/RayTracing/vtkOSPRayMoleculeMapperNode.cxx b/Rendering/RayTracing/vtkOSPRayMoleculeMapperNode.cxx
index ade942af43..eee3da5460 100644
--- a/Rendering/RayTracing/vtkOSPRayMoleculeMapperNode.cxx
+++ b/Rendering/RayTracing/vtkOSPRayMoleculeMapperNode.cxx
@@ -303,8 +303,8 @@ void vtkOSPRayMoleculeMapperNode::Render(bool prepass)
         ospRelease(oMaterial);
       }

-      ospSetInt(bonds, "type", OSP_ROUND);
-      ospSetInt(bonds, "basis", OSP_BEZIER);
+      ospSetUInt(bonds, "type", OSP_ROUND);
+      ospSetUInt(bonds, "basis", OSP_BEZIER);

       this->GeometricModels.emplace_back(bondsModel);
       ospCommit(bonds);
@@ -405,8 +405,8 @@ void vtkOSPRayMoleculeMapperNode::Render(bool prepass)
       ocolor[3] = opacity;
       ospSetVec3f(latticeModel, "color", ocolor[0], ocolor[1], ocolor[2]);

-      ospSetInt(lattice, "type", OSP_ROUND);
-      ospSetInt(lattice, "basis", OSP_LINEAR);
+      ospSetUInt(lattice, "type", OSP_ROUND);
+      ospSetUInt(lattice, "basis", OSP_LINEAR);

       this->GeometricModels.emplace_back(latticeModel);
       ospCommit(lattice);
diff --git a/Rendering/RayTracing/vtkOSPRayPolyDataMapperNode.cxx b/Rendering/RayTracing/vtkOSPRayPolyDataMapperNode.cxx
index 4d6e0333de..616460940f 100644
--- a/Rendering/RayTracing/vtkOSPRayPolyDataMapperNode.cxx
+++ b/Rendering/RayTracing/vtkOSPRayPolyDataMapperNode.cxx
@@ -315,8 +315,8 @@ OSPGeometricModel RenderAsCylinders(std::vector<osp::vec3f>& vertices,
     _mdata = ospNewCopyData1D(mdata.data(), OSP_VEC4F, mdata.size());
     ospCommit(_mdata);
     ospSetObject(ospMesh, "vertex.position_radius", _mdata);
-    ospSetInt(ospMesh, "type", OSP_ROUND);
-    ospSetInt(ospMesh, "basis", OSP_BEZIER);
+    ospSetUInt(ospMesh, "type", OSP_ROUND);
+    ospSetUInt(ospMesh, "basis", OSP_BEZIER);
   }
   else
   {
@@ -330,8 +330,8 @@ OSPGeometricModel RenderAsCylinders(std::vector<osp::vec3f>& vertices,
     ospCommit(_mdata);
     ospSetObject(ospMesh, "vertex.position", _mdata);
     ospSetFloat(ospMesh, "radius", lineWidth);
-    ospSetInt(ospMesh, "type", OSP_ROUND);
-    ospSetInt(ospMesh, "basis", OSP_LINEAR);
+    ospSetUInt(ospMesh, "type", OSP_ROUND);
+    ospSetUInt(ospMesh, "basis", OSP_LINEAR);
   }

   std::vector<unsigned int> indices;
@@ -541,13 +541,13 @@ OSPGeometricModel RenderAsTriangles(OSPData vertices, std::vector<unsigned int>&
       if (interpolationType == VTK_PBR)
       {
         ospSetObject(actorMaterial, "map_normal", t2d);
-        ospSetVec4f(actorMaterial, "map_normal.transform", textureTransform.x, textureTransform.y,
-          textureTransform.z, textureTransform.w);
+        ospSetLinear2f(actorMaterial, "map_normal.transform", textureTransform.x,
+          textureTransform.y, textureTransform.z, textureTransform.w);
       }
       else
       {
         ospSetObject(actorMaterial, "map_Bump", t2d);
-        ospSetVec4f(actorMaterial, "map_Bump.transform", textureTransform.x, textureTransform.y,
+        ospSetLinear2f(actorMaterial, "map_Bump.transform", textureTransform.x, textureTransform.y,
           textureTransform.z, textureTransform.w);
       }
       ospCommit(actorMaterial);
@@ -573,13 +573,13 @@ OSPGeometricModel RenderAsTriangles(OSPData vertices, std::vector<unsigned int>&

         OSPTexture t2dR = vtkOSPRayMaterialHelpers::VTKToOSPTexture(backend, vRoughnessTextureMap);
         ospSetObject(actorMaterial, "map_roughness", t2dR);
-        ospSetVec4f(actorMaterial, "map_roughness.transform", textureTransform.x,
+        ospSetLinear2f(actorMaterial, "map_roughness.transform", textureTransform.x,
           textureTransform.y, textureTransform.z, textureTransform.w);

         OSPTexture t2dM = vtkOSPRayMaterialHelpers::VTKToOSPTexture(backend, vMetallicTextureMap);
         ospSetObject(actorMaterial, "map_metallic", t2dM);
-        ospSetVec4f(actorMaterial, "map_metallic.transform", textureTransform.x, textureTransform.y,
-          textureTransform.z, textureTransform.w);
+        ospSetLinear2f(actorMaterial, "map_metallic.transform", textureTransform.x,
+          textureTransform.y, textureTransform.z, textureTransform.w);

         ospCommit(actorMaterial);
         ospRelease(t2dR);
@@ -604,14 +604,14 @@ OSPGeometricModel RenderAsTriangles(OSPData vertices, std::vector<unsigned int>&
         OSPTexture t2dA =
           vtkOSPRayMaterialHelpers::VTKToOSPTexture(backend, vAnisotropyValueTextureMap);
         ospSetObject(actorMaterial, "map_anisotropy", t2dA);
-        ospSetVec4f(actorMaterial, "map_anisotropy.transform", textureTransform.x,
+        ospSetLinear2f(actorMaterial, "map_anisotropy.transform", textureTransform.x,
           textureTransform.y, textureTransform.z, textureTransform.w);

         OSPTexture t2dR =
           vtkOSPRayMaterialHelpers::VTKToOSPTexture(backend, vAnisotropyRotationTextureMap);
         ospSetObject(actorMaterial, "map_rotation", t2dR);
-        ospSetVec4f(actorMaterial, "map_rotation.transform", textureTransform.x, textureTransform.y,
-          textureTransform.z, textureTransform.w);
+        ospSetLinear2f(actorMaterial, "map_rotation.transform", textureTransform.x,
+          textureTransform.y, textureTransform.z, textureTransform.w);
         ospCommit(actorMaterial);
         ospRelease(t2dA);
         ospRelease(t2dR);
@@ -621,7 +621,7 @@ OSPGeometricModel RenderAsTriangles(OSPData vertices, std::vector<unsigned int>&
       {
         OSPTexture t2d = vtkOSPRayMaterialHelpers::VTKToOSPTexture(backend, vCoatNormalTextureMap);
         ospSetObject(actorMaterial, "map_coatNormal", t2d);
-        ospSetVec4f(actorMaterial, "map_coatNormal.transform", textureTransform.x,
+        ospSetLinear2f(actorMaterial, "map_coatNormal.transform", textureTransform.x,
           textureTransform.y, textureTransform.z, textureTransform.w);
         ospCommit(actorMaterial);
         ospRelease(t2d);
@@ -635,13 +635,13 @@ OSPGeometricModel RenderAsTriangles(OSPData vertices, std::vector<unsigned int>&
       if (interpolationType == VTK_PBR)
       {
         ospSetObject(actorMaterial, "map_baseColor", ((OSPTexture)(t2d)));
-        ospSetVec4f(actorMaterial, "map_baseColor.transform", textureTransform.x,
+        ospSetLinear2f(actorMaterial, "map_baseColor.transform", textureTransform.x,
           textureTransform.y, textureTransform.z, textureTransform.w);
       }
       else
       {
         ospSetObject(actorMaterial, "map_kd", ((OSPTexture)(t2d)));
-        ospSetVec4f(actorMaterial, "map_kd.transform", textureTransform.x, textureTransform.y,
+        ospSetLinear2f(actorMaterial, "map_kd.transform", textureTransform.x, textureTransform.y,
           textureTransform.z, textureTransform.w);
       }
       ospCommit(actorMaterial);
diff --git a/Rendering/RayTracing/vtkOSPRayUnstructuredVolumeMapperNode.cxx b/Rendering/RayTracing/vtkOSPRayUnstructuredVolumeMapperNode.cxx
index ae552773d4..d9dd14029d 100644
--- a/Rendering/RayTracing/vtkOSPRayUnstructuredVolumeMapperNode.cxx
+++ b/Rendering/RayTracing/vtkOSPRayUnstructuredVolumeMapperNode.cxx
@@ -344,7 +344,7 @@ void vtkOSPRayUnstructuredVolumeMapperNode::Render(bool prepass)
       ospSetObject(oTF, "color", colorData);
       OSPData tfAlphaData = ospNewCopyData1D(&tfOVals[0], OSP_FLOAT, NumColors);
       ospSetObject(oTF, "opacity", tfAlphaData);
-      ospSetVec2f(oTF, "valueRange", range[0], range[1]);
+      ospSetBox1f(oTF, "value", (float)range[0], (float)range[1]);
       ospCommit(oTF);

       ospRelease(colorData);
diff --git a/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.cxx b/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.cxx
index 354fb4e096..018ad907a7 100644
--- a/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.cxx
+++ b/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.cxx
@@ -311,7 +311,7 @@ void vtkOSPRayVolumeMapperNode::Render(bool prepass)
         ospCommit(isosurfaces);

         ospSetObject(OSPRayIsosurface, "isovalue", isosurfaces);
-        ospSetObject(OSPRayIsosurface, "volume", this->OSPRayVolumeModel);
+        ospSetObject(OSPRayIsosurface, "volume", this->OSPRayVolume);
         ospCommit(OSPRayIsosurface);
         ospRelease(isosurfaces);

@@ -319,12 +319,15 @@ void vtkOSPRayVolumeMapperNode::Render(bool prepass)
         OSPInstance instance = ospNewInstance(group);

         OSPGeometricModel OSPRayGeometricModel = ospNewGeometricModel(OSPRayIsosurface);
-
+        OSPData ospIsoColors = ospNewCopyData1D(this->IsoColors.data(), OSP_VEC4F, nbContours);
+        ospCommit(ospIsoColors);
+        ospSetObject(OSPRayGeometricModel, "color", ospIsoColors);
         OSPMaterial material =
           vtkOSPRayMaterialHelpers::NewMaterial(orn, orn->GetORenderer(), "obj");
         ospCommit(material);
         ospSetObjectAsData(OSPRayGeometricModel, "material", OSP_MATERIAL, material);
         ospCommit(OSPRayGeometricModel);
+        ospRelease(ospIsoColors);
         ospRelease(material);
         ospRelease(OSPRayIsosurface);

@@ -406,7 +409,7 @@ void vtkOSPRayVolumeMapperNode::UpdateTransferFunction(
   ospCommit(colorData);
   ospSetObject(this->TransferFunction, "color", colorData);

-  ospSetVec2f(this->TransferFunction, "valueRange", tfRange.x, tfRange.y);
+  ospSetBox1f(this->TransferFunction, "value", tfRange.x, tfRange.y);

   OSPData tfAlphaData = ospNewCopyData1D(&this->TFOVals[0], OSP_FLOAT, this->NumColors);
   ospCommit(tfAlphaData);
@@ -416,5 +419,20 @@ void vtkOSPRayVolumeMapperNode::UpdateTransferFunction(
   ospRelease(colorData);
   ospRelease(tfAlphaData);

+  vtkContourValues* contours = volProperty->GetIsoSurfaceValues();
+  this->IsoColors.clear();
+  if (contours)
+  {
+    double* p = contours->GetValues();
+    for (auto i = 0; i < contours->GetNumberOfContours(); ++i)
+    {
+      double* ncol = colorTF->GetColor(p[i]);
+      this->IsoColors.push_back(ncol[0]);
+      this->IsoColors.push_back(ncol[1]);
+      this->IsoColors.push_back(ncol[2]);
+      this->IsoColors.push_back(scalarTF->GetValue(p[i]));
+    }
+  }
+
   this->PropertyTime.Modified();
 }
diff --git a/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.h b/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.h
index 78b1d5fb66..e71c72716b 100644
--- a/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.h
+++ b/Rendering/RayTracing/vtkOSPRayVolumeMapperNode.h
@@ -79,6 +79,7 @@ protected:

   std::vector<float> TFVals;
   std::vector<float> TFOVals;
+  std::vector<float> IsoColors;

   vtkOSPRayCache<vtkOSPRayCacheItemObject>* Cache;

--
2.25.1


EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch $current_patch/$count_patches for vtkOSPRayVolumeMapper.cxx failed."
        return 1
    fi

}


function apply_vtk9_vtkRectilinearGridReader_patch
{
  # patch vtkRectilinearGridReader.cxx, per this issue:
  # https://gitlab.kitware.com/vtk/vtk/-/issues/18447
   patch -p0 << \EOF
*** IO/Legacy/vtkRectilinearGridReader.cxx.orig	Thu Jan 27 10:55:12 2022
--- IO/Legacy/vtkRectilinearGridReader.cxx	Thu Jan 27 11:01:04 2022
***************
*** 95,101 ****
          break;
        }
  
!       if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
        {
          int dim[3];
          if (!(this->Read(dim) && this->Read(dim + 1) && this->Read(dim + 2)))
--- 95,108 ----
          break;
        }
  
!       // Have to read field data because it may be binary.
!       if (!strncmp(this->LowerCase(line), "field", 5))
!       {
!         vtkFieldData* fd = this->ReadFieldData();
!         fd->Delete();
!       }
! 
!       else if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
        {
          int dim[3];
          if (!(this->Read(dim) && this->Read(dim + 1) && this->Read(dim + 2)))
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch for vtkRectilinearGridReader.cxx failed."
        return 1
    fi
}

function apply_vtk9_vtkCutter_patch
{
  # patch vtkCutter to remove use of vtk3DLinearPlaneCutter, because it 'promotes'
  # all cell and point data to float/double arrays. See VTK issue:
  # https://gitlab.kitware.com/vtk/vtk/-/issues/18450
   patch -p0 << \EOF
*** Filters/Core/vtkCutter.cxx.orig	Mon Jan 31 09:05:34 2022
--- Filters/Core/vtkCutter.cxx	Mon Jan 31 11:09:02 2022
***************
*** 381,386 ****
--- 381,389 ----
      // See if the input can be fully processed by the fast vtk3DLinearGridPlaneCutter.
      // This algorithm can provide a substantial speed improvement over the more general
      // algorithm for vtkUnstructuredGrids.
+ 
+ // Don't want to use 3DlinearGridPlaneCutter, it 'promotes' all cell and point data to float arrays
+ #if 0
      if (this->GetGenerateTriangles() && this->GetCutFunction() &&
        this->GetCutFunction()->IsA("vtkPlane") && this->GetNumberOfContours() == 1 &&
        this->GetGenerateCutScalars() == 0 &&
***************
*** 417,422 ****
--- 420,426 ----
  
        return retval;
      }
+ #endif
  
      vtkDebugMacro(<< "Executing Unstructured Grid Cutter");
      this->UnstructuredGridCutter(input, output);
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch for vtkCutter.cxx failed."
        return 1
    fi
}

function apply_vtk9_vtkgeotransform_patch
{
  # patch vtk's vtkGeoTransform (patch taken from 9.3)
  # can  be removed when version changed to >= 9.3

   patch -p0 << \EOF
--- Geovis/Core/vtkGeoTransform.cxx.orig	2024-01-10 10:22:40.143031000 -0800
+++ Geovis/Core/vtkGeoTransform.cxx	2024-01-10 10:22:49.698983000 -0800
@@ -212,7 +212,7 @@
 #if PROJ_VERSION_MAJOR >= 5
       c.lp.lam = coord[0];
       c.lp.phi = coord[1];
-      c_out = proj_trans(src, PJ_FWD, c);
+      c_out = proj_trans(dst, PJ_FWD, c);
       coord[0] = c_out.xy.x;
       coord[1] = c_out.xy.y;
 #else
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkGeoTransform failed."
      return 1
    fi
    return 0;
}

function apply_vtk9_osmesa_render_patch
{
    # I updated this patch for VTK 9.2.6, but not really sure it is still needed
    # VTK modified the Resize method ... perhaps that change fixes what this
    # patch was trying to fix?

    # Apply a patch where the OSMesaMakeCurrent could sometimes pass the
    # wrong window size for the buffer.
    patch -p0 << \EOF
diff -u Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx.orig Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx
--- Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx.orig    2023-09-11 08:28:50.273945000 -0700
+++ Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx 2023-09-11 08:37:20.796870000 -0700
@@ -187,7 +187,6 @@
   {
     this->Internal->OffScreenContextId = OSMesaCreateContext(GL_RGBA, nullptr);
   }
-  this->MakeCurrent();
 
   this->Mapped = 0;
   this->Size[0] = width;
@@ -301,8 +300,8 @@
 {
   if ((this->Size[0] != width) || (this->Size[1] != height))
   {
-    this->Superclass::SetSize(width, height);
     this->ResizeOffScreenWindow(width, height);
+    this->Superclass::SetSize(width, height);
     this->Modified();
   }
 }
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for osmesa render window failed."
      return 1
    fi

    return 0;
}

function apply_vtk_patch
{
    apply_vtk9_allow_onscreen_and_osmesa_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtk9_vtkospray_patches
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtk9_vtkRectilinearGridReader_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtk9_vtkCutter_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtk9_vtkdatawriter_patch
    if [[ $? != 0 ]] ; then
       return 1
    fi

    apply_vtk9_osmesa_render_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtk9_vtkgeotransform_patch
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

    # Make a build directory for an out-of-source build. Change the
    # VTK_BUILD_DIR variable to represent the out-of-source build directory.
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
    vopts="${vopts} -DVTK_LEGACY_REMOVE:BOOL=true"
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DVTK_BUILD_TESTING:STRING=OFF"
    vopts="${vopts} -DVTK_BUILD_DOCUMENTATION:BOOL=false"
    # setting this to true causes errors when building debug versions of
    # visit, so set it to false
    vopts="${vopts} -DVTK_REPORT_OPENGL_ERRORS:BOOL=false"

    if test "${OPSYS}" = "Darwin" ; then

        vopts="${vopts} -DVTK_USE_COCOA:BOOL=ON"
        vopts="${vopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${vtk_inst_path}/lib"

        # On Intel-Mac Monterey, VTK 9.2.6 is installing with names like
        # libvtkxxx9.2.dylib --> libvtkxxx9.2.1.dylib --> libvtkxxx9.2.9.2.6.dylib
        # Aside from being completely baffeling and wrong, when these got copied to
        # the install point, they were missing the intermediate 9.2.1 symlinks and
        # so were completely broken. After trying a large number of variations, the
        # only way I found to have it do something close to the right thing was to
        # tweek the CMakeLists.txt and set the custom lib suffix to "".
        if test "${VTK_VERSION}" = "9.2.6" ; then
            vopts="${vopts} -DVTK_CUSTOM_LIBRARY_SUFFIX:STRING=\"\""
            sed -i.orig -e 's/^  SOVERSION           "1"/  SOVERSION           "9.2"/' ./${VTK_SRC_DIR}/CMakeLists.txt
        fi

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
                vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GUISupportQt:STRING=YES"
                if [[ "$DO_QT6" == "yes" ]]; then
                    vopts="${vopts} -DQt6_DIR:FILEPATH=${QT6_INSTALL_DIR}/lib/cmake/Qt6"
                else
                    vopts="${vopts} -DQt5_DIR:FILEPATH=${QT_INSTALL_DIR}/lib/cmake/Qt5"
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
            vopts="${vopts} -DVTK_PYTHON_VERSION:STRING=3"
            vopts="${vopts} -DPython3_EXECUTABLE:FILEPATH=${py}"
            vopts="${vopts} -DPython3_EXTRA_LIBS:STRING=\"${VTK_PY_LIBS}\""
            vopts="${vopts} -DPython3_INCLUDE_DIR:PATH=${pyinc}"
            vopts="${vopts} -DPython3_LIBRARY:FILEPATH=${pylib}"
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi

    # Use Mesa as GL?
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
        vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING=${MESAGL_OPENGL_LIB}"
        vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH=${MESAGL_GLU_LIB}"
        # for now, until Mesa can be updated to a version that supports GLVND, set LEGACY preference
        vopts="${vopts} -DOpenGL_GL_PREFERENCE:STRING=LEGACY"
        vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
        vopts="${vopts} -DOSMESA_LIBRARY:STRING=${MESAGL_OSMESA_LIB}"
        vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"

        if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
                vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
                vopts="${vopts} -DOSMESA_LIBRARY:STRING=${MESAGL_OSMESA_LIB}"
                vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
                vopts="${vopts} -DVTK_USE_X:BOOL=OFF"
            fi
        fi
    elif [[ "$DO_OSMESA" == "yes" ]] ; then
        vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH="
        vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH="
        vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
        vopts="${vopts} -DOSMESA_LIBRARY:STRING=\"${OSMESA_LIB}\""
        vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${OSMESA_INCLUDE_DIR}"
        vopts="${vopts} -DVTK_USE_X:BOOL=OFF"
    fi

    # Use OSPRay?
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=YES"
        if [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib/cmake/ospray-${OSPRAY_VERSION}"
        elif [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib64 ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib64/cmake/ospray-${OSPRAY_VERSION}"
        else
            warn "Disabling ospray because its lib dir couldn't be found"
            vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=NO"
        fi
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
    if [[ "$DO_MESAGL" == "yes" || "$DO_OSMESA" == "yes" ]] ; then
        export LD_LIBRARY_PATH="${LLVM_LIB_DIR}:$LD_LIBRARY_PATH"
    fi

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
