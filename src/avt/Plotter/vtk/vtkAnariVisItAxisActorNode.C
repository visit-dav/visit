// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkAnariVisItAxisActorNode.h"
#include "vtkVisItAxisActor.h"

#include <vtkAnariSceneGraph.h>
#include <vtkRenderer.h>

//============================================================================
vtkStandardNewMacro(vtkAnariVisItAxisActorNode);

//----------------------------------------------------------------------------
void vtkAnariVisItAxisActorNode::Build(bool prepass)
{
  if(prepass)
  {
    vtkVisItAxisActor* axisActor = vtkVisItAxisActor::SafeDownCast(
        this->GetRenderable());
    vtkAnariSceneGraph* anariSceneGraph = static_cast<vtkAnariSceneGraph*>(
        this->GetFirstAncestorOfType("vtkAnariSceneGraph"));
    vtkRenderer* ren = anariSceneGraph->GetRenderer();
    vtkCollection* objCollection = vtkCollection::New();

    axisActor->BuildGeometry(ren, objCollection);
    this->PrepareNodes();
    this->AddMissingNodes(objCollection);
    this->RemoveUnusedNodes();
    objCollection->Delete();
  }
}

//----------------------------------------------------------------------------
vtkMTimeType vtkAnariVisItAxisActorNode::GetMTime()
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
