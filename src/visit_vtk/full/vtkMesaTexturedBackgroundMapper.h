/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#ifndef VTK_MESA_TEXTURED_BACKGROUND_MAPPER_H
#define VTK_MESA_TEXTURED_BACKGROUND_MAPPER_H
#include <vtkMapper2D.h>

class vtkTexture;
class vtkRenderer;

// ****************************************************************************
// Class: vtkMesaTexturedBackgroundMapper
//
// Purpose:
//   Mesa version of the textured background mapper class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   
//
// Modifications:
//
// ****************************************************************************

// Not exposed in DLL on purpose
class vtkMesaTexturedBackgroundMapper : public vtkMapper2D
{
public:
  static vtkMesaTexturedBackgroundMapper *New();

  vtkTypeMacro(vtkMesaTexturedBackgroundMapper, vtkMapper2D);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  virtual void RenderOpaqueGeometry(vtkViewport *, vtkActor2D *);

  void SetTextureAndRenderers(vtkTexture *t, vtkRenderer *bg, vtkRenderer *canvas);
  void SetSphereMode(bool);
  void SetImageRepetitions(int,int);
protected:
   vtkMesaTexturedBackgroundMapper();
  ~vtkMesaTexturedBackgroundMapper();
private:
  void DrawSphere(int X_RES, int Y_RES, float radius, const double *);
  void DrawImageSphere();
  void DrawImageFlat();

  vtkTexture  *tex;
  vtkRenderer *background;
  vtkRenderer *canvas;
  bool         sphereMode;
  int          imageRepeatX;
  int          imageRepeatY;
};

#endif
