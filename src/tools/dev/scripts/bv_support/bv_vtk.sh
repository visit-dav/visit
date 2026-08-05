function bv_vtk_initialize
{
    info "bv_vtk_initialize"
    export DO_VTK="yes"
}

function bv_vtk_enable
{
    info "bv_vtk_enable"
    DO_VTK="yes"

}

function bv_vtk_disable
{
    DO_VTK="no"
}


function bv_vtk_depends_on
{
    depends_on="cmake zlib"

    if [[ "$DO_PYTHON" == "yes" ]]; then
        depends_on="${depends_on} python"
    fi

    if [[ "$DO_MESAGL" == "yes" ]]; then
        depends_on="${depends_on} mesagl glu"
    #elif [[ "$DO_OSMESA" == "yes" ]]; then
    #    depends_on="${depends_on} osmesa"
    fi

    if [[ "$DO_OSPRAY" == "yes" ]]; then
        depends_on="${depends_on} ospray"
    fi

    if [[ "$DO_ANARI" == "yes" ]]; then
        depends_on="${depends_on} anari"
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

function bv_vtk_info
{
    info "setting up vtk for version 9.5.0"
    export VTK_VERSION=${VTK_VERSION:-"9.5.0"}
    export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"9.5"}
    export VTK_SHA256_CHECKSUM="04ae86246b9557c6b61afbc534a6df099244fbc8f3937f82e6bc0570953af87d"
    export VTK_URL=""
    export VTK_FILE=${VTK_FILE:-"VTK-${VTK_VERSION}.tar.gz"}
    export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
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
}

function bv_vtk_host_profile
{
    echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## VTK" >> $HOSTCONF
    echo "##" >> $HOSTCONF

    echo "SETUP_APP_VERSION(VTK $VTK_VERSION)" >> $HOSTCONF
    echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR \${VISITHOME}/${VTK_INSTALL_DIR}/\${VTK_VERSION}/\${VISITARCH})" >> $HOSTCONF
}

function bv_vtk_initialize_vars
{
    info "initalizing vtk vars"
}

function bv_vtk_ensure
{
    if [[ "$DO_VTK" == "yes" ]] ; then
        ensure_built_or_ready $VTK_INSTALL_DIR $VTK_VERSION $VTK_BUILD_DIR $VTK_FILE $VTK_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}

function apply_vtk95_macos_dock_popup_fux

{
  # patch to prevent engine popup in macOS dock
  # https://github.com/visit-dav/visit/issues/20840
   patch -p0 << \EOF
--- Rendering/OpenGL2/vtkCocoaRenderWindow.mm
+++ Rendering/OpenGL2/vtkCocoaRenderWindow.mm
@@ -684,10 +684,11 @@ - (void)viewFrameDidChange:(NSNotification*)aNotification
   if (!this->GetRootWindow() && !this->GetWindowId() && !this->GetParentId() &&
     this->GetConnectContextToNSView())
   {
-    // Ordinarily, only .app bundles get proper mouse and keyboard interaction,
-    // but here we change the 'activation policy' to behave as if we were a
-    // .app bundle (which we may or may not be).
-    (void)[app setActivationPolicy:NSApplicationActivationPolicyRegular];
+    // FOR VISIT AVOID SETTING ACTIVATION POLICY
+    // // Ordinarily, only .app bundles get proper mouse and keyboard interaction,
+    // // but here we change the 'activation policy' to behave as if we were a
+    // // .app bundle (which we may or may not be).
+    // (void)[app setActivationPolicy:NSApplicationActivationPolicyRegular];
 
     NSWindow* theWindow = nil;

EOF

    if [[ $? != 0 ]] ; then
        warn "vtk patch for macOS dock fix for vtkCocoaRenderWindow.mm failed."
        return 1
    fi
}

function apply_vtk95_vtkRectilinearGridReader_patch
{
  # patch vtkRectilinearGridReader.cxx, per this issue:
  # https://gitlab.kitware.com/vtk/vtk/-/issues/18447
   patch -p0 << \EOF
--- IO/Legacy/vtkRectilinearGridReader.cxx.orig	2025-05-15 13:43:55.914685000 -0700
+++ IO/Legacy/vtkRectilinearGridReader.cxx	2025-05-15 13:45:09.376599000 -0700
@@ -83,8 +83,14 @@
       {
         break;
       }
+      // Have to read field data because it may be binary.
+      if (!strncmp(this->LowerCase(line), "field", 5))
+      {
+        vtkFieldData* fd = this->ReadFieldData();
+        fd->Delete();
+      }

-      if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
+      else if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
       {
         int dim[3];
         if (!(this->Read(dim) && this->Read(dim + 1) && this->Read(dim + 2)))
@@ -116,6 +122,22 @@

         dimsRead = true;
       }
+
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
+
     }
   }

EOF

    if [[ $? != 0 ]] ; then
        warn "vtk patch for vtkRectilinearGridReader.cxx failed."
        return 1
    fi
}

function apply_vtk95_vtkdatawriter_patch
{
  # patch vtkDataWriter to fix a bug when writing a vtkBitArray
  # Make it use the same calculation as the reader.
   patch -p0 << \EOF
--- IO/Legacy/vtkDataWriter.cxx.orig	2025-05-15 13:43:55.914685000 -0700
+++ IO/Legacy/vtkDataWriter.cxx	2025-05-15 13:45:09.376599000 -0700
@@ -1121,7 +1121,7 @@
       else
       {
         unsigned char* cptr = static_cast<vtkBitArray*>(data)->GetPointer(0);
-        fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num - 1) / 8 + 1));
+        fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num*numComp+7)/8));
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

function apply_vtk95_vtkopenfoamreader_patch
{
  # patch vtkOpenFOAMReader to correctly handle dynamic-meshes with
  # inactive faces. 
   patch -p0 << \EOF
--- IO/Geometry/vtkOpenFOAMReader.cxx	2025-06-23 12:12:36.000000000 -0700
+++ IO/Geometry/vtkOpenFOAMReader.cxx	2026-07-24 15:12:54.833824000 -0700
@@ -6840,7 +6840,24 @@
 
     // Store owner faces
     this->FaceOwner = ownerDict.ReleasePtr<vtkDataArray>();
-    const vtkIdType nFaces = this->FaceOwner->GetNumberOfTuples();
+    vtkIdType nFaces = this->FaceOwner->GetNumberOfTuples();
+
+    // Some dynamic meshes pad trailing inactive faces with -1 owner labels.
+    // Trim these entries the same way neighbour padding is handled below.
+    vtkIdType nActiveFaces = nFaces;
+    for (vtkIdType facei = 0; facei < nFaces; ++facei)
+    {
+      if (GetLabelValue(this->FaceOwner, facei, use64BitLabels) < 0)
+      {
+        nActiveFaces = facei;
+        break;
+      }
+    }
+    if (nActiveFaces < nFaces)
+    {
+      this->FaceOwner->Resize(nActiveFaces);
+      nFaces = nActiveFaces;
+    }
 
     // Check for max cell, check validity
     for (vtkIdType facei = 0; facei < nFaces; ++facei)
@@ -6912,27 +6929,26 @@
 
     // Store neighbour faces
     this->FaceNeigh = neighDict.ReleasePtr<vtkDataArray>();
-    const vtkIdType nFaces = this->FaceOwner->GetNumberOfTuples();
+    vtkIdType nNeighbourFaces = this->FaceNeigh->GetNumberOfTuples();
 
-    if (nFaces == this->FaceNeigh->GetNumberOfTuples())
+    // Some meshes pad trailing inactive entries in neighbour with -1 values.
+    // Trim these entries regardless of whether owner/neighbour lengths match.
+    vtkIdType nInternalFaces = nNeighbourFaces;
+    for (vtkIdType facei = 0; facei < nNeighbourFaces; ++facei)
     {
-      // Extremely old meshes had identical size for owner/neighbour and -1 padding
-      vtkIdType nInternalFaces = 0;
-      for (vtkIdType facei = 0; facei < nFaces; ++facei)
+      if (GetLabelValue(this->FaceNeigh, facei, use64BitLabels) < 0)
       {
-        if (GetLabelValue(this->FaceNeigh, facei, use64BitLabels) < 0)
-        {
-          break;
-        }
-        else
-        {
-          ++nInternalFaces;
-        }
+        nInternalFaces = facei;
+        break;
       }
+    }
+    if (nInternalFaces < nNeighbourFaces)
+    {
       this->FaceNeigh->Resize(nInternalFaces);
+      nNeighbourFaces = nInternalFaces;
     }
 
-    const vtkIdType nInternalFaces = this->FaceNeigh->GetNumberOfTuples();
+    nInternalFaces = this->FaceNeigh->GetNumberOfTuples();
 
     // Check for max cell, check validity
     for (vtkIdType facei = 0; facei < nInternalFaces; ++facei)
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkOpenFOAMReader.cxx failed."
      return 1
    fi
    return 0;

}

function apply_vtk95_vtkospray_patches
{
    count_patches=3
    # patch vtkOSPRay files:

    # 1) expose vtkViewNodeFactory via vtkOSPRayPass.h
    current_patch=1
    patch -p0 << \EOF
--- Rendering/RayTracing/vtkOSPRayPass.h.orig	2025-05-15 13:50:01.170625000 -0700
+++ Rendering/RayTracing/vtkOSPRayPass.h	2025-05-15 13:52:33.330669000 -0700
@@ -39,6 +39,8 @@
 class vtkRenderPassCollection;
 class vtkSequencePass;
 class vtkVolumetricPass;
+// Used by VisIt
+class vtkViewNodeFactory;

 class VTKRENDERINGRAYTRACING_EXPORT vtkOSPRayPass : public vtkRenderPass
 {
@@ -65,6 +67,11 @@
    */
   virtual void RenderInternal(const vtkRenderState* s);

+  /**
+   * Called by VisIt
+   */
+  virtual vtkViewNodeFactory* GetViewNodeFactory();
+
   ///@{
   /**
    * Wrapper around ospray's init and shutdown that protect
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 patch ${current_patch}/${count_patches} for vtkOSPRayPass.h failed."
        return 1
    fi

    # 2) expose vtkViewNodeFactory via vtkOSPRayPass.cxx
    ((current_patch++))
    patch -p0 << \EOF
--- Rendering/RayTracing/vtkOSPRayPass.cxx.orig	2025-05-15 13:55:03.700624000 -0700
+++ Rendering/RayTracing/vtkOSPRayPass.cxx	2025-05-15 13:55:50.006697000 -0700
@@ -417,6 +417,12 @@
 }

 //------------------------------------------------------------------------------
+vtkViewNodeFactory* vtkOSPRayPass::GetViewNodeFactory()
+{
+  return this->Internal->Factory;
+}
+
+//------------------------------------------------------------------------------
 bool vtkOSPRayPass::IsSupported()
 {
   static bool detected = false;
EOF

    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 patch ${current_patch}/${count_patches} for vtkOSPRayPass.cxx failed."
        return 1
    fi

    # 3) Set the samples in the VolumeMapper
    ((current_patch++))
    patch -p0 << \EOF
--- Rendering/RayTracing/vtkOSPRayVolumeMapper.cxx.orig	2025-05-15 13:57:16.832663000 -0700
+++ Rendering/RayTracing/vtkOSPRayVolumeMapper.cxx	2025-05-15 13:58:31.661593000 -0700
@@ -61,6 +61,10 @@
   {
     this->Init();
   }
+  vtkOSPRayRendererNode::SetSamplesPerPixel(
+    vtkOSPRayRendererNode::GetSamplesPerPixel(ren), this->InternalRenderer);
+  vtkOSPRayRendererNode::SetAmbientSamples(
+    vtkOSPRayRendererNode::GetAmbientSamples(ren), this->InternalRenderer);
   this->InternalRenderer->SetRenderWindow(ren->GetRenderWindow());
   this->InternalRenderer->SetActiveCamera(ren->GetActiveCamera());
   this->InternalRenderer->SetBackground(ren->GetBackground());
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 patch $current_patch/$count_patches for vtkOSPRayVolumeMapper.cxx failed."
        return 1
    fi
}

function apply_vtk95_texture_anari_patches
{
    count_patches=5
    # patch vtkAnari files:

    #1) vtkAnariGlyph3DMapperNode.cxx update
    current_patch=1
    patch -p0 << \EOF
*** Rendering/ANARI/vtkAnariGlyph3DMapperNode.cxx.orig  2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariGlyph3DMapperNode.cxx       2026-02-19 18:40:29.768743155 -0600
***************
*** 277,283 ****
    anari::Geometry InitializeSpheres(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double pointSize,
      vtkDataArray* radiusArray, vtkPiecewiseFunction* scaleFunction,
!     std::vector<vec2>& textureCoords, std::vector<float>& pointValueTextureCoords,
      std::vector<vec4>& pointColors, int cellFlag) override;

    const char* GetSpheresPostfix() const override;
--- 277,283 ----
    anari::Geometry InitializeSpheres(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double pointSize,
      vtkDataArray* radiusArray, vtkPiecewiseFunction* scaleFunction,
!     std::vector<vec2>& textureCoords, std::vector<vec2>& pointValueTextureCoords,
      std::vector<vec4>& pointColors, int cellFlag) override;

    const char* GetSpheresPostfix() const override;
***************
*** 332,338 ****
  anari::Geometry vtkAnariGlyph3DMapperInheritInterface::InitializeSpheres(vtkPolyData* polyData,
    vtkProperty* property, std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray,
    double pointSize, vtkDataArray* radiusArray, vtkPiecewiseFunction* scaleFunction,
!   std::vector<vec2>& textureCoords, std::vector<float>& pointValueTextureCoords,
    std::vector<vec4>& pointColors, int cellFlag)
  {
    using MapperInternals = vtkAnariGlyph3DMapperNodeInternals;
--- 332,338 ----
  anari::Geometry vtkAnariGlyph3DMapperInheritInterface::InitializeSpheres(vtkPolyData* polyData,
    vtkProperty* property, std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray,
    double pointSize, vtkDataArray* radiusArray, vtkPiecewiseFunction* scaleFunction,
!   std::vector<vec2>& textureCoords, std::vector<vec2>& pointValueTextureCoords,
    std::vector<vec4>& pointColors, int cellFlag)
  {
    using MapperInternals = vtkAnariGlyph3DMapperNodeInternals;
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch ${current_patch}/${count_patches} for vtkAnariGlyph3DMapperNode.cxx failed."
        return 1
    fi
    
    # 2) vtkAnariPolyDataMapperInheritInterface.h update
    ((current_patch++))
    patch -p0 << \EOF
*** Rendering/ANARI/vtkAnariPolyDataMapperInheritInterface.h.orig   2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariPolyDataMapperInheritInterface.h        2026-02-19 18:38:29.910100983 -0600
***************
*** 66,83 ****
    virtual anari::Geometry InitializeSpheres(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double pointSize,
      vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!     std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors, int cellFlag);
    virtual anari::Geometry InitializeCurves(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double lineWidth,
      vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!     std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors, int cellFlag);
    virtual anari::Geometry InitializeCylinders(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double lineWidth,
      vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!     std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors, int cellFlag);
    virtual anari::Geometry InitializeTriangles(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, std::vector<vec3>& normals,
!     std::vector<vec2>& textureCoords, std::vector<float>& pointValueTextureCoords,
      std::vector<vec4>& pointColors, int cellFlag);

    /**
--- 66,83 ----
    virtual anari::Geometry InitializeSpheres(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double pointSize,
      vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!     std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors, int cellFlag);
    virtual anari::Geometry InitializeCurves(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double lineWidth,
      vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!     std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors, int cellFlag);
    virtual anari::Geometry InitializeCylinders(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, double lineWidth,
      vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!     std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors, int cellFlag);
    virtual anari::Geometry InitializeTriangles(vtkPolyData* polyData, vtkProperty* property,
      std::vector<vec3>& vertices, std::vector<uint32_t>& indexArray, std::vector<vec3>& normals,
!     std::vector<vec2>& textureCoords, std::vector<vec2>& pointValueTextureCoords,
      std::vector<vec4>& pointColors, int cellFlag);

    /**
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch ${current_patch}/${count_patches} for vtkAnariPolyDataMapperInheritInterface.h failed."
        return 1
    fi
    
    # 3) vtkAnariPolyDataMapperInheritInterface.cxx update
    ((current_patch++))
    patch -p0 << \EOF
*** Rendering/ANARI/vtkAnariPolyDataMapperInheritInterface.cxx.orig	2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariPolyDataMapperInheritInterface.cxx	    2026-02-19 18:49:46.478906325 -0600
***************
*** 33,39 ****
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeSpheres(vtkPolyData*,
    vtkProperty*, std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*,
!   vtkPiecewiseFunction*, std::vector<vec2>&, std::vector<float>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "sphere");
  }
--- 33,39 ----
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeSpheres(vtkPolyData*,
    vtkProperty*, std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*,
!   vtkPiecewiseFunction*, std::vector<vec2>&, std::vector<vec2>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "sphere");
  }
***************
*** 41,47 ****
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeCurves(vtkPolyData*, vtkProperty*,
    std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*,
!   std::vector<vec2>&, std::vector<float>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "curve");
  }
--- 41,47 ----
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeCurves(vtkPolyData*, vtkProperty*,
    std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*,
!   std::vector<vec2>&, std::vector<vec2>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "curve");
  }
***************
*** 49,55 ****
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeCylinders(vtkPolyData*,
    vtkProperty*, std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*,
!   vtkPiecewiseFunction*, std::vector<vec2>&, std::vector<float>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "cylinder");
  }
--- 49,55 ----
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeCylinders(vtkPolyData*,
    vtkProperty*, std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*,
!   vtkPiecewiseFunction*, std::vector<vec2>&, std::vector<vec2>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "cylinder");
  }
***************
*** 57,63 ****
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeTriangles(vtkPolyData*,
    vtkProperty*, std::vector<vec3>&, std::vector<uint32_t>&, std::vector<vec3>&, std::vector<vec2>&,
!   std::vector<float>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "triangle");
  }
--- 57,63 ----
  //----------------------------------------------------------------------------
  anari::Geometry vtkAnariPolyDataMapperInheritInterface::InitializeTriangles(vtkPolyData*,
    vtkProperty*, std::vector<vec3>&, std::vector<uint32_t>&, std::vector<vec3>&, std::vector<vec2>&,
!   std::vector<vec2>&, std::vector<vec4>&, int)
  {
    return anari::newObject<anari::Geometry>(this->AnariDevice, "triangle");
  }
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch ${current_patch}/${count_patches} for vtkAnariPolyDataMapperInheritInterface.cxx failed."
        return 1
    fi

     # 4) vtkAnariPolyDataMapperNode.cxx update
    ((current_patch++))
    patch -p0 << \EOF
*** Rendering/ANARI/vtkAnariPolyDataMapperNode.cxx.orig	2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariPolyDataMapperNode.cxx	    2026-02-20 10:26:00.019021590 -0600
***************
*** 56,62 ****
    void Execute(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eventId),
      void* vtkNotUsed(callData)) override
    {
!     this->RendererNode->InvalidateSceneStructure();
    }
  
    vtkAnariSceneGraph* RendererNode{ nullptr };
--- 56,65 ----
    void Execute(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eventId),
      void* vtkNotUsed(callData)) override
    {
!     if (this->RendererNode)
!     {
!       this->RendererNode->InvalidateSceneStructure();
!     }
    }
  
    vtkAnariSceneGraph* RendererNode{ nullptr };
***************
*** 157,163 ****
     */
    void RenderSurfaces(anari::Sampler, vtkActor*, vtkPolyData*, std::vector<vec3>&,
      std::vector<uint32_t>&, bool, double, double, vtkDataArray*, vtkPiecewiseFunction*,
!     std::vector<vec2>&, std::vector<float>&, std::vector<vec4>&, AttributeArrayCollection&,
      vtkPolyDataMapperNode::vtkPDConnectivity& conn, int);
  
    /**
--- 160,166 ----
     */
    void RenderSurfaces(anari::Sampler, vtkActor*, vtkPolyData*, std::vector<vec3>&,
      std::vector<uint32_t>&, bool, double, double, vtkDataArray*, vtkPiecewiseFunction*,
!     std::vector<vec2>&, std::vector<vec2>&, std::vector<vec4>&, AttributeArrayCollection&,
      vtkPolyDataMapperNode::vtkPDConnectivity& conn, int);
  
    /**
***************
*** 166,172 ****
     */
    anari::Surface RenderAsSpheres(anari::Sampler, vtkProperty*, vtkPolyData*, std::vector<vec3>&,
      std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*, std::vector<vec2>&,
!     std::vector<float>&, std::vector<vec4>&, AttributeArrayCollection&, int);
  
    /**
     * Create an ANARI surface with a geometry consisting of individual cylinders, each
--- 169,175 ----
     */
    anari::Surface RenderAsSpheres(anari::Sampler, vtkProperty*, vtkPolyData*, std::vector<vec3>&,
      std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*, std::vector<vec2>&,
!     std::vector<vec2>&, std::vector<vec4>&, AttributeArrayCollection&, int);
  
    /**
     * Create an ANARI surface with a geometry consisting of individual cylinders, each
***************
*** 174,180 ****
     */
    anari::Surface RenderAsCylinders(anari::Sampler, vtkProperty* property, vtkPolyData*,
      std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*,
!     std::vector<vec2>&, std::vector<float>&, std::vector<vec4>&, AttributeArrayCollection&, int);
  
    /**
     * Create an ANARI surface with a geometry consisting of curves, each
--- 177,183 ----
     */
    anari::Surface RenderAsCylinders(anari::Sampler, vtkProperty* property, vtkPolyData*,
      std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*,
!     std::vector<vec2>&, std::vector<vec2>&, std::vector<vec4>&, AttributeArrayCollection&, int);
  
    /**
     * Create an ANARI surface with a geometry consisting of curves, each
***************
*** 182,195 ****
     */
    anari::Surface RenderAsCurves(anari::Sampler, vtkProperty* property, vtkPolyData*,
      std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*,
!     std::vector<vec2>&, std::vector<float>&, std::vector<vec4>&, AttributeArrayCollection&, int);
  
    /**
     * Create an ANARI surface with a geometry consisting of triangles.
     */
    anari::Surface RenderAsTriangles(anari::Sampler anariSampler, vtkProperty* property, vtkPolyData*,
      std::vector<vec3>&, std::vector<uint32_t>& indexArray, std::vector<vec3>& normals,
!     std::vector<vec2>& textureCoords, std::vector<float>&, std::vector<vec4>&,
      AttributeArrayCollection&, int);
  
    /**
--- 185,198 ----
     */
    anari::Surface RenderAsCurves(anari::Sampler, vtkProperty* property, vtkPolyData*,
      std::vector<vec3>&, std::vector<uint32_t>&, double, vtkDataArray*, vtkPiecewiseFunction*,
!     std::vector<vec2>&, std::vector<vec2>&, std::vector<vec4>&, AttributeArrayCollection&, int);
  
    /**
     * Create an ANARI surface with a geometry consisting of triangles.
     */
    anari::Surface RenderAsTriangles(anari::Sampler anariSampler, vtkProperty* property, vtkPolyData*,
      std::vector<vec3>&, std::vector<uint32_t>& indexArray, std::vector<vec3>& normals,
!     std::vector<vec2>& textureCoords, std::vector<vec2>&, std::vector<vec4>&,
      AttributeArrayCollection&, int);
  
    /**
***************
*** 199,205 ****
     * So make sure reservedAttribs matches the rest of the logic.
     */
    void SetAttributeArrays(AttributeArrayCollection& attributeArrays, anari::Geometry& anariGeometry,
!     const int reservedAttribs = 1);
  
    /**
     * Sets time metadata on an ANARI geometry.
--- 202,208 ----
     * So make sure reservedAttribs matches the rest of the logic.
     */
    void SetAttributeArrays(AttributeArrayCollection& attributeArrays, anari::Geometry& anariGeometry,
!     int reservedAttribs = 1);
  
    /**
     * Sets time metadata on an ANARI geometry.
***************
*** 236,242 ****
    /**
     * Converts a 2D VTK texture to a 2D ANARI sampler.
     */
!   anari::Sampler VTKToAnariSampler(std::string, std::string, mat4 inTransform, vtkImageData*, bool);
  
    /**
     * Extracts individual textures (occlusion, roughness, metallic) from the combined VTK
--- 239,245 ----
    /**
     * Converts a 2D VTK texture to a 2D ANARI sampler.
     */
!   anari::Sampler VTKToAnariSampler(std::string, std::string, mat4 inTransform, vtkTexture*);
  
    /**
     * Extracts individual textures (occlusion, roughness, metallic) from the combined VTK
***************
*** 291,297 ****
  
    std::vector<anari::Surface> Surfaces;
  
!   double DataTimeStep = std::numeric_limits<float>::quiet_NaN();
    std::string ActorName;
    int TrianglesId = 0;
    int CylindersId = 0;
--- 294,300 ----
  
    std::vector<anari::Surface> Surfaces;
  
!   double DataTimeStep = std::numeric_limits<double>::quiet_NaN();
    std::string ActorName;
    int TrianglesId = 0;
    int CylindersId = 0;
***************
*** 393,399 ****
  
  //----------------------------------------------------------------------------
  anari::Sampler vtkAnariPolyDataMapperNodeInternals::ExtractORMFromVTK(std::string name,
!   int textureIdx, std::string inAttribute, mat4 inTransform, vtkImageData* imageData, bool sRGB)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::ExtractORMFromVTK", vtkAnariProfiling::LIME);
  
--- 396,402 ----
  
  //----------------------------------------------------------------------------
  anari::Sampler vtkAnariPolyDataMapperNodeInternals::ExtractORMFromVTK(std::string name,
!   const int textureIdx, std::string inAttribute, mat4 inTransform, vtkImageData* imageData, bool sRGB)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::ExtractORMFromVTK", vtkAnariProfiling::LIME);
  
***************
*** 418,428 ****
    anari::setParameter(this->AnariDevice, anariSampler, "filter", "linear");
  
    // Get the needed image data attributes
!   int xsize = (imageData->GetExtent()[1] - imageData->GetExtent()[0]) + 1;
!   int ysize = (imageData->GetExtent()[3] - imageData->GetExtent()[2]) + 1;
  
    if (xsize <= 0 || ysize <= 0)
    {
      return nullptr;
    }
  
--- 421,435 ----
    anari::setParameter(this->AnariDevice, anariSampler, "filter", "linear");
  
    // Get the needed image data attributes
!   const int* const imageSize = imageData->GetDimensions();
!   const int xsize = imageSize[0];
!   const int ysize = imageSize[1];
  
    if (xsize <= 0 || ysize <= 0)
    {
+     vtkWarningWithObjectMacro(
+       this->Owner, << "[ExtractORMFromVTK] Invalid image data extent: " << xsize << "x" << ysize);
+     anari::release(this->AnariDevice, anariSampler);
      return nullptr;
    }
  
***************
*** 445,456 ****
  
  //----------------------------------------------------------------------------
  anari::Sampler vtkAnariPolyDataMapperNodeInternals::VTKToAnariSampler(
!   std::string name, std::string inAttribute, mat4 inTransform, vtkImageData* imageData, bool sRGB)
  {
!   vtkAnariProfiling startProfiling("VTKAPDMNInternals::VTKToAnariSampler", vtkAnariProfiling::LIME);
  
    if (imageData == nullptr)
    {
      return nullptr;
    }
  
--- 452,474 ----
  
  //----------------------------------------------------------------------------
  anari::Sampler vtkAnariPolyDataMapperNodeInternals::VTKToAnariSampler(
!   std::string name, std::string inAttribute, mat4 inTransform, vtkTexture* texture)
  {
!   vtkAnariProfiling startProfiling(
!     "VTKAPDMNInternals::VTKToAnariSampler", vtkAnariProfiling::LIME);
! 
!   if (texture == nullptr)
!   {
!     return nullptr;
!   }
! 
!   // Update the texture to ensure the image data is current
!   texture->Update();
!   vtkImageData* imageData = texture->GetInput();
  
    if (imageData == nullptr)
    {
+     vtkWarningWithObjectMacro(this->Owner, << "[VTKToAnariSampler] Texture has no input image data.");
      return nullptr;
    }
  
***************
*** 460,475 ****
    anari::setParameter(this->AnariDevice, anariSampler, "name", ANARI_STRING, samplerName.c_str());
    anari::setParameter(this->AnariDevice, anariSampler, "inAttribute", inAttribute);
    anari::setParameter(this->AnariDevice, anariSampler, "inTransform", inTransform);
!   anari::setParameter(this->AnariDevice, anariSampler, "wrapMode1", "clampToEdge");
!   anari::setParameter(this->AnariDevice, anariSampler, "wrapMode2", "clampToEdge");
!   anari::setParameter(this->AnariDevice, anariSampler, "filter", "linear");
  
    // Get the needed image data attributes
!   int xsize = (imageData->GetExtent()[1] - imageData->GetExtent()[0]) + 1;
!   int ysize = (imageData->GetExtent()[3] - imageData->GetExtent()[2]) + 1;
  
    if (xsize <= 0 || ysize <= 0)
    {
      return nullptr;
    }
  
--- 478,513 ----
    anari::setParameter(this->AnariDevice, anariSampler, "name", ANARI_STRING, samplerName.c_str());
    anari::setParameter(this->AnariDevice, anariSampler, "inAttribute", inAttribute);
    anari::setParameter(this->AnariDevice, anariSampler, "inTransform", inTransform);
! 
!   std::string wrapMode = "clampToEdge";
!   switch (texture->GetWrap())
!   {
!     case vtkTexture::Repeat:
!       wrapMode = "repeat";
!       break;
!     case vtkTexture::MirroredRepeat:
!       wrapMode = "mirroredRepeat";
!       break;
!     case vtkTexture::ClampToEdge:
!     case vtkTexture::ClampToBorder:
!     default:
!       wrapMode = "clampToEdge";
!       break;
!   }
! 
!   anari::setParameter(this->AnariDevice, anariSampler, "wrapMode1", wrapMode);
!   anari::setParameter(this->AnariDevice, anariSampler, "wrapMode2", wrapMode);
  
    // Get the needed image data attributes
!   const int* const imageSize = imageData->GetDimensions();
!   const int xsize = imageSize[0];
!   const int ysize = imageSize[1];
  
    if (xsize <= 0 || ysize <= 0)
    {
+     vtkWarningWithObjectMacro(
+       this->Owner, << "[VTKToAnariSampler] Invalid image data extent: " << xsize << "x" << ysize);
+     anari::release(this->AnariDevice, anariSampler);
      return nullptr;
    }
  
***************
*** 491,516 ****
  
        if (comps > 4)
        {
          uint8_t* imageDataPtr = (uint8_t*)imageData->GetScalarPointer(0, 0, 0);
  
!         for (int i = 0; i < xsize; i++)
          {
!           for (int j = 0; j < ysize; j++)
            {
!             for (int k = 0; k < 3; k++)
              {
                charData.emplace_back(imageDataPtr[k]);
              }
-           }
  
!           imageDataPtr += comps;
          }
- 
-         comps = 3;
        }
  
        const auto* appMemory = charData.empty() ? imageData->GetScalarPointer() : charData.data();
!       auto dataType = sRGB ? anariLinearColorFormats[comps - 1] : anariColorFormats[comps - 1];
  
        anari::setParameterArray2D(
          this->AnariDevice, anariSampler, "image", dataType, appMemory, xsize, ysize);
--- 529,555 ----
  
        if (comps > 4)
        {
+         const int originalComps = comps;
+         comps = 4;
          uint8_t* imageDataPtr = (uint8_t*)imageData->GetScalarPointer(0, 0, 0);
  
!         for (int i = 0; i < ysize; i++)
          {
!           for (int j = 0; j < xsize; j++)
            {
!             for (int k = 0; k < comps; k++)
              {
                charData.emplace_back(imageDataPtr[k]);
              }
  
!             imageDataPtr += originalComps;
!           }
          }
        }
  
        const auto* appMemory = charData.empty() ? imageData->GetScalarPointer() : charData.data();
!       auto dataType = texture->GetUseSRGBColorSpace() ? anariLinearColorFormats[comps - 1] 
!                                                       : anariColorFormats[comps - 1];
  
        anari::setParameterArray2D(
          this->AnariDevice, anariSampler, "image", dataType, appMemory, xsize, ysize);
***************
*** 525,542 ****
  
        if (comps > 4)
        {
          for (int i = 0; i < ysize; i++)
          {
            for (int j = 0; j < xsize; j++)
            {
!             for (int k = 0; k < 3; k++)
              {
                floatData.emplace_back(imageData->GetScalarComponentAsFloat(j, i, 0, k));
              }
            }
          }
- 
-         comps = 3;
        }
  
        const auto* appMemory = floatData.empty() ? imageData->GetScalarPointer() : floatData.data();
--- 564,581 ----
  
        if (comps > 4)
        {
+         comps = 4;
+ 
          for (int i = 0; i < ysize; i++)
          {
            for (int j = 0; j < xsize; j++)
            {
!             for (int k = 0; k < comps; k++)
              {
                floatData.emplace_back(imageData->GetScalarComponentAsFloat(j, i, 0, k));
              }
            }
          }
        }
  
        const auto* appMemory = floatData.empty() ? imageData->GetScalarPointer() : floatData.data();
***************
*** 554,575 ****
  
        if (comps > 4)
        {
          uint16_t* imageDataPtr = reinterpret_cast<uint16_t*>(imageData->GetScalarPointer(0, 0, 0));
  
!         for (int i = 0; i < xsize; i++)
          {
!           for (int j = 0; j < ysize; j++)
            {
!             for (int k = 0; k < 3; k++)
              {
                shortData.emplace_back(imageDataPtr[k]);
              }
-           }
  
!           imageDataPtr += comps;
          }
- 
-         comps = 3;
        }
  
        const auto* appMemory = shortData.empty() ? imageData->GetScalarPointer() : shortData.data();
--- 593,614 ----
  
        if (comps > 4)
        {
+         const int originalComps = comps;
+         comps = 4;
          uint16_t* imageDataPtr = reinterpret_cast<uint16_t*>(imageData->GetScalarPointer(0, 0, 0));
  
!         for (int i = 0; i < ysize; i++)
          {
!           for (int j = 0; j < xsize; j++)
            {
!             for (int k = 0; k < comps; k++)
              {
                shortData.emplace_back(imageDataPtr[k]);
              }
  
!             imageDataPtr += originalComps;
!           }
          }
        }
  
        const auto* appMemory = shortData.empty() ? imageData->GetScalarPointer() : shortData.data();
***************
*** 582,588 ****
        anari::DataType anariColorFormats[4] = { ANARI_FLOAT32, ANARI_FLOAT32_VEC2,
          ANARI_FLOAT32_VEC3, ANARI_FLOAT32_VEC4 };
  
!       comps = comps > 4 ? 3 : comps;
        std::vector<float> floatData;
  
        for (int i = 0; i < ysize; i++)
--- 621,627 ----
        anari::DataType anariColorFormats[4] = { ANARI_FLOAT32, ANARI_FLOAT32_VEC2,
          ANARI_FLOAT32_VEC3, ANARI_FLOAT32_VEC4 };
  
!       comps = comps > 4 ? 4 : comps;
        std::vector<float> floatData;
  
        for (int i = 0; i < ysize; i++)
***************
*** 612,622 ****
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::MakeMaterial", vtkAnariProfiling::LIME);
  
    std::string materialName = this->ActorName + "_material";
-   const char* vtkMaterialName = property->GetMaterialName();
- 
    anari::Material anariMaterial = nullptr;
  
!   if (property->GetInterpolation() == VTK_PBR && this->StrToLower(vtkMaterialName) != "matte")
    {
      if (this->AnariDeviceExtensions.ANARI_KHR_MATERIAL_PHYSICALLY_BASED)
      {
--- 651,659 ----
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::MakeMaterial", vtkAnariProfiling::LIME);
  
    std::string materialName = this->ActorName + "_material";
    anari::Material anariMaterial = nullptr;
  
!   if (property->GetInterpolation() == VTK_PBR)
    {
      if (this->AnariDeviceExtensions.ANARI_KHR_MATERIAL_PHYSICALLY_BASED)
      {
***************
*** 669,677 ****
    anari::Material anariMaterial, vtkProperty* vtkProperty, float* color,
    anari::Sampler baseColorSampler, const char* colorStr)
  {
-   vtkTexture* texture = nullptr;
-   vtkTexture* ormTexture = vtkProperty->GetTexture("materialTex");
- 
    mat4 inTransform = { vec4{ 1.0f, 0.0f, 0.0f, 0.0f }, vec4{ 0.0f, 1.0f, 0.0f, 0.0f },
      vec4{ 0.0f, 0.0f, 1.0f, 0.0f }, vec4{ 0.0f, 0.0f, 0.0f, 1.0f } };
  
--- 706,711 ----
***************
*** 686,712 ****
    else
    {
      // base color
!     float materialColor[3] = { 0.0f, 0.0f, 0.0f };
  
!     if (baseColorSampler == nullptr && colorStr == nullptr)
      {
!       if (color != nullptr)
        {
!         for (int i = 0; i < 3; i++)
!         {
!           materialColor[i] = color[i];
!         }
        }
!       else
!       {
!         double* actorColor = vtkProperty->GetColor();
  
!         if (actorColor != nullptr)
          {
!           for (int i = 0; i < 3; i++)
!           {
!             materialColor[i] = static_cast<float>(actorColor[i]);
!           }
          }
        }
      }
--- 720,743 ----
    else
    {
      // base color
!     float materialColor[3] = { 1.0f, 1.0f, 1.0f };
  
!     if (color != nullptr)
      {
!       for (int i = 0; i < 3; i++)
        {
!         materialColor[i] = color[i];
        }
!     }
!     else
!     {
!       double* actorColor = vtkProperty->GetColor();
  
!       if (actorColor != nullptr)
!       {
!         for (int i = 0; i < 3; i++)
          {
!           materialColor[i] = static_cast<float>(actorColor[i]);
          }
        }
      }
***************
*** 718,780 ****
    const float opacity = static_cast<float>(vtkProperty->GetOpacity());
    anari::setParameter(this->AnariDevice, anariMaterial, "opacity", opacity);
  
    // metalness
    if (ormTexture)
    {
      vtkImageData* ormImageData = ormTexture->GetInput();
      auto metallicSampler =
        this->ExtractORMFromVTK("metallicTex", 2, "attribute0", inTransform, ormImageData, false);
!     anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "metallic", metallicSampler);
    }
    else
    {
-     const float metallic = static_cast<float>(vtkProperty->GetMetallic());
      anari::setParameter(this->AnariDevice, anariMaterial, "metallic", metallic);
    }
  
    // roughness
    if (ormTexture)
    {
      vtkImageData* ormImageData = ormTexture->GetInput();
      auto roughnessSampler =
        this->ExtractORMFromVTK("roughnessTex", 1, "attribute0", inTransform, ormImageData, false);
!     anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "roughness", roughnessSampler);
    }
    else
    {
-     const float roughness = static_cast<float>(vtkProperty->GetRoughness());
      anari::setParameter(this->AnariDevice, anariMaterial, "roughness", roughness);
    }
  
    // normal map for the base layer
!   texture = vtkProperty->GetTexture("normalTex");
  
!   if (texture)
    {
-     vtkImageData* normalImageData = texture->GetInput();
      auto normalSampler =
!       this->VTKToAnariSampler("normalTex", "attribute0", inTransform, normalImageData, false);
!     anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "normal", normalSampler);
    }
  
    // emissive
!   texture = vtkProperty->GetTexture("emissiveTex");
  
!   if (texture)
    {
-     vtkImageData* emissiveImageData = texture->GetInput();
      auto emissiveSampler =
!       this->VTKToAnariSampler("emissiveTex", "attribute0", inTransform, emissiveImageData, true);
!     anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "emissive", emissiveSampler);
    }
  
    // occlusion map
    if (ormTexture)
    {
!     vtkImageData* ormImageData = texture->GetInput();
      auto occlusionSampler =
        this->ExtractORMFromVTK("occlusionTex", 0, "attribute0", inTransform, ormImageData, false);
!     anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "occlusion", occlusionSampler);
    }
  
    // strength of the specular reflection
--- 749,844 ----
    const float opacity = static_cast<float>(vtkProperty->GetOpacity());
    anari::setParameter(this->AnariDevice, anariMaterial, "opacity", opacity);
  
+   vtkTexture* ormTexture = vtkProperty->GetTexture("materialTex");
+ 
    // metalness
+   const float metallic = static_cast<float>(vtkProperty->GetMetallic());
+ 
    if (ormTexture)
    {
+     ormTexture->Update();
      vtkImageData* ormImageData = ormTexture->GetInput();
      auto metallicSampler =
        this->ExtractORMFromVTK("metallicTex", 2, "attribute0", inTransform, ormImageData, false);
!     
!     if (metallicSampler != nullptr)
!     {
!       anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "metallic", metallicSampler);
!     }
!     else
!     {
!       anari::setParameter(this->AnariDevice, anariMaterial, "metallic", metallic);
!     }
    }
    else
    {
      anari::setParameter(this->AnariDevice, anariMaterial, "metallic", metallic);
    }
  
    // roughness
+   const float roughness = static_cast<float>(vtkProperty->GetRoughness());
+   
    if (ormTexture)
    {
      vtkImageData* ormImageData = ormTexture->GetInput();
      auto roughnessSampler =
        this->ExtractORMFromVTK("roughnessTex", 1, "attribute0", inTransform, ormImageData, false);
!     
!       if (roughnessSampler != nullptr)
!     {
!       anari::setAndReleaseParameter(
!         this->AnariDevice, anariMaterial, "roughness", roughnessSampler);
!     }
!     else
!     {
!       anari::setParameter(this->AnariDevice, anariMaterial, "roughness", roughness);
!     }
    }
    else
    {
      anari::setParameter(this->AnariDevice, anariMaterial, "roughness", roughness);
    }
  
    // normal map for the base layer
!   vtkTexture* normalTexture = vtkProperty->GetTexture("normalTex");
  
!   if (normalTexture)
    {
      auto normalSampler =
!       this->VTKToAnariSampler("normalTex", "attribute0", inTransform, normalTexture);
! 
!     if (normalSampler != nullptr)
!     {
!       anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "normal", normalSampler);
!     }
    }
  
    // emissive
!   vtkTexture* emissiveTexture = vtkProperty->GetTexture("emissiveTex");
  
!   if (emissiveTexture)
    {
      auto emissiveSampler =
!       this->VTKToAnariSampler("emissiveTex", "attribute0", inTransform, emissiveTexture);
! 
!     if (emissiveSampler != nullptr)
!     {
!       anari::setAndReleaseParameter(this->AnariDevice, anariMaterial, "emissive", emissiveSampler);
!     }
    }
  
    // occlusion map
    if (ormTexture)
    {
!     vtkImageData* ormImageData = ormTexture->GetInput();
      auto occlusionSampler =
        this->ExtractORMFromVTK("occlusionTex", 0, "attribute0", inTransform, ormImageData, false);
!     
!     if (occlusionSampler != nullptr)
!     {
!       anari::setAndReleaseParameter(
!         this->AnariDevice, anariMaterial, "occlusion", occlusionSampler);
!     }
    }
  
    // strength of the specular reflection
***************
*** 798,812 ****
    anari::setParameter(this->AnariDevice, anariMaterial, "clearcoatRoughness", coatRoughness);
  
    // normal map for the clearcoat layer
!   texture = vtkProperty->GetTexture("coatNormalTex");
  
!   if (texture)
    {
-     vtkImageData* coatNormalImageData = texture->GetInput();
      auto coatNormalSampler = this->VTKToAnariSampler(
!       "coatNormalTex", "attribute0", inTransform, coatNormalImageData, false);
!     anari::setAndReleaseParameter(
!       this->AnariDevice, anariMaterial, "clearCoatNormal", coatNormalSampler);
    }
  
    // index of refraction
--- 862,879 ----
    anari::setParameter(this->AnariDevice, anariMaterial, "clearcoatRoughness", coatRoughness);
  
    // normal map for the clearcoat layer
!   vtkTexture* coatNormalTexture = vtkProperty->GetTexture("coatNormalTex");
  
!   if (coatNormalTexture)
    {
      auto coatNormalSampler = this->VTKToAnariSampler(
!       "coatNormalTex", "attribute0", inTransform, coatNormalTexture);
! 
!     if (coatNormalSampler != nullptr)
!     {
!       anari::setAndReleaseParameter(
!         this->AnariDevice, anariMaterial, "clearCoatNormal", coatNormalSampler);
!     }
    }
  
    // index of refraction
***************
*** 901,906 ****
--- 968,974 ----
  
    if (texture)
    {
+     texture->Update();
      return texture->GetInput();
    }
  
***************
*** 912,918 ****
    vtkActor* actor, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, bool isTriangleIndex, double pointSize, double lineWidth,
    vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, vtkPolyDataMapperNode::vtkPDConnectivity& conn,
    int cellFlag)
  {
--- 980,986 ----
    vtkActor* actor, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, bool isTriangleIndex, double pointSize, double lineWidth,
    vtkDataArray* scaleArray, vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, vtkPolyDataMapperNode::vtkPDConnectivity& conn,
    int cellFlag)
  {
***************
*** 934,940 ****
            attributeArrays, cellFlag);
        }
  
!       this->Surfaces.emplace_back(anariSurface);
        break;
      }
      case VTK_WIREFRAME:
--- 1002,1011 ----
            attributeArrays, cellFlag);
        }
  
!       if (anariSurface != nullptr)
!       {
!         this->Surfaces.emplace_back(anariSurface);
!       }
        break;
      }
      case VTK_WIREFRAME:
***************
*** 954,964 ****
            attributeArrays, cellFlag);
        }
  
!       this->Surfaces.emplace_back(anariSurface);
        break;
      }
      default:
      {
        if (property->GetEdgeVisibility())
        {
          // Edge material
--- 1025,1041 ----
            attributeArrays, cellFlag);
        }
  
!       if (anariSurface != nullptr)
!       {
!         this->Surfaces.emplace_back(anariSurface);
!       }
        break;
      }
      default:
      {
+       double originalColor[3];
+       property->GetColor(originalColor);
+ 
        if (property->GetEdgeVisibility())
        {
          // Edge material
***************
*** 969,975 ****
          const auto useLineWidthForEdgeThickness = property->GetUseLineWidthForEdgeThickness();
  
          std::vector<vec2> edgeTextureCoords;
!         std::vector<float> edgePointValueTextureCoords;
          std::vector<vec4> edgePointColors;
  
          auto anariSurface = this->RenderAsCylinders(nullptr, property, poly, vertices,
--- 1046,1052 ----
          const auto useLineWidthForEdgeThickness = property->GetUseLineWidthForEdgeThickness();
  
          std::vector<vec2> edgeTextureCoords;
!         std::vector<vec2> edgePointValueTextureCoords;
          std::vector<vec4> edgePointColors;
  
          auto anariSurface = this->RenderAsCylinders(nullptr, property, poly, vertices,
***************
*** 977,983 ****
            useLineWidthForEdgeThickness ? lineWidth : edgeWidth, scaleArray, scaleFunction,
            edgeTextureCoords, edgePointValueTextureCoords, edgePointColors, attributeArrays,
            cellFlag);
!         this->Surfaces.emplace_back(anariSurface);
        }
  
        std::vector<vec3> vertexNormals;
--- 1054,1064 ----
            useLineWidthForEdgeThickness ? lineWidth : edgeWidth, scaleArray, scaleFunction,
            edgeTextureCoords, edgePointValueTextureCoords, edgePointColors, attributeArrays,
            cellFlag);
! 
!         if (anariSurface != nullptr)
!         {
!           this->Surfaces.emplace_back(anariSurface);
!         }
        }
  
        std::vector<vec3> vertexNormals;
***************
*** 1006,1015 ****
          }
        }
  
        auto anariSurface =
          this->RenderAsTriangles(anariSampler, property, poly, vertices, indexArray, vertexNormals,
            textureCoords, pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
!       this->Surfaces.emplace_back(anariSurface);
      }
    }
  }
--- 1087,1102 ----
          }
        }
  
+       property->SetColor(originalColor);
+ 
        auto anariSurface =
          this->RenderAsTriangles(anariSampler, property, poly, vertices, indexArray, vertexNormals,
            textureCoords, pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
!       
!       if (anariSurface != nullptr)
!       {
!         this->Surfaces.emplace_back(anariSurface);
!       }
      }
    }
  }
***************
*** 1018,1024 ****
  anari::Surface vtkAnariPolyDataMapperNodeInternals::RenderAsTriangles(anari::Sampler anariSampler,
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, std::vector<vec3>& normals, std::vector<vec2>& textureCoords,
!   std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsTriangles", vtkAnariProfiling::LIME);
--- 1105,1111 ----
  anari::Surface vtkAnariPolyDataMapperNodeInternals::RenderAsTriangles(anari::Sampler anariSampler,
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, std::vector<vec3>& normals, std::vector<vec2>& textureCoords,
!   std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsTriangles", vtkAnariProfiling::LIME);
***************
*** 1115,1121 ****
  
    if (updateResponsibility.Texcoords && (numTextureCoords > 0 || numPointValueTextureCoords > 0))
    {
-     std::vector<vec2> tcoords;
      anari::Array1D tcoordsArray = nullptr;
  
      if (numPointValueTextureCoords > 0)
--- 1202,1207 ----
***************
*** 1127,1133 ****
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = vec2{ pointValueTextureCoords[i], 0.0f };
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
--- 1213,1219 ----
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = pointValueTextureCoords[i];
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
***************
*** 1177,1183 ****
      }
      else
      {
!       int colorRepeatCount = numTriangles / numPointColors;
        colorRepeatCount = colorRepeatCount <= 0 ? 1 : colorRepeatCount;
  
        anari::Array1D colorArray =
--- 1263,1269 ----
      }
      else
      {
!       int colorRepeatCount = numTriangles / (numPointColors > 0 ? numPointColors : 1);
        colorRepeatCount = colorRepeatCount <= 0 ? 1 : colorRepeatCount;
  
        anari::Array1D colorArray =
***************
*** 1254,1260 ****
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, double lineWidth, vtkDataArray* scaleArray,
    vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsCylinders", vtkAnariProfiling::LIME);
--- 1340,1346 ----
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, double lineWidth, vtkDataArray* scaleArray,
    vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsCylinders", vtkAnariProfiling::LIME);
***************
*** 1374,1380 ****
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = vec2{ pointValueTextureCoords[i], 0.0f };
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
--- 1460,1466 ----
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = pointValueTextureCoords[i];
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
***************
*** 1404,1411 ****
  
    if (updateResponsibility.Colors && numPointColors > 0)
    {
-     // if(cellFlag == 0)
-     // {
      anari::Array1D colorArray =
        anari::newArray1D(this->AnariDevice, ANARI_FLOAT32_VEC4, numPointColors);
      {
--- 1490,1495 ----
***************
*** 1479,1485 ****
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, double lineWidth, vtkDataArray* scaleArray,
    vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsCurves", vtkAnariProfiling::LIME);
--- 1563,1569 ----
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, double lineWidth, vtkDataArray* scaleArray,
    vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsCurves", vtkAnariProfiling::LIME);
***************
*** 1597,1603 ****
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = vec2{ pointValueTextureCoords[i], 0.0f };
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
--- 1681,1687 ----
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = pointValueTextureCoords[i];
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
***************
*** 1627,1634 ****
  
    if (updateResponsibility.Colors && numPointColors > 0)
    {
-     // if(cellFlag == 0)
-     // {
      anari::Array1D colorArray =
        anari::newArray1D(this->AnariDevice, ANARI_FLOAT32_VEC4, numPointColors);
      {
--- 1711,1716 ----
***************
*** 1701,1707 ****
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, double pointSize, vtkDataArray* scaleArray,
    vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<float>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsSpheres", vtkAnariProfiling::LIME);
--- 1783,1789 ----
    vtkProperty* property, vtkPolyData* poly, std::vector<vec3>& vertices,
    std::vector<uint32_t>& indexArray, double pointSize, vtkDataArray* scaleArray,
    vtkPiecewiseFunction* scaleFunction, std::vector<vec2>& textureCoords,
!   std::vector<vec2>& pointValueTextureCoords, std::vector<vec4>& pointColors,
    AttributeArrayCollection& attributeArrays, int cellFlag)
  {
    vtkAnariProfiling startProfiling("VTKAPDMNInternals::RenderAsSpheres", vtkAnariProfiling::LIME);
***************
*** 1825,1831 ****
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = vec2{ pointValueTextureCoords[i], 0.0f };
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
--- 1907,1913 ----
  
          for (size_t i = 0; i < numPointValueTextureCoords; i++)
          {
!           tcoordsArrayPtr[i] = pointValueTextureCoords[i];
          }
  
          anari::unmap(this->AnariDevice, tcoordsArray);
***************
*** 2091,2096 ****
--- 2173,2196 ----
          vec4{ static_cast<float>(transform[12]), static_cast<float>(transform[13]),
            static_cast<float>(transform[14]), static_cast<float>(transform[15]) };
      }
+     else if (length == 9)
+     {
+       anariSamplerInTransform[0] =
+         vec4{ static_cast<float>(transform[0]), static_cast<float>(transform[1]),
+           static_cast<float>(transform[2]), 0.0f };
+       anariSamplerInTransform[1] =
+         vec4{ static_cast<float>(transform[3]), static_cast<float>(transform[4]),
+           static_cast<float>(transform[5]), 0.0f };
+       anariSamplerInTransform[2] =
+         vec4{ static_cast<float>(transform[6]), static_cast<float>(transform[7]),
+           static_cast<float>(transform[8]), 0.0f };
+       anariSamplerInTransform[3] = vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
+     }
+     else 
+     {
+       vtkWarningWithObjectMacro(
+         actor, "[AnariRenderPoly] Unsupported texture transform length: " << length);
+     }
    }
  
    // cellFlag == 0 => PointData - length must equal the number of points
***************
*** 2107,2114 ****
    if (mapper)
    {
      // Geometry
!     mapper->MapScalars(poly, 1.0, cellFlag);
!     mapperColors = mapper->GetColorMapColors();
  
      // Material
      mapperColorCoords = mapper->GetColorCoordinates();
--- 2207,2213 ----
    if (mapper)
    {
      // Geometry
!     mapperColors = mapper->MapScalars(poly, 1.0, cellFlag);
  
      // Material
      mapperColorCoords = mapper->GetColorCoordinates();
***************
*** 2116,2159 ****
    }
  
    // texture
!   int numTextureCoordinates = 0;
!   std::vector<vec2> textureCoords;
    vtkDataArray* da = poly->GetPointData()->GetTCoords();
  
    if (da != nullptr)
    {
!     numTextureCoordinates = da->GetNumberOfTuples();
  
!     for (int i = 0; i < numTextureCoordinates; i++)
      {
!       textureCoords.emplace_back(
          vec2{ static_cast<float>(da->GetTuple(i)[0]), static_cast<float>(da->GetTuple(i)[1]) });
      }
  
!     numTextureCoordinates *= 2;
    }
  
!   bool sRGB = false;
!   vtkImageData* albedoTextureMap = nullptr; // vColorTextureMap
!   vtkTexture* texture = nullptr;
  
    if (property->GetInterpolation() == VTK_PBR)
    {
!     texture = property->GetTexture("albedoTex");
    }
    else
    {
!     texture = actor->GetTexture();
!   }
! 
!   if (texture != nullptr)
!   {
!     sRGB = texture->GetUseSRGBColorSpace();
!     albedoTextureMap = texture->GetInput();
    }
  
    // Setup Material or Colors
-   std::vector<float> pointValueTextureCoords;
    std::vector<vec4> pointColors;
  
    if (mapperColors)
--- 2215,2259 ----
    }
  
    // texture
!   std::vector<vec2> pointValueTextureCoords;
    vtkDataArray* da = poly->GetPointData()->GetTCoords();
+   
+   vtkDataArray* cellTCoords = poly->GetCellData()->GetTCoords();
+   std::vector<vec2> cellValueTextureCoords;
  
    if (da != nullptr)
    {
!     const int numPointValueTextureCoords = da->GetNumberOfTuples();
  
!     for (int i = 0; i < numPointValueTextureCoords; i++)
      {
!       pointValueTextureCoords.emplace_back(
          vec2{ static_cast<float>(da->GetTuple(i)[0]), static_cast<float>(da->GetTuple(i)[1]) });
      }
+   }
  
!   if (cellTCoords != nullptr)
!   {
!     const int numCellValueTextureCoords = cellTCoords->GetNumberOfTuples();
!     for (int i = 0; i < numCellValueTextureCoords; i++)
!     {
!       cellValueTextureCoords.emplace_back(vec2{ static_cast<float>(cellTCoords->GetTuple(i)[0]),
!         static_cast<float>(cellTCoords->GetTuple(i)[1]) });
!     }
    }
  
!   vtkSmartPointer<vtkTexture> tmpAlbedoTex;
  
    if (property->GetInterpolation() == VTK_PBR)
    {
!     tmpAlbedoTex = property->GetTexture("albedoTex");
    }
    else
    {
!     tmpAlbedoTex = actor->GetTexture();
    }
  
    // Setup Material or Colors
    std::vector<vec4> pointColors;
  
    if (mapperColors)
***************
*** 2171,2184 ****
          int cflag2 = -1;
          vtkAbstractArray* scalars = mapper->GetAbstractScalars(poly, mapper->GetScalarMode(),
            mapper->GetArrayAccessMode(), mapper->GetArrayId(), mapper->GetArrayName(), cflag2);
-         vtkVariant v = scalars->GetVariantValue(mapper->GetFieldDataTupleId());
-         vtkIdType idx = s2c->GetAnnotatedValueIndex(v);
  
!         if (idx > -1)
          {
!           std::string name(s2c->GetAnnotation(idx));
!           property->SetMaterialName(name.c_str());
!           useMaterial = true;
          }
        }
  
--- 2271,2288 ----
          int cflag2 = -1;
          vtkAbstractArray* scalars = mapper->GetAbstractScalars(poly, mapper->GetScalarMode(),
            mapper->GetArrayAccessMode(), mapper->GetArrayId(), mapper->GetArrayName(), cflag2);
  
!         if (scalars != nullptr)
          {
!           vtkVariant v = scalars->GetVariantValue(mapper->GetFieldDataTupleId());
!           vtkIdType idx = s2c->GetAnnotatedValueIndex(v);
! 
!           if (idx > -1)
!           {
!             std::string name(s2c->GetAnnotation(idx));
!             property->SetMaterialName(name.c_str());
!             useMaterial = true;
!           }
          }
        }
  
***************
*** 2187,2196 ****
          // Use the color for the field data value
          int ncomps = mapperColors->GetNumberOfComponents();
          uint8_t* mapperColorsPtr = mapperColors->GetPointer(0);
!         mapperColorsPtr = mapperColorsPtr + mapper->GetFieldDataTupleId() * ncomps;
          double diffuseColor[3] = { mapperColorsPtr[0] * property->GetDiffuse() / 255.0,
            mapperColorsPtr[1] * property->GetDiffuse() / 255.0,
!           mapperColorsPtr[2] * property->GetDiffuse() / 255.0f };
          property->SetDiffuseColor(diffuseColor);
        }
      }
--- 2291,2300 ----
          // Use the color for the field data value
          int ncomps = mapperColors->GetNumberOfComponents();
          uint8_t* mapperColorsPtr = mapperColors->GetPointer(0);
!         mapperColorsPtr = mapperColorsPtr + (mapper->GetFieldDataTupleId() * ncomps);
          double diffuseColor[3] = { mapperColorsPtr[0] * property->GetDiffuse() / 255.0,
            mapperColorsPtr[1] * property->GetDiffuse() / 255.0,
!           mapperColorsPtr[2] * property->GetDiffuse() / 255.0 };
          property->SetDiffuseColor(diffuseColor);
        }
      }
***************
*** 2214,2230 ****
      if (mapperColorCoords && mapperColorTextureMap)
      {
        // color on point interpolated values (subsequently colormapped via 1D LUT)
!       int numPointValueTextureCoords = mapperColorCoords->GetNumberOfTuples();
!       pointValueTextureCoords.resize(numPointValueTextureCoords);
!       float* tc = mapperColorCoords->GetPointer(0);
  
!       for (int i = 0; i < numPointValueTextureCoords; i++)
        {
!         pointValueTextureCoords[i] = *tc;
!         tc += 2;
        }
  
!       albedoTextureMap = mapperColorTextureMap;
      }
    }
  
--- 2318,2356 ----
      if (mapperColorCoords && mapperColorTextureMap)
      {
        // color on point interpolated values (subsequently colormapped via 1D LUT)
!       const int numOfTuples = mapperColorCoords->GetNumberOfTuples();
!       const int ncomps = mapperColorCoords->GetNumberOfComponents();
  
!       const int numTexCoords = ncomps < 2 ? (numOfTuples / 2) : numOfTuples;
!       cellValueTextureCoords.resize(numTexCoords);
! 
!       for (int i = 0, j = 0; i < numOfTuples && j < numTexCoords; i++, j++)
        {
!         vec2 tcoord;
! 
!         if (ncomps >= 2)
!         {
!           tcoord = vec2{ static_cast<float>(mapperColorCoords->GetTuple(i)[0]),
!             static_cast<float>(mapperColorCoords->GetTuple(i)[1]) };
!         }
!         else if (ncomps == 1)
!         {
!           tcoord = vec2{ static_cast<float>(mapperColorCoords->GetTuple(i)[0]),
!             static_cast<float>(mapperColorCoords->GetTuple(++i)[0]) };
!         }
!         else
!         {
!           tcoord = vec2{ static_cast<float>(*mapperColorCoords->GetTuple(i)),
!             static_cast<float>(*mapperColorCoords->GetTuple(++i)) };
!         }
! 
!         cellValueTextureCoords[j] = tcoord;
! 
        }
  
!       tmpAlbedoTex = vtkSmartPointer<vtkTexture>::New();
!       tmpAlbedoTex->SetInputData(mapperColorTextureMap);
!       tmpAlbedoTex->SetUseSRGBColorSpace(false);
      }
    }
  
***************
*** 2329,2337 ****
      if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_SPHERE)
      {
        auto anariSampler = this->Internal->VTKToAnariSampler(
!         "albedoTex", "attribute0", anariSamplerInTransform, albedoTextureMap, sRGB);
        anariSurface = this->Internal->RenderAsSpheres(anariSampler, property, poly, vertices,
!         conn.vertex_index, pointSize, scaleArray, scaleFunction, textureCoords,
          pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
      }
  
--- 2455,2463 ----
      if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_SPHERE)
      {
        auto anariSampler = this->Internal->VTKToAnariSampler(
!         "albedoTex", "attribute0", anariSamplerInTransform, tmpAlbedoTex);
        anariSurface = this->Internal->RenderAsSpheres(anariSampler, property, poly, vertices,
!         conn.vertex_index, pointSize, scaleArray, scaleFunction, cellValueTextureCoords,
          pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
      }
  
***************
*** 2342,2355 ****
    {
      anari::Surface anariSurface = nullptr;
  
!     if (property->GetRepresentation() == VTK_POINTS)
      {
        if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_SPHERE)
        {
          auto anariSampler = this->Internal->VTKToAnariSampler(
!           "albedoTex", "attribute0", anariSamplerInTransform, albedoTextureMap, sRGB);
          anariSurface = this->Internal->RenderAsSpheres(anariSampler, property, poly, vertices,
!           conn.line_index, pointSize, scaleArray, scaleFunction, textureCoords,
            pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
        }
      }
--- 2468,2481 ----
    {
      anari::Surface anariSurface = nullptr;
  
!     if (connRepresentation == VTK_POINTS)
      {
        if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_SPHERE)
        {
          auto anariSampler = this->Internal->VTKToAnariSampler(
!           "albedoTex", "attribute0", anariSamplerInTransform, tmpAlbedoTex);
          anariSurface = this->Internal->RenderAsSpheres(anariSampler, property, poly, vertices,
!           conn.line_index, pointSize, scaleArray, scaleFunction, cellValueTextureCoords,
            pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
        }
      }
***************
*** 2358,2374 ****
        if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_CYLINDER)
        {
          auto anariSampler = this->Internal->VTKToAnariSampler(
!           "albedoTex", "attribute0", anariSamplerInTransform, albedoTextureMap, sRGB);
          anariSurface = this->Internal->RenderAsCylinders(anariSampler, property, poly, vertices,
!           conn.line_index, lineWidth, scaleArray, scaleFunction, textureCoords,
            pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
        }
        else if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_CURVE)
        {
          auto anariSampler = this->Internal->VTKToAnariSampler(
!           "albedoTex", "attribute0", anariSamplerInTransform, albedoTextureMap, sRGB);
          anariSurface = this->Internal->RenderAsCurves(anariSampler, property, poly, vertices,
!           conn.line_index, lineWidth, scaleArray, scaleFunction, textureCoords,
            pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
        }
      }
--- 2484,2500 ----
        if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_CYLINDER)
        {
          auto anariSampler = this->Internal->VTKToAnariSampler(
!           "albedoTex", "attribute0", anariSamplerInTransform, tmpAlbedoTex);
          anariSurface = this->Internal->RenderAsCylinders(anariSampler, property, poly, vertices,
!           conn.line_index, lineWidth, scaleArray, scaleFunction, cellValueTextureCoords,
            pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
        }
        else if (anariDeviceExtensions.ANARI_KHR_GEOMETRY_CURVE)
        {
          auto anariSampler = this->Internal->VTKToAnariSampler(
!           "albedoTex", "attribute0", anariSamplerInTransform, tmpAlbedoTex);
          anariSurface = this->Internal->RenderAsCurves(anariSampler, property, poly, vertices,
!           conn.line_index, lineWidth, scaleArray, scaleFunction, cellValueTextureCoords,
            pointValueTextureCoords, pointColors, attributeArrays, cellFlag);
        }
      }
***************
*** 2382,2399 ****
    if (!conn.triangle_index.empty())
    {
      auto anariSampler = this->Internal->VTKToAnariSampler(
!       "albedoTex", "attribute0", anariSamplerInTransform, albedoTextureMap, sRGB);
      this->Internal->RenderSurfaces(anariSampler, actor, poly, vertices, conn.triangle_index, true,
!       pointSize, lineWidth, scaleArray, scaleFunction, textureCoords, pointValueTextureCoords,
        pointColors, attributeArrays, conn2, cellFlag);
    }
  
    if (!conn.strip_index.empty())
    {
      auto anariSampler = this->Internal->VTKToAnariSampler(
!       "albedoTex", "attribute0", anariSamplerInTransform, albedoTextureMap, sRGB);
      this->Internal->RenderSurfaces(anariSampler, actor, poly, vertices, conn.strip_index, false,
!       pointSize, lineWidth, scaleArray, scaleFunction, textureCoords, pointValueTextureCoords,
        pointColors, attributeArrays, conn2, cellFlag);
    }
  }
--- 2508,2525 ----
    if (!conn.triangle_index.empty())
    {
      auto anariSampler = this->Internal->VTKToAnariSampler(
!       "albedoTex", "attribute0", anariSamplerInTransform, tmpAlbedoTex);
      this->Internal->RenderSurfaces(anariSampler, actor, poly, vertices, conn.triangle_index, true,
!       pointSize, lineWidth, scaleArray, scaleFunction, cellValueTextureCoords, pointValueTextureCoords,
        pointColors, attributeArrays, conn2, cellFlag);
    }
  
    if (!conn.strip_index.empty())
    {
      auto anariSampler = this->Internal->VTKToAnariSampler(
!       "albedoTex", "attribute0", anariSamplerInTransform, tmpAlbedoTex);
      this->Internal->RenderSurfaces(anariSampler, actor, poly, vertices, conn.strip_index, false,
!       pointSize, lineWidth, scaleArray, scaleFunction, cellValueTextureCoords, pointValueTextureCoords,
        pointColors, attributeArrays, conn2, cellFlag);
    }
  }
***************
*** 2540,2546 ****
    }
    else
    {
!     this->Internal->ActorName = &"vtk_actor_"[this->RendererNode->ReservePropId()];
    }
  }
  
--- 2666,2672 ----
    }
    else
    {
!     this->Internal->ActorName = "vtk_actor_" + std::to_string(this->RendererNode->ReservePropId());
    }
  }

EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch ${current_patch}/${count_patches} for vtkAnariPolyDataMapperNode.cxx failed."
        return 1
    fi
    
    # 5) vtkAnariSceneGraph update
    ((current_patch++))
    patch -p0 << \EOF 
*** Rendering/ANARI/vtkAnariSceneGraph.cxx.orig	2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariSceneGraph.cxx	    2026-02-20 10:45:43.271451336 -0600
***************
*** 227,242 ****
  {
    const uvec2 frameSize = { static_cast<uint32_t>(this->Size[0]),
      static_cast<uint32_t>(this->Size[1]) };
!   if ((uint32_t)this->Internal->ImageX == frameSize[0] &&
!     (uint32_t)this->Internal->ImageY == frameSize[1])
    {
      return;
    }
  
    this->Internal->ImageX = frameSize[0];
    this->Internal->ImageY = frameSize[1];
! 
!   const size_t totalSize = this->Size[0] * this->Size[1];
    this->Internal->ColorBuffer.resize(totalSize * sizeof(float));
    this->Internal->DepthBuffer.resize(totalSize);
  
--- 227,244 ----
  {
    const uvec2 frameSize = { static_cast<uint32_t>(this->Size[0]),
      static_cast<uint32_t>(this->Size[1]) };
!   const size_t totalSize = this->Size[0] * this->Size[1];
!   if ((uint32_t)this->Internal->ImageY == frameSize[1] && !this->Internal->ColorBuffer.empty() &&
!     !this->Internal->DepthBuffer.empty() &&
!     this->Internal->ColorBuffer.size() == totalSize * sizeof(float) &&
!     this->Internal->DepthBuffer.size() == totalSize)
    {
      return;
    }
  
    this->Internal->ImageX = frameSize[0];
    this->Internal->ImageY = frameSize[1];
!   
    this->Internal->ColorBuffer.resize(totalSize * sizeof(float));
    this->Internal->DepthBuffer.resize(totalSize);
  
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch $current_patch/$count_patches for vtkAnariSceneGraph.cxx failed."
        return 1
    fi
}

function apply_vtk95_vktanari_patches
{
    count_patches=2
    # patch vtkAnari files:
   
    # 1) support panning and zooming
    current_patch=1
    patch -p0 << \EOF
*** Rendering/ANARI/vtkAnariCameraNode.cxx.orig  2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariCameraNode.cxx	2025-08-14 10:10:09.887501406 -0500
***************
*** 144,155 ****
      right = true;
    }
  
!   int* const ts = this->Internals->RendererNode->GetScale();
  
    if (this->Internals->IsParallelProjection)
    {
      // height of the image plane in world units
!     double height = cam->GetParallelScale() * 2 * ts[0];
      anari::setParameter(this->Internals->AnariDevice, this->Internals->AnariCamera, "height",
        static_cast<float>(height));
    }
--- 144,164 ----
      right = true;
    }
  
!   int* const ts = this->Internals->RendererNode->GetScale();  
!   vtkHomogeneousTransform* transform = cam->GetUserTransform();
!   double zoomFactor = 1.0;
!   
!   // Support zooming
!   if (transform != nullptr)
!   {
!     auto matrix = transform->GetMatrix();
!     zoomFactor = matrix->GetElement(0, 0);
!   }
  
    if (this->Internals->IsParallelProjection)
    {
      // height of the image plane in world units
!     double height = (cam->GetParallelScale() * 2 * ts[0]) / zoomFactor;
      anari::setParameter(this->Internals->AnariDevice, this->Internals->AnariCamera, "height",
        static_cast<float>(height));
    }
***************
*** 157,162 ****
--- 166,172 ----
    {
      // The field of view (angle in radians) of the frame's height
      float fovyDegrees = static_cast<float>(cam->GetViewAngle()) * static_cast<float>(ts[0]);
+     fovyDegrees /= static_cast<float>(zoomFactor);
      float fovyRadians = vtkMath::RadiansFromDegrees(fovyDegrees);
      anari::setParameter(
        this->Internals->AnariDevice, this->Internals->AnariCamera, "fovy", fovyRadians);
***************
*** 234,261 ****
      static_cast<float>(myFocalPoint[2] - shiftedCamPos[2]) };
    anari::setParameter(
      this->Internals->AnariDevice, this->Internals->AnariCamera, "direction", cameraDirection);
! 
!   // Additional world-space transformation matrix
!   vtkHomogeneousTransform* transform = cam->GetUserTransform();
! 
!   if (transform != nullptr)
!   {
!     double* matrix = transform->GetMatrix()->GetData();
!     float matrixF[16];
! 
!     for (int i = 0; i < 16; i++)
!     {
!       matrixF[i] = static_cast<float>(matrix[i]);
!     }
! 
!     anari::setParameter(
!       this->Internals->AnariDevice, this->Internals->AnariCamera, "transform", matrixF);
!   }
! 
    // Region of the sensor in normalized screen-space coordinates
    double viewPort[4] = { 0, 0, 1, 1 };
    this->Internals->RendererNode->GetViewport(viewPort);
  
    box2 imageRegion = { vec2{ static_cast<float>(viewPort[0]), static_cast<float>(viewPort[1]) },
      vec2{ static_cast<float>(viewPort[2]), static_cast<float>(viewPort[3]) } };
    anari::setParameter(
--- 244,274 ----
      static_cast<float>(myFocalPoint[2] - shiftedCamPos[2]) };
    anari::setParameter(
      this->Internals->AnariDevice, this->Internals->AnariCamera, "direction", cameraDirection);
!     
    // Region of the sensor in normalized screen-space coordinates
    double viewPort[4] = { 0, 0, 1, 1 };
    this->Internals->RendererNode->GetViewport(viewPort);
  
+   // Support image panning in applications (e.g. VisIt)
+   if(!cam->GetUseExplicitProjectionTransformMatrix())
+   {
+     // Convert VTK camera window center in viewport coordinates (range is: [-1,+1],[-1,+1])
+     // to normalized screen-space coordinates (range is: [0,1],[0,1]).
+     auto windowCenter = cam->GetWindowCenter();
+     double wcx = windowCenter[0] / 2.0 + 0.5;
+     double wcy = windowCenter[1] / 2.0 + 0.5;
+ 
+     // Offset based on the width of the current viewport
+     double offsetX = (viewPort[2] - viewPort[0]) / 2.0;
+     double offsetY = (viewPort[3] - viewPort[1]) / 2.0;
+ 
+     // Adjust viewport to center around window center
+     viewPort[0] = wcx - offsetX;
+     viewPort[1] = wcy - offsetY;
+     viewPort[2] = wcx + offsetX;
+     viewPort[3] = wcy + offsetY;
+   }
+ 
    box2 imageRegion = { vec2{ static_cast<float>(viewPort[0]), static_cast<float>(viewPort[1]) },
      vec2{ static_cast<float>(viewPort[2]), static_cast<float>(viewPort[3]) } };
    anari::setParameter(
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch ${current_patch}/${count_patches} for vtkAnariCameraNode.cxx failed."
        return 1
    fi
    
    # 2) expose internal vtkAnariPass via vtkAnariVolumeMapper
    ((current_patch++))
    patch -p0 << \EOF
*** Rendering/ANARI/vtkAnariVolumeMapper.h.orig	 2025-06-23 14:12:36.000000000 -0500
--- Rendering/ANARI/vtkAnariVolumeMapper.h	2025-08-06 16:10:00.556764266 -0500
***************
*** 52,57 ****
--- 52,62 ----
     * Allow vtkAnariSceneGraph properties to be set on the internal vtkRenderer.
     */
    vtkRenderer* GetInternalRenderer() const { return this->InternalRenderer; }
+   
+   /**
+    * Get the internal ANARI render pass.
+    */
+   vtkAnariPass* GetAnariPass() const { return this->InternalAnariPass; }
  
    //@{
    /**
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 ANARI patch $current_patch/$count_patches for vtkAnariVolumeMapper.h failed."
        return 1
    fi
}

function apply_vtk95_vtk_convex_point_set_patch
{
    # patch vtkConvexPointSet to fix a contouring bug.
    patch -p0 << \EOF
--- Common/DataModel/vtkConvexPointSet.cxx.orig	2026-05-19 14:58:47.244708000 -0700
+++ Common/DataModel/vtkConvexPointSet.cxx	2026-05-19 15:01:39.993741000 -0700
@@ -59,7 +59,13 @@
   if (numPts < 1)
     return;

-  this->Triangulate(0, this->TetraIds, this->TetraPoints);
+  this->TriangulateLocalIds(0, this->TetraIds);
+
+  this->TetraPoints->SetNumberOfPoints(this->TetraIds->GetNumberOfIds());
+  for (int i = 0; i < this->TetraIds->GetNumberOfIds(); i++)
+  {
+    this->TetraPoints->SetPoint(i, this->Points->GetPoint(this->TetraIds->GetId(i)));
+  }
 }

 //------------------------------------------------------------------------------
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkConvexPointSet.cxx failed."
      return 1
    fi
    return 0;
}

function apply_vtk_patch
{
    if [[ ${VTK_VERSION} == 9.5.0 ]] ; then
        apply_vtk95_vtkospray_patches
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk95_macos_dock_popup_fux
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk95_vtkdatawriter_patch
        if [[ $? != 0 ]] ; then
           return 1
        fi

        apply_vtk95_vtkopenfoamreader_patch
        if [[ $? != 0 ]] ; then
           return 1
        fi

        # should submit a ticket to kitware
        apply_vtk95_vtkRectilinearGridReader_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
        
        # MR submitted to kitware for these updates
        # Remove when upgrading to 9.6.0 or later
        apply_vtk95_vktanari_patches
        if [[ $? != 0 ]] ; then
            return 1
        fi

        # MR merged for this patch in VTK "master" branch, but we need to 
        # apply it to 9.5 branch. 
        # see: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12759
        # Remove when upgrading to 9.6.0 or later
        apply_vtk95_texture_anari_patches
        if [[ $? != 0 ]] ; then
            return 1    
        fi

        # should submit a MR to kitware
        apply_vtk95_vtk_convex_point_set_patch
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
    prepare_build_dir $VTK_BUILD_DIR $VTK_FILE SHA256 $VTK_SHA256_CHECKSUM
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
    vopts="${vopts} -DVTK_BUILD_DOCUMENTATION:BOOL=OFF"
    vopts="${vopts} -DVTK_FORBID_DOWNLOADS:BOOL=ON"
    # setting this to true causes errors when building debug versions of
    # visit, so set it to false
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
    # one of the vtk modules introduced this case for StandALone
    # Probably a mistake, but guard against it anyways as it shows up
    # in the Cache.
    vopts="${vopts} -DVTK_GROUP_ENABLE_STANDALONE:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Views:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Web:STRING=DONT_WANT"

    # Turn on individual modules. dependent modules are turned on automatically
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_CommonCore:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersFlowPaths:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersHybrid:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersModeling:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersVerdict:STRING=YES"
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
    vopts="${vopts} -DVTK_ENABLE_REMOTE_MODULES:BOOL=OFF"

    # Tell VTK where to locate qmake if we're building graphical support. We
    # do not add graphical support for server-only builds.
    if [[ "$DO_QT" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GUISupportQt:STRING=YES"
        vopts="${vopts} -DQt6_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6"
        vopts="${vopts} -DQt6CoreTools_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6CoreTools"
        vopts="${vopts} -DQt6GuiTools_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6GuiTools"
        vopts="${vopts} -DQt6WidgetsTools_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6WidgetsTools"
    fi

    # Add python wrapping
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        # python... but static libs and python filters are incompatible.
        if [[ "$DO_STATIC_BUILD" != "yes" ]]; then
            py="${PYTHON_COMMAND}"
            pyinc="${PYTHON_INCLUDE_DIR}"
            pylib="${PYTHON_LIBRARY}"

            vopts="${vopts} -DVTK_WRAP_PYTHON:BOOL=true"
            vopts="${vopts} -DPython3_EXECUTABLE:FILEPATH=${py}"
            vopts="${vopts} -DPython3_EXTRA_LIBS:STRING=\"${VTK_PY_LIBS}\""
            vopts="${vopts} -DPython3_INCLUDE_DIR:PATH=${pyinc}"
            vopts="${vopts} -DPython3_LIBRARY:FILEPATH=${pylib}"
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi

    # For now, turn off EGL (Our large-image regression tests fail)
    vopts="${vopts} -DOPENGL_EGL_INCLUDE_DIR:PATH=\"\""
    vopts="${vopts} -DOPENGL_egl_LIBRARY:FILEPATH=\"\""
    vopts="${vopts} -DVTK_OPENGL_HAS_EGL:BOOL=OFF"

    if [[ "$OPSYS" == "Linux" ]]; then
        vopts="${vopts} -DVTK_USE_X:BOOL=ON"
    fi

    # Use Mesa as GL?
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
        vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING=${MESAGL_OPENGL_LIB}"
        vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH=${MESAGL_GLU_LIB}"
        # for now, until Mesa can be updated to a version that supports GLVND,
        # set LEGACY preference
        vopts="${vopts} -DOpenGL_GL_PREFERENCE:STRING=LEGACY"
        vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
        vopts="${vopts} -DOSMESA_LIBRARY:STRING=${MESAGL_OSMESA_LIB}"
        vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"

    #elif [[ "$DO_OSMESA" == "yes" ]] ; then
    #    # Is there a use-case where VTK-9.5 would need to build against only OSMesa?
    #    vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH="
    #    vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING="
    #    vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
    #    vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH="
    #    vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
    #    vopts="${vopts} -DOSMESA_LIBRARY:STRING=\"${OSMESA_LIB}\""
    #    vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${OSMESA_INCLUDE_DIR}"
    #    vopts="${vopts} -DVTK_USE_X:BOOL=OFF"
    fi

    # Use OSPRay?
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=YES"
        if [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib/cmake/ospray-${OSPRAY_VERSION}"
        elif [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib64 ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib64/cmake/ospray-${OSPRAY_VERSION}"
        # newer versions of ospray (at least on macOS) install layout lack the extra ospray dir
        elif [[ -d ${OSPRAY_INSTALL_DIR}/lib ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/lib/cmake/ospray-${OSPRAY_VERSION}"
        elif [[ -d ${OSPRAY_INSTALL_DIR}/lib64 ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/lib64/cmake/ospray-${OSPRAY_VERSION}"
        else
            warn "Disabling ospray because its lib dir couldn't be found"
            vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=NO"
        fi
    fi

    # Use ANARI?
    if [[ "$DO_ANARI" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingAnari:STRING=YES"
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersTexture:STRING=YES"
        if [[ -d ${VISITDIR}/anari/${ANARI_VERSION}/${VISITARCH}/lib64 ]] ; then
            vopts="${vopts} -Danari_DIR=${VISITDIR}/anari/${ANARI_VERSION}/${VISITARCH}/lib64/cmake/anari-${ANARI_VERSION}"
        else
            vopts="${vopts} -Danari_DIR=${VISITDIR}/anari/${ANARI_VERSION}/${VISITARCH}/lib/cmake/anari-${ANARI_VERSION}"
        fi

        if [[ "$DO_ANARI_NVTX" == "yes" ]] ; then
            vopts="${vopts} -DVTK_ANARI_ENABLE_NVTX:BOOL=ON"
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

    if [[ "$DO_MESAGL" == "yes" || "$DO_OSMESA" == "yes"  ]] ; then
        export LD_LIBRARY_PATH="${LLVM_LIB_DIR}:$LD_LIBRARY_PATH"
    fi

    #
    # VTK's CMake logic for version numbering the created shared libraries has
    # a number of issues. It can wind up duplicating the maj/min pair in the
    # the names. It can also wind up adding a 1 patch digit when the patch
    # level is 0. So, we need to enforce it by adding these CMake variable settings.
    #
    vopts="${vopts} -DVTK_CUSTOM_LIBRARY_SUFFIX:STRING=\"${VTK_SHORT_VERSION}\" -DVTK_CUSTOM_LIBRARY_VERSION:STRING=\"\" -DVTK_CUSTOM_LIBRARY_SOVERSION:STRING=\"\""

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command".  This was first discovered on
    # BGQ and then showed up in random cases for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - we write a simple
    # script that we invoke with bash which calls cmake with all of the proper
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
    env DYLD_LIBRARY_PATH=`pwd`/bin ${CMAKE_COMMAND} --build . $MAKE_OPT_FLAGS || \
        error "VTK did not build correctly.  Giving up."

    info "Installing VTK . . . "
    ${CMAKE_COMMAND} --install . || error "VTK did not install correctly."

    cleanup_build_dirs $VTK_BUILD_DIR $VTK_SRC_DIR

    # Filter out an include that references the user's VTK build directory
    configdir="${vtk_inst_path}/lib/cmake/vtk-${VTK_SHORT_VERSION}"
    cat ${configdir}/VTKConfig.cmake | grep -v "vtkTestingMacros" > ${configdir}/VTKConfig.cmake.new
    mv ${configdir}/VTKConfig.cmake.new ${configdir}/VTKConfig.cmake

    change_install_dir_perms ${VISITDIR}/${VTK_INSTALL_DIR}

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

    if [[ "$DO_VTK" == "yes" ]] ; then
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
