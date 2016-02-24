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

// .NAME vtkOSPRayObjectFactory -
// .SECTION Description
//

#include "vtkOSPRay.h"
#include "vtkOSPRayRenderer.h"

#include "vtkOSPRayObjectFactory.h"

#include "vtkDebugLeaks.h"
#include "vtkDynamicLoader.h"
#include "vtkOverrideInformation.h"
#include "vtkOverrideInformationCollection.h"
#include "vtkVersion.h"

#include "vtkOSPRayActor.h"
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayLight.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayTexture.h"

#ifdef VTKOSPRAY_FOR_PARAVIEW
#include "vtkOSPRayLODActor.h"
#endif

#include <DebugStream.h>

vtkStandardNewMacro(vtkOSPRayObjectFactory);

//----------------------------------------------------------------------------
void vtkOSPRayObjectFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "VTK OSPRay object factory" << endl;
}

//----------------------------------------------------------------------------
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayActor);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayCamera);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayLight);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayPolyDataMapper);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayProperty);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayRenderer);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayTexture);
#ifdef VTKOSPRAY_FOR_PARAVIEW
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayLODActor);
#endif

//----------------------------------------------------------------------------
vtkOSPRayObjectFactory::vtkOSPRayObjectFactory()
{
#ifdef VTKOSPRAY_FOR_PARAVIEW
  this->RegisterOverride("vtkPVLODActor",
                         "vtkOSPRayLODActor",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayLODActor);
#endif

  vtkOverrideInformationCollection *oic = 
      vtkOverrideInformationCollection::New();

  vtkObjectFactory::GetOverrideInformation("vtkActor", oic);
  if(oic->GetNumberOfItems() != 0)
      vtkObjectFactory::SetAllEnableFlags(0, "vtkActor");
  this->RegisterOverride("vtkActor",
                         "vtkOSPRayActor",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayActor);
  vtkObjectFactory::GetOverrideInformation("vtkCamera", oic);
  if(oic->GetNumberOfItems() != 0)
      vtkObjectFactory::SetAllEnableFlags(0, "vtkCamera");
  this->RegisterOverride("vtkCamera",
                         "vtkOSPRayCamera",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayCamera);
  vtkObjectFactory::GetOverrideInformation("vtkPolyDataMapper", oic);
  if(oic->GetNumberOfItems() != 0)
      vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper");
  this->RegisterOverride("vtkPolyDataMapper",
                         "vtkOSPRayPolyDataMapper",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayPolyDataMapper);
  vtkObjectFactory::GetOverrideInformation("vtkProperty", oic);
  if(oic->GetNumberOfItems() != 0)
      vtkObjectFactory::SetAllEnableFlags(0, "vtkProperty");
  this->RegisterOverride("vtkProperty",
                         "vtkOSPRayProperty",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayProperty);
  vtkObjectFactory::GetOverrideInformation("vtkRenderer", oic);
  if(oic->GetNumberOfItems() != 0)
      vtkObjectFactory::SetAllEnableFlags(0, "vtkRenderer");
  this->RegisterOverride("vtkRenderer",
                         "vtkOSPRayRenderer",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayRenderer);
  vtkObjectFactory::GetOverrideInformation("vtkTexture", oic);
  if(oic->GetNumberOfItems() != 0)
      vtkObjectFactory::SetAllEnableFlags(0, "vtkTexture");
  this->RegisterOverride("vtkTexture",
                         "vtkOSPRayTexture",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayTexture);
}

//----------------------------------------------------------------------------
//ALOK: removing this because it conflicts with vtkManta
//VTK_FACTORY_INTERFACE_IMPLEMENT(vtkOSPRayObjectFactory);

//----------------------------------------------------------------------------
const char *vtkOSPRayObjectFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}

//----------------------------------------------------------------------------
const char *vtkOSPRayObjectFactory::GetDescription()
{
  return "VTK OSPRay Object Factory";
}
