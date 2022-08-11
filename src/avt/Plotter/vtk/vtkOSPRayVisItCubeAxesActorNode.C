// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkOSPRayVisItCubeAxesActorNode.h"
#include "vtkVisItCubeAxesActor.h"
#include "vtkOSPRayRendererNode.h"
#include "vtkRenderer.h"

//============================================================================
vtkStandardNewMacro(vtkOSPRayVisItCubeAxesActorNode);

//----------------------------------------------------------------------------
vtkOSPRayVisItCubeAxesActorNode::vtkOSPRayVisItCubeAxesActorNode()
{
}

//----------------------------------------------------------------------------
vtkOSPRayVisItCubeAxesActorNode::~vtkOSPRayVisItCubeAxesActorNode()
{
}

//----------------------------------------------------------------------------
void vtkOSPRayVisItCubeAxesActorNode::Build(bool prepass)
{
  if(prepass)
  {
    vtkVisItCubeAxesActor* cubeAxesActor = vtkVisItCubeAxesActor::SafeDownCast
      (this->GetRenderable());
    vtkOSPRayRendererNode* ospRenderer = static_cast<vtkOSPRayRendererNode*>(
        this->GetFirstAncestorOfType("vtkOSPRayRendererNode"));
    vtkRenderer* ren = vtkRenderer::SafeDownCast(ospRenderer->GetRenderable());
    vtkCollection* objCollection = vtkCollection::New();
    cubeAxesActor->BuildGeometry(ren, objCollection);
    this->PrepareNodes();
    this->AddMissingNodes(objCollection);
    this->RemoveUnusedNodes();
    objCollection->Delete();
  }
}

//----------------------------------------------------------------------------
vtkMTimeType vtkOSPRayVisItCubeAxesActorNode::GetMTime()
{
  vtkMTimeType mtime = this->Superclass::GetMTime();
  vtkVisItCubeAxesActor* cubeAxesActor = vtkVisItCubeAxesActor::SafeDownCast
      (this->GetRenderable());
  vtkCamera *cam = cubeAxesActor->GetCamera();

  if (cam->GetMTime() > mtime)
  {
    mtime = cam->GetMTime();
  }

  return mtime;
}
