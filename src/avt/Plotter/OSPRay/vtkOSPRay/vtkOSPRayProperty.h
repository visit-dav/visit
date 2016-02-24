/* =======================================================================================
   Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin
   All rights reserved.

   Licensed under the BSD 3-Clause License, (the "License"); you may not use this file
   except in compliance with the License.
   A copy of the License is included with this software in the file LICENSE.
   If your copy does not contain the License, you may obtain a copy of the License at:

       http://opensource.org/licenses/BSD-3-Clause

   Unless required by applicable law or agreed to in writing, software distributed under
   the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.
   See the License for the specific language governing permissions and limitations under
   limitations under the License.

   pvOSPRay is derived from VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
   Copyright (c) 2007, Los Alamos National Security, LLC
   ======================================================================================= */

// .NAME vtkOpenProperty - OSPRay property
// .SECTION Description
// vtkOSPRayProperty is a concrete implementation of the abstract class
// vtkProperty. vtkOSPRayProperty interfaces to the OSPRay Raytracer library.

#ifndef __vtkOSPRayProperty_h
#define __vtkOSPRayProperty_h

#include "vtkOSPRayModule.h"
#include "vtkProperty.h"

//BTX
namespace OSPRay {
  class Material;
}
//ETX

namespace osp
{
  class Material;
  class OSPTexture2D;
}

class vtkOSPRayRenderer;
class vtkOSPRayManager;

class VTKOSPRAY_EXPORT vtkOSPRayProperty : public vtkProperty
{
public:
  static vtkOSPRayProperty *New();
  vtkTypeMacro(vtkOSPRayProperty,vtkProperty) ;
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Render(vtkActor *a, vtkRenderer *ren);

  // Description:
  // Implement base class method.
  void BackfaceRender(vtkActor *a, vtkRenderer *ren);

  // Description:
  // Release any graphics resources that are being consumed by this
  // property. The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *win);

  // functions that change parameters of various materials
  vtkSetStringMacro(MaterialType);
  vtkGetStringMacro(MaterialType);
  vtkSetMacro(Reflectance, float);
  vtkGetMacro(Reflectance, float);
  vtkSetMacro(Eta, float);
  vtkGetMacro(Eta, float);
  vtkSetMacro(Thickness, float);
  vtkGetMacro(Thickness, float);
  vtkSetMacro(N, float);
  vtkGetMacro(N, float);
  vtkSetMacro(Nt, float);
  vtkGetMacro(Nt, float);
    vtkSetMacro(OSPRayMaterial, osp::Material*);
  vtkGetMacro(OSPRayMaterial, osp::Material*);

  //Description:
  //Internal callbacks for OSPRay thread use.
  //Do not call them directly.
  void CreateOSPRayProperty();

protected:
  vtkOSPRayProperty();
  ~vtkOSPRayProperty();

private:
  vtkOSPRayProperty(const vtkOSPRayProperty&);  // Not implemented.
  void operator=(const vtkOSPRayProperty&);  // Not implemented.

  // the last time OSPRayMaterial is modified
  vtkTimeStamp OSPRayMaterialMTime;

  osp::OSPTexture2D* DiffuseTexture;

  // type of material to use. possible values are: "lambertian", "phong",
  // "transparent", "thindielectric", "dielectric", "metal", "orennayer"
  char * MaterialType;

  // amount of reflection to use. should be between 0.0 and 1.0
  float Reflectance;

  // the index of refraction for a material. used with the thin dielectric
  // material
  float Eta;

  // how thick the material is. used with the thin dielectric material
  float Thickness;

  // index of refraction for outside material. used in dielectric material
  float N;

  // index of refraction for inside material (transmissive). used in
  // dielectric material
  float Nt;

  vtkOSPRayManager *OSPRayManager;
  osp::Material* OSPRayMaterial;
};

#endif
