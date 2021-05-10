// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
/**
 * @class   vtkOSPRayVisItAxisActorNode
 * @brief   links vtkVisItAxisActor to OSPRay
 *
 * Translates vtkVisItAxisActor state into OSPRay rendering calls
*/

#ifndef vtkOSPRayVisItAxisActorNode_h
#define vtkOSPRayVisItAxisActorNode_h

#include <plotter_exports.h>
#include <vtkOSPRayActorNode.h>

class PLOTTER_API vtkOSPRayVisItAxisActorNode :
  public vtkOSPRayActorNode
{
public:
  static vtkOSPRayVisItAxisActorNode* New();
  vtkTypeMacro(vtkOSPRayVisItAxisActorNode, vtkOSPRayActorNode);

  vtkMTimeType GetMTime() override;

  void Build(bool prepass) override;

protected:
  vtkOSPRayVisItAxisActorNode();
  ~vtkOSPRayVisItAxisActorNode();

private:
  vtkOSPRayVisItAxisActorNode(const vtkOSPRayVisItAxisActorNode&) = delete;

  void operator=(const vtkOSPRayVisItAxisActorNode&) = delete;
};
#endif
