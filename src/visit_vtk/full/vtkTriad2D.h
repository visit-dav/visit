/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkTriad2D.h,v $
  Language:  C++
  Date:      $Date: 2000/07/11 18:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Hank Childs, B Division, Lawrence Livermore Nat'l Laboratory

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

// .NAME vtkTriad2D Create a triad showing the x-y-z axes for the current view.
// .SECTION Description
// vtkTriad2D draws the x, y, and z unit vectors on the screen and updates them
// every time the camera changes.
// The triad's size stays constant despite changes in the camera location and
// is overlayed over all 3D actors.
// You may set the location of the origin of the triad through SetOrigin and
// may set the length of the unit vectors with SetAxisLength.
// You may also set the properties of the axes by getting each access and
// setting the properties through its (vtkVisItAxisActor2D) methods.
//
// .SECTION See Also
// vtkActor2D vtkVisItAxisActor2D

#ifndef __vtkTriad2D_h
#define __vtkTriad2D_h
#include <visit_vtk_exports.h>

#include "vtkActor2D.h"
#include "vtkVisItAxisActor2D.h"
#include "vtkCamera.h"


class VISIT_VTK_API vtkTriad2D : public vtkActor2D
{
public:
  vtkTypeMacro(vtkTriad2D, vtkActor2D);
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Instantiate a triad with origin at (0.1, 0.1) in normalized viewport 
  // coordinates, with a unit vector length of 0.8.  
  static vtkTriad2D *New();

  // Description:
  // Draw the triad as per the vtkProp superclass' API.
  int RenderOverlay(vtkViewport *);
  int RenderOpaqueGeometry(vtkViewport *);
  int RenderTranslucentGeometry(vtkViewport *) { return 0;};

  // Description:
  // Set the origin of the triad in normalized viewport coordinates.
  void SetOrigin(float new_origin[2]);
  void SetOrigin(float origin_x, float origin_y);
  vtkGetVector2Macro(Origin, float);

  // Description:
  // Set the length of the unit vector in normalized viewport coordinates.
  vtkSetClampMacro(AxisLength, float, 0., 1.);
  vtkGetMacro(AxisLength, float);

  // Description:
  // Set/Get the camera to perform scaling and translation of the
  // vtkCubeAxesActor2D.
  virtual void SetCamera(vtkCamera*);
  vtkGetObjectMacro(Camera,vtkCamera);

  // Description:
  // Allow for axes to be changed directly - changing colors and such.  
  vtkGetObjectMacro(XAxis, vtkVisItAxisActor2D);
  vtkGetObjectMacro(YAxis, vtkVisItAxisActor2D);
  vtkGetObjectMacro(ZAxis, vtkVisItAxisActor2D);

protected:
  vtkTriad2D();
  ~vtkTriad2D();

  vtkCamera       *Camera;

  float            Origin[2];
  float            AxisLength;

  vtkVisItAxisActor2D  *XAxis, *YAxis, *ZAxis;

  void              TranslateVectorToViewport(const float world_vector[4], 
                                              float vport[2]);

private:
  vtkTriad2D(const vtkTriad2D&);
  void operator=(const vtkTriad2D&);
};


#endif
