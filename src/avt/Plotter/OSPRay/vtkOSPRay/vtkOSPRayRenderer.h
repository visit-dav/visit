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

// .NAME vtkOSPRayRenderer - Renderer that uses OSPRay ray tracer instead of GL.
// .SECTION Description
// vtkOSPRayRenderer is a concrete implementation of the abstract class
// vtkRenderer. vtkOSPRayRenderer interfaces to the OSPRay graphics library.

#ifndef __vtkOSPRayRenderer_h
#define __vtkOSPRayRenderer_h

#include "vtkOSPRayModule.h"
#include "vtkOpenGLRenderer.h"

#include "vtkOSPRay.h"

#include "vtkOSPRayRenderable.h"
#include <queue>
#include <utility>
#include <string>


//BTX
namespace OSPRay {
class OSPRayInterface;
class Scene;
class Group;
class LightSet;
class Factory;
class Camera;
class SyncDisplay;
class Light;
}
//ETX

class vtkOSPRayManager;

class VTKOSPRAY_EXPORT vtkOSPRayRenderer : public vtkOpenGLRenderer
{
public:
  static vtkOSPRayRenderer *New();
  vtkTypeMacro(vtkOSPRayRenderer, vtkOpenGLRenderer);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  // Overridden to use OSPRay callbacks to do the work.
  virtual void SetBackground(double rgb[3])
    { this->Superclass::SetBackground(rgb); }
  virtual void SetBackground(double r, double g, double b);

  //Description:
  //Changes the number of OSPRay rendering threads.
  //More is faster.
  //Default is 1.
  void SetNumberOfWorkers(int);
  vtkGetMacro(NumberOfWorkers, int);

  //Description:
  //Turns on or off shadow rendering.
  //Default is off.
  void SetEnableShadows(int);
  vtkGetMacro(EnableShadows, int);

    //Description:
  //Turns on or off shadow rendering.
  //Default is off.
  void SetEnableAO(int);
  vtkGetMacro(EnableAO, int);

  //Description:
  //Turns on or off shadow rendering.
  //Default is off.
  void SetEnablePathtracing(int);
  vtkGetMacro(EnablePathtracing, int);
  //Description:
  //Turns on or off shadow rendering.
  //Default is off.
  void SetEnableVolumeShading(int);
  vtkGetMacro(EnableVolumeShading, int);

  //Description:
  //Controls multisample (anitaliased) rendering.
  //More looks better, but is slower.
  //Default is 1.
  void SetSamples(int);
  vtkGetMacro(Samples, int);

  vtkGetMacro(Accumulate, bool);
  void SetAccumulate(bool st)
  {
    Accumulate = st;
  }

  //Description:
  //Controls maximum ray bounce depth.
  //More looks better, but is slower.
  //Default is 5 meaning a couple of bounces.
  void SetMaxDepth(int);
  vtkGetMacro(MaxDepth, int);

  // Description:
  // Ask lights to load themselves into graphics pipeline.
  int UpdateLights(void);

  // Description:
  // Turns off all lighting
  void ClearLights(void);


  void Clear();
  void ClearAccumulation();

  //Description:
  //Access to the OSPRay rendered image
  float * GetColorBuffer()
  {
    return this->ColorBuffer;
  }
  float * GetDepthBuffer()
  {
    return this->DepthBuffer;
  }

  //Setup ospray world, camera etc. before
  // renders are called in the geometry
  void PreRender();
  // Description:
  // Concrete render method. Do not call this directly. The pipeline calls
  // it during Renderwindow::Render()
  void DeviceRender();

  void LayerRender();

  //Description:
  //All classes that make OSPRay calls should get hold of this and
  //Register it so that the Manager, and thus the OSPRay engine
  //outlive themselves, and thus guarantee that they can safely make
  //OSPRay API calls whenever they need to.
  vtkOSPRayManager* GetOSPRayManager()
  {
    return this->OSPRayManager;
  }

  void SetHasVolume(bool st) { HasVolume=st;}
  void SetProgressiveRenderFlag() {prog_flag = true; }
  void SetClearAccumFlag() {ClearAccumFlag = true; }
  int GetAccumCounter() { return AccumCounter; }
  int GetMaxAccumulation() { return MaxAccum; }
  int GetFrame() { return Frame; }
  vtkSetMacro(ComputeDepth, bool);

  void AddOSPRayRenderable(vtkOSPRayRenderable* inst);

protected:
  vtkOSPRayRenderer();
  ~vtkOSPRayRenderer();

  //lets OSPRay engine know when viewport changes
  void UpdateSize();

  // OSPRay renderer does not support picking.
  virtual void DevicePickRender() {};
  virtual void StartPick(unsigned int vtkNotUsed(pickFromSizev)) {};
  virtual void UpdatePickId() {};
  virtual void DonePick() {};
  virtual unsigned int GetPickedId() { return 0; };
  virtual unsigned int GetNumPickedIds() { return 0; };
  virtual int GetPickedIds( unsigned int vtkNotUsed(atMost), unsigned int * vtkNotUsed(callerBuffer) )
  {
    return 0;
  };
  virtual double GetPickedZ() { return 0.0f; };
  //creates the internal OSPRay renderer object

  void UpdateOSPRayRenderer();

private:
  vtkOSPRayRenderer(const vtkOSPRayRenderer&); // Not implemented.
  void operator=(const vtkOSPRayRenderer&); // Not implemented.

  void InitEngine();

  //Description:
  // Overriden to help ensure that a OSPRay compatible class is created.
  vtkCamera * MakeCamera();
  std::vector<vtkOSPRayRenderable*> renderables;

  bool IsStereo;
  bool EngineInited;
  bool EngineStarted;

  int ImageX;
  int ImageY;
  OSPFrameBuffer osp_framebuffer;

  float *ColorBuffer;
  float *DepthBuffer;


  int ChannelId;

  vtkOSPRayManager *OSPRayManager;

  int NumberOfWorkers;
  int EnableShadows;
  int EnableAO;
  int EnablePathtracing;
  int EnableVolumeShading;
  int Samples;
  int MaxDepth;
  bool Accumulate;
  int AccumCounter;
  int MaxAccum;
  bool prog_flag;
  int Frame;
  bool ComputeDepth;
  bool HasVolume;
  bool ClearAccumFlag;

  double backgroundRGB[3];
  std::queue<std::pair<std::string, double> > Statistics;
  int StatisticFramesPerOutput;
};

#endif
