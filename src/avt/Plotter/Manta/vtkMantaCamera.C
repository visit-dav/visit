/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMantaCamera.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    $RCSfile: vtkMantaCamera.cxx,v $

Copyright (c) 2007, Los Alamos National Security, LLC

All rights reserved.

Copyright 2007. Los Alamos National Security, LLC.
This software was produced under U.S. Government contract DE-AC52-06NA25396
for Los Alamos National Laboratory (LANL), which is operated by
Los Alamos National Security, LLC for the U.S. Department of Energy.
The U.S. Government has rights to use, reproduce, and distribute this software.
NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY,
EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.
If software is modified to produce derivative works, such modified software
should be clearly marked, so as not to confuse it with the version available
from LANL.

Additionally, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions
are met:
-   Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
-   Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
-   Neither the name of Los Alamos National Security, LLC, Los Alamos National
    Laboratory, LANL, the U.S. Government, nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "vtkManta.h"
#include "vtkMantaCamera.h"
#include "vtkMantaManager.h"
#include "vtkMantaRenderer.h"

#include "vtkObjectFactory.h"

#include <Interface/Camera.h>
#include <Engine/Control/RTRT.h>
#include <math.h>
#include <fstream>

#include <Core/Math/MiscMath.h>
#include <Core/Math/Trig.h>

vtkStandardNewMacro(vtkMantaCamera);

//----------------------------------------------------------------------------
vtkMantaCamera::vtkMantaCamera() : MantaCamera (0)
{
  //TODO: Observe my own modified event, and call OrientCamera then
  //cerr << "MC(" << this << ") CREATE" << endl;
  this->MantaManager = NULL;
}

//----------------------------------------------------------------------------
vtkMantaCamera::~vtkMantaCamera()
{
  //cerr << "MC(" << this << ") DESTROY" << endl;
  if (this->MantaManager)
    {
    //cerr << "MC(" << this << ") DESTROY " << this->MantaManager << " "
    //     << this->MantaManager->GetReferenceCount() << endl;
    this->MantaManager->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMantaCamera::OrientMantaCamera(vtkRenderer *ren)
{
 // cerr << "MC(" << this << ") ORIENT" << endl;
  vtkMantaRenderer * mantaRenderer = vtkMantaRenderer::SafeDownCast(ren);
  if (!mantaRenderer)
    {
    return;
    }

  if (!this->MantaCamera)
    {
    this->MantaCamera = mantaRenderer->GetMantaCamera();
    if (!this->MantaCamera)
      {
      return;
      }
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
  Manta::Real disk   = this->GetFocalDisk();

  Manta::Real hfov = vfov;
  Manta::Real fov_min = 0;
  Manta::Real fov_max = 180;
  hfov = Manta::RtoD(2*Manta::Atan(disk*Manta::Tan(Manta::DtoR(hfov/2))));
  hfov = Manta::Clamp((Manta::Real)hfov, (Manta::Real)fov_min, (Manta::Real)fov_max);
  vfov = Manta::RtoD(2*Manta::Atan(disk*Manta::Tan(Manta::DtoR(vfov/2))));
  vfov = Manta::Clamp((Manta::Real)vfov, (Manta::Real)fov_min, (Manta::Real)fov_max);
//  hfov = vfov*usize/vsize;

  //Carson: import camera data
#if 0

  static std::string path("");
  static bool once1 = false;
  if (!once1)
  {
    once1 = true;
    char* cpath = getenv("VISIT_MANTA_CAMERA_FILENAME");
    if (cpath)
      path = std::string(cpath);
  }
  if (path != "")
  {
    static double teye[3], tlookat[3], tup[3], thfov, tvfov;
    static bool once = false;
    if (!once)
    {
      std::ifstream in(path.c_str());
      once = true;
      std::string str;
      while (in >> str)
      {
        if (str == "BasicCameraData:")
        {
          in >> teye[0] >> teye[1] >> teye[2];
          in >> tlookat[0] >> tlookat[1] >> tlookat[2];
          in >> tup[0] >> tup[1] >> tup[2];
          in >> thfov >> tvfov;
        }
      }
    }
    printf("found camera data:\n%f%f%f\n", teye[0], teye[1], teye[2]);
    eye[0] = teye[0];  eye[1] = teye[1]; eye[2] = teye[2];
    lookat[0] = tlookat[0];  lookat[1] = tlookat[1]; lookat[2] = tlookat[2];
    up[0] = tup[0];  up[1] = tup[1]; up[2] = tup[2];
    hfov = thfov;
    vfov = tvfov;
  }

#endif

  const Manta::BasicCameraData bookmark
    (
     Manta::Vector(eye[0], eye[1], eye[2]),
     Manta::Vector(lookat[0], lookat[1], lookat[2]),
     Manta::Vector(up[0], up[1], up[2]),
     vfov * usize / vsize, vfov
    );


  //cout << "updating camera info " << eye[0] << " " << eye[1] << " " << eye[2] << " " << vfov << " " << disk << "\n";
  //cout << lookat[0] << " " << lookat[1] << " " << lookat[2] << endl;
  mantaRenderer->GetMantaEngine()->addTransaction
    ("update camera",
     Manta::Callback::create(this->MantaCamera,
       &Manta::Camera::setBasicCameraData, bookmark)
    );
}

//----------------------------------------------------------------------------
// called by Renderer::UpdateCamera()
void vtkMantaCamera::Render(vtkRenderer *ren)
{
  if (this->GetMTime() > this->LastRenderTime)
  {
    this->OrientMantaCamera(ren);

    this->LastRenderTime.Modified();
  }
}
