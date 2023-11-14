// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
/**
 * @class   vtkAnariVisItCubeAxesActorNode
 * @brief   links vtkVisItCubeAxesActor to ANARI
 *
 * Translates vtkVisItCubeAxesActor state into ANARI rendering calls
*/

#ifndef vtkAnariVisItCubeActorNode_h
#define vtkAnariVisItCubeActorNode_h

#include <plotter_exports.h>
#include <vtkAnariActorNode.h>

class PLOTTER_API vtkAnariVisItCubeAxesActorNode : public vtkAnariActorNode
{
public:
  static vtkAnariVisItCubeAxesActorNode* New();
  vtkTypeMacro(vtkAnariVisItCubeAxesActorNode, vtkAnariActorNode);

  vtkMTimeType GetMTime() override;

  void Build(bool prepass) override;

protected:
  vtkAnariVisItCubeAxesActorNode() = default;
  ~vtkAnariVisItCubeAxesActorNode() = default;

private:
  vtkAnariVisItCubeAxesActorNode(const vtkAnariVisItCubeAxesActorNode&) = delete;
  void operator=(const vtkAnariVisItCubeAxesActorNode&) = delete;
};
#endif
