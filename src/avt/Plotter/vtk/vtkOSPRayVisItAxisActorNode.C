// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkOSPRayVisItAxisActorNode.h"
#include "vtkVisItAxisActor.h"
#include "vtkOSPRayRendererNode.h"
#include "vtkRenderer.h"

//============================================================================
vtkStandardNewMacro(vtkOSPRayVisItAxisActorNode);

//----------------------------------------------------------------------------
vtkOSPRayVisItAxisActorNode::vtkOSPRayVisItAxisActorNode()
{
}

//----------------------------------------------------------------------------
vtkOSPRayVisItAxisActorNode::~vtkOSPRayVisItAxisActorNode()
{
}

//----------------------------------------------------------------------------
void vtkOSPRayVisItAxisActorNode::Build(bool prepass)
{
  if(prepass)
  {
    vtkVisItAxisActor* axisActor = vtkVisItAxisActor::SafeDownCast(
        this->GetRenderable());
    vtkOSPRayRendererNode* ospRenderer = static_cast<vtkOSPRayRendererNode*>(
        this->GetFirstAncestorOfType("vtkOSPRayRendererNode"));

    vtkRenderer* ren = vtkRenderer::SafeDownCast(ospRenderer->GetRenderable());
    
    vtkCollection* objCollection = vtkCollection::New();
    axisActor->BuildGeometry(ren, objCollection);
    this->PrepareNodes();
    this->AddMissingNodes(objCollection);
    this->RemoveUnusedNodes();
    objCollection->Delete();
  }
}

//----------------------------------------------------------------------------
vtkMTimeType vtkOSPRayVisItAxisActorNode::GetMTime()
{
  vtkMTimeType mtime = this->Superclass::GetMTime();
  vtkVisItAxisActor* axisActor = vtkVisItAxisActor::SafeDownCast
      (this->GetRenderable());
  vtkCamera *cam = axisActor->GetCamera();

  if (cam->GetMTime() > mtime)
  {
    mtime = cam->GetMTime();
  }

  return mtime;
}
