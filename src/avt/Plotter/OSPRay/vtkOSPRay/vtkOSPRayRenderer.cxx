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

#include "vtkActor.h"
#include "vtkCuller.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkTimerLog.h"

#include "vtkImageData.h"
#include "vtkPNGWriter.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
//#include "vtkMultiProcessController.h"

#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


#include "vtkOSPRayActor.h"
#include <DebugStream.h>

//  VBOs
//
#if USE_VBOS
#include <GL/glu.h>
#include <assert.h>
#endif


#include <string>

vtkStandardNewMacro(vtkOSPRayRenderer);

class vtkTimerCallback : public vtkCommand
{
public:
  static vtkTimerCallback *New()
  {
    vtkTimerCallback *cb = new vtkTimerCallback;
    cb->TimerCount = 0;
    return cb;
  }

  virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId,
                       void *vtkNotUsed(callData))
  {
    if (vtkCommand::TimerEvent == eventId)
    {
      ++this->TimerCount;
    }
  }

private:
  int TimerCount;

};

//----------------------------------------------------------------------------
vtkOSPRayRenderer::vtkOSPRayRenderer()
:
prog_flag(false),
Accumulate(false)
{
    debug5 << "ALOK: vtkOSPRayRenderer constructor" << endl;
  Frame=0;
  HasVolume= false;
  ClearAccumFlag=false;
  //ComputeDepth = vtkMultiProcessController::GetGlobalController()->GetNumberOfProcesses() > 1;
  ComputeDepth = 0;

  this->EngineInited=false;
  this->NumberOfWorkers = 1;
  this->EnableShadows = -1;
  this->Samples = 1;
  this->MaxDepth = 5;
  this->EnableVolumeShading = 0;

  this->ImageX = -1;
  this->ImageY = -1;

  this->backgroundRGB[0] = 0.0;
  this->backgroundRGB[1] = 0.0;
  this->backgroundRGB[2] = 0.0;
  AccumCounter=0;
  MaxAccum=1024;
  this->SetAmbient( 0.1, 0.1, 0.1 );

  this->OSPRayManager = vtkOSPRayManager::Singleton();

  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;
  this->EnableAO=false;
  bool ao = EnableAO;
  EnableAO=-1;
  SetEnableAO(ao);
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
  ospSet3f(vRenderer, "bgColor", backgroundRGB[0], backgroundRGB[1], backgroundRGB[2]);
  OSPModel vModel = (OSPModel)this->OSPRayManager->OSPRayVolumeModel;
  SetEnableShadows(0);

  ospSetObject(vRenderer,"world",vModel);
  ospSetObject(vRenderer,"model",vModel);
  ospSetObject(vRenderer,"camera",oCamera);
  ospCommit(vRenderer);

  Assert(oRenderer != NULL && "could not create renderer");
  Assert(vRenderer != NULL && "could not create renderer");

  ospSetObject(oRenderer,"world",oModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);
  ospSet1i(oRenderer,"spp",Samples);
  ospSet3f(oRenderer,"bgColor",1,1,1);
  ospCommit(oModel);
  ospCommit(oCamera);
  ospCommit(oRenderer);

  this->ColorBuffer = NULL;
  this->DepthBuffer = NULL;
  this->osp_framebuffer = NULL;

  StatisticFramesPerOutput = 100;
}

//----------------------------------------------------------------------------
vtkOSPRayRenderer::~vtkOSPRayRenderer()
{
  if (this->osp_framebuffer)
  {
    ospFreeFrameBuffer(this->osp_framebuffer);
    this->osp_framebuffer = NULL;
  }

  if (this->ColorBuffer)
  {
    delete[] this->ColorBuffer;
    this->ColorBuffer = NULL;
  }

  if (this->DepthBuffer)
  {
    delete[] this->DepthBuffer;
    this->DepthBuffer = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InitEngine()
{
  this->EngineInited = true;
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetBackground(double r, double g, double b)
{
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
  ospSet3f(oRenderer,"bgColor",r,g,b);
  ospSet3f(vRenderer,"bgColor",r,g,b);

  backgroundRGB[0] = r;
  backgroundRGB[1] = g;
  backgroundRGB[2] = b;
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::ClearLights(void)
{
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::Clear()
{
}
//----------------------------------------------------------------------------
void vtkOSPRayRenderer::ClearAccumulation()
{
  if (osp_framebuffer)
    ospFrameBufferClear(osp_framebuffer, OSP_FB_ACCUM);
  AccumCounter=0;
}


//----------------------------------------------------------------------------
// Ask lights to load themselves into graphics pipeline.
int vtkOSPRayRenderer::UpdateLights()
{
  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  OSPRenderer vRenderer = ((OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer);
  std::vector<OSPLight> lights;


  // convert VTK lights into OSPRay lights
  vtkCollectionSimpleIterator sit;
  this->Lights->InitTraversal( sit );

  vtkLight *vLight = NULL;
  bool noneOn = true;
  for ( this->Lights->InitTraversal( sit );
       ( vLight = this->Lights->GetNextLight( sit ) ) ; )
  {
    if ( vLight->GetSwitch() )
    {
      noneOn = false;
    }
    vtkLight* light = vLight;

    double *color, *position, *focal, direction[3];

    // OSPRay Lights only have one "color"
    color    = light->GetDiffuseColor();
    position = light->GetTransformedPosition();
    focal    = light->GetTransformedFocalPoint();

    if (light->GetPositional())
    {
      OSPLight ospLight = ospNewLight(renderer, "OBJPointLight");
      ospSetString(ospLight, "name", "point" );
      ospSet3f(ospLight, "color", color[0],color[1],color[2]);
      ospSet3f(ospLight, "position", position[0],position[1],position[2]);
      ospCommit(ospLight);
      lights.push_back(ospLight);
    }
    else
    {
      direction[0] = position[0] - focal[0];
      direction[1] = position[1] - focal[1];
      direction[2] = position[2] - focal[2];
      OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
      ospSetString(ospLight, "name", "directional" );
      float scale = 0.8;
      ospSet3f(ospLight, "color", color[0]*scale,color[1]*scale,color[2]*scale);
      osp::vec3f dir(-direction[0],-direction[1],-direction[2]);
      dir = normalize(dir);
      ospSet3f(ospLight, "direction", dir.x,dir.y,dir.z);
      ospCommit(ospLight);
      lights.push_back(ospLight);
    }

    if (noneOn)
    {
      {
        cerr
        << "No light defined, creating a headlight at camera position" << endl;
      }
    }
    else
    {
    }
  }
  {

  }
  {
  }

  OSPData lightsArray = ospNewData(lights.size(), OSP_OBJECT, &lights[0], 0);
  ospSetData(renderer, "lights",lightsArray);
  ospSetData(vRenderer, "lights",lightsArray);
  ospCommit(renderer);

  return 0;
}

//----------------------------------------------------------------------------
vtkCamera* vtkOSPRayRenderer::MakeCamera()
{
  return vtkOSPRayCamera::New();
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::UpdateSize()
{
}

void vtkOSPRayRenderer::PreRender()
{
  if ((!prog_flag) || ClearAccumFlag)
  {
    if (osp_framebuffer)
      ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR | (ComputeDepth ? OSP_FB_DEPTH : 0) | OSP_FB_ACCUM);
    AccumCounter=0;
    ClearAccumFlag=false;
  }
  else
    prog_flag = false;

  if (this->GetLayer() != 0 && this->GetActors()->GetNumberOfItems() == 0)
  {
    return;
  }

  vtkTimerLog::MarkStartEvent("OSPRay Dev Render");

  if (!this->EngineInited )
  {
    this->InitEngine();
  }

  vtkTimerLog::MarkStartEvent("Geometry");

  this->Clear();

  this->UpdateSize();


  HasVolume = false;
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  this->OSPRayManager->OSPRayModel = ospNewModel();
  this->OSPRayManager->OSPRayVolumeModel = this->OSPRayManager->OSPRayModel;  //TODO: the volume and geometry are now managed in the same model object, can remove volumemodel entirely
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;
  ospSetObject(oRenderer,"world",oModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);


  ospCommit(this->OSPRayManager->OSPRayModel);
  ospCommit(this->OSPRayManager->OSPRayRenderer);

  this->UpdateCamera();


  this->UpdateLightGeometry();
  this->UpdateLights();
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::DeviceRender()
{
    debug5 << "ALOK: beginning of vtkOSPRayRenderer::DeviceRender()" << endl;
  static vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
  PreRender();

  this->UpdateGeometry();

  vtkTimerLog::MarkEndEvent("Geometry");

  vtkTimerLog::MarkStartEvent("Total LayerRender");
  this->LayerRender();

  vtkTimerLog::MarkEndEvent("Total LayerRender");

  vtkTimerLog::MarkEndEvent("OSPRay Dev Render");
  Frame++;
  timer->StopTimer();
  #if GATHER_STATS
  Statistics.push(std::make_pair("DeviceRender", timer->GetElapsedTime()));
  if ( (StatisticFramesPerOutput > 0) && ( (Frame % StatisticFramesPerOutput) == 0) )
  {
    while (!Statistics.empty())
    {
      std::cout << Statistics.front().first << " " << Statistics.front().second;
      Statistics.pop();
    }
  }
  #endif
    debug5 << "ALOK: end of vtkOSPRayRenderer::DeviceRender()" << endl;
}

//----------------------------------------------------------------------------
// let the renderer display itself appropriately based on its layer index
void vtkOSPRayRenderer::LayerRender()
{
    debug5 << "ALOK: beginning of vtkOSPRayRenderer::Layereender()" << endl;
  int     i, j;
  int     rowLength,  OSPRaySize[2];
  int     minWidth,   minHeight;
  int     hOSPRayDiff, hRenderDiff;
  int     renderPos[2];
  int*    renderSize  = NULL;
  int*    renWinSize  = NULL;
  bool    stereoDumy;
  float*  OSPRayBuffer = NULL;
  double* renViewport = NULL;

  // collect some useful info
  renderSize = this->GetSize();
  renWinSize = this->GetRenderWindow()->GetActualSize();
  renViewport= this->GetViewport();
  renderPos[0] = int( renViewport[0] * renWinSize[0] + 0.5f );
  renderPos[1] = int( renViewport[1] * renWinSize[1] + 0.5f );
  minWidth = renderSize[0];
  minHeight =renderSize[1];
  hOSPRayDiff = 0;
  hRenderDiff = 0;
  // memory allocation and acess to the OSPRay image
  int size = renderSize[0]*renderSize[1];
  if (this->ImageX != renderSize[0] || this->ImageY != renderSize[1])
  {
    this->ImageX = renderSize[0];
    this->ImageY = renderSize[1];

    if (this->ColorBuffer) delete[] this->ColorBuffer;
    this->ColorBuffer = new float[ size ];

    if (this->DepthBuffer) delete[] this->DepthBuffer;
    this->DepthBuffer = new float[ size ];

    if (this->osp_framebuffer) ospFreeFrameBuffer(this->osp_framebuffer);
    this->osp_framebuffer = ospNewFrameBuffer(osp::vec2i(renderSize[0], renderSize[1]), OSP_RGBA_I8, OSP_FB_COLOR | (ComputeDepth ? OSP_FB_DEPTH : 0) | OSP_FB_ACCUM);
    ospFrameBufferClear(osp_framebuffer, OSP_FB_ACCUM);
    AccumCounter=0;
  }
  if (HasVolume && !EnableAO)
  {
    OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
    OSPModel vModel = (OSPModel)this->OSPRayManager->OSPRayVolumeModel;
    OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

    ospSetObject(vRenderer,"world",vModel);
    ospSetObject(vRenderer,"model",vModel);
    ospSetObject(vRenderer,"camera",oCamera);

    ospCommit(vModel);
    ospCommit(vRenderer);


    ospRenderFrame(this->osp_framebuffer,vRenderer,OSP_FB_COLOR|OSP_FB_ACCUM);
    AccumCounter++;
  }
  else
  {
    OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
    OSPModel ospModel = ((OSPModel)this->OSPRayManager->OSPRayModel);

    ospCommit(renderer);
    ospCommit(ospModel);

    ospRenderFrame(this->osp_framebuffer,renderer,OSP_FB_COLOR|OSP_FB_ACCUM);
    AccumCounter++;
  }

  //
  // Copy Depth Buffer
  //
  if (ComputeDepth)
  {
    double *clipValues = this->GetActiveCamera()->GetClippingRange();
    double viewAngle = this->GetActiveCamera()->GetViewAngle();

    // Closest point is center of near clipping plane - farthest is
    // corner of far clipping plane
    double clipMin = clipValues[0];
    double clipMax = clipValues[1] / pow(cos(viewAngle / 2.0), 2.0);
    double clipDiv = 1.0 / (clipMax - clipMin);

    const void *b = ospMapFrameBuffer(this->osp_framebuffer, OSP_FB_DEPTH);

    float *s = (float *)b;
    float *d = this->DepthBuffer;
    for (int i = 0; i < size; i++, s++, d++)
      *d = std::isinf(*s) ? 1.0 : (*s - clipMin) * clipDiv;
    ospUnmapFrameBuffer(b, this->osp_framebuffer);

    this->GetRenderWindow()->MakeCurrent();
    glDepthFunc(GL_ALWAYS);

    this->GetRenderWindow()->SetZbufferData(renderPos[0], renderPos[1],
                                            renderPos[0] + renderSize[0] - 1, renderPos[1] + renderSize[1] - 1, this->DepthBuffer);
  }
  //
  // Copy RGBA Buffer
  //

  const void* rgba = ospMapFrameBuffer(this->osp_framebuffer);
  // memcpy((void *)this->ColorBuffer, rgba, size*sizeof(float));
  glDrawPixels(renderSize[0],renderSize[1],GL_RGBA,GL_UNSIGNED_BYTE,rgba);
  ospUnmapFrameBuffer(rgba, this->osp_framebuffer);
    debug5 << "ALOK: end of vtkOSPRayRenderer::LayerRender()" << endl;
  return;
  //ALOK: Nothing below this runs!

  vtkTimerLog::MarkStartEvent("Image Conversion");

  // let layer #0 initialize GL depth buffer
  if ( this->GetLayer() == 0 )
  {
    this->GetRenderWindow()->
    SetRGBACharPixelData( renderPos[0],  renderPos[1],
                         renderPos[0] + renderSize[0] - 1,
                         renderPos[1] + renderSize[1] - 1,
                         (unsigned char*)this->ColorBuffer, 0, 0 );
    glFinish();
  }
  else
  {
    //layers on top add the colors of their non background pixels
    unsigned char*  GLbakBuffer = NULL;
    GLbakBuffer = this->GetRenderWindow()->
    GetRGBACharPixelData( renderPos[0],  renderPos[1],
                         renderPos[0] + renderSize[0] - 1,
                         renderPos[1] + renderSize[1] - 1, 0 );

    bool anyhit = false;
    unsigned char *optr = GLbakBuffer;
    unsigned char *iptr = (unsigned char*)this->ColorBuffer;
    float *zptr = this->DepthBuffer;
    for ( j = 0; j < renderSize[1]; j++)
    {
      for ( i = 0; i < renderSize[0]; i++)
      {
        const float z = *zptr;
        if (z > 0 && z < 1.0)
        {
          anyhit = true;
          *(optr+0) = *(iptr+0);
          *(optr+1) = *(iptr+1);
          *(optr+2) = *(iptr+2);
          *(optr+3) = *(iptr+3);
        }
        optr+=4;
        iptr+=4;
        zptr++;
      }
    }

    if (anyhit)
    {
      // submit the modified RGB colors to GL BACK buffer
      this->GetRenderWindow()->
      SetRGBACharPixelData( renderPos[0],  renderPos[1],
                           renderPos[0] + renderSize[0] - 1,
                           renderPos[1] + renderSize[1] - 1,
                           GLbakBuffer, 0, 0 );
    }

    delete [] GLbakBuffer;
  }

  vtkTimerLog::MarkEndEvent("Image Conversion");
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetNumberOfWorkers( int newval )
{
  if (this->NumberOfWorkers == newval)
  {
    return;
  }
}

void vtkOSPRayRenderer::AddOSPRayRenderable(vtkOSPRayRenderable* inst)
{
  ospAddGeometry((OSPModel)this->OSPRayManager->OSPRayModel,inst->instance);
  renderables.push_back(inst);
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableShadows( int newval )
{
  if (this->EnableShadows == newval)
  {
    return;
  }
  this->EnableShadows = newval;

  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  ospSet1i(renderer,"shadowsEnabled", this->EnableShadows);
  ospCommit(renderer);
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetSamples( int newval )
{
  if (this->Samples == newval || newval < 1)
  {
    return;
  }

  this->Samples = newval;


  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);

  Assert(renderer);

  ospSet1i(renderer,"spp",Samples);
  ospCommit(renderer);

  OSPRenderer vRenderer = ((OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer);

  Assert(vRenderer);

  ospSet1i(vRenderer,"spp",Samples);
  ospCommit(vRenderer);

}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableAO( int newval )
{
  if (this->EnableAO == newval)
  {
    return;
  }

  this->EnableAO = newval;

  UpdateOSPRayRenderer();

}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnablePathtracing( int newval )
{
  if (this->EnablePathtracing == newval)
  {
    return;
  }

  this->EnablePathtracing = newval;

  UpdateOSPRayRenderer();

}


void vtkOSPRayRenderer::SetEnableVolumeShading( int newval )
{
  EnableVolumeShading = newval;
}

void vtkOSPRayRenderer::UpdateOSPRayRenderer()
{
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

  if (EnableAO != 0)
  {
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  }
  else if (EnablePathtracing != 0)
  {
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("pathtracer");
  }
  else
  {
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("raycast_volume_renderer");
    // this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
  }
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;

  Assert(oRenderer != NULL && "could not create renderer");

  ospSetObject(oRenderer,"dynamic_model",ospNewModel());
  ospSetObject(oRenderer,"world",oModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);
  ospCommit(oRenderer);

  ospSet1i(oRenderer,"spp",Samples);
  ospSet1f(oRenderer,"epsilon", 10e-2);
  ospSet1i(oRenderer,"shadowsEnabled", this->EnableShadows);

  ospCommit(oRenderer);
  SetBackground(backgroundRGB[0],backgroundRGB[1],backgroundRGB[2]);

  vtkActorCollection *actorList = this->GetActors();
  actorList->InitTraversal();

  int numActors = actorList->GetNumberOfItems();
  for(int i=0; i<numActors; i++) {
    vtkActor *a = actorList->GetNextActor();
    a->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetMaxDepth( int newval )
{
  if (this->MaxDepth == newval)
  {
    return;
  }

  this->MaxDepth = newval;
}
