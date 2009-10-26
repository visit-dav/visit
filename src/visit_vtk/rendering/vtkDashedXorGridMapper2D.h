/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDashedXorGridMapper2D.h,v $
  Language:  C++
  Date:      $Date: 2000/02/04 17:09:14 $
  Version:   $Revision: 1.11 $

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkDashedXorGridMapper2D 
// .SECTION Description
// Based upon vtkRubberBandMapper2D. Draws horizontal and vertical dashed
// lines. For a given line, pixels are placed on a 1D grid to determine
// where the dashes are. Thus, drawing a line from [0, 10], [10, 20] has
// the same effect as drawing from [0, 20], or [20, 0].
// When drawing a single pixel, use SetHorizontalBias to say whether you
// want to use the horizontal or vertical grid.

// .SECTION See Also
// vtkPolyDataMapper2D

#ifndef __vtkDashedXorGridMapper2D_h
#define __vtkDashedXorGridMapper2D_h
#include <rendering_visit_vtk_exports.h>

#include "vtkPolyDataMapper2D.h"

#ifdef VTK_USE_COCOA
class QLabel;
#else
struct vtkRubberBandMapper2DOverlay;
#endif

struct vtkDashedXorGridMapper2DOverlay;

class RENDERING_VISIT_VTK_API vtkDashedXorGridMapper2D : public vtkPolyDataMapper2D
{
public:
  vtkTypeMacro(vtkDashedXorGridMapper2D,vtkPolyDataMapper2D);
  static vtkDashedXorGridMapper2D *New();

  // Description:
  // Actually draw the poly data.
  void RenderOverlay(vtkViewport* viewport, vtkActor2D* actor);

  // Description:
  // Release graphics resources.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Sets length of dashes and spaces. Call before first use.
  void SetDots(int drawn, int spaced);
 
  // Description:
  // In the case of drawing a point, it will use the horizontal grid
  // if the hb is set, otherwise it will use the vertical grid.
  void SetHorizontalBias(bool hb)
  { horizontalBias = hb;    }
  
protected:
  int pixelDrawn, pixelSpaced;
  bool horizontalBias;

#ifdef VTK_USE_COCOA
  QLabel *overlay;
#else
  vtkDashedXorGridMapper2DOverlay *overlay;
#endif

  vtkDashedXorGridMapper2D();
  ~vtkDashedXorGridMapper2D();

  // Returns whether a point in is on a dashed line.
  bool IsDash(int x)
  {
    return ((x % (pixelDrawn + pixelSpaced)) <= pixelDrawn);
  }

  bool IsBeginningDash(int x)
  {
    return (x % (pixelDrawn + pixelSpaced) == 0);
  }
  
  // Moves forward to the next dash from a given point. Point does not
  // have to be on a dash.
  int NextDash(int x)
  {
    return (x + pixelDrawn + pixelSpaced - (x % (pixelDrawn + pixelSpaced)));
  }
  
private:
  vtkDashedXorGridMapper2D(const vtkDashedXorGridMapper2D&);
  void operator=(const vtkDashedXorGridMapper2D&);
  
};


#endif

