/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayVisItDataSetMapperNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkOSPRayVisItDataSetMapperNode
 * @brief   links vtkActor and vtkMapper to OSPRay
 *
 * Translates vtkActor/Mapper state into OSPRay rendering calls
 */

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
