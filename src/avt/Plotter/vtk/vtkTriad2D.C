// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkTriad2D.h"

// *****************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
//
//    Jeremy Meredith, Thu Jan 31 10:30:25 EST 2008
//    Made use of new string offsets to center the titles of
//    each axis of the triad a little better.
//
//    Jeremy Meredith, Fri Feb  1 13:07:30 EST 2008
//    Removed the setting of the text offsets.  It wasn't
//    enough of an improvement to justify the effort in updating
//    baselines.  Plus, the full 0.5 offset for vertical was too
//    much, possibly due to whitespace being included in the height.
//    If this is reinstated, it should be more like 0.25 to 0.40.
//
//    Brad Whitlock, Thu Sep 29 17:05:04 PDT 2011
//    Override title text justification.
//
// *****************************************************************

vtkStandardNewMacro(vtkTriad2D);
vtkCxxSetObjectMacro(vtkTriad2D, Camera,vtkCamera); 


vtkTriad2D::vtkTriad2D()
{
  this->Camera           = NULL;
  this->Origin[0]        = 0.10;
  this->Origin[1]        = 0.10;
  this->AxisLength = 0.08;

  this->XAxis = vtkVisItAxisActor2D::New();
  this->XAxis->SetTitle("X");
  this->XAxis->SetTickVisibility(0);
  this->XAxis->SetLabelVisibility(0);
  this->XAxis->SetShadow(0);
  this->XAxis->SetTitleAtEnd(1);
  this->XAxis->SetTitleJustification(0);
  this->XAxis->SetTitleVerticalJustification(0);
  this->XAxis->GetPoint1Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 
  this->XAxis->GetPoint2Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 

  this->YAxis = vtkVisItAxisActor2D::New();
  this->YAxis->SetTitle("Y");
  this->YAxis->SetTickVisibility(0);
  this->YAxis->SetLabelVisibility(0);
  this->YAxis->SetShadow(0);
  this->YAxis->SetTitleAtEnd(1);
  this->YAxis->SetTitleJustification(0);
  this->YAxis->SetTitleVerticalJustification(0);
  this->YAxis->GetPoint1Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 
  this->YAxis->GetPoint2Coordinate()
                   ->SetCoordinateSystemToNormalizedViewport(); 

  this->ZAxis = vtkVisItAxisActor2D::New();
  this->ZAxis->SetTitle("Z");
  this->ZAxis->SetTickVisibility(0);
  this->ZAxis->SetLabelVisibility(0);
  this->ZAxis->SetShadow(0);
  this->ZAxis->SetTitleAtEnd(1);
  this->ZAxis->SetTitleJustification(0);
  this->ZAxis->SetTitleVerticalJustification(0);
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

