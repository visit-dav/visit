/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkMantaRenderer.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
Module:    $RCSfile: vtkMantaRenderer.cxx,v $

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
#include "vtkMantaActor.h"
#include "vtkMantaCamera.h"
#include "vtkMantaManager.h"
#include "vtkMantaRenderer.h"

#include "vtkActor.h"
#include "vtkCuller.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkTimerLog.h"

#include <Core/Color/Color.h>
#include <Core/Color/ColorDB.h>
#include <Core/Color/RGBColor.h>
#include <Engine/Control/RTRT.h>
#include <Engine/Display/NullDisplay.h>
#include <Engine/Display/SyncDisplay.h>
#include <Engine/Factory/Create.h>
#include <Engine/Factory/Factory.h>
#include <Image/SimpleImage.h>
#include <Interface/Context.h>
#include <Interface/Light.h>
#include <Interface/LightSet.h>
#include <Interface/Scene.h>
#include <Interface/Object.h>
#include <Interface/AccelerationStructure.h>
#include <Model/AmbientLights/ConstantAmbient.h>
#include <Model/Backgrounds/ConstantBackground.h>
#include <Model/Groups/Group.h>
#include <Model/Groups/Mesh.h>
#include <Model/Lights/HeadLight.h>
#include <Model/Groups/DynBVH.h>
#include <Model/Primitives/KenslerShirleyTriangle.h>
#include <Model/Materials/Lambertian.h>


#include <vtkstd/string>


#include <sys/time.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <string>
#include <sstream>
//#include <mpi.h>


//#include <IceT.h>
//#include <IceTGL.h>
//#include <IceTDevGLImage.h>

vtkCxxRevisionMacro(vtkMantaRenderer, "$Revision: 1.18 $");
vtkStandardNewMacro(vtkMantaRenderer);

typedef float BUFFERT;

std::string GetVarString4(std::string var)
{
  std::string v = "";
  char* str = getenv(var.c_str());
  //if (!str)
  //  cerr << "WARNING: \"" << var << "\" ENV not found\n";
  if (str)
    v = std::string(str);
  return v;
}

  template<typename T>
bool GetVar4(std::string var, T& val)
{
  T v;
  std::stringstream str(GetVarString4(var));
  if (str.str() == "")
  {
    cout << "unable to find " << var << endl;
    return false;
  }
  str >> v;
  val = v;
  return true;
}

//----------------------------------------------------------------------------
vtkMantaRenderer::vtkMantaRenderer() :
  EngineInited( false ), EngineStarted( false ),
  IsStereo( false ), MantaScene( 0 ), MantaWorldGroup( 0 ),
  MantaLightSet( 0 ), MantaCamera( 0 ), SyncDisplay( 0 )
{
  //glClearColor(1,0,0,1);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  //cerr << "MR(" << this << ") CREATE" << endl;

  // Default options
  this->NumberOfWorkers = 8;
  this->EnableShadows = 0;
  this->Samples = 1;
  this->MaxDepth = 5;
  int newsamples = 1;
  GetVar4<int>("VISIT_MANTA_SPP", this->Samples);
  GetVar4<int>("VISIT_MANTA_SHADOWS", this->EnableShadows);

  // the default global ambient light created by vtkRenderer is too bright.
  this->SetAmbient( 0.0, 0.0, 0.0 );

  this->MantaManager = vtkMantaManager::New();
  //this->MantaManager = vtkMantaManager::GetSingleton();

  this->MantaEngine = this->MantaManager->GetMantaEngine();
  this->MantaEngine->changeNumWorkers( this->NumberOfWorkers );

  this->MantaFactory = this->MantaManager->GetMantaFactory();

  this->ColorBuffer = NULL;
  this->DepthBuffer = NULL;
  this->ImageSize = -1;

  //  this->MantaFactory->selectImageType( "rgb8" );
  this->MantaFactory->selectImageType("rgba8zfloat" );

  this->MantaFactory->selectImageTraverser( "tiled(-square)" );
  //this->MantaFactory->selectImageTraverser( "deadline()" );

  this->MantaFactory->selectLoadBalancer( "workqueue" );

  if (this->EnableShadows)
  {
    this->MantaFactory->selectShadowAlgorithm( "hard(-attenuateShadows)" );
  }
  else
  {
    this->MantaFactory->selectShadowAlgorithm( "noshadows" );
  }

  if (this->Samples <= 1)
  {
    this->MantaFactory->selectPixelSampler( "singlesample" );
  }
  else
  {
    char buff[80];
    sprintf(buff, "regularsample(-numberOfSamples %d)", this->Samples);
    this->MantaFactory->selectPixelSampler(buff);
    //this->MantaFactory->selectPixelSampler(
    //"jittersample(-numberOfSamples 16)");
  }

  this->MantaFactory->selectRenderer( "raytracer" );

  this->DefaultLight = NULL;
}

//----------------------------------------------------------------------------
vtkMantaRenderer::~vtkMantaRenderer()
{
  //cerr << "MR(" << this << ") DESTROY " << this->MantaManager << " "
  //     << this->MantaManager->GetReferenceCount() << endl;

  if (this->DefaultLight && this->MantaLightSet)
  {
    Manta::Callback::create( this->MantaLightSet, &Manta::LightSet::remove,
        this->DefaultLight );
    this->DefaultLight = NULL;
  }

  this->MantaManager->Delete();

  if (this->ColorBuffer)
  {
    delete[] this->ColorBuffer;
  }
  if (this->DepthBuffer)
  {
    delete[] this->DepthBuffer;
  }
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InitEngine()
{
  //cerr << "MR(" << this << ")#" << this->GetLayer() << " INIT "  << this->MantaManager << endl;
  this->Ambient[0] = this->Ambient[1] = this->Ambient[2] = 0; //CARSON_HACK:  ambient light is really highg for some reason
  this->MantaManager->StartEngine(this->MaxDepth,
      this->GetBackground(),
      this->Ambient,
      this->IsStereo,
      this->GetSize());

  this->MantaScene = this->MantaManager->GetMantaScene();
  this->MantaWorldGroup = this->MantaManager->GetMantaWorldGroup();
  this->MantaLightSet = this->MantaManager->GetMantaLightSet();
  this->MantaCamera = this->MantaManager->GetMantaCamera();
  this->SyncDisplay = this->MantaManager->GetSyncDisplay();
  this->ChannelId = this->MantaManager->GetChannelId();

  this->EngineInited = true;
  //cerr << "MR(" << this << ")#" << this->GetLayer() << " INIT DONE "  << this->MantaManager << endl;
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::SetBackground(double r, double g, double b)
{
  if ((this->Background[0] != r)||
      (this->Background[1] != g)||
      (this->Background[2] != b))
  {
    this->Superclass::SetBackground(r,g,b);
    this->MantaEngine->addTransaction
      ( "set background",
        Manta::Callback::create(this, &vtkMantaRenderer::InternalSetBackground));
  };
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InternalSetBackground()
{
  if (this->MantaManager->customBackground)
    return;
  double *color = this->GetBackground();
  Manta::ConstantBackground * background = new Manta::ConstantBackground(
      Manta::Color(  Manta::RGBColor( color[0], color[1], color[2] )  )  );

  delete this->MantaScene->getBackground();
  this->MantaScene->setBackground( background );
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::ClearLights(void)
{
  this->MantaEngine->addTransaction
    ( "clear lights",
      Manta::Callback::create( this, &vtkMantaRenderer::InternalClearLights));
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InternalClearLights(void)
{
  if (this->MantaLightSet)
  {
    delete this->MantaLightSet->getAmbientLight();
    for ( unsigned int i = 0; i < this->MantaLightSet->numLights(); i ++ )
    {
      Manta::Light *light = this->MantaLightSet->getLight( i );
      this->MantaLightSet->remove( light );
      delete light;
    }
  }
}

//----------------------------------------------------------------------------
// Ask lights to load themselves into graphics pipeline.
int vtkMantaRenderer::UpdateLights()
{
  // convert VTK lights into Manta lights
  vtkCollectionSimpleIterator sit;
  this->Lights->InitTraversal( sit );

  // TODO: schedule ClearLight here?
  vtkLight *vLight = NULL;
  bool noneOn = true;
  for ( this->Lights->InitTraversal( sit );
      ( vLight = this->Lights->GetNextLight( sit ) ) ; )
  {
    if ( vLight->GetSwitch() )
    {
      noneOn = false;
    }
    //manta lights set intensity to 0.0 if switched off, so render regardless
    vLight->Render( this, 0 /* not used */ );
  }

  if (noneOn)
  {
    if (this->MantaLightSet->numLights()==0 )
    {
      // there is no VTK light nor MantaLight defined, create a Manta headlight
      cerr << "No light defined, creating a headlight at camera position" << endl;
      this->DefaultLight =
        new Manta::HeadLight( 0, Manta::Color( Manta::RGBColor( 1, 1, 1 ) ) );
      /*this->MantaEngine->addTransaction
        ("add headlight",
        Manta::Callback::create( this->MantaLightSet, &Manta::LightSet::add,
        this->DefaultLight ) );*/
    }
  }
  else
  {
    if (this->DefaultLight)
    {
      Manta::Callback::create( this->MantaLightSet, &Manta::LightSet::remove,
          this->DefaultLight );
      this->DefaultLight = NULL;
    }
  }

  return 0;
}

//----------------------------------------------------------------------------
vtkCamera* vtkMantaRenderer::MakeCamera()
{
  return vtkMantaCamera::New();
}


//----------------------------------------------------------------------------
void vtkMantaRenderer::DeviceRender()
{
  this->RenderWindow->MakeCurrent();

  printf("vtkMantaRenderer::DeviceRender (%d) , layer %d\n", this, this->GetLayer());
  // In ParaView, we are wasting time in rendering the "sync layer" with
  // empty background image just to be dropped in LayerRender(). We just
  // don't start the engine with sync layer.
  // TODO: this may not be the right way to check if it is a sync layer
  vtkMantaManager* mms = vtkMantaManager::GetSingleton();
  //printf("MR(%p)::DeviceRender MM(%p) MMS(%p): layer: %d numitems: %d numPolys: %d\n", this, this->MantaManager, mms, this->GetLayer(), this->GetActors()->GetNumberOfItems(), this->MantaManager->numPolys);
  //vtkActor* actor = this->GetActors()->GetNextActor();
  //while(actor)
  //{
  //  printf("found actor: %d\n", actor);
  //  actor = this->GetActors()->GetNextActor();
  //}

  /*if (this->GetLayer() != 1 && this->GetActors()->GetNumberOfItems() == 0)
    {
    return; //CARSON DEBUG : visit launches multiple renderers some render nothing
    }*/

  if (!this->EngineInited )
  {
    this->InitEngine();
  }

  vtkTimerLog::MarkStartEvent("Geometry");

  this->UpdateCamera();

  // TODO: call ClearLights here?

  // call Light::Render()
  this->UpdateLightGeometry();
  this->UpdateLights();

  if (!this->EngineStarted)
  {
    this->MantaEngine->beginRendering( false );
    this->EngineStarted = true;
    this->GetSyncDisplay()->waitOnFrameReady();
  }

  this->UpdateGeometry();

  vtkTimerLog::MarkEndEvent("Geometry");

  //CARSON DEBUG

  vtkTimerLog::MarkStartEvent("Total LayerRender");
  if ( this->EngineStarted )
  {
    this->LayerRender();
  }
  vtkTimerLog::MarkEndEvent("Total LayerRender");
  //fflush(stdout);
}




//----------------------------------------------------------------------------
// let the renderer display itself appropriately based on its layer index
void vtkMantaRenderer::LayerRender()
{
  struct timeval ts;
  gettimeofday(&ts, 0);
  double st = (double)ts.tv_sec + (double)ts.tv_usec/1e6;
  int     i, j;
  int     rowLength,  mantaSize[2];
  int     minWidth,   minHeight;
  int     hMantaDiff, hRenderDiff;
  int     renderPos0[] = {0,0};
  int*    renderSize  = NULL;
  int*    renWinSize  = NULL;
  bool    stereoDumy;
  BUFFERT*  mantaBuffer = NULL;
  double* renViewport = NULL;
  const   Manta::SimpleImageBase* mantaBase = NULL;

  vtkTimerLog::MarkStartEvent("ThreadSync");
  // let the render threads draw what we've asked them to
  this->GetSyncDisplay()->doneRendering();
  this->GetSyncDisplay()->waitOnFrameReady();

  vtkTimerLog::MarkEndEvent("ThreadSync");
  // collect some useful info
  renderSize = this->GetSize();
  renWinSize = this->GetRenderWindow()->GetScreenSize();
  if (!this->GetRenderWindow())
  {
    //printf("error: no renderwindow\n");
    return;
  }
  if (!renWinSize)
  {
    //printf("error: no renwinsize\n");
    renWinSize = new int[2];
    renWinSize[0] = 512;
    renWinSize[1] = 512;
  }
  renViewport= this->GetViewport();
  if (!renViewport)
  {
    //printf("error: no renviewport\n");
    return;
  }
  renderPos0[0] = int( renViewport[0] * renWinSize[0] + 0.5f );
  renderPos0[1] = int( renViewport[1] * renWinSize[1] + 0.5f );
  this->GetSyncDisplay()->getCurrentImage()->
    getResolution( stereoDumy, mantaSize[0], mantaSize[1] );
  mantaBase = dynamic_cast< const Manta::SimpleImageBase * >
    ( this->GetSyncDisplay()->getCurrentImage() );
  rowLength = mantaBase->getRowLength();
  // for window re-sizing
  minWidth    = ( mantaSize[0] < renderSize[0] )
    ? mantaSize[0] : renderSize[0];
  minHeight   = ( mantaSize[1] < renderSize[1] )
    ? mantaSize[1] : renderSize[1];
  hMantaDiff  = mantaSize[1] - minHeight;
  hRenderDiff = renderSize[1] - minHeight;

  // memory allocation and acess to the Manta image
  int size = renderSize[0]*renderSize[1];
  if (this->ImageSize != size || hMantaDiff != 0 || hRenderDiff != 0)
  {
    delete[] this->ColorBuffer;
    delete[] this->DepthBuffer;
    this->ImageSize = size;
    this->DepthBuffer = new float[ size ];
    this->ColorBuffer = new float[ size ];
  }
  if (hMantaDiff != 0 || hRenderDiff != 0)
  {
    // cerr << "MR(" << this << ") "
    //      << "Layer: " << this->GetLayer() << ", "
    //      << "Props: " << this->NumberOfPropsRendered << endl
    //      << "  MantaSize: " << mantaSize[0] << ", " << mantaSize[1] << ", "
    //      << "  renWinSize: " << renWinSize[0] << ", " << renWinSize[1] << ", "
    //      << "  renderSize: " << renderSize[0] << ", " << renderSize[1] << endl;

    this->GetMantaEngine()->addTransaction
      ("resize",
       Manta::Callback::create
       (this->GetMantaEngine(),
        &Manta::MantaInterface::changeResolution,
        0 /*mantaRenderer->GetChannelId()*/, renderSize[0], renderSize[1],
        true));
    this->GetSyncDisplay()->doneRendering();
    this->GetSyncDisplay()->waitOnFrameReady();
    this->GetSyncDisplay()->doneRendering();
    this->GetSyncDisplay()->waitOnFrameReady();
    this->GetSyncDisplay()->getCurrentImage()->
      getResolution( stereoDumy, mantaSize[0], mantaSize[1] );
    mantaBase = dynamic_cast< const Manta::SimpleImageBase * >
      ( this->GetSyncDisplay()->getCurrentImage() );
    rowLength = mantaBase->getRowLength();
    // for window re-sizing
    minWidth    = ( mantaSize[0] < renderSize[0] )
      ? mantaSize[0] : renderSize[0];
    minHeight   = ( mantaSize[1] < renderSize[1] )
      ? mantaSize[1] : renderSize[1];
    hMantaDiff  = mantaSize[1] - minHeight;
    hRenderDiff = renderSize[1] - minHeight;
  }

  mantaBuffer = static_cast< BUFFERT * >( mantaBase->getRawData(0) );

  int xres,yres;
  bool stereo;  static bool big_endian = false;
  static bool once = false;
  if (!once){

    int x = 0x12345678;
    char* p = (char*)&x;
    if(p[0] == 0x12)
      big_endian = true;
    else
      big_endian = false;
    once = true;
  }

  this->GetSyncDisplay()->getCurrentImage()->getResolution(stereo,xres,yres);

  const Manta::Image* image = this->GetSyncDisplay()->getCurrentImage();
  char* imagedata = static_cast<char*>( mantaBase->getRawData(0) );



  // update this->ColorBuffer and this->DepthBuffer from the Manta
  // RGBA8ZfloatPixel array
  double *clipValues = this->GetActiveCamera()->GetClippingRange();
  double depthScale  = 1.0f / ( clipValues[1] - clipValues[0] );

  vtkTimerLog::MarkStartEvent("Image Conversion");
  // This double for loop costs about 0.01 seconds per frames on the
  // 8 cores machine. This can be fixed with RGBA8ZFloatP
  for ( j = 0; j < minHeight; j ++ )
  {
    // there are two floats in each pixel in Manta buffer
    int mantaIndex = ( ( j + hMantaDiff  ) * rowLength     ) * 2;
    // there is only one float in each pixel in the GL RGBA or Z buffer
    int tupleIndex = ( ( j + hRenderDiff ) * renderSize[0] ) * 1;

    for ( i = 0; i < minWidth; i ++ )
    {
      this->ColorBuffer[ tupleIndex + i ]
        = mantaBuffer[ mantaIndex + i*2     ];
      float depthValue   = mantaBuffer[ mantaIndex + i*2 + 1 ];
      // normalize the depth values to [ 0.0f, 1.0f ], since we are using a
      // software buffer for Z values and never write them to OpenGL buffers,
      // we don't have to clamp them any more
      // TODO: On a second thought, we probably don't even have to normalize Z
      // values at all
      this->DepthBuffer[ tupleIndex + i ]
        = ( depthValue - clipValues[0] ) * depthScale;
      //this->DepthBuffer[tupleIndex+i] = 0.1;  // CD DEBUG
    }
  }

  //This is a work around to render into an opengl context without a custom window interface for vtk.  This is where the code diverges from paraview a bit.

  int xmin = std::min(xres, renderSize[0]), ymin = std::min(yres, renderSize[1]);


  // let layer #0 initialize GL depth buffer
  if ( this->GetLayer() == 1 )
  {
    //memcpy(this->ColorBuffer, mms->ColorBufferStatic, sizeof(float)*xmin*ymin);
    this->GetRenderWindow()->
      SetZbufferData( renderPos0[0],  renderPos0[1],
          renderPos0[0] + renderSize[0] - 1,
          renderPos0[1] + renderSize[1] - 1,
          this->DepthBuffer );

    //printf("renderPos: %d %d renderSize: %d %d colorbuffersize: %d\n", renderPos0[0], renderPos0[1],
    //    renderSize[0], renderSize[1], int(size));
    this->GetRenderWindow()->
      SetRGBACharPixelData( renderPos0[0],  renderPos0[1],
          renderPos0[0] + renderSize[0] - 1,
          renderPos0[1] + renderSize[1] - 1,
          (unsigned char*)this->ColorBuffer, 0, 0 );

    //glFinish();
  }
  else
  {
    //layers on top add the colors of their non background pixels
    unsigned char*  GLbakBuffer = NULL;
    GLbakBuffer = this->GetRenderWindow()->
      GetRGBACharPixelData( renderPos0[0],  renderPos0[1],
          renderPos0[0] + renderSize[0] - 1,
          renderPos0[1] + renderSize[1] - 1, 0 );

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
        SetRGBACharPixelData( renderPos0[0],  renderPos0[1],
            renderPos0[0] + renderSize[0] - 1,
            renderPos0[1] + renderSize[1] - 1,
            GLbakBuffer, 0, 0 );
    }

    delete [] GLbakBuffer;
  }



  /*IceTImage result = icetGetImage();
    IceTEnum color_format =icetImageGetColorFormat(result);
    IceTEnum depth_format = icetImageGetDepthFormat(result);
    icetUseGLRead(0);
    IceTUInt* colorBuffer = icetImageGetColrui(result);
    memcpy((void*)colorBuffer, (void*)this->ColorBuffer, sizeof(IceTUInt)*osize[0]*osize[1]);
    IceTFloat* depthBuffer = icetImageGetDepthf(result);
    memcpy((void*)depthBuffer, (void*)this->DepthBuffer, sizeof(IceTFloat)*osize[0]*osize[1]);
    */


  /*glPixelStorei(GL_UNPACK_ROW_LENGTH, xmin);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);


  //glDisable(GL_DEPTH_TEST);
  static float* testbuffer = new float[1024*1024];
  for (int i = 0; i < 1024*1024*4; i++)
  {
  ((unsigned char*)testbuffer)[i] = 128;
  }
  if (this->MantaManager->numPolys < 10 )
  {
  } else
  {
  //glClearColor(0,1,0,1);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  //glClearColor(0,0,1,1);
  //glDrawPixels(xmin, ymin, GL_RGBA, GL_UNSIGNED_BYTE, this->ColorBuffer);
  //glDrawPixels(xmin, ymin, GL_DEPTH_COMPONENT, GL_FLOAT, this->DepthBuffer);
  //glFinish();

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  }*/
  //glFinish();

  /*if (this->MantaManager->numPolys > 1000 )
    {

    std::string filename = "";
    char cfilename[256];
    static int frame_number = 0;
    sprintf(cfilename, "render_%04d_%dx%d.rgb", frame_number++, xmin, ymin);
    filename = std::string(cfilename);
  //unsigned char* test = new unsigned char[xres*yres*3];
  //glReadPixels(0,0,xres,yres,GL_RGB, GL_UNSIGNED_BYTE, test);
  FILE* pFile = fopen(filename.c_str(), "w");
  //assert(pFile);
  fwrite(this->ColorBuffer, 1, xmin*ymin*4, pFile);
  fclose(pFile);
  std::stringstream s("");
  //TODO: this fudge factor on teh sizes makes no sense... I'm assuming it's because they have row padding in the data but it doesn't show up in drawpixels... perplexing.  It can also crash just a hack for now
  s  << "convert -flip -size " << xmin << "x" << ymin << " -depth 8 rgba:" << filename << " " << filename << ".png";
  system(s.str().c_str());

  memcpy(mms->ColorBufferStatic, this->ColorBuffer, sizeof(float)*xmin*ymin);
  }
  */
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::SetNumberOfWorkers( int newval )
{
  if (this->NumberOfWorkers == newval)
  {
    return;
  }
  this->NumberOfWorkers = newval;
  this->MantaEngine->addTransaction
    ( "set max depth",
      Manta::Callback::create(this, &vtkMantaRenderer::InternalSetNumberOfWorkers));
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InternalSetNumberOfWorkers()
{
  this->MantaEngine->changeNumWorkers( this->NumberOfWorkers );
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::SetEnableShadows( int newval )
{
  if (this->EnableShadows == newval)
  {
    return;
  }

  this->EnableShadows = newval;
  this->MantaEngine->addTransaction
    ( "set shadows",
      Manta::Callback::create(this, &vtkMantaRenderer::InternalSetShadows));
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InternalSetShadows()
{
  if (this->EnableShadows)
  {
    this->MantaFactory->selectShadowAlgorithm( "hard(-attenuateShadows)" );
  }
  else
  {
    this->MantaFactory->selectShadowAlgorithm( "noshadows" );
  }
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::SetSamples( int newval )
{
  if (this->Samples == newval)
  {
    return;
  }

  this->Samples = newval;
  this->MantaEngine->addTransaction
    ( "set samples",
      Manta::Callback::create(this, &vtkMantaRenderer::InternalSetSamples));
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InternalSetSamples()
{
  if (this->Samples <= 1)
  {
    this->MantaFactory->selectPixelSampler( "singlesample" );
  }
  else
  {
    char buff[80];
    sprintf(buff, "regularsample(-numberOfSamples %d)", this->Samples);
    this->MantaFactory->selectPixelSampler(buff);
  }
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::SetMaxDepth( int newval )
{
  if (this->MaxDepth == newval)
  {
    return;
  }

  this->MaxDepth = newval;
  this->MantaEngine->addTransaction
    ( "set max depth",
      Manta::Callback::create(this, &vtkMantaRenderer::InternalSetMaxDepth));
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMantaRenderer::InternalSetMaxDepth()
{
  this->MantaScene->getRenderParameters().setMaxDepth( this->MaxDepth );
}

