/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkTriad2D.h,v $
  Language:  C++
  Date:      $Date: 2000/07/11 18:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Hank Childs, B Division, Lawrence Livermore Nat'l Laboratory

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkTriad2D.h"

// *****************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
// *****************************************************************

vtkStandardNewMacro(vtkTriad2D);
vtkCxxSetObjectMacro(vtkTriad2D, Camera,vtkCamera); 


vtkTriad2D::vtkTriad2D()
{
  this->Camera           = NULL;
  this->Origin[0]        = 0.10;
  this->Origin[1]        = 0.10;
  this->AxisLength = 0.08;

  this->XAxis = vtkHankAxisActor2D::New();
  this->XAxis->SetTitle("X");
  this->XAxis->SetTickVisibility(0);
  this->XAxis->SetLabelVisibility(0);
  this->XAxis->SetShadow(0);
  this->XAxis->SetTitleAtEnd(1);
  this->XAxis->GetPoint1Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 
  this->XAxis->GetPoint2Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 

  this->YAxis = vtkHankAxisActor2D::New();
  this->YAxis->SetTitle("Y");
  this->YAxis->SetTickVisibility(0);
  this->YAxis->SetLabelVisibility(0);
  this->YAxis->SetShadow(0);
  this->YAxis->SetTitleAtEnd(1);
  this->YAxis->GetPoint1Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 
  this->YAxis->GetPoint2Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 

  this->ZAxis = vtkHankAxisActor2D::New();
  this->ZAxis->SetTitle("Z");
  this->ZAxis->SetTickVisibility(0);
  this->ZAxis->SetLabelVisibility(0);
  this->ZAxis->SetShadow(0);
  this->ZAxis->SetTitleAtEnd(1);
  this->ZAxis->GetPoint1Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 
  this->ZAxis->GetPoint2Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 
}


vtkTriad2D::~vtkTriad2D()
{
   this->SetCamera(NULL);
   this->XAxis->Delete();
   this->YAxis->Delete();
   this->ZAxis->Delete();
}


int vtkTriad2D::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;

  if ( this->Camera == NULL )
    {
    return 0;
    }

  renderedSomething += this->XAxis->RenderOverlay(viewport);
  renderedSomething += this->YAxis->RenderOverlay(viewport);
  renderedSomething += this->ZAxis->RenderOverlay(viewport);

  return renderedSomething;
}


int vtkTriad2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  float  x_viewport[2];
  float  y_viewport[2];
  float  z_viewport[2];
  int    renderedValue = 0;
  float  x_unit_vector[4] = { 1., 0., 0., 0. };
  float  y_unit_vector[4] = { 0., 1., 0., 0. };
  float  z_unit_vector[4] = { 0., 0., 1., 0. };

  if ( this->Camera == NULL )
    {
    vtkErrorMacro(<<"Camera was not set.");
    return 0;
    }

  TranslateVectorToViewport(x_unit_vector, x_viewport);
  this->XAxis->GetPoint1Coordinate()->SetValue(this->Origin[0], 
                                               this->Origin[1]);
  this->XAxis->GetPoint2Coordinate()->SetValue(x_viewport[0], x_viewport[1]);

  TranslateVectorToViewport(y_unit_vector, y_viewport);
  this->YAxis->GetPoint1Coordinate()->SetValue(this->Origin[0],
                                               this->Origin[1]);
  this->YAxis->GetPoint2Coordinate()->SetValue(y_viewport[0], y_viewport[1]);

  TranslateVectorToViewport(z_unit_vector, z_viewport);
  this->ZAxis->GetPoint1Coordinate()->SetValue(this->Origin[0],
                                               this->Origin[1]);
  this->ZAxis->GetPoint2Coordinate()->SetValue(z_viewport[0], z_viewport[1]);

  // Changing the point coordinates doesn't make the axes realize that they
  // are modified.  Tell them so.
  this->XAxis->Modified();
  this->YAxis->Modified();
  this->ZAxis->Modified();

  renderedValue += this->XAxis->RenderOpaqueGeometry(viewport);
  renderedValue += this->YAxis->RenderOpaqueGeometry(viewport);
  renderedValue += this->ZAxis->RenderOpaqueGeometry(viewport);

  return renderedValue;
}


void vtkTriad2D::TranslateVectorToViewport(const float world_vector[4], 
                                           float vport[2])
{
  float          origin_world[4] = { 0., 0., 0., 0. };
  float          origin_view[4];
  float          view_vector[4];
  float          unit_vector[3];
  vtkMatrix4x4  *view = this->Camera->GetViewTransformMatrix();

  // Translate origin from world coordinates to view coordinates.
  view->MultiplyPoint(origin_world, origin_view);

  // Translate vector to view coordinates.
  view->MultiplyPoint(world_vector, view_vector);

  // Find the resulting vector between the vector and the origin in
  // view coordinates and normalize it.
  unit_vector[0] = view_vector[0] - origin_view[0];
  unit_vector[1] = view_vector[1] - origin_view[1];
  unit_vector[2] = view_vector[2] - origin_view[2];
  vtkMath::Normalize(unit_vector);

  // Project onto a 2D plane.
  vport[0] = this->AxisLength*unit_vector[0] + this->Origin[0];
  vport[1] = this->AxisLength*unit_vector[1] + this->Origin[1];
}


void vtkTriad2D::SetOrigin(float new_origin[2])
{
  this->SetOrigin(new_origin[0], new_origin[1]);
}


void vtkTriad2D::SetOrigin(float new_x_origin, float new_y_origin)
{
  if (   new_x_origin >= 0. && new_x_origin <= 1. 
      && new_y_origin >= 0. && new_y_origin <= 1.)
    {
    this->Origin[0] = new_x_origin;
    this->Origin[1] = new_y_origin;
    }
  else
    {
    vtkErrorMacro(<<"Origin must be in normalized coordinates.");
    }
}


void vtkTriad2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  if ( this->Camera )
    {
    os << indent << "Camera:\n";
    this->Camera->PrintSelf(os, indent.GetNextIndent());
    }

  os << indent << "Origin: " << this->Origin[0] << ", " << this->Origin[1]
     << "\n";
  os << indent << "AxisLength: " << this->AxisLength << "\n";
  
  os << indent << "X-Axis:\n";
  this->XAxis->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Y-Axis:\n";
  this->YAxis->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Z-Axis:\n";
  this->ZAxis->PrintSelf(os, indent.GetNextIndent());
}

