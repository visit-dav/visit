/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkBackgroundActor.h,v $
  Language:  C++
  Date:      $Date: 2000/07/11 18:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Brad Whitlock, B Division, Lawrence Livermore Nat'l Laboratory

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

// .NAME vtkBackgroundActor Create a gradient colored background.
// .SECTION Description
// vtkBackgroundActor draws a colored gradient background polygon over the
// entire viewport. This is a cheap special effect that makes images look nice.
// The colors of the gradient can be set for each of the corners of the image
// and the gradient pattern can also be set to radiate from the center of
// the viewport.
//
// .SECTION See Also
// vtkActor2D

#ifndef __vtkBackgroundActor_h
#define __vtkBackgroundActor_h
#include <plotter_exports.h>

#include "vtkActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include "vtkActor2D.h"
#include "vtkTimeStamp.h"

class PLOTTER_API vtkBackgroundActor : public vtkActor2D
{
public:
  vtkTypeMacro(vtkBackgroundActor, vtkActor2D);
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Instantiate a background actor. 
  static vtkBackgroundActor *New();

  // Description:
  // Draw the actor as per the vtkProp superclass' API.
  int RenderOverlay(vtkViewport *);
  int RenderOpaqueGeometry(vtkViewport *);
  int RenderTranslucentGeometry(vtkViewport *) { return 0;};

  // Description:
  // Sets the corner colors.
  void SetColors(const double colors[4][3]);
  void SetColor(int id, const double color[3]);

  // Description:
  // Sets the gradient fill mode. A value of 0 is top to bottom and a value
  // of 1 is radial.
  void SetGradientFillMode(int mode);
  vtkGetMacro(GradientFillMode, int);

  // Description:
  // Sets the number of radial rings used when doing the radial gradient mode.
  void SetNumRings(int rings);
  vtkGetMacro(NumRings, int);

  // Description:
  // Sets the number of radial steps used to do the radial gradient.
  void SetNumRadialSteps(int steps);
  vtkGetMacro(NumRadialSteps, int);
protected:
  vtkBackgroundActor();
  ~vtkBackgroundActor();

  void DestroyBackground();
  void CreateBackground();

  int                 GradientFillMode;
  double               GradientCoords[4];
  double               GradientColors[4][3];
  int                 NumRings;
  int                 NumRadialSteps;
  vtkPolyData         *GradientData;
  vtkActor2D          *GradientActor;
  vtkPolyDataMapper2D *GradientMapper;

private:
  vtkBackgroundActor(const vtkBackgroundActor&); 
  void operator=(const vtkBackgroundActor&);
};


#endif
