/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItScalarBarWithOpacityActor.h,v $
  Language:  C++
  Date:      $Date: 2000/11/03 14:10:27 $
  Version:   $Revision: 1.28 $

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
// .NAME vtkVisItScalarBarWithOpacityActor - Create a scalar bar with labels, title and 
// range
// .SECTION Description
// vtkVisItScalarBarWithOpacityActor creates a scalar bar with annotation text. A scalar
// bar is a legend that indicates to the viewer the correspondence between
// color value and data value. The legend consists of a rectangular bar 
// made of rectangular pieces each colored a constant value. Since 
// vtkVisItScalarBarWithOpacityActor is a subclass of vtkActor2D, it is drawn in the 
// image plane (i.e., in the renderer's viewport) on top of the 3D graphics 
// window.
//
// To use vtkVisItScalarBarWithOpacityActor you must associate a vtkScalarsToColors (or
// subclass) with it. The lookup table defines the colors and the
// range of scalar values used to map scalar data.  Typically, the
// number of colors shown in the scalar bar is not equal to the number
// of colors in the lookup table, in which case sampling of
// the lookup table is performed. 
//
// Other optional capabilities include specifying the fraction of the
// viewport size (both x and y directions) which will control the size
// of the scalar bar, the number of annotation labels, and the font
// attributes of the annotation text. The actual position of the
// scalar bar on the screen is controlled by using the
// vtkActor2D::SetPosition() method (by default the scalar bar is
// position on the right side of the viewport).  Other features include 
// the ability control the format (print style) with which to print the 
// labels on the scalar bar. Also, the vtkVisItScalarBarWithOpacityActor's property 
// is applied to the scalar bar and annotation (including color, layer, and
// compositing operator).  

// .SECTION See Also
// vtkActor2D vtkTextMapper vtkPolyDataMapper2D vtkVisItScalarBarActor

#ifndef __vtkVisItScalarBarWithOpacityActor_h
#define __vtkVisItScalarBarWithOpacityActor_h
#include <rendering_visit_vtk_exports.h>
#include <vtkVisItScalarBarActor.h>

class RENDERING_VISIT_VTK_API vtkVisItScalarBarWithOpacityActor : public vtkVisItScalarBarActor
{
public:
  vtkTypeMacro(vtkVisItScalarBarWithOpacityActor,vtkVisItScalarBarActor);

  // Description:
  // Instantiate object. 
  static vtkVisItScalarBarWithOpacityActor *New();

  void SetLegendOpacities(const unsigned char *opacity);

protected:
  vtkVisItScalarBarWithOpacityActor();
  virtual ~vtkVisItScalarBarWithOpacityActor();

  virtual void BuildColorBar(vtkViewport *);

  unsigned char opacities[256];

private:
  vtkVisItScalarBarWithOpacityActor(const vtkVisItScalarBarWithOpacityActor&);
  void operator=(const vtkVisItScalarBarWithOpacityActor&);
};

#endif
