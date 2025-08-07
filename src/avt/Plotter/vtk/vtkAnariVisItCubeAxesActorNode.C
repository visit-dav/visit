// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkAnariVisItCubeAxesActorNode.h"
#include "vtkVisItCubeAxesActor.h"

#include <vtkAnariSceneGraph.h>
#include <vtkRenderer.h>

//============================================================================
vtkStandardNewMacro(vtkAnariVisItCubeAxesActorNode);

//----------------------------------------------------------------------------
void vtkAnariVisItCubeAxesActorNode::Build(bool prepass)
{
  if(prepass)
  {
    vtkVisItCubeAxesActor* cubeAxesActor = vtkVisItCubeAxesActor::SafeDownCast
      (this->GetRenderable());
     vtkAnariSceneGraph* anariSceneGraph = static_cast<vtkAnariSceneGraph*>(
        this->GetFirstAncestorOfType("vtkAnariSceneGraph"));
    vtkRenderer* ren = anariSceneGraph->GetRenderer();
    vtkCollection* objCollection = vtkCollection::New();

    cubeAxesActor->BuildGeometry(ren, objCollection);
    this->PrepareNodes();
    this->AddMissingNodes(objCollection);
    this->RemoveUnusedNodes();
    objCollection->Delete();
  }
}

//----------------------------------------------------------------------------
vtkMTimeType vtkAnariVisItCubeAxesActorNode::GetMTime()
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
