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
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

#include <math.h>

#ifndef __APPLE__
#include <GL/glu.h>
#else
#include <OpenGL/glu.h>
#endif

   vtkStandardNewMacro(vtkOSPRayCamera);

//----------------------------------------------------------------------------
   vtkOSPRayCamera::vtkOSPRayCamera()
   {
    this->OSPRayManager = NULL;
  }

//----------------------------------------------------------------------------
  vtkOSPRayCamera::~vtkOSPRayCamera()
  {
    if (this->OSPRayManager)
    {
      this->OSPRayManager->Delete();
    }
  }

//----------------------------------------------------------------------------
  void vtkOSPRayCamera::OrientOSPRayCamera(vtkRenderer *ren)
  {
    vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
    if (!OSPRayRenderer)
    {
      return;
    }
    OSPRayRenderer->ClearAccumulation();

    if (!this->OSPRayManager)
    {
      this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
      this->OSPRayManager->Register(this);
    }

  // for figuring out aspect ratio
    int lowerLeft[2];
    int usize, vsize;
    ren->GetTiledSizeAndOrigin(&usize, &vsize, lowerLeft, lowerLeft + 1);

    double *eye, *lookat, *up, vfov;
    eye    = this->Position;
    lookat = this->FocalPoint;
    up     = this->ViewUp;
    vfov   = this->ViewAngle;

    OSPCamera ospCamera = ((OSPCamera)this->OSPRayManager->OSPRayCamera);
    if (vsize == 0)
      return;
    ospSetf(ospCamera,"aspect",float(usize)/float(vsize));
    ospSetf(ospCamera,"fovy",vfov);
    Assert(ospCamera != NULL && "could not create camera");
    ospSet3f(ospCamera,"pos",eye[0], eye[1], eye[2]);
    ospSet3f(ospCamera,"up",up[0], up[1], up[2]);
    ospSet3f(ospCamera,"dir",lookat[0]-eye[0],lookat[1]-eye[1],lookat[2]-eye[2]);
    ospCommit(ospCamera);

  }

//----------------------------------------------------------------------------
// called by Renderer::UpdateCamera()
  void vtkOSPRayCamera::Render(vtkRenderer *ren)
  {
    int lowerLeft[2];
    int usize, vsize;
    ren->GetTiledSizeAndOrigin(&usize, &vsize, lowerLeft, lowerLeft + 1);
    double newAspect = float(usize)/float(vsize);
    if (this->GetMTime() > this->LastRenderTime || (newAspect != this->Aspect) )
    {
      this->Aspect = newAspect;
      this->OrientOSPRayCamera(ren);

      this->LastRenderTime.Modified();

    }
  }
