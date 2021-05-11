/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayVisItDataSetMapperNode.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOSPRayVisItDataSetMapperNode.h"

#include "vtkActor.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkOSPRayActorNode.h"
#include "vtkOSPRayRendererNode.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"

#include "vtkVisItDataSetMapper.h"

//============================================================================
vtkStandardNewMacro(vtkOSPRayVisItDataSetMapperNode);

//------------------------------------------------------------------------------
vtkOSPRayVisItDataSetMapperNode::vtkOSPRayVisItDataSetMapperNode() {}

//------------------------------------------------------------------------------
vtkOSPRayVisItDataSetMapperNode::~vtkOSPRayVisItDataSetMapperNode()
{
   if ( this->GeometryExtractor )
   {
       this->GeometryExtractor->Delete();
   }
}

//------------------------------------------------------------------------------
void vtkOSPRayVisItDataSetMapperNode::Render(bool prepass)
{
  if (prepass)
  {
    // we use a lot of params from our parent
    vtkOSPRayActorNode* aNode = vtkOSPRayActorNode::SafeDownCast(this->Parent);
    vtkActor* act = vtkActor::SafeDownCast(aNode->GetRenderable());

    if (act->GetVisibility() == false)
    {
      return;
    }

    vtkOSPRayRendererNode* orn =
      static_cast<vtkOSPRayRendererNode*>(this->GetFirstAncestorOfType("vtkOSPRayRendererNode"));

    // if there are no changes, just reuse last result
    vtkMTimeType inTime = aNode->GetMTime();
    if (this->RenderTime >= inTime)
    {
      this->RenderGeometricModels();
      return;
    }
    this->RenderTime = inTime;
    this->ClearGeometricModels();

    vtkPolyData* poly = nullptr;
    vtkVisItDataSetMapper* mapper = vtkVisItDataSetMapper::SafeDownCast(act->GetMapper());
    if (mapper && mapper->GetNumberOfInputPorts() > 0)
    {
      if (mapper->GetInput()->GetDataObjectType() == VTK_POLY_DATA)
      {
        poly = (vtkPolyData*)(mapper->GetInput());
      }
      else
      {
        if (! this->GeometryExtractor)
        {
          this->GeometryExtractor = vtkDataSetSurfaceFilter::New();
        }
        this->GeometryExtractor->SetInputData(mapper->GetInput());
        this->GeometryExtractor->Update();
        poly = (vtkPolyData*)this->GeometryExtractor->GetOutput();
      }
    }
    if (poly)
    {
      vtkProperty* property = act->GetProperty();
      double ambient[3];
      double diffuse[3];
      property->GetAmbientColor(ambient);
      property->GetDiffuseColor(diffuse);
      this->ORenderPoly(
        orn->GetORenderer(), aNode, poly, ambient, diffuse, property->GetOpacity(), "");
    }
    this->RenderGeometricModels();
  }
}
