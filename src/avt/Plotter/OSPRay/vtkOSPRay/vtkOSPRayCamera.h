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

// .NAME vtkOSPRayCamera - OSPRay camera
// .SECTION Description
// vtkOSPRayCamera is a concrete implementation of the abstract class
// vtkCamera.  vtkOSPRayCamera interfaces to the OSPRay Raytracer library.

#ifndef __vtkOSPRayCamera_h
#define __vtkOSPRayCamera_h

#include "vtkOSPRayModule.h"
#include "vtkCamera.h"

//BTX
namespace OSPRay {
class Camera;
}
//ETX

class vtkTimeStamp;
class vtkOSPRayManager;

class VTKOSPRAY_EXPORT vtkOSPRayCamera : public vtkCamera
{
public:
  static vtkOSPRayCamera *New();
  vtkTypeMacro(vtkOSPRayCamera,vtkCamera);

  void Render(vtkRenderer *ren);

protected:
   vtkOSPRayCamera();
  ~vtkOSPRayCamera();

private:
  vtkOSPRayCamera(const vtkOSPRayCamera&);  // Not implemented.
  void operator=(const vtkOSPRayCamera&);  // Not implemented.

  void OrientOSPRayCamera(vtkRenderer *);

  vtkTimeStamp LastRenderTime;

  vtkOSPRayManager *OSPRayManager;
  double Aspect;
};

#endif
