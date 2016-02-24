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

#include "vtkOSPRayManager.h"
#include "vtkObjectFactory.h"
#include "vtkOSPRay.h"

vtkOSPRayManager* vtkOSPRayManager::__singleton = NULL;

// vtkStandardNewMacro(vtkOSPRayManager);

vtkOSPRayManager* vtkOSPRayManager::New()
{
  return vtkOSPRayManager::Singleton();
}

//----------------------------------------------------------------------------
vtkOSPRayManager::vtkOSPRayManager()
{
  VolumeModelLastFrame=-1;
  if (1)
  {
    int ac =1;
    const char* av[] = {"pvOSPRay\0","--osp:verbose\0"};
    ospInit(&ac, av);
  }
  else  //coi
  {
    int ac =2;
    const char* av[] = {"pvOSPRay\0","--osp:coi","\0"};
    ospInit(&ac, av);
  }
  this->OSPRayVolumeRenderer = (osp::Renderer*)ospNewRenderer("raycast_volume_renderer");
  this->OSPRayCamera = ospNewCamera("perspective");
  this->OSPRayModel = ospNewModel();
  this->OSPRayVolumeModel = this->OSPRayModel;
}

//----------------------------------------------------------------------------
vtkOSPRayManager::~vtkOSPRayManager()
{

}

//----------------------------------------------------------------------------
void vtkOSPRayManager::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

vtkOSPRayManager* vtkOSPRayManager::Singleton()
{
  if (!__singleton)
    __singleton = new vtkOSPRayManager();
  return __singleton;
}
