/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItDataSetMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItDataSetMapper.h"

#include "vtkDataSet.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRectilinearGridMapper.h"

#include "vtkOpenGLRectilinearGridMapper.h"
#include "vtkMesaRectilinearGridMapper.h"
#include <vtkVisItOpenGLPolyDataMapper.h>
#include <vtkVisItMesaPolyDataMapper.h>


vtkCxxRevisionMacro(vtkVisItDataSetMapper, "$Revision: 1.70 $");
vtkStandardNewMacro(vtkVisItDataSetMapper);

// ****************************************************************************
// Modifications:
//   Brad Whitlock, Fri Aug 26 10:07:55 PDT 2005
//   Added PointTextureMethod.
//
//   Brad Whitlock, Fri Aug 25 10:44:31 PDT 2006
//   Added EnableColorTexturing.
//
// ****************************************************************************

vtkVisItDataSetMapper::vtkVisItDataSetMapper()
{
  this->GeometryExtractor = NULL;
  this->PolyDataMapper = NULL;
  this->RectilinearGridMapper = NULL;
  this->PointTextureMethod = TEXTURE_NO_POINTS;
  this->EnableColorTexturing = false;
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
  if ( this->RectilinearGridMapper )
    {
    this->RectilinearGridMapper->Delete();
    }
}

void vtkVisItDataSetMapper::SetInput(vtkDataSet *input)
{
  if (input)
    {
    this->SetInputConnection(0, input->GetProducerPort());
    }
  else 
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0, 0);
    }
}

vtkDataSet *vtkVisItDataSetMapper::GetInput()
{
  return this->Superclass::GetInputAsDataSet();
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
//   Brad Whitlock, Fri Aug 25 10:46:54 PDT 2006
//   Added a call to SetEnableColorTexturing.
//
//   Hank Childs, Mon Dec 18 15:13:10 PST 2006
//   Add support for rectilinear rendering.
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
    vtkRectilinearGridMapper *rgm = vtkRectilinearGridMapper::New();
    pm->SetInput(gf->GetOutput());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    this->RectilinearGridMapper = rgm;
    this->SetPointTextureMethod(this->PointTextureMethod);
    this->SetEnableColorTexturing(this->EnableColorTexturing);
    }

  // For efficiency: if input type is vtkPolyData, there's no need to 
  // pass it thru the geometry filter.
  //
  if ( this->GetInput()->GetDataObjectType() == VTK_RECTILINEAR_GRID )
    {
    this->RectilinearGridMapper->SetInput((vtkRectilinearGrid *)(this->GetInput()));
    }
  else if ( this->GetInput()->GetDataObjectType() == VTK_POLY_DATA )
    {
    this->PolyDataMapper->SetInput((vtkPolyData *)(this->GetInput()));
    }
  else
    {
    this->GeometryExtractor->SetInput(this->GetInput());
    this->PolyDataMapper->SetInput(this->GeometryExtractor->GetOutput());
    }
  
  vtkMapper *mapper = NULL;
  if ( this->GetInput()->GetDataObjectType() == VTK_RECTILINEAR_GRID )
    {
    mapper = this->RectilinearGridMapper;
    }
  else
    {
    mapper = this->PolyDataMapper;
    }
    
  // share clipping planes with the mapper
  //
  if (this->ClippingPlanes != mapper->GetClippingPlanes()) 
    {
    mapper->SetClippingPlanes(this->ClippingPlanes);
    }

  mapper->SetLookupTable(this->GetLookupTable());
  mapper->SetScalarVisibility(this->GetScalarVisibility());
  mapper->SetUseLookupTableScalarRange(
      this->GetUseLookupTableScalarRange());
  mapper->SetScalarRange(this->GetScalarRange());
  mapper->SetImmediateModeRendering(
      this->GetImmediateModeRendering());
  mapper->SetColorMode(this->GetColorMode());
  mapper->SetInterpolateScalarsBeforeMapping(
                                 this->GetInterpolateScalarsBeforeMapping());
  mapper->SetScalarMode(this->GetScalarMode());
  if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
       this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
    {
    if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
      {
    mapper->ColorByArrayComponent(this->ArrayId,ArrayComponent);
      }
    else
      {
    mapper->ColorByArrayComponent(this->ArrayName,ArrayComponent);
      }
    }
  mapper->Render(ren,act);
  this->TimeToDraw = mapper->GetTimeToDraw();
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

  if ( this->RectilinearGridMapper )
    {
    os << indent << "RGrid Mapper: (" << this->RectilinearGridMapper << ")\n";
    }
  else
    {
    os << indent << "RGrid Mapper: (none)\n";
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

//----------------------------------------------------------------------------
int vtkVisItDataSetMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkVisItDataSetMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  vtkGarbageCollectorReport(collector, this->GeometryExtractor,
                            "GeometryExtractor");
  vtkGarbageCollectorReport(collector, this->PolyDataMapper,
                            "PolyDataMapper");
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

// ****************************************************************************
// Method: vtkVisItDataSetMapper::SetEnableColorTexturing
//
// Purpose: 
//   Sets whether the mapper will perform color texturing.
//
// Arguments:
//   val : Whether color texturing will be performed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 10:45:51 PDT 2006
//
// Modifications:
//   
//   Hank Childs, Tue Dec 19 10:43:30 PST 2006
//   Add support for rectilinear mappers.
//
// ****************************************************************************

void
vtkVisItDataSetMapper::SetEnableColorTexturing(bool val)
{
  this->EnableColorTexturing = val;

  if(this->PolyDataMapper != NULL)
    {
    if(strcmp(this->PolyDataMapper->GetClassName(),
              "vtkVisItOpenGLPolyDataMapper") == 0)
      {
        vtkVisItOpenGLPolyDataMapper *m = 
            (vtkVisItOpenGLPolyDataMapper *)this->PolyDataMapper;
        m->SetEnableColorTexturing(val);
      }
    else if(strcmp(this->PolyDataMapper->GetClassName(), 
                   "vtkVisItMesaPolyDataMapper") == 0)
      {
        vtkVisItMesaPolyDataMapper *m = 
            (vtkVisItMesaPolyDataMapper *)this->PolyDataMapper;
        m->SetEnableColorTexturing(val);
      }
    }
  if(this->RectilinearGridMapper != NULL)
    {
    if(strcmp(this->RectilinearGridMapper->GetClassName(),
              "vtkOpenGLRectilinearGridMapper") == 0)
      {
        vtkOpenGLRectilinearGridMapper *m = 
            (vtkOpenGLRectilinearGridMapper *)this->RectilinearGridMapper;
        m->SetEnableColorTexturing(val);
      }
    else if(strcmp(this->RectilinearGridMapper->GetClassName(), 
                   "vtkMesaRectilinearGridMapper") == 0)
      {
        vtkMesaRectilinearGridMapper *m = 
            (vtkMesaRectilinearGridMapper *)this->RectilinearGridMapper;
        m->SetEnableColorTexturing(val);
      }
    }
}
