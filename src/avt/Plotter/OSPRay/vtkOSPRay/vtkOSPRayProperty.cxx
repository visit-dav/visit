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

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

#include "vtkOSPRay.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"


#include <cstring>




vtkStandardNewMacro(vtkOSPRayProperty);

//----------------------------------------------------------------------------
vtkOSPRayProperty::vtkOSPRayProperty()
{
  this->MaterialType = NULL;
  this->SetMaterialType("default");
  this->OSPRayManager = NULL;
  this->OSPRayMaterial= NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayProperty::~vtkOSPRayProperty()
{
  if (this->OSPRayManager)
  {
    this->OSPRayManager->Delete();
  }
  delete[] this->MaterialType;
}

//----------------------------------------------------------------------------
void vtkOSPRayProperty::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
void vtkOSPRayProperty::ReleaseGraphicsResources(vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  if (!this->OSPRayManager)
  {
    return;
  }
}

//----------------------------------------------------------------------------
void vtkOSPRayProperty::Render( vtkActor *vtkNotUsed(anActor),
 vtkRenderer * ren)
{
  vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast( ren );
  if (!OSPRayRenderer)
  {
    return;
  }
  if (!this->OSPRayManager)
  {
    this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
    this->OSPRayManager->Register(this);
  }

  double * diffuse  = this->GetDiffuseColor();


  if ( this->GetMTime() > this->OSPRayMaterialMTime )
  {
      CreateOSPRayProperty();
    this->OSPRayMaterialMTime.Modified();
  }

}

//----------------------------------------------------------------------------
// Implement base class method.
void vtkOSPRayProperty::BackfaceRender( vtkActor * vtkNotUsed( anActor ),
 vtkRenderer * vtkNotUsed( ren ) )
{
  cerr
  << "vtkOSPRayProperty::BackfaceRender(), backface rendering "
  << "is not supported by OSPRay"
  << endl;
}


//----------------------------------------------------------------------------
void vtkOSPRayProperty::CreateOSPRayProperty()
{

  double * diffuse  = this->GetDiffuseColor();
  double * specular = this->GetSpecularColor();



          OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);

          this->OSPRayMaterial = ospNewMaterial(renderer,"OBJMaterial");
          OSPMaterial oMaterial = (OSPMaterial)this->OSPRayMaterial;
          Assert(oMaterial);
          float diffusef[] = {(float)diffuse[0], (float)diffuse[1], (float)diffuse[2]};
          float specularf[] = {(float)specular[0],(float)specular[1],(float)specular[2]};
          ospSet3fv(oMaterial,"Kd",diffusef);
          ospSet3fv(oMaterial,"Ks",specularf);
          ospSet1f(oMaterial,"Ns",float(this->GetSpecularPower()*.5));
          ospSet1f(oMaterial,"d", float(this->GetOpacity()));

          ospCommit(oMaterial);



  }
