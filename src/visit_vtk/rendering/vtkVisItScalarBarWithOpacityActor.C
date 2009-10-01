/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

//=========================================================================
//
//  Class:     vtkVisItScalarBarWithOpacityActor
//  
//  Purpose:
//    Derived type of vtkVisItScalarBarActor. 
//    Creates a legend that indicates to the viewer the correspondence between
//    color/opacity values and data value. 
//
//=========================================================================
#include "vtkVisItScalarBarWithOpacityActor.h"

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

vtkStandardNewMacro(vtkVisItScalarBarWithOpacityActor);

// Instantiate this object
vtkVisItScalarBarWithOpacityActor::vtkVisItScalarBarWithOpacityActor() :
    vtkVisItScalarBarActor()
{
   for(int i = 0; i < 256; ++i)
       opacities[i] = 255;
}

vtkVisItScalarBarWithOpacityActor::~vtkVisItScalarBarWithOpacityActor()
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
//    Dave Bremer, Mon Oct 13 12:36:09 PDT 2008
//    I added support for horizontal color bars, and fixed a bug that 
//    made the bar a little short.
//
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Updated to handle vertical, text on left or right, and horizontal,
//    text on top or bottom.
// ****************************************************************************

void vtkVisItScalarBarWithOpacityActor::BuildColorBar(vtkViewport *viewport)
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

  if (Orientation == VERTICAL_TEXT_ON_RIGHT || 
      Orientation == VERTICAL_TEXT_ON_LEFT)
    {
    barWidth = (int) (this->BarWidth * viewSize[0]);
    }
  else
    {
    //Subtract space for the tick marks and text, if horizontal.
    int tickSpace = (int)(2*halfFontSize + 0.2*(barHeight-2*halfFontSize));
    barHeight -= tickSpace;
    if (Orientation == HORIZONTAL_TEXT_ON_BOTTOM)
      barOrigin += tickSpace;
    barWidth = this->LastSize[0];
    }

  //
  // Determine the number of colors in the color bar.
  //
  int numColors, numLabels; 
  if ( this->Type == VTK_DISCRETE && !this->definedLabels.empty() )
    {
    numColors = this->definedLabels.size();
    if (Orientation == VERTICAL_TEXT_ON_RIGHT || 
        Orientation == VERTICAL_TEXT_ON_LEFT)
      {
      if ((this->FontHeight * 1.1 * numColors * viewSize[1]) > barHeight)
          numColors = (int) (barHeight / (this->FontHeight * 1.1 * viewSize[1]));
      }
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
  float xval = 0, yval = barOrigin;
  float delta;
  float alphaXi;
  float barWidthDiv2 = barWidth * 0.5;
  float barHeightDiv2 = barHeight * 0.5;
  float testVal;
  if (Orientation == VERTICAL_TEXT_ON_RIGHT || 
      Orientation == VERTICAL_TEXT_ON_LEFT)
  {
      if (numColors > 1)
          delta = (float)barHeight/(float)(numColors-1);
      else
          delta = (float)barHeight;
      alphaXi = (float(opacities[0]) / 255.) * barWidthDiv2;
      testVal = barWidthDiv2;
  }
  else
  {
      if (numColors > 1)
          delta = (float)barWidth/(float)(numColors-1);
      else
          delta = (float)barWidth;
      alphaXi = (float(opacities[0]) / 255.) * barHeightDiv2;
      testVal = barHeightDiv2;
  }
  float coord[3];
  coord[2] = 0.;
  int pointIndex = 0;
  int colorIndex = 0;
  int nextOpacityIndex = 0;
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
      if (this->Type == VTK_DISCRETE && !this->definedLabels.empty() )
      {
          rgba = this->LookupTable->MapValue((float) i);
      }
      else
      {
          rgba = this->LookupTable->MapValue(range[0] + (range[1] - range[0])*
                          ((float)i /(numColors-1.0)));
      }

      // Figure out the points for the opacity part of the legend.
      if (Orientation == VERTICAL_TEXT_ON_RIGHT || 
          Orientation == VERTICAL_TEXT_ON_LEFT)
      {
          double hOffset = 0;
          if (Orientation == VERTICAL_TEXT_ON_LEFT)
              hOffset = barWidth;

          alphaXnexti = (float(opacities[nextOpacityIndex]) / 255.) * barWidthDiv2;

          coord[0] = hOffset;
          coord[1] = yval;
          pts->SetPoint(pointIndex, coord);
          ++pointIndex;
    
          coord[0] = hOffset + barWidthDiv2 - alphaXi;
          coord[1] = yval;
          pts->SetPoint(pointIndex, coord);
          ++pointIndex;
    
          coord[0] = hOffset + barWidthDiv2;
          coord[1] = yval;
          pts->SetPoint(pointIndex, coord);
          ++pointIndex;
      }
      else
      {
          alphaXnexti = (float(opacities[nextOpacityIndex]) / 255.) * barHeightDiv2;

          coord[0] = xval;
          coord[1] = barOrigin + barHeight;
          pts->SetPoint(pointIndex, coord);
          ++pointIndex;
    
          coord[0] = xval;
          coord[1] = barOrigin + barHeightDiv2 + alphaXi;
          pts->SetPoint(pointIndex, coord);
          ++pointIndex;
    
          coord[0] = xval;
          coord[1] = barOrigin + barHeightDiv2;
          pts->SetPoint(pointIndex, coord);
          ++pointIndex;
      }

      // Decide which (or both) opacity quads must be added.
      int addLeft = 1, addRight = 1;
      if(alphaXi == testVal && alphaXnexti == testVal)
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
      if (Orientation == VERTICAL_TEXT_ON_RIGHT)
      {
          coord[0] = barWidth;
          coord[1] = yval;
      }
      else if (Orientation == VERTICAL_TEXT_ON_LEFT)
      {
          coord[0] = barWidth*2;
          coord[1] = yval;
      }
      else
      {
          coord[0] = xval;
          coord[1] = barOrigin;
      }
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

      // Move to the next Y-value or X-value.
      xval += delta;
      yval += delta;
      alphaXi = alphaXnexti;

  } // loop on numColors

  this->BuildTics(barOrigin, barWidth, barHeight, numLabels);
  if (this->DrawMode != DRAW_NO_LABELS)
    {
    this->BuildLabels(viewport, barOrigin, barWidth, barHeight, numLabels);
    }

} // BuildColorBar

// ****************************************************************************
// Method: vtkVisItScalarBarWithOpacityActor::SetLegendOpacities
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
vtkVisItScalarBarWithOpacityActor::SetLegendOpacities(
    const unsigned char *opacity)
{
    for(int i = 0; i < 256; ++i)
        opacities[i] = opacity[i];
    Modified();
}
