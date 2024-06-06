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
    if [[ $DO_QT6 == "yes" ]]; then
        DO_VTK9="yes"
    fi

    if [[ "$DO_VTK9" == "yes" ]] ; then
        info "setting up vtk for version 9"
        export VTK_FILE=${VTK_FILE:-"VTK-9.2.6.tar.gz"}
        export VTK_VERSION=${VTK_VERSION:-"9.2.6"}
        export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"9.2"}
        export VTK_SHA256_CHECKSUM="06fc8d49c4e56f498c40fcb38a563ed8d4ec31358d0101e8988f0bb4d539dd12"
    else
        export VTK_FILE=${VTK_FILE:-"VTK-8.1.0.tar.gz"}
        export VTK_VERSION=${VTK_VERSION:-"8.1.0"}
        export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"8.1"}
        export VTK_SHA256_CHECKSUM="6e269f07b64fb13774f5925161fb4e1f379f4e6a0131c8408c555f6b58ef3cb7"
    fi
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
    printf "%-20s %s [%s]\n" "--vtk9" "Build VTK9 (must also use --vtk)" "$DO_VTK9"
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
--- IO/Legacy/vtkRectilinearGridReader.cxx.orig	2024-06-05 14:21:59.105807000 -0700
+++ IO/Legacy/vtkRectilinearGridReader.cxx	2024-06-05 14:26:30.561802000 -0700
@@ -95,7 +95,16 @@
         break;
       }
 
-      if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
+      // If data file is binary and FieldData is present, it
+      // must be read here, otherwise a ReadString will fail and the
+      // loop will terminate before reading dimensions.
+      if (!strncmp(this->LowerCase(line), "field", 5))
+      {
+        vtkFieldData* fd = this->ReadFieldData();
+        fd->Delete();
+      }
+
+      else if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
       {
         int dim[3];
         if (!(this->Read(dim) && this->Read(dim + 1) && this->Read(dim + 2)))
@@ -127,6 +136,20 @@
 
         dimsRead = true;
       }
+      // if the coordinates have been reached, should be no reason
+      // to keep reading
+      else if (strncmp(this->LowerCase(line), "x_coordinate", 12) == 0)
+      {
+        break;
+      }
+      else if (strncmp(this->LowerCase(line), "y_coordinate", 12) == 0)
+      {
+        break;
+      }
+      else if (strncmp(this->LowerCase(line), "z_coordinate", 12) == 0)
+      {
+        break;
+      }
     }
   }
 
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

function apply_vtk8_vtkxopenglrenderwindow_patch
{
  # patch vtk's vtkXOpenRenderWindow to fix segv when deleting windows in
  # offscreen mode.

   patch -p0 << \EOF
*** Rendering/OpenGL2/vtkXOpenGLRenderWindow.cxx.orig 2018-03-30 14:38:07.000000000
--- Rendering/OpenGL2/vtkXOpenGLRenderWindow.cxx 2018-03-30 14:38:40.000000000
***************
*** 1148,1160 ****

  void vtkXOpenGLRenderWindow::PopContext()
  {
    GLXContext current = glXGetCurrentContext();
    GLXContext target = static_cast<GLXContext>(this->ContextStack.top());
    this->ContextStack.pop();
!   if (target != current)
    {
      glXMakeCurrent(this->DisplayStack.top(),
        this->DrawableStack.top(),
        target);
    }
    this->DisplayStack.pop();
--- 1148,1160 ----

  void vtkXOpenGLRenderWindow::PopContext()
  {
    GLXContext current = glXGetCurrentContext();
    GLXContext target = static_cast<GLXContext>(this->ContextStack.top());
    this->ContextStack.pop();
!   if (target && target != current)
    {
      glXMakeCurrent(this->DisplayStack.top(),
        this->DrawableStack.top(),
        target);
    }
    this->DisplayStack.pop();

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkXOpenGLRenderWindow failed."
      return 1
    fi
    return 0;

}

function apply_vtk8_vtkopenglspheremapper_h_patch
{
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

function apply_vtk8_vtkopenglspheremapper_patch
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

function apply_vtk8_vtkdatawriter_patch
{
  # patch vtk's vtkDataWriter to fix bug when writing binary vtkBitArray,
  # and non-AOS data arrays (as used with LibSim).

   patch -p0 << \EOF
*** IO/Legacy/vtkDataWriter.cxx.original 2018-01-19 13:52:19.000000000
--- IO/Legacy/vtkDataWriter.cxx 2018-01-19 13:52:49.000000000
***************
*** 1015,1034 ****
    *fp << "\n";
  }
  
  // Returns a pointer to the data ordered in original VTK style ordering
  // of the data. If this is an SOA array it has to allocate the memory
  // for that in which case the calling function must delete it.
! template <class T>
! T* GetArrayRawPointer(vtkAbstractArray* array, T* ptr, int isAOSArray)
  {
    if (isAOSArray)
    {
!     return ptr;
!   }
!   T* data = new T[array->GetNumberOfComponents()*array->GetNumberOfTuples()];
!   vtkSOADataArrayTemplate<T>* typedArray =
!     vtkSOADataArrayTemplate<T>::SafeDownCast(array);
!   typedArray->ExportToVoidPointer(data);
    return data;
  }
  
--- 1015,1043 ----
    *fp << "\n";
  }
  
+ //------------------------------------------------------------------------------
+ template <class Value, class Array>
+ Value* GetPointer(vtkAbstractArray* array)
+ {
+   return static_cast<Array*>(array)->GetPointer(0);
+ }
+ //------------------------------------------------------------------------------
  // Returns a pointer to the data ordered in original VTK style ordering
  // of the data. If this is an SOA array it has to allocate the memory
  // for that in which case the calling function must delete it.
! template <class T, class Array>
! T* GetArrayRawPointer(vtkAbstractArray* array, int isAOSArray)
  {
    if (isAOSArray)
    {
!     return GetPointer<T, Array>(array);
! }
! 
!   auto nc = array->GetNumberOfComponents();
!   auto nt = array->GetNumberOfTuples();
! 
!   T* data = new T[nc * nt];
!   array->ExportToVoidPointer(data);
    return data;
  }
  
***************
*** 1072,1082 ****
        }
        else
        {
!         unsigned char *cptr=
!           static_cast<vtkUnsignedCharArray *>(data)->GetPointer(0);
!         fp->write(reinterpret_cast<char *>(cptr),
!                   (sizeof(unsigned char))*((num-1)/8+1));
! 
        }
        *fp << "\n";
      }
--- 1081,1088 ----
        }
        else
        {
!         unsigned char* cptr = static_cast<vtkBitArray*>(data)->GetPointer(0);
!         fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num*numComp - 1) / 8 + 1));
        }
        *fp << "\n";
      }
***************
*** 1084,1092 ****
  
      case VTK_CHAR:
      {
!       snprintf (str, sizeof(str), format, "char"); *fp << str;
!       char *s=GetArrayRawPointer(
!         data, static_cast<vtkCharArray *>(data)->GetPointer(0), isAOSArray);
  #if VTK_TYPE_CHAR_IS_SIGNED
        vtkWriteDataArray(fp, s, this->FileType, "%hhd ", num, numComp);
  #else
--- 1090,1098 ----
  
      case VTK_CHAR:
      {
!       snprintf(str, sizeof(str), format, "char");
!       *fp << str;
!       char* s = GetArrayRawPointer<char, vtkCharArray>(data, isAOSArray);
  #if VTK_TYPE_CHAR_IS_SIGNED
        vtkWriteDataArray(fp, s, this->FileType, "%hhd ", num, numComp);
  #else
***************
*** 1101,1109 ****
  
      case VTK_SIGNED_CHAR:
      {
!       snprintf (str, sizeof(str), format, "signed_char"); *fp << str;
!       signed char *s=GetArrayRawPointer(
!         data, static_cast<vtkSignedCharArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hhd ", num, numComp);
        if (!isAOSArray)
        {
--- 1107,1115 ----
  
      case VTK_SIGNED_CHAR:
      {
!       snprintf(str, sizeof(str), format, "signed_char");
!       *fp << str;
!       signed char* s = GetArrayRawPointer<signed char, vtkSignedCharArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hhd ", num, numComp);
        if (!isAOSArray)
        {
***************
*** 1114,1122 ****
  
      case VTK_UNSIGNED_CHAR:
      {
!       snprintf (str, sizeof(str), format, "unsigned_char"); *fp << str;
!       unsigned char *s=GetArrayRawPointer(
!         data, static_cast<vtkUnsignedCharArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hhu ", num, numComp);
        if (!isAOSArray)
        {
--- 1120,1128 ----
  
      case VTK_UNSIGNED_CHAR:
      {
!       snprintf(str, sizeof(str), format, "unsigned_char");
!       *fp << str;
!       unsigned char* s = GetArrayRawPointer<unsigned char, vtkUnsignedCharArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hhu ", num, numComp);
        if (!isAOSArray)
        {
***************
*** 1127,1135 ****
  
      case VTK_SHORT:
      {
!       snprintf (str, sizeof(str), format, "short"); *fp << str;
!       short *s=GetArrayRawPointer(
!         data, static_cast<vtkShortArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hd ", num, numComp);
        if (!isAOSArray)
        {
--- 1133,1141 ----
  
      case VTK_SHORT:
      {
!       snprintf(str, sizeof(str), format, "short");
!       *fp << str;
!       short* s = GetArrayRawPointer<short, vtkShortArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hd ", num, numComp);
        if (!isAOSArray)
        {
***************
*** 1140,1260 ****
  
      case VTK_UNSIGNED_SHORT:
      {
!       snprintf (str, sizeof(str), format, "unsigned_short"); *fp << str;
!       unsigned short *s=GetArrayRawPointer(
!         data, static_cast<vtkUnsignedShortArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hu ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_INT:
      {
!       snprintf (str, sizeof(str), format, "int"); *fp << str;
!       int *s=GetArrayRawPointer(
!         data, static_cast<vtkIntArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%d ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_UNSIGNED_INT:
      {
!       snprintf (str, sizeof(str), format, "unsigned_int"); *fp << str;
!       unsigned int *s=GetArrayRawPointer(
!         data, static_cast<vtkUnsignedIntArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%u ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_LONG:
      {
!       snprintf (str, sizeof(str), format, "long"); *fp << str;
!       long *s=GetArrayRawPointer(
!         data, static_cast<vtkLongArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%ld ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_UNSIGNED_LONG:
      {
!       snprintf (str, sizeof(str), format, "unsigned_long"); *fp << str;
!       unsigned long *s=GetArrayRawPointer(
!         data, static_cast<vtkUnsignedLongArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%lu ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_LONG_LONG:
      {
!       snprintf (str, sizeof(str), format, "vtktypeint64"); *fp << str;
!       long long *s=GetArrayRawPointer(
!         data, static_cast<vtkTypeInt64Array *>(data)->GetPointer(0), isAOSArray);
        strcpy(outputFormat, vtkTypeTraits<long long>::ParseFormat());
        strcat(outputFormat, " ");
        vtkWriteDataArray(fp, s, this->FileType, outputFormat, num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_UNSIGNED_LONG_LONG:
      {
!       snprintf (str, sizeof(str), format, "vtktypeuint64"); *fp << str;
!       unsigned long long *s=GetArrayRawPointer(
!         data, static_cast<vtkTypeUInt64Array *>(data)->GetPointer(0), isAOSArray);
        strcpy(outputFormat, vtkTypeTraits<unsigned long long>::ParseFormat());
        strcat(outputFormat, " ");
        vtkWriteDataArray(fp, s, this->FileType, outputFormat, num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_FLOAT:
      {
!       snprintf (str, sizeof(str), format, "float"); *fp << str;
!       float *s=GetArrayRawPointer(
!         data, static_cast<vtkFloatArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%g ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
  
      case VTK_DOUBLE:
      {
!       snprintf (str, sizeof(str), format, "double"); *fp << str;
!       double *s=GetArrayRawPointer(
!         data, static_cast<vtkDoubleArray *>(data)->GetPointer(0), isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%.11lg ", num, numComp);
        if (!isAOSArray)
        {
!         delete [] s;
        }
      }
      break;
--- 1146,1268 ----
  
      case VTK_UNSIGNED_SHORT:
      {
!       snprintf(str, sizeof(str), format, "unsigned_short");
!       *fp << str;
!       unsigned short* s =
!         GetArrayRawPointer<unsigned short, vtkUnsignedShortArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%hu ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_INT:
      {
!       snprintf(str, sizeof(str), format, "int");
!       *fp << str;
!       int* s = GetArrayRawPointer<int, vtkIntArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%d ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_UNSIGNED_INT:
      {
!       snprintf(str, sizeof(str), format, "unsigned_int");
!       *fp << str;
!       unsigned int* s = GetArrayRawPointer<unsigned int, vtkUnsignedIntArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%u ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_LONG:
      {
!       snprintf(str, sizeof(str), format, "long");
!       *fp << str;
!       long* s = GetArrayRawPointer<long, vtkLongArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%ld ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_UNSIGNED_LONG:
      {
!       snprintf(str, sizeof(str), format, "unsigned_long");
!       *fp << str;
!       unsigned long* s = GetArrayRawPointer<unsigned long, vtkUnsignedLongArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%lu ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_LONG_LONG:
      {
!       snprintf(str, sizeof(str), format, "vtktypeint64");
!       *fp << str;
!       long long* s = GetArrayRawPointer<long long, vtkTypeInt64Array>(data, isAOSArray);
        strcpy(outputFormat, vtkTypeTraits<long long>::ParseFormat());
        strcat(outputFormat, " ");
        vtkWriteDataArray(fp, s, this->FileType, outputFormat, num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_UNSIGNED_LONG_LONG:
      {
!       snprintf(str, sizeof(str), format, "vtktypeuint64");
!       *fp << str;
!       unsigned long long* s =
!         GetArrayRawPointer<unsigned long long, vtkTypeUInt64Array>(data, isAOSArray);
        strcpy(outputFormat, vtkTypeTraits<unsigned long long>::ParseFormat());
        strcat(outputFormat, " ");
        vtkWriteDataArray(fp, s, this->FileType, outputFormat, num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_FLOAT:
      {
!       snprintf(str, sizeof(str), format, "float");
!       *fp << str;
!       float* s = GetArrayRawPointer<float, vtkFloatArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%g ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;
  
      case VTK_DOUBLE:
      {
!       snprintf(str, sizeof(str), format, "double");
!       *fp << str;
!       double* s = GetArrayRawPointer<double, vtkDoubleArray>(data, isAOSArray);
        vtkWriteDataArray(fp, s, this->FileType, "%.11lg ", num, numComp);
        if (!isAOSArray)
        {
!         delete[] s;
        }
      }
      break;

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkDataWriter failed."
      return 1
    fi
    return 0;
}

function apply_vtk8_vtkospray_patches
{
	count_patches=4
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

	# 3) fix vtkOSPRayVolumeMapper
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

	# 4) Add include string to vtkOSPRayMaterialHelpers.h for gcc 10.3.
	((current_patch++))
    patch -p0 << \EOF
diff -c Rendering/OSPRay/vtkOSPRayMaterialHelpers.h.original Rendering/OSPRay/vtkOSPRayMaterialHelpers.h
*** Rendering/OSPRay/vtkOSPRayMaterialHelpers.h.original	Mon Jul 26 16:14:55 2021
--- Rendering/OSPRay/vtkOSPRayMaterialHelpers.h	Mon Jul 26 16:15:11 2021
***************
*** 33,38 ****
--- 33,39 ----
  
  #include "ospray/ospray.h"
  #include <map>
+ #include <string>
  
  class vtkImageData;
  class vtkOSPRayRendererNode;
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch $current_patch/$count_patches for vtkOSPRayMaterialHelpers."
        return 1
    fi
}

function apply_vtk8_vtkospraypolydatamappernode_patch
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


function apply_vtk8_vtkospray_linking_patch
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

function apply_vtk8_python3_python_args_patch
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

function apply_vtk8_compilerversioncheck_patch
{
    # Need to fix the REGEX so that version strings with 2digit major are matched correctly.
    patch -p0 << \EOF
diff -c CMake/VTKGenerateExportHeader.cmake.orig VTKGenerateExportHeader.cmake
*** CMake/VTKGenerateExportHeader.cmake.orig	Wed Jun 30 18:30:42 2021
--- CMake/VTKGenerateExportHeader.cmake	Wed Jun 30 18:31:06 2021
***************
*** 174,180 ****
      execute_process(COMMAND ${CMAKE_C_COMPILER} --version
        OUTPUT_VARIABLE _gcc_version_info
        ERROR_VARIABLE _gcc_version_info)
!     string(REGEX MATCH "[3-9]\\.[0-9]\\.[0-9]*"
        _gcc_version "${_gcc_version_info}")
      # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the
      # patch level, handle this here:
--- 174,180 ----
      execute_process(COMMAND ${CMAKE_C_COMPILER} --version
        OUTPUT_VARIABLE _gcc_version_info
        ERROR_VARIABLE _gcc_version_info)
!     string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]*"
        _gcc_version "${_gcc_version_info}")
      # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the
      # patch level, handle this here:

EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for compiler version check failed."
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

function apply_vtk8_osmesa_render_patch
{
    # Apply a patch where the OSMesaMakeCurrent could sometimes pass the
    # wrong window size for the buffer.
    patch -p0 << \EOF
diff -u Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx.orig Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx
--- Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx.orig  2022-10-31 14:53:21.228362000 -0700
+++ Rendering/OpenGL2/vtkOSOpenGLRenderWindow.cxx       2022-10-31 14:55:24.072140000 -0700
@@ -201,7 +201,6 @@
       this->Internal->OffScreenContextId = OSMesaCreateContext(GL_RGBA, nullptr);
     }
   }
-  this->MakeCurrent();
 
   this->Mapped = 0;
   this->Size[0] = width;
@@ -330,8 +329,8 @@
 {
   if ((this->Size[0] != width)||(this->Size[1] != height))
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

    if [[ "$DO_VTK9" == "yes" ]] ; then
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

    else
        apply_vtk8_vtkdatawriter_patch
        if [[ $? != 0 ]] ; then
           return 1
        fi

        apply_vtk8_vtkopenglspheremapper_h_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_vtkopenglspheremapper_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_vtkxopenglrenderwindow_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        # Note: don't guard ospray patches by if ospray is selected 
        # b/c subsequent calls to build_visit won't get a chance to patch
        # given the if test logic used above
        apply_vtk8_vtkospraypolydatamappernode_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_vtkospray_patches
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_vtkospray_linking_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_python3_python_args_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_compilerversioncheck_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk8_osmesa_render_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
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
    if [[ "$DO_VTK9" == "yes" ]] ; then
        vopts="${vopts} -DVTK_BUILD_TESTING:STRING=OFF"
        vopts="${vopts} -DVTK_BUILD_DOCUMENTATION:BOOL=false"
        # setting this to true causes errors when building debug versions of
        # visit, so set it to false
        vopts="${vopts} -DVTK_REPORT_OPENGL_ERRORS:BOOL=false"
    else
        vopts="${vopts} -DBUILD_TESTING:BOOL=false"
        vopts="${vopts} -DBUILD_DOCUMENTATION:BOOL=false"
        vopts="${vopts} -DVTK_REPORT_OPENGL_ERRORS:BOOL=true"
    fi

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
    if [[ "$DO_VTK9" == "yes" ]] ; then
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
    else
        # Turn off module groups
        vopts="${vopts} -DVTK_Group_Imaging:BOOL=false"
        vopts="${vopts} -DVTK_Group_MPI:BOOL=false"
        vopts="${vopts} -DVTK_Group_Qt:BOOL=false"
        vopts="${vopts} -DVTK_Group_Rendering:BOOL=false"
        vopts="${vopts} -DVTK_Group_StandAlone:BOOL=false"
        vopts="${vopts} -DVTK_Group_Tk:BOOL=false"
        vopts="${vopts} -DVTK_Group_Views:BOOL=false"
        vopts="${vopts} -DVTK_Group_Web:BOOL=false"

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
        vopts="${vopts} -DModule_vtkRenderingOpenGL2:BOOL=true"
        vopts="${vopts} -DModule_vtklibxml2:BOOL=true"
    fi

    # Tell VTK where to locate qmake if we're building graphical support. We
    # do not add graphical support for server-only builds.
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then
                if [[ "$DO_VTK9" == "yes" ]]; then
                    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GUISupportQt:STRING=YES"
                    if [[ "$DO_QT6" == "yes" ]]; then
                        vopts="${vopts} -DQt6_DIR:FILEPATH=${QT6_INSTALL_DIR}/lib/cmake/Qt6"
                    else
                        vopts="${vopts} -DQt5_DIR:FILEPATH=${QT_INSTALL_DIR}/lib/cmake/Qt5"
                    fi
                else
                    vopts="${vopts} -DModule_vtkGUISupportQtOpenGL:BOOL=true"
                    if [[ "$DO_QT6" == "yes" ]]; then
                        vopts="${vopts} -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT6_INSTALL_DIR}/bin/qmake"
                        vopts="${vopts} -DVTK_QT_VERSION=6"
                        vopts="${vopts} -DCMAKE_PREFIX_PATH=${QT6_INSTALL_DIR}/lib/cmake"
                    else
                        vopts="${vopts} -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_BIN_DIR}/qmake"
                        vopts="${vopts} -DVTK_QT_VERSION=5"
                        vopts="${vopts} -DCMAKE_PREFIX_PATH=${QT_INSTALL_DIR}/lib/cmake"
                    fi
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
            if [[ "$DO_VTK9" == "yes" ]]; then
                vopts="${vopts} -DVTK_PYTHON_VERSION:STRING=3"
                vopts="${vopts} -DPython3_EXECUTABLE:FILEPATH=${py}"
                vopts="${vopts} -DPython3_EXTRA_LIBS:STRING=\"${VTK_PY_LIBS}\""
                vopts="${vopts} -DPython3_INCLUDE_DIR:PATH=${pyinc}"
                vopts="${vopts} -DPython3_LIBRARY:FILEPATH=${pylib}"
            else
                vopts="${vopts} -DPYTHON_EXECUTABLE:FILEPATH=${py}"
                vopts="${vopts} -DPYTHON_EXTRA_LIBS:STRING=\"${VTK_PY_LIBS}\""
                vopts="${vopts} -DPYTHON_INCLUDE_DIR:PATH=${pyinc}"
                vopts="${vopts} -DPYTHON_LIBRARY:FILEPATH=${pylib}"
                if [[ "$DO_PYTHON2" == "no" ]]; then
                  vopts="${vopts} -DVTK_PYTHON_VERSION:STRING=3"
                fi
                #            vopts="${vopts} -DPYTHON_UTIL_LIBRARY:FILEPATH="
            fi
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi

    # Use Mesa as GL?
    if [[ "$DO_VTK9" == "yes" ]] ; then
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
    else
        if [[ "$DO_MESAGL" == "yes" ]] ; then
            vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
            vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
            vopts="${vopts} -DOPENGL_gl_LIBRARY:PATH=\"${MESAGL_OPENGL_LIB};${LLVM_LIB}\""
            vopts="${vopts} -DOPENGL_glu_LIBRARY:PATH=${MESAGL_GLU_LIB}"
            vopts="${vopts} -DOSMESA_LIBRARY:FILEPATH=\"${MESAGL_OSMESA_LIB};${LLVM_LIB}\""
            vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"

            if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
                if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
                    vopts="${vopts} -DVTK_USE_X:BOOL=OFF"
                fi
            fi
        fi
    fi

    # Use OSPRay?
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        if [[ "$DO_VTK9" == "yes" ]] ; then
            vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=YES"
            if [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib ]] ; then
                vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib/cmake/ospray-${OSPRAY_VERSION}"
            elif [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib64 ]] ; then
                vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib64/cmake/ospray-${OSPRAY_VERSION}"
            else
                warn "Disabling ospray because its lib dir couldn't be found"
                vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=NO"
            fi
        else
            vopts="${vopts} -DModule_vtkRenderingOSPRay:BOOL=ON"
            vopts="${vopts} -DOSPRAY_INSTALL_DIR=${OSPRAY_INSTALL_DIR}"
            vopts="${vopts} -Dembree_DIR=${EMBREE_INSTALL_DIR}"
        fi
    fi

    # zlib support, use the one we build
    if [[ "$DO_VTK9" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_USE_EXTERNAL_VTK_zlib:BOOL=ON"
    else
        vopts="${vopts} -DVTK_USE_SYSTEM_ZLIB:BOOL=ON"
    fi
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
