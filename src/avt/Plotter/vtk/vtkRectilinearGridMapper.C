/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkRectilinearGridMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkRectilinearGridMapper.h"

#include "vtkExecutive.h"
#include "vtkGraphicsFactory.h"
#include "vtkInformation.h"
#include "vtkMath.h"
#include "vtkRectilinearGrid.h"
#include "vtkRenderWindow.h"
#include "vtkToolkits.h"

#include "vtkOpenGLRectilinearGridMapper.h"

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkRectilinearGridMapper);

//----------------------------------------------------------------------------
// return the correct type of RectilinearGridMapper 
vtkRectilinearGridMapper *vtkRectilinearGridMapper::New()
{
  return vtkOpenGLRectilinearGridMapper::New();
}


//----------------------------------------------------------------------------
vtkRectilinearGridMapper::vtkRectilinearGridMapper()
{
  this->SceneIs3D = true;
}

//----------------------------------------------------------------------------
void vtkRectilinearGridMapper::SetInputData(vtkRectilinearGrid *input)
{
    this->SetInputDataInternal(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkRectilinearGrid *vtkRectilinearGridMapper::GetInput()
{
  return vtkRectilinearGrid::SafeDownCast(
    this->GetExecutive()->GetInputData(0, 0));
}


void vtkRectilinearGridMapper::ShallowCopy(vtkAbstractMapper *mapper)
{
  vtkRectilinearGridMapper *m = vtkRectilinearGridMapper::SafeDownCast(mapper);
  if ( m != NULL )
    {
    this->SetInputConnection(m->GetInputConnection(0, 0));
    }

  // Now do superclass
  this->vtkMapper::ShallowCopy(mapper);
}

void vtkRectilinearGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkRectilinearGridMapper::FillInputPortInformation(
  int vtkNotUsed( port ), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkRectilinearGrid");
  return 1;
}

// Get the bounds for the input of this mapper as
// (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *vtkRectilinearGridMapper::GetBounds()
{
  static double bounds[] = {-1.0,1.0, -1.0,1.0, -1.0,1.0};

  // do we have an input
  if (this->GetNumberOfInputConnections(0))
    {
    this->Update();
    if(this->GetInput() != NULL)
      {
      this->GetInput()->GetBounds(this->Bounds);
      return this->Bounds;
      }
    }

  return bounds;
}


