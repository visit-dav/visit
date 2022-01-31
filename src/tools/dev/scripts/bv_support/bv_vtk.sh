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
    export VTK_FILE=${VTK_FILE:-"VTK-9.1.0.tar.gz"}
    export VTK_VERSION=${VTK_VERSION:-"9.1.0"}
    export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"9.1"}
    export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
    export VTK_URL=${VTK_URL:-"https://www.vtk.org/files/release/${VTK_SHORT_VERSION}"}
    export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK-9.1.0"}
    export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
    #export VTK_MD5_CHECKSUM="fa61cd36491d89a17edab18522bdda49"
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

function apply_vtkopenfoamreader_header_patch
{
  # patch vtk's OpenFOAMReader to provide more meta data information
  # useful for VisIt's plugin.

   patch -p0 << \EOF
*** IO/Geometry/vtkOpenFOAMReader.h.orig	Mon Apr 19 13:46:49 2021
--- IO/Geometry/vtkOpenFOAMReader.h	Mon Apr 19 13:50:25 2021
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

function apply_vtkopenfoamreader_source_patch
{
  # patch vtk's OpenFOAMReader to provide more meta data information
  # useful for VisIt's plugin.

   patch -p0 << \EOF
*** IO/Geometry/vtkOpenFOAMReader.cxx	Thu Apr  8 05:26:53 2021
--- IO/Geometry/vtkOpenFOAMReader.cxx	Tue Apr 20 16:35:35 2021
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

function apply_libxmlversionheader_patch
{
  # patch vtk's libxml CMakeLists.txt so that xml_version.h is installed
   patch -p0 << \EOF
*** ThirdParty/libxml2/vtklibxml2/CMakeLists.txt.orig	Wed Jan 12 11:24:42 2022
--- ThirdParty/libxml2/vtklibxml2/CMakeLists.txt	Wed Jan 12 11:25:57 2022
***************
*** 771,779 ****
  endif ()
  
  configure_file(include/libxml/xmlversion.h.in include/libxml/xmlversion.h)
! if (FALSE) # XXX(kitware): mask installation rules
! install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libxml/xmlversion.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libxml2/libxml COMPONENT development)
! endif ()
  
  if(MSVC)
  	configure_file(include/libxml/xmlwin32version.h.in libxml/xmlwin32version.h)
--- 771,779 ----
  endif ()
  
  configure_file(include/libxml/xmlversion.h.in include/libxml/xmlversion.h)
! #if (FALSE) # XXX(kitware): mask installation rules
! install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/libxml/xmlversion.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libxml2/libxml)
! #endif ()
  
  if(MSVC)
  	configure_file(include/libxml/xmlwin32version.h.in libxml/xmlwin32version.h)

EOF
    if [[ $? != 0 ]] ; then
        warn "vtk patch for xml_version.h installation failed."
        return 1
    fi
}

function apply_vtkRectilinearGridReader_patch
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

function apply_vtkCutter_patch
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

function apply_vtk_patch
{
    # this needs to be reworked for 9.1.0
    #apply_vtkopenfoamreader_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    # this needs to be reworked for 9.1.0
    #apply_vtkopenglspheremapper_h_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    # this needs to be reworked for 9.1.0
    #apply_vtkopenglspheremapper_patch
    #if [[ $? != 0 ]] ; then
    #    return 1
    #fi

    apply_libxmlversionheader_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtkRectilinearGridReader_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    apply_vtkCutter_patch
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
        vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING=${MESAGL_OPENGL_LIB}"
        vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH=${MESAGL_GLU_LIB}"
        # for now, until Mesa can be updated to a version that supports GLVND, set LEGACY preference
        vopts="${vopts} -DOpenGL_GL_PREFERENCE:STRING=LEGACY"
        # Cannot build onscreen and offscreen this way anymore
        #vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
        #vopts="${vopts} -DOSMESA_LIBRARY:STRING=${MESAGL_OSMESA_LIB}"
        #vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"

        if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" || "$DO_ENGINE_ONLY" == "yes" ]] ; then
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
            vopts="${vopts} -DVTK_PYTHON_VERSION:STRING=3"
            vopts="${vopts} -DPython3_EXECUTABLE:FILEPATH=${py}"
            vopts="${vopts} -DPython3_EXTRA_LIBS:STRING=${VTK_PY_LIBS}"
            vopts="${vopts} -DPython3_INCLUDE_DIR:PATH=${pyinc}"
            vopts="${vopts} -DPython3_LIBRARY:FILEPATH=${pylib}"
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi


    # Use OSPRay?
    #if [[ "$DO_OSPRAY" == "yes" ]] ; then
    #    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:BOOL=ON"
    #    vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}"
    #    vopts="${vopts} -Dembree_DIR=${EMBREE_INSTALL_DIR}"
    #fi

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
