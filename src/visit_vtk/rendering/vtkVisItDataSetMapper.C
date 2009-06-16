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
#include "vtkToolkits.h"

#include "vtkOpenGLRectilinearGridMapper.h"
#include "vtkOpenGLStructuredGridMapper.h"
#include <vtkVisItOpenGLPolyDataMapper.h>
#ifdef VTK_USE_MANGLED_MESA
#include "vtkMesaRectilinearGridMapper.h"
#include "vtkMesaStructuredGridMapper.h"
#include <vtkVisItMesaPolyDataMapper.h>
#endif

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
//   Hank Childs, Wed Dec 27 09:51:10 PST 2006
//   Initialize structured grid mapper.
//
//   Hank Childs, Thu Dec 28 11:04:05 PST 2006
//   Initialize SceneIs3D.
//
//   Dave Bremer, Wed Feb 27 15:59:48 PST 2008
//   Make this class derive from vtkDataSetMapper, letting it
//   initialize two members that we had been initializing.
// ****************************************************************************

vtkVisItDataSetMapper::vtkVisItDataSetMapper()
{
  this->RectilinearGridMapper = NULL;
  this->StructuredGridMapper = NULL;
  this->PointTextureMethod = TEXTURE_NO_POINTS;
  this->EnableColorTexturing = false;
  this->SceneIs3D = true;
}


// ****************************************************************************
// Modifications:
//   Dave Bremer, Wed Feb 27 15:59:48 PST 2008
//   Make this class derive from vtkDataSetMapper, letting it
//   delete two members that we had been deleting.
// ****************************************************************************
vtkVisItDataSetMapper::~vtkVisItDataSetMapper()
{
  if ( this->RectilinearGridMapper )
    {
    this->RectilinearGridMapper->Delete();
    }
  if ( this->StructuredGridMapper )
    {
    this->StructuredGridMapper->Delete();
    }
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sun Apr  1 11:21:14 PDT 2007
//    Tell rgrid and sgrid mappers to release graphics resources.
//
//    Dave Bremer, Wed Feb 27 15:59:48 PST 2008
//    Make this class derive from vtkDataSetMapper, and use its 
//    implementation of ReleaseGraphicsResources.
// ****************************************************************************

void vtkVisItDataSetMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  vtkDataSetMapper::ReleaseGraphicsResources(renWin);

  if ( this->RectilinearGridMapper )
    {
    this->RectilinearGridMapper->ReleaseGraphicsResources( renWin );
    }
  if ( this->StructuredGridMapper )
    {
    this->StructuredGridMapper->ReleaseGraphicsResources( renWin );
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
//   Hank Childs, Wed Dec 27 09:51:10 PST 2006
//   Add support for curvilinear rendering.
//
//   Hank Childs, Thu Dec 28 11:08:51 PST 2006
//   Tell our rectilinear mapper whether or not the scene is 3D.
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
    vtkStructuredGridMapper *sgm = vtkStructuredGridMapper::New();
    pm->SetInput(gf->GetOutput());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    this->RectilinearGridMapper = rgm;
    this->StructuredGridMapper = sgm;
    this->SetPointTextureMethod(this->PointTextureMethod);
    this->SetEnableColorTexturing(this->EnableColorTexturing);
    this->SetSceneIs3D(this->SceneIs3D);
    }

  // For efficiency: if input type is vtkPolyData, there's no need to 
  // pass it thru the geometry filter.
  //
  if ( this->GetInput()->GetDataObjectType() == VTK_RECTILINEAR_GRID )
    {
    this->RectilinearGridMapper->SetInput((vtkRectilinearGrid *)(this->GetInput()));
    }
  else if ( this->GetInput()->GetDataObjectType() == VTK_STRUCTURED_GRID )
    {
    this->StructuredGridMapper->SetInput((vtkStructuredGrid *)(this->GetInput()));
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
  else if ( this->GetInput()->GetDataObjectType() == VTK_STRUCTURED_GRID )
    {
    mapper = this->StructuredGridMapper;
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




// ****************************************************************************
// Modifications:
//   Dave Bremer, Wed Feb 27 15:59:48 PST 2008
//   Make this class derive from vtkDataSetMapper, and use its PrintSelf
//   method to print two of the members.
// ****************************************************************************

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

//----------------------------------------------------------------------------
// Modifications:
//
//   Hank Childs, Sun Apr  1 11:21:14 PDT 2007
//   Fix memory leak! ... added mappers to garbage collector.
//
//   Dave Bremer, Wed Feb 27 15:59:48 PST 2008
//   Make this class derive from vtkDataSetMapper, and use its ReportReferences
//   method for two members.
// ****************************************************************************
void vtkVisItDataSetMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);

  vtkGarbageCollectorReport(collector, this->RectilinearGridMapper,
                            "RectilinearGridMapper");
  vtkGarbageCollectorReport(collector, this->StructuredGridMapper,
                            "StructuredGridMapper");
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
//   Brad Whitlock, Wed Jun 10 11:54:59 PDT 2009
//   Conditionally compiler mesa classes.
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
#ifdef VTK_USE_MANGLED_MESA
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
#endif
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
//   Hank Childs, Wed Dec 27 09:51:10 PST 2006
//   Add support for curvilinear mappers.
//
//   Brad Whitlock, Wed Jun 10 11:54:42 PDT 2009
//   Conditionally compile mangled mesa classes.
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
#ifdef VTK_USE_MANGLED_MESA
    else if(strcmp(this->PolyDataMapper->GetClassName(), 
                   "vtkVisItMesaPolyDataMapper") == 0)
      {
        vtkVisItMesaPolyDataMapper *m = 
            (vtkVisItMesaPolyDataMapper *)this->PolyDataMapper;
        m->SetEnableColorTexturing(val);
      }
#endif
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
#ifdef VTK_USE_MANGLED_MESA
    else if(strcmp(this->RectilinearGridMapper->GetClassName(), 
                   "vtkMesaRectilinearGridMapper") == 0)
      {
        vtkMesaRectilinearGridMapper *m = 
            (vtkMesaRectilinearGridMapper *)this->RectilinearGridMapper;
        m->SetEnableColorTexturing(val);
      }
#endif
    }
  if(this->StructuredGridMapper != NULL)
    {
    if(strcmp(this->StructuredGridMapper->GetClassName(),
              "vtkOpenGLStructuredGridMapper") == 0)
      {
        vtkOpenGLStructuredGridMapper *m = 
            (vtkOpenGLStructuredGridMapper *)this->StructuredGridMapper;
        m->SetEnableColorTexturing(val);
      }
#ifdef VTK_USE_MANGLED_MESA
    else if(strcmp(this->StructuredGridMapper->GetClassName(), 
                   "vtkMesaStructuredGridMapper") == 0)
      {
        vtkMesaStructuredGridMapper *m = 
            (vtkMesaStructuredGridMapper *)this->StructuredGridMapper;
        m->SetEnableColorTexturing(val);
      }
#endif
    }
}


// ****************************************************************************
// Method: vtkVisItDataSetMapper::SetSceneIs3D
//
// Purpose: 
//     Sets whether the scene is 3D.
//
// Arguments:
//   val : Whether the scene is 3D.
//
// Programmer: Hank Childs
// Creation:   December 28, 2006
//
// ****************************************************************************

void
vtkVisItDataSetMapper::SetSceneIs3D(bool val)
{
  this->SceneIs3D = val;
  if (this->RectilinearGridMapper != NULL)
     this->RectilinearGridMapper->SetSceneIs3D(val);
  if (this->StructuredGridMapper != NULL)
     this->StructuredGridMapper->SetSceneIs3D(val);
}


