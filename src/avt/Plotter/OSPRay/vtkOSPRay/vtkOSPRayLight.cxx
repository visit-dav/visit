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
#include "vtkOSPRayLight.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

#include <math.h>

vtkStandardNewMacro(vtkOSPRayLight);

//----------------------------------------------------------------------------
vtkOSPRayLight::vtkOSPRayLight()
{
  this->OSPRayManager = NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayLight::~vtkOSPRayLight()
{
  if (this->OSPRayManager)
    {
    this->OSPRayManager->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkOSPRayLight::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkOSPRayLight::Render(vtkRenderer *ren, int /* not used */)
{
  vtkOSPRayRenderer *renderer = vtkOSPRayRenderer::SafeDownCast(ren);
  if (!renderer)
    {
    return;
    }

  // if (!this->OSPRayLight)
    {
    CreateLight(ren);
    }
  // else
    {
    // UpdateLight(ren);
    }
}

//----------------------------------------------------------------------------
// called in Transaction context, it is safe to modify the engine state here
void vtkOSPRayLight::CreateLight(vtkRenderer *ren)
{
  vtkOSPRayRenderer *OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
  if (!OSPRayRenderer)
    {
    return;
    }

    if (!this->OSPRayManager)
    {
    this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
    this->OSPRayManager->Register(this);
    }

  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
      std::vector<OSPLight> pointLights;
      std::vector<OSPLight> directionalLights;


  double *color, *position, *focal, direction[3];

  // OSPRay Lights only have one "color"
  color    = this->GetDiffuseColor();
  position = this->GetTransformedPosition();
  focal    = this->GetTransformedFocalPoint();

  if (this->GetPositional())
    {
    OSPLight ospLight = ospNewLight(renderer, "OBJPointLight");
    ospSetString(ospLight, "name", "point" );
    ospSet3f(ospLight, "color", color[0],color[1],color[2]);
    ospSet3f(ospLight, "position", position[0],position[1],position[2]);
    ospCommit(ospLight);
    pointLights.push_back(ospLight);
    OSPData pointLightArray = ospNewData(pointLights.size(), OSP_OBJECT, &pointLights[0], 0);
    ospSetData(renderer, "pointLights", pointLightArray);
    }
  else
    {
    direction[0] = position[0] - focal[0];
    direction[1] = position[1] - focal[1];
    direction[2] = position[2] - focal[2];
    OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
    ospSetString(ospLight, "name", "sun" );
    ospSet3f(ospLight, "color", color[0],color[1],color[2]);
    ospSet3f(ospLight, "direction", direction[0],direction[1],direction[2]);
    ospCommit(ospLight);
    directionalLights.push_back(ospLight);
    OSPData pointLightArray = ospNewData(directionalLights.size(), OSP_OBJECT, &directionalLights[0], 0);
    ospSetData(renderer, "directionalLights", pointLightArray);

    }
}

//------------------------------------------------------------------------------
void vtkOSPRayLight::UpdateLight(vtkRenderer *ren)
{
  CreateLight(ren);
}
