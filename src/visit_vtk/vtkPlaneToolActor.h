/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPlaneToolActor.h,v $
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

// .NAME vtkPlaneToolActor Create a plane with axes.
// .SECTION Description
// vtkPlaneToolActor draws a plane with axes and is intended to be used to 
// represent the location of slice planes.
//
// .SECTION See Also
// vtkActor

#ifndef __vtkPlaneToolActor_h
#define __vtkPlaneToolActor_h
#include <visit_vtk_exports.h>

#include "vtkAssembly.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkActor.h"

class vtkArrowActor;

class VISIT_VTK_API vtkPlaneToolActor : public vtkActor
{
public:
  vtkTypeMacro(vtkPlaneToolActor, vtkAssembly);
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Instantiate a background actor. 
  static vtkPlaneToolActor *New();

  // Description:
  // Draw the actor as per the vtkProp superclass' API.
  int RenderOpaqueGeometry(vtkViewport *);
  void ReleaseGraphicsResources(vtkWindow *);

  void SetLineColor(const float c[3]);

  void AddPosition(float delta[3]);

protected:
  vtkPlaneToolActor();
  ~vtkPlaneToolActor();

  void CreatePlane();
  void DestroyPlane();
  void CreateVectors();
  void CreateVectorData();
  void DestroyVectors();
  void CopyTransform(vtkActor *actor);
  void UpdateActorTransform();

  float              LineColor[3];

  vtkPolyData       *PlaneData;
  vtkActor          *PlaneActor;
  vtkPolyDataMapper *PlaneMapper;

  vtkPolyData       *ArrowData;
  vtkPolyDataMapper *ArrowMapper1;
  vtkPolyDataMapper *ArrowMapper2;
  vtkPolyDataMapper *ArrowMapper3;
  vtkActor          *XAxisActor;
  vtkActor          *YAxisActor;
  vtkActor          *ZAxisActor;

private:
  vtkPlaneToolActor(const vtkPlaneToolActor&);
  void operator=(const vtkPlaneToolActor&);
};


#endif
