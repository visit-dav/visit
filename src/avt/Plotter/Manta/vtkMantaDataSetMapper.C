/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDataSetMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMantaDataSetMapper.h"

#include "vtkDataSet.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkMantaPolyDataMapper.h"

//vtkStandardNewMacro(vtkDataSetMapper);



//----------------------------------------------------------------------------
vtkMantaDataSetMapper::vtkMantaDataSetMapper()
{
  this->GeometryExtractor = NULL;
  this->PolyDataMapper = NULL;
}

//----------------------------------------------------------------------------
vtkMantaDataSetMapper::~vtkMantaDataSetMapper()
{
  // delete internally created objects.
  if ( this->GeometryExtractor )
    {
    this->GeometryExtractor->Delete();
    }
  if ( this->PolyDataMapper )
    {
    this->PolyDataMapper->Delete();
    }
}


//Vtkmantadatasetmapper  *vtkMantaDataSetMapper::New() { return new vtkMantaDataSetMapper(); }

void vtkMantaDataSetMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  cout << "manta rendering\n";
   if ( this->PolyDataMapper == NULL )
    {
    vtkDataSetSurfaceFilter *gf = vtkDataSetSurfaceFilter::New();
    vtkMantaPolyDataMapper *pm = vtkMantaPolyDataMapper::New();
    pm->SetInputConnection(gf->GetOutputPort());
    mapper = pm;
    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    }
   // vtkDataSetMapper::Render(ren, act);
   mapper->Draw(ren,act);
}
