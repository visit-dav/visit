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

// .NAME vtkOSPRayManager - persistant access to OSPRay engine
// .SECTION Description
// vtkOSPRayManager is a reference counted wrapper around the OSPRay engine.
// Because it is reference counted, it outlives all vtkOSPRay classes that
// reference it. That means that they can safely use it to manage their
// own OSPRay side resources and that the engine itself will be destructed
// when the wrapper is.
//
// Carson: This class initializes OSPRay and must be called through
// the singleton before any OSPRay code is used.
//

#ifndef __vtkOSPRayManager_h
#define __vtkOSPRayManager_h

#include "vtkObject.h"
#include "vtkOSPRayModule.h"
#include <vector>


//BTX
namespace OSPRay {
class Camera;
class Factory;
class Group;
class LightSet;
class OSPRayInterface;
class Scene;
class SyncDisplay;
};
//ETX

namespace osp
{
class Renderer;
class Model;
class Camera;
}


class VTKOSPRAY_EXPORT vtkOSPRayManager : public vtkObject
{
public:
  vtkTypeMacro(vtkOSPRayManager,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  static vtkOSPRayManager* Singleton();
  static vtkOSPRayManager* New();

protected:
  vtkOSPRayManager();
  ~vtkOSPRayManager();

private:
  vtkOSPRayManager(const vtkOSPRayManager&);  // Not implemented.
  void operator=(const vtkOSPRayManager&);  // Not implemented.

  static vtkOSPRayManager* __singleton;

public:
//
//  OSPRay vars
//
  osp::Model* OSPRayModel;
  osp::Renderer*    OSPRayRenderer;
  osp::Camera*      OSPRayCamera;
  osp::Model* OSPRayDynamicModel;
  osp::Renderer*    OSPRayVolumeRenderer;
  osp::Model* OSPRayVolumeModel;
  vtkTimeStamp  VolumeModelBuildTime;
  int VolumeModelLastFrame;
};

#endif
