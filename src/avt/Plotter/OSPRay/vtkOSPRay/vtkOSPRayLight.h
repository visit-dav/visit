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

// .NAME vtkOSPRayLight - OSPRay light
// .SECTION Description
// vtkOSPRayLight is a concrete implementation of the abstract class vtkLight.
// vtkOSPRayLight interfaces to the OSPRay Raytracer library.

#ifndef __vtkOSPRayLight_h
#define __vtkOSPRayLight_h

#include "vtkOSPRayModule.h"
#include "vtkLight.h"

//BTX
namespace OSPRay {
class Light;
}
//ETX

class vtkOSPRayRenderer;
class vtkTimeStamp;
class vtkOSPRayManager;

class VTKOSPRAY_EXPORT vtkOSPRayLight : public vtkLight
{
public:
  static vtkOSPRayLight *New();
  vtkTypeMacro(vtkOSPRayLight, vtkLight);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Render(vtkRenderer *ren, int light_index);

protected:
  vtkOSPRayLight();
  ~vtkOSPRayLight();

private:
  vtkOSPRayLight(const vtkOSPRayLight&); // Not implemented.
  void operator=(const vtkOSPRayLight&); // Not implemented.

  void CreateLight(vtkRenderer *);
  void UpdateLight(vtkRenderer *ren);

  vtkOSPRayManager *OSPRayManager;

};

#endif
