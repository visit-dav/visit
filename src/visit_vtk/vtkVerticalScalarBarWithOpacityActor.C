//=========================================================================
//
//  Class:     vtkVerticalScalarBarWithOpacityActor
//  
//  Purpose:
//    Derived type of vtkVerticalScalarBarActor. 
//    Creates a legend that indicates to the viewer the correspondence between
//    color/opacity values and data value. 
//
//=========================================================================
#include "vtkVerticalScalarBarWithOpacityActor.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkTextMapper.h>
#include <vtkViewport.h>
#include <float.h>

//------------------------------------------------------------------------------
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:10:03 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
//------------------------------------------------------------------------------

vtkStandardNewMacro(vtkVerticalScalarBarWithOpacityActor);

// Instantiate this object
vtkVerticalScalarBarWithOpacityActor::vtkVerticalScalarBarWithOpacityActor() :
    vtkVerticalScalarBarActor()
{
   for(int i = 0; i < 256; ++i)
       opacities[i] = 255;
}

vtkVerticalScalarBarWithOpacityActor::~vtkVerticalScalarBarWithOpacityActor()
{
   // nothing here
}


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001 
//    Make ptIds of type vtkIdType to match VTK 4.0 API. 
//
//    Hank Childs, Tue Feb 26 09:33:01 PST 2002
//    Make sure that we don't reference a pointer to someone else's object --
//    this was causing a "freeing freed memory" which was hanging on Linux.
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray
//    directly to access colors.
//
//    Eric Brugger, Mon Jul 14 12:11:14 PDT 2003
//    I changed the way the color bar is built.
//
//    Eric Brugger, Wed Jul 16 08:54:38 PDT 2003
//    Add logic to reduce the number of colors and labels shown when user
//    defined labels are specified and there are  are too many to fit in
//    the available space.
//
// ****************************************************************************

void vtkVerticalScalarBarWithOpacityActor::BuildColorBar(vtkViewport *viewport)
{
  int *viewSize = viewport->GetSize();

  //
  // Determine the size and position of the color bar
  //
  int halfFontSize = (int)((this->FontHeight * viewSize[1]) / 2.);

  float barOrigin;
  int rsizePixels[2];
  this->RangeMapper->GetSize(viewport, rsizePixels);
  if (this->RangeVisibility)
    barOrigin = (float)(rsizePixels[1] + halfFontSize);
  else
    barOrigin = 0.;

  int *titleOrigin;
  float barHeight, barWidth;

  titleOrigin = this->TitleActor->GetPositionCoordinate()->
                                  GetComputedViewportValue(viewport);

  if (this->TitleOkayToDraw && this->TitleVisibility) 
    {
    barHeight = titleOrigin[1] - LastOrigin[1] - halfFontSize - barOrigin; 
    }
  else
    {
    barHeight = LastSize[1] - barOrigin;
    }

  barWidth = (int) (this->BarWidth * viewSize[0]);

  //
  // Determine the number of colors in the color bar.
  //
  int numColors, numLabels; 
  if ( this->UseDefinedLabels && !this->definedLabels.empty() )
    {
    numColors = this->definedLabels.size();
    if ((this->FontHeight * 1.1 * numColors * viewSize[1]) > barHeight)
        numColors = (int) (barHeight / (this->FontHeight * 1.1 * viewSize[1]));
    numLabels = numColors;
    }
  else
    {
    numColors = this->MaximumNumberOfColors;
    numLabels = this->NumberOfLabels;
    }

  //
  // Build color bar object
  //
  int numPts = 4*(numColors + 1);
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(numPts);
  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(polys->EstimateSize(numColors*3,4)); 
  vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
  colors->SetNumberOfComponents(3);
  colors->SetNumberOfTuples(numColors*3);

  this->ColorBarActor->SetProperty(this->GetProperty());
  this->ColorBar->Initialize();
  this->ColorBar->SetPoints(pts);
  this->ColorBar->SetPolys(polys);
  this->ColorBar->GetCellData()->SetScalars(colors);
  pts->Delete(); polys->Delete(); colors->Delete(); 

  //
  //polygons & cell colors for color bar
  //
  float yval = barOrigin;
  float delta = (float)barHeight/(float)numColors;
  float barWidthDiv2 = barWidth * 0.5;
  float coord[3];
  coord[2] = 0.;
  int pointIndex = 0;
  int colorIndex = 0;
  int nextOpacityIndex = 0;
  float alphaXi = (float(opacities[0]) / 255.) * barWidthDiv2;
  float alphaXnexti = 0;

  for (int i = 0; i < numColors; ++i)
  {
      unsigned char *rgba, *rgb;
      vtkIdType ptIds[4];

      // Compute the next index used for the opacity array.
      nextOpacityIndex = int((float(i+1) / float(numColors - 1)) * 255.);
      if(nextOpacityIndex > 255)
          nextOpacityIndex = 255;

      // Get the RGBA color for the i'th index.
      if (this->UseDefinedLabels && !this->definedLabels.empty() )
      {
          rgba = this->LookupTable->MapValue((float) i);
      }
      else
      {
          rgba = this->LookupTable->MapValue(range[0] + (range[1] - range[0])*
                          ((float)i /(numColors-1.0)));
      }

      // Figure out the points for the opacity part of the legend.
      alphaXnexti = (float(opacities[nextOpacityIndex]) / 255.) * barWidthDiv2;
      coord[0] = 0.;
      coord[1] = yval;
      pts->SetPoint(pointIndex, coord);
      ++pointIndex;

      coord[0] = barWidthDiv2 - alphaXi;
      coord[1] = yval;
      pts->SetPoint(pointIndex, coord);
      ++pointIndex;

      coord[0] = barWidthDiv2;
      coord[1] = yval;
      pts->SetPoint(pointIndex, coord);
      ++pointIndex;

      // Decide which (or both) opacity quads must be added.
      int addLeft = 1, addRight = 1;
      if(alphaXi == barWidthDiv2 && alphaXnexti == barWidthDiv2)
          addLeft = 0;
      else if(alphaXi == 0. && alphaXnexti == 0.)
          addRight = 0;

      // Add the left quad.
      if(addLeft && (i < numColors-1))
      {
          rgb = colors->GetPointer(colorIndex);
          rgb[0] = 0;
          rgb[1] = 0;
          rgb[2] = 0;
          colorIndex += 3;

          ptIds[0] = pointIndex - 3;
          ptIds[1] = ptIds[0] + 1;
          ptIds[2] = ptIds[1] + 4;
          ptIds[3] = ptIds[0] + 4;
          polys->InsertNextCell(4,ptIds);
      }

      // Add the right quad.
      if(addRight && (i < numColors-1))
      {
          rgb = colors->GetPointer(colorIndex);
          rgb[0] = 200;
          rgb[1] = 200;
          rgb[2] = 200;
          colorIndex += 3;

          ptIds[0] = pointIndex - 2;
          ptIds[1] = ptIds[0] + 1;
          ptIds[2] = ptIds[1] + 4;
          ptIds[3] = ptIds[0] + 4;
          polys->InsertNextCell(4,ptIds);
      }

      // Add the coordinate for the color part of the legend.
      coord[0] = barWidth;
      coord[1] = yval;
      pts->SetPoint(pointIndex, coord);
      ++pointIndex;

      // Add the polygon.
      if(i < numColors-1)
      {
          rgb = colors->GetPointer(colorIndex); //write into array directly
          rgb[0] = rgba[0];
          rgb[1] = rgba[1];
          rgb[2] = rgba[2];
          colorIndex += 3;

          ptIds[0] = pointIndex - 2;
          ptIds[1] = ptIds[0] + 1;
          ptIds[2] = ptIds[1] + 4;
          ptIds[3] = ptIds[0] + 4;
          polys->InsertNextCell(4,ptIds);
      }

      // Move to the next Y-value.
      yval += delta;
      alphaXi = alphaXnexti;

  } // loop on numColors

  this->BuildTics(barOrigin, barWidth, barHeight, numLabels);
  if (this->LabelVisibility)
    {
    this->BuildLabels(viewport, barOrigin, barWidth, barHeight, numLabels);
    }

} // BuildColorBar

// ****************************************************************************
// Method: vtkVerticalScalarBarWithOpacityActor::SetLegendOpacities
//
// Purpose: 
//   Sets the opacities used for drawing the legend.
//
// Arguments:
//   opacity : An array of 256 opacity values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 20 12:24:33 PDT 2001
//
// Modifications:
//   
//   Hank Childs, Wed Aug 15 09:56:09 PDT 2001
//   Added a Modified to make sure the legend gets redrawn.
//
// ****************************************************************************

void
vtkVerticalScalarBarWithOpacityActor::SetLegendOpacities(
    const unsigned char *opacity)
{
    for(int i = 0; i < 256; ++i)
        opacities[i] = opacity[i];
    Modified();
}
