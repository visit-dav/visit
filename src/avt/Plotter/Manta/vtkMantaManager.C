/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkMantaManager.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkMantaManager.h"
#include "vtkObjectFactory.h"
#include "vtkManta.h"

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
#include <Model/AmbientLights/ConstantAmbient.h>
#include <Model/AmbientLights/AmbientOcclusion.h>
#include <Model/AmbientLights/AmbientOcclusionBackground.h>
#include <Model/Backgrounds/ConstantBackground.h>
#include <Model/Primitives/Sphere.h>
#include <Model/Materials/Lambertian.h>
#include <Model/Groups/Group.h>
#include <Model/Lights/HeadLight.h>
#include <Model/Lights/PointLight.h>
#include <Model/Groups/DynBVH.h>
#include <Model/Backgrounds/EnvMapBackground.h>
#include <Model/Textures/ImageTexture.h>
#include <Model/Primitives/KenslerShirleyTriangle.h>
#include <Engine/Display/FileDisplay.h>
#include <sstream>
#include <string>

vtkCxxRevisionMacro(vtkMantaManager, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkMantaManager);


vtkMantaManager* vtkMantaManager::singleton = NULL;
//Manta::Group* vtkMantaManager::MantaWorldGroup = NULL;
Manta::DynBVH* vtkMantaManager::as = NULL;
Manta::Mesh* vtkMantaManager::m2 = NULL;
float* vtkMantaManager::ColorBufferStatic = NULL;


std::string GetVarString(std::string var)
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
bool GetVar(std::string var, T& val)
{
  T v;
  std::stringstream str(GetVarString(var));
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
vtkMantaManager::vtkMantaManager()
{
  //cerr << "MX(" << this << ") CREATE" << endl;
  this->MantaEngine = Manta::createManta();
  customBackground = false;
  materialType = "default";
  reflectance = 0.0;
  specularPower = 0;
  GetVar<std::string>("VISIT_MANTA_MATERIAL", materialType);
  GetVar<double>("VISIT_MANTA_REFLECTANCE", reflectance);
  GetVar<double>("VISIT_MANTA_SPECULAR_POWER", specularPower);
  this->MantaFactory = new Manta::Factory( this->MantaEngine );
  this->Started = false;

  this->MantaScene = NULL;
  this->MantaLightSet = NULL;
  this->MantaCamera = NULL;
  this->SyncDisplay = NULL;
  this->ChannelId = NULL;
  this->numPolys = 0;
  this->MantaWorldGroup = NULL;
  if (!ColorBufferStatic)
  {
    ColorBufferStatic = new float[1024*1024];
  }
}


vtkMantaManager* vtkMantaManager::GetSingleton()
{
  if (singleton == NULL)
    singleton = new vtkMantaManager();
  return singleton;
}

//----------------------------------------------------------------------------
vtkMantaManager::~vtkMantaManager()
{
  //cerr << "MX(" << this << ") DESTROY" << endl;
  int v =-1;
  //TODO: This is screwy but the only way I've found to get it to consistently
  //shutdown without hanging.
  //int i = 0;
  if (this->SyncDisplay)
  {
    this->SyncDisplay->doneRendering();
  }
  v = this->MantaEngine->numWorkers();
  this->MantaEngine->setDisplayBeforeRender(true);
  this->MantaEngine->changeNumWorkers(0);
  while (v != 0)
  {
    //cerr << "MX(" << this << ") SYNC " << i++ << " " << v << endl;
    if (this->SyncDisplay)
    {
      this->SyncDisplay->waitOnFrameReady();
      this->SyncDisplay->doneRendering();
    }
    v = this->MantaEngine->numWorkers();
  }
  //cerr << "MX(" << this << ") SYNC DONE " << i << " " << v << endl;
  //cerr << "MX(" << this << ") wait" << endl;
  this->MantaEngine->blockUntilFinished();

  if (this->MantaLightSet)
  {
    delete this->MantaLightSet->getAmbientLight();
    /*
    //let vtkMantaLight's delete themselves
    Manta::Light *light;
    for (unsigned int i = 0; i < this->MantaLightSet->numLights(); i++)
    {
    light = this->MantaLightSet->getLight(i);
    delete light;
    }
    */
  }
  delete this->MantaLightSet;

  delete this->MantaCamera;

  if (this->MantaScene)
  {
    delete this->MantaScene->getBackground();
  }
  delete this->MantaScene;

  // delete this->MantaWorldGroup;

  delete this->MantaFactory;

  //delete this->SyncDisplay; //engine does this

  delete this->MantaEngine;

  //cerr << "MX(" << this << ") good night Gracie" << endl;

}

//----------------------------------------------------------------------------
void vtkMantaManager::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
void vtkMantaManager::StartEngine(int maxDepth,
    double *bgColor,
    double *ambient,
    bool stereo,
    int *size
    )
{
  int intvar;
  std::string stringvar;

  //printf("MX(%p) START\n", this);
  if (this->Started)
  {
    cerr << "WARNING: Manta is already initted, ignoring reinitialize." << endl;
    return;
  }
  this->Started = true;

  // create an empty Manta scene with background
  this->MantaScene = new Manta::Scene();
  this->MantaScene->getRenderParameters().setMaxDepth( maxDepth );
  this->MantaEngine->setScene( this->MantaScene );

  Manta::Background * background = new Manta::ConstantBackground(
      Manta::Color(Manta::RGBColor( bgColor[0], bgColor[1], bgColor[2] )));

  if (GetVar<std::string>("VISIT_MANTA_BACKGROUND_IMAGE", stringvar))
  {
    Manta::EnvMapBackground::MappingType mapping_type = Manta::EnvMapBackground::DebevecSphere;
    //CylindricalEqualArea;

    Manta::ImageTexture<Manta::Color>* t = Manta::LoadColorImageTexture( stringvar, &std::cerr );
    t->setInterpolationMethod(Manta::ImageTexture<Manta::Color>::Bilinear);
    Manta::Vector up( 0.0f, 1.0f, 0.0f );
    Manta::Vector right( 1.0f, 0.0f, 0.0f );

    background =  new Manta::EnvMapBackground( t,
        mapping_type, right, up ) ;
    customBackground = true;
  }
  this->MantaScene->setBackground( background );
  float aoIntensity = 0.7;
  GetVar<float>("VISIT_MANTA_AO_INTENSITY", aoIntensity);

  // create empty world group
  if (!MantaWorldGroup)
    this->MantaWorldGroup = new Manta::Group();
  this->MantaScene->setObject( this->MantaWorldGroup );

  this->MantaWorldGroup->add(new Manta::Sphere(new Manta::Lambertian(Manta::Color(Manta::RGBColor(0,0,0))), Manta::Vector(0,0,0), 0.00001f));

  // create empty LightSet with ambient light
  this->MantaLightSet = new Manta::LightSet();
  this->MantaLightSet->setAmbientLight(
      new Manta::ConstantAmbient(
        Manta::Color(Manta::RGBColor( ambient[0], ambient[1], ambient[2] ))));
  if (GetVar<int>("VISIT_MANTA_AO_SAMPLES",intvar))
  {
    //if (customBackground)
    //  this->MantaLightSet->setAmbientLight(new Manta::AmbientOcclusionBackground(Manta::Color(Manta::RGBColor(aoIntensity,aoIntensity,aoIntensity)), 10.0f, intvar, true, background));
    //else
      this->MantaLightSet->setAmbientLight(new Manta::AmbientOcclusion(Manta::Color(Manta::RGBColor(aoIntensity,aoIntensity,aoIntensity)), 10.0f, intvar, false));
  }
  this->MantaLightSet->add(new Manta::PointLight(Manta::Vector(-2,4,-8), Manta::Color(Manta::RGB(0.0,0.0,0.0))));
  this->MantaScene->setLights( this->MantaLightSet );

  // create the mantaCamera singleton,
  // it is the only camera we create per renderer
  this->MantaCamera = this->MantaFactory->
    createCamera( "pinhole(-normalizeRays -createCornerRays)" );

  // Use SyncDisplay with Null Display to stop Manta engine at each frame,
  // the image is combined with OpenGL framebuffer by vtkXMantaRenderWindow
  vtkstd::vector<vtkstd::string> vs;
  this->SyncDisplay = new Manta::SyncDisplay( vs );
  this->SyncDisplay->setChild(  new Manta::NullDisplay( vs )  );

  //Set image size
  this->ChannelId = this->MantaEngine->createChannel
    ( this->SyncDisplay,
      this->MantaCamera,
      stereo, size[0], size[1] );

  //CDDEBUG
  /*static bool once = false;
    if (!once)
    {
    printf("adding fake mesh for mantamanager\n");
    once = true;
    Manta::PreprocessContext context(this->GetMantaEngine(), 0, 1,
    this->GetMantaLightSet());
  //CDDEBUG
  // if (mesh)
  {
  as = new Manta::DynBVH(true);
  m2 = new Manta::Mesh();
  m2->materials.push_back(new Manta::Lambertian(Manta::Color(Manta::RGB(1,0,0))));

  for(size_t i = 0; i < 512; i+=3)
  {
  for(int j = 0; j < 3; j++)
  {
  float v1 = drand48()*2.0 - 1.0;
  float v2 = drand48()*2.0 - 1.0;
  float v3 = drand48()*2.0 - 1.0;
  m2->vertices.push_back(Manta::Vector(v1,v2,v3)*4.0);
  //  printf("fake vert: %f %f %f\n", v1, v2, v3);
  }
  //m2->vertices.push_back(mesh->vertices[i]);
  //m2->vertices.push_back(mesh->vertices[i+1]);
  //m2->vertices.push_back(mesh->vertices[i+2]);
  m2->vertex_indices.push_back(i);
  m2->vertex_indices.push_back(i+1);
  m2->vertex_indices.push_back(i+2);
  m2->addTriangle(new Manta::KenslerShirleyTriangle());
  m2->face_material.push_back(0);
  }
  as->setGroup(m2);
  as->preprocess(context);
  this->MantaWorldGroup->add(as);
  }
  }*/

  //...CDDEBUG
  //printf("MX(%p) END\n", this);
  //fflush(stdout);
}
