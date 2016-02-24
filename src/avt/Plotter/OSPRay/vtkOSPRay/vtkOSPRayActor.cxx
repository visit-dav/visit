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

#define GL_GLEXT_PROTOTYPES

#include "vtkOSPRay.h"
#include "vtkOSPRayActor.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"
#include "vtkMapper.h"

#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkTimerLog.h"

#include <map>
#include <algorithm>


#include "vtkInformation.h"
#include "vtkInformationVector.h"

vtkStandardNewMacro(vtkOSPRayActor);

//----------------------------------------------------------------------------
vtkOSPRayActor::vtkOSPRayActor()
{
  this->OSPRayManager = NULL;
  this->SortType = DYNBVH;
  this->OSPRayModel = ospNewModel();
  LastFrame=-1;
  Renderable = NULL;
}

// now some OSPRay resources, ignored previously, can be de-allocated safely
//
vtkOSPRayActor::~vtkOSPRayActor()
{
  if (this->OSPRayManager)
  {
    this->ReleaseGraphicsResources(NULL);
    this->OSPRayManager->Delete();
  }
  // delete this->OSPRayModel;
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
vtkProperty *vtkOSPRayActor::MakeProperty()
{
  return vtkOSPRayProperty::New();
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::ReleaseGraphicsResources( vtkWindow * win )
{
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::Render( vtkRenderer * ren, vtkMapper * mapper )
{
  if ( vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast( ren ) )
  {
    if (!this->OSPRayManager)
    {
      this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
      this->OSPRayManager->Register(this);
    }
    // if (LastFrame < OSPRayRenderer->GetFrame())
    // {
      // printf("creating new model for actor\n");
      // this->OSPRayModel = ospNewModel();
    // }

    // TODO: be smarter on update or create rather than create every time
    // build transformation (with AffineTransfrom and Instance?)

    // TODO: the way "real FLAT" shading is done right now (by not supplying vertex
    // normals), changing from FLAT to Gouraud shading needs to create a new mesh.

      mapper->Render(ren, this);
      LastFrame = OSPRayRenderer->GetFrame();
    }
      UpdateObjects(ren);
  }

//----------------------------------------------------------------------------
void vtkOSPRayActor::SetVisibility(int newval)
{
  if (newval == this->GetVisibility())
  {
    return;
  }
  if (this->OSPRayManager && !newval)
  {
  }
  this->Superclass::SetVisibility(newval);
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::RemoveObjects()
{
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::PreRender()
{
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  // this->OSPRayModel = ospNewModel();
}


//----------------------------------------------------------------------------
void vtkOSPRayActor::UpdateObjects( vtkRenderer * ren )
{
  vtkOSPRayRenderer * OSPRayRenderer =
  vtkOSPRayRenderer::SafeDownCast( ren );
  if (!OSPRayRenderer)
  {
    return;
  }

  //Remove whatever we used to show in the scene
  if (!this->OSPRayManager)
  {
    return;
  }

  if (!this->OSPRayModel)
    return;

  if (!this->GetVisibility())
    return;
  
  Renderable = new vtkOSPRayRenderable(this->OSPRayModel);
  OSPRayRenderer->AddOSPRayRenderable(Renderable);

}
