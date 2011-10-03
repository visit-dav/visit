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

#include <vtkPainterPolyDataMapper.h>
#include <vtkChooserPainter.h>
#include <vtkTexturedPointsPainter.h>

#include "vtkOpenGLRectilinearGridMapper.h"
#include "vtkOpenGLStructuredGridMapper.h"

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
//
//   Brad Whitlock, Wed Aug 24 16:24:03 PDT 2011
//   Added texture point painter.
//
// ****************************************************************************

vtkVisItDataSetMapper::vtkVisItDataSetMapper()
{
  this->TexturedPointsPainter = NULL;
  this->RectilinearGridMapper = NULL;
  this->StructuredGridMapper = NULL;
  this->PointTextureMethod = TEXTURE_NO_POINTS;
  this->EnableColorTexturing = false;
  this->SceneIs3D = true;
  this->VertsReplacedWithGeomGlyphs = false;
}


// ****************************************************************************
// Modifications:
//   Dave Bremer, Wed Feb 27 15:59:48 PST 2008
//   Make this class derive from vtkDataSetMapper, letting it
//   delete two members that we had been deleting.
//
//   Brad Whitlock, Wed Aug 24 16:24:03 PDT 2011
//   Added texture point painter.
//
// ****************************************************************************

vtkVisItDataSetMapper::~vtkVisItDataSetMapper()
{
  if(this->TexturedPointsPainter)
    {
    this->TexturedPointsPainter->Delete();
    }
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
//   Brad Whitlock, Wed Aug 24 16:24:38 PDT 2011
//   I added code to create a textured points painter if we create a 
//   painter polydata mapper. We use this to do sphere points.
//
//   Brad Whitlock, Wed Sep 14 15:30:14 PDT 2011
//   Remove setting the painting delegate to make textured points work in SR.
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

    // Install a textured point painter on the polydata mapper.
    vtkPainterPolyDataMapper *ppdm = vtkPainterPolyDataMapper::SafeDownCast(pm);
    if(ppdm != NULL)
    {
        vtkChooserPainter *cp = vtkChooserPainter::SafeDownCast(ppdm->GetPainter()->GetDelegatePainter());
        if(cp != NULL)
        {
           this->TexturedPointsPainter = vtkTexturedPointsPainter::New();
           cp->SetVertPainter(this->TexturedPointsPainter);
        }
    }

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
  bool haveVertices = false;
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
    vtkPolyData *pd = (vtkPolyData *)(this->GetInput());
    haveVertices = (pd->GetNumberOfVerts() > 0);
    this->PolyDataMapper->SetInput(pd);
    }
  else
    {
    this->GeometryExtractor->SetInput(this->GetInput());
    vtkPolyData *pd = this->GeometryExtractor->GetOutput();
    pd->Update();
    haveVertices = (pd->GetNumberOfVerts() > 0);
    this->PolyDataMapper->SetInput(pd);
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
  bool spriteTexturing = this->PointTextureMethod != TEXTURE_NO_POINTS;
  mapper->SetInterpolateScalarsBeforeMapping((spriteTexturing || haveVertices || this->VertsReplacedWithGeomGlyphs)?0:
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
//   Tom Fogal, Tue Apr 27 11:19:53 MDT 2010
//   Remove Mesa-specific coding.
//
//   Brad Whitlock, Wed Aug 24 16:28:03 PDT 2011
//   Rewrote for new VTK.
//
// ****************************************************************************

void
vtkVisItDataSetMapper::SetPointTextureMethod(
    vtkVisItDataSetMapper::PointTextureMode m)
{
    this->PointTextureMethod = m;

    int doIt = (this->PointTextureMethod == TEXTURE_USING_POINTSPRITES)?1:0;
    if(this->TexturedPointsPainter != NULL)
        this->TexturedPointsPainter->SetDoTexturing(doIt);

    // We can't do color texturing if we are also doing point sprites.
    if(doIt > 0)
        SetEnableColorTexturing(false);
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
//   Tom Fogal, Tue Apr 27 11:20:11 MDT 2010
//   Remove Mesa-specific code.
//
//   Brad Whitlock, Tue Aug  9 14:17:40 PDT 2011
//   Rely on SetInterpolateScalarsBeforeMapping.
//
// ****************************************************************************

void
vtkVisItDataSetMapper::SetEnableColorTexturing(bool val)
{
  this->EnableColorTexturing = val;

  if(this->PolyDataMapper != NULL)
    {
       this->PolyDataMapper->SetInterpolateScalarsBeforeMapping(val?1:0);
    }
  if(this->RectilinearGridMapper != NULL)
    {
       this->RectilinearGridMapper->SetInterpolateScalarsBeforeMapping(val?1:0);
    }
  if(this->StructuredGridMapper != NULL)
    {
       this->StructuredGridMapper->SetInterpolateScalarsBeforeMapping(val?1:0);
    }

  // We can't do point sprites if we are color texturing.
  if(val)
      SetPointTextureMethod(TEXTURE_NO_POINTS);
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
