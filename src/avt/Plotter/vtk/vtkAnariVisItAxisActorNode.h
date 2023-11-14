// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
/**
 * @class   vtkAnariVisItAxisActorNode
 * @brief   links vtkVisItAxisActor to ANARI
 *
 * Translates vtkVisItAxisActor state into ANARI rendering calls
*/

#ifndef vtkAnariVisItAxisActorNode_h
#define vtkAnariVisItAxisActorNode_h

#include <plotter_exports.h>
#include <vtkAnariActorNode.h>

class  PLOTTER_API vtkAnariVisItAxisActorNode : public vtkAnariActorNode
{
public:
  static vtkAnariVisItAxisActorNode* New();
  vtkTypeMacro(vtkAnariVisItAxisActorNode, vtkAnariActorNode);

  vtkMTimeType GetMTime() override;

  void Build(bool prepass) override;

protected:
  vtkAnariVisItAxisActorNode() = default;
  ~vtkAnariVisItAxisActorNode() = default;

private:
  vtkAnariVisItAxisActorNode(const vtkAnariVisItAxisActorNode&) = delete;
  void operator=(const vtkAnariVisItAxisActorNode&) = delete;
};
#endif
