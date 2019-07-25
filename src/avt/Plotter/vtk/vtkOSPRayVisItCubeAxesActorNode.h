// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
/**
 * @class   vtkOSPRayVisItCubeAxesActorNode
 * @brief   links vtkVisItCubeAxesActor to OSPRay
 *
 * Translates vtkVisItCubeAxesActor state into OSPRay rendering calls
*/

#ifndef vtkOSPRayVisItCubeAxesActorNode_h
#define vtkOSPRayVisItCubeAxesActorNode_h

#include <plotter_exports.h>
#include <vtkOSPRayActorNode.h>

class PLOTTER_API vtkOSPRayVisItCubeAxesActorNode :
  public vtkOSPRayActorNode
{
public:
  static vtkOSPRayVisItCubeAxesActorNode* New();
  vtkTypeMacro(vtkOSPRayVisItCubeAxesActorNode, vtkOSPRayActorNode);

  vtkMTimeType GetMTime() override;

  void Build(bool prepass) override;

protected:
  vtkOSPRayVisItCubeAxesActorNode();
  ~vtkOSPRayVisItCubeAxesActorNode();

private:
  vtkOSPRayVisItCubeAxesActorNode(const vtkOSPRayVisItCubeAxesActorNode&) = delete;
  void operator=(const vtkOSPRayVisItCubeAxesActorNode&) = delete;
};
#endif
