/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItDataSetMapper.cxx,v $
  Language:  C++
  Date:      $Date: 2002/11/03 22:52:54 $
  Version:   $Revision: 1.63 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItDataSetMapper.h"

#include "vtkPolyDataMapper.h"
#include "vtkObjectFactory.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkDataSet.h"

#include <vtkVisItOpenGLPolyDataMapper.h>
#include <vtkVisItMesaPolyDataMapper.h>

vtkCxxRevisionMacro(vtkVisItDataSetMapper, "$Revision: 1.63 $");
vtkStandardNewMacro(vtkVisItDataSetMapper);

// ****************************************************************************
// Modifications:
//   Brad Whitlock, Fri Aug 26 10:07:55 PDT 2005
//   Added PointTextureMethod.
//
// ****************************************************************************

vtkVisItDataSetMapper::vtkVisItDataSetMapper()
{
  this->GeometryExtractor = NULL;
  this->PolyDataMapper = NULL;
  this->PointTextureMethod = TEXTURE_NO_POINTS;
}

vtkVisItDataSetMapper::~vtkVisItDataSetMapper()
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

void vtkVisItDataSetMapper::SetInput(vtkDataSet *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

vtkDataSet *vtkVisItDataSetMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Inputs[0]);
}

void vtkVisItDataSetMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  if (this->PolyDataMapper)
    {
    this->PolyDataMapper->ReleaseGraphicsResources( renWin );
    }
}

// ****************************************************************************
//
// Receives from Actor -> maps data to primitives
//
// Modifications:
//   Brad Whitlock, Fri Aug 26 10:07:55 PDT 2005
//   Added PointTextureMethod and made sure that the point texturing mode
//   for the polydata mapper gets initialized on creation.
//
// ****************************************************************************

void vtkVisItDataSetMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  // make sure that we've been properly initialized
  //
  if ( !this->GetInput() )
    {
    vtkErrorMacro(<< "No input!\n");
    return;
    } 

  // Need a lookup table
  //
  if ( this->LookupTable == NULL )
    {
    this->CreateDefaultLookupTable();
    }
  this->LookupTable->Build();

  // Now can create appropriate mapper
  //
  if ( this->PolyDataMapper == NULL ) 
    {
    vtkDataSetSurfaceFilter *gf = vtkDataSetSurfaceFilter::New();
    vtkPolyDataMapper *pm = vtkPolyDataMapper::New();
    pm->SetInput(gf->GetOutput());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    this->SetPointTextureMethod(this->PointTextureMethod);
    }

  // share clipping planes with the PolyDataMapper
  //
  if (this->ClippingPlanes != this->PolyDataMapper->GetClippingPlanes()) 
    {
    this->PolyDataMapper->SetClippingPlanes(this->ClippingPlanes);
    }

  // For efficiency: if input type is vtkPolyData, there's no need to 
  // pass it thru the geometry filter.
  //
  if ( this->GetInput()->GetDataObjectType() == VTK_POLY_DATA )
    {
    this->PolyDataMapper->SetInput((vtkPolyData *)(this->GetInput()));
    }
  else
    {
    this->GeometryExtractor->SetInput(this->GetInput());
    this->PolyDataMapper->SetInput(this->GeometryExtractor->GetOutput());
    }
  
  // update ourselves in case something has changed
  this->PolyDataMapper->SetLookupTable(this->GetLookupTable());
  this->PolyDataMapper->SetScalarVisibility(this->GetScalarVisibility());
  this->PolyDataMapper->SetUseLookupTableScalarRange(
    this->GetUseLookupTableScalarRange());
  this->PolyDataMapper->SetScalarRange(this->GetScalarRange());
  this->PolyDataMapper->SetImmediateModeRendering(
    this->GetImmediateModeRendering());
  this->PolyDataMapper->SetColorMode(this->GetColorMode());
  this->PolyDataMapper->SetScalarMode(this->GetScalarMode());
  if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
       this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
    {
    if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
      {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayId,ArrayComponent);
      }
    else
      {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayName,ArrayComponent);
      }
    }
  
  this->PolyDataMapper->Render(ren,act);
  this->TimeToDraw = this->PolyDataMapper->GetTimeToDraw();
}

void vtkVisItDataSetMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->PolyDataMapper )
    {
    os << indent << "Poly Mapper: (" << this->PolyDataMapper << ")\n";
    }
  else
    {
    os << indent << "Poly Mapper: (none)\n";
    }

  if ( this->GeometryExtractor )
    {
    os << indent << "Geometry Extractor: (" << this->GeometryExtractor << ")\n";
    }
  else
    {
    os << indent << "Geometry Extractor: (none)\n";
    }
}

unsigned long vtkVisItDataSetMapper::GetMTime()
{
  unsigned long mTime=this->vtkMapper::GetMTime();
  unsigned long time;

  if ( this->LookupTable != NULL )
    {
    time = this->LookupTable->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

// ****************************************************************************
// Method: vtkVisItDataSetMapper::SetPointTextureMethod
//
// Purpose: 
//   Sets the point texturing method into this class's PointTextureMethod
//   member and the polydata mapper.
//
// Arguments:
//   m : The new texture mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 26 10:23:34 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
vtkVisItDataSetMapper::SetPointTextureMethod(
    vtkVisItDataSetMapper::PointTextureMode m)
{
  this->PointTextureMethod = m;

  if(this->PolyDataMapper != NULL)
    {
    if(strcmp(this->PolyDataMapper->GetClassName(),
              "vtkVisItOpenGLPolyDataMapper") == 0)
      {
        vtkVisItOpenGLPolyDataMapper *m = 
            (vtkVisItOpenGLPolyDataMapper *)this->PolyDataMapper;
        if(this->PointTextureMethod == TEXTURE_NO_POINTS)
            m->SetPointTextureMethod(
                vtkVisItOpenGLPolyDataMapper::TEXTURE_NO_POINTS);
        else if(this->PointTextureMethod == TEXTURE_USING_POINTSPRITES)
            m->SetPointTextureMethod(
                vtkVisItOpenGLPolyDataMapper::TEXTURE_USING_POINTSPRITES);
      }
    else if(strcmp(this->PolyDataMapper->GetClassName(), 
                   "vtkVisItMesaPolyDataMapper") == 0)
      {
        vtkVisItMesaPolyDataMapper *m = 
            (vtkVisItMesaPolyDataMapper *)this->PolyDataMapper;
        if(this->PointTextureMethod == TEXTURE_NO_POINTS)
            m->SetPointTextureMethod(
               vtkVisItMesaPolyDataMapper::TEXTURE_NO_POINTS);
        else if(this->PointTextureMethod == TEXTURE_USING_POINTSPRITES)
            m->SetPointTextureMethod(
               vtkVisItMesaPolyDataMapper::TEXTURE_USING_POINTSPRITES);
      }
    }
}
