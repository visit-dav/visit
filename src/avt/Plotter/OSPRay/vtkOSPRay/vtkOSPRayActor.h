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

// .NAME vtkOSPRayActor - vtkActor for OSPRay Ray traced scenes
// .SECTION Description
// vtkOSPRayActor is a concrete implementation of the abstract class vtkActor.
// vtkOSPRayActor interfaces to the OSPRay Raytracer library.

#ifndef __vtkOSPRayActor_h
#define __vtkOSPRayActor_h

#include "vtkOSPRayModule.h"
#include "vtkOSPRayRenderable.h"
#include "vtkActor.h"
#include <map>
#include <vector>


//BTX
namespace OSPRay {
class Group;
class AccelerationStructure;
class Object;
};
//ETX

namespace osp
{
  class Model;
}

class vtkTimeStamp;
class vtkOSPRayProperty;
class vtkOSPRayRenderer;
class vtkOSPRayManager;



class VTKOSPRAY_EXPORT vtkOSPRayActor : public vtkActor
{
public:
  static vtkOSPRayActor *New();
  vtkTypeMacro(vtkOSPRayActor,vtkActor);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  // Overriden to help ensure that a OSPRay compatible class is created.
  vtkProperty * MakeProperty();

  // Description:
  // This causes the actor to be rendered. It in turn will render the actor's
  // property, texture map and then mapper. If a property hasn't been
  // assigned, then the actor will create one automatically. Note that a side
  // effect of this method is that the pipeline will be updated.
  void Render(vtkRenderer *ren, vtkMapper *mapper);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  //Description:
  // Overridden to schedule a transaction to hide the object
  virtual void SetVisibility(int);

  //Description:
  // Transaction callback that hides the object
  void RemoveObjects();

  void PreRender();

  //Description:
  //Lets you choose the OSPRay space sorting (acceleration) structure
  //type used internally. Default is 0=DYNBVH
  vtkSetMacro(SortType, int);
  vtkGetMacro(SortType, int);

  int GetLastFrame() { return LastFrame; }

 protected:
  vtkOSPRayActor();
  ~vtkOSPRayActor();

  virtual void CreateOwnLODs () {}
  virtual void UpdateOwnLODs() {}

 private:
  vtkOSPRayActor(const vtkOSPRayActor&);  // Not implemented.
  void operator=(const vtkOSPRayActor&);  // Not implemented.

  void UpdateObjects(vtkRenderer *);

  int SortType;

  //BTX
  enum {DYNBVH, RECURSIVEGRID3};
  //ETX

  vtkOSPRayManager *OSPRayManager;
  vtkOSPRayRenderable* Renderable;

public:
  int LastFrame;
  vtkTimeStamp MeshMTime;
  osp::Model* OSPRayModel;
  std::map<int, osp::Model*> cache;
};

#endif // __vtkOSPRayActor_h
