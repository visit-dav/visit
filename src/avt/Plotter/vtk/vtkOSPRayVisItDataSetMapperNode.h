// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
// Class: vtkOSPRayVisItDataSetMapperNode
//
// Purpose:
//   Translates vtkActor/Mapper state into OSPRay rendering calls but
//   inserts a vtkDataSetSurfaceFilter if the data is not VTK_POLY_DATA.
//
// Notes:
//
// Programmer: Allen Sanderson
// Creation:
//
// Modifications:
//
// ****************************************************************************

#ifndef vtkOSPRayVisItDataSetMapperNode_h
#define vtkOSPRayVisItDataSetMapperNode_h

#include <plotter_exports.h>
#include <vtkOSPRayPolyDataMapperNode.h>

class vtkDataSetSurfaceFilter;

class PLOTTER_API vtkOSPRayVisItDataSetMapperNode :
  public vtkOSPRayPolyDataMapperNode
{
public:
  static vtkOSPRayVisItDataSetMapperNode* New();
  vtkTypeMacro(vtkOSPRayVisItDataSetMapperNode, vtkOSPRayPolyDataMapperNode);

  /**
   * Make ospray calls to render me.
   */
  void Render(bool prepass) override;

protected:
  vtkOSPRayVisItDataSetMapperNode();
  ~vtkOSPRayVisItDataSetMapperNode() override;

  vtkDataSetSurfaceFilter *GeometryExtractor {nullptr};

private:
  vtkOSPRayVisItDataSetMapperNode(const vtkOSPRayVisItDataSetMapperNode&) = delete;
  void operator=(const vtkOSPRayVisItDataSetMapperNode&) = delete;
};

#endif
