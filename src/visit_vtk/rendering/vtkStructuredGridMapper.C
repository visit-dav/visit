/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkStructuredGridMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStructuredGridMapper.h"

#include "vtkExecutive.h"
#include "vtkGraphicsFactory.h"
#include "vtkInformation.h"
#include "vtkMath.h"
#include "vtkStructuredGrid.h"
#include "vtkRenderWindow.h"

#ifdef VTK_USE_MANGLED_MESA
#include "vtkMesaStructuredGridMapper.h"
#endif
#include "vtkOpenGLStructuredGridMapper.h"

vtkCxxRevisionMacro(vtkStructuredGridMapper, "$Revision: 1.38 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkStructuredGridMapper);

//----------------------------------------------------------------------------
// return the correct type of StructuredGridMapper 
vtkStructuredGridMapper *vtkStructuredGridMapper::New()
{
#ifdef VTK_USE_MANGLED_MESA
  if ( vtkGraphicsFactory::GetUseMesaClasses() )
    {
    return vtkMesaStructuredGridMapper::New();
    }
#endif
  return vtkOpenGLStructuredGridMapper::New();
}


//----------------------------------------------------------------------------
vtkStructuredGridMapper::vtkStructuredGridMapper()
{
  this->SceneIs3D = true;
}

//----------------------------------------------------------------------------
void vtkStructuredGridMapper::SetInput(vtkStructuredGrid *input)
{
  if(input)
    {
    this->SetInputConnection(0, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0, 0);
    }
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkStructuredGrid *vtkStructuredGridMapper::GetInput()
{
  return vtkStructuredGrid::SafeDownCast(
    this->GetExecutive()->GetInputData(0, 0));
}


void vtkStructuredGridMapper::ShallowCopy(vtkAbstractMapper *mapper)
{
  vtkStructuredGridMapper *m = vtkStructuredGridMapper::SafeDownCast(mapper);
  if ( m != NULL )
    {
    this->SetInput(m->GetInput());
    }

  // Now do superclass
  this->vtkMapper::ShallowCopy(mapper);
}

void vtkStructuredGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkStructuredGridMapper::FillInputPortInformation(
  int vtkNotUsed( port ), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkStructuredGrid");
  return 1;
}

// Get the bounds for the input of this mapper as
// (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *vtkStructuredGridMapper::GetBounds()
{
  static double bounds[] = {-1.0,1.0, -1.0,1.0, -1.0,1.0};

  // do we have an input
  if (this->GetNumberOfInputConnections(0))
    {
    this->Update();
    this->GetInput()->GetBounds(this->Bounds);
    return this->Bounds;
    }

  return bounds;
}


