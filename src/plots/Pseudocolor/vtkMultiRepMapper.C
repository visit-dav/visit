// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkMultiRepMapper.h"

#include <vtkActor.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMultiRepMapper)

//-----------------------------------------------------------------------------
vtkMultiRepMapper::vtkMultiRepMapper()
{
  this->DrawSurface = true;
  this->DrawWireframe = false;
  this->DrawPoints = false;
  this->WireframeColor[0] = 1.;
  this->WireframeColor[1] = 1.;
  this->WireframeColor[2] = 1.;
  this->PointsColor[0] = 1.;
  this->PointsColor[1] = 1.;
  this->PointsColor[2] = 1.;
  this->currentScalarVis = this->ScalarVisibility;
}

//-----------------------------------------------------------------------------
vtkMultiRepMapper::~vtkMultiRepMapper()
{
}

//----------------------------------------------------------------------------
void vtkMultiRepMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  if (this->DrawSurface)
    {
    act->GetProperty()->SetRepresentationToSurface();
    this->Superclass::Render(ren,act);
    }
  if (this->DrawWireframe)
    {
    bool sv = this->ScalarVisibility;
    this->ScalarVisibilityOff();
    
    act->GetProperty()->SetRepresentationToWireframe();
    act->GetProperty()->SetColor(this->WireframeColor);
    this->Superclass::Render(ren,act);
    this->SetScalarVisibility(sv);
    }
  if (this->DrawPoints)
    {
    bool sv = this->ScalarVisibility;
    this->ScalarVisibilityOff();
    act->GetProperty()->SetRepresentationToPoints();
    act->GetProperty()->SetColor(this->PointsColor);
    this->Superclass::Render(ren,act);
    this->SetScalarVisibility(sv);
    }
}

//-----------------------------------------------------------------------------
void vtkMultiRepMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  os << "Draw surface: "  << this->DrawSurface << endl;
  os << "Draw wireframe: " << this->DrawWireframe << endl;
  os << "Draw points: "    << this->DrawPoints << endl;
  os << "Wireframe color: " << this->WireframeColor[0]  << " "
                            << this->WireframeColor[1]  << " "
                            << this->WireframeColor[2] ;
  os << "Points color: " << this->PointsColor[0]  << " "
                         << this->PointsColor[1]  << " "
                         << this->PointsColor[2] ;
  this->Superclass::PrintSelf(os, indent);

}
