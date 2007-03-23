/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVectorGlyph.h,v $
  Language:  C++
  Date:      $Date: 2001/03/21 14:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Hank Childs, B Division, Lawrence Livermore Nat'l Laboratory

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen
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

// .NAME vtkVectorGlyph -- Creates the appropriate glyph for a vector.
//
// .SECTION Description
// Creates the poly data to be used by a glyph filter for vector data.  This
// is a line with a head.  The head is not capped.  The size of the head may
// be set and the head may be turned off completely.
//

#ifndef __vtkVectorGlyph_h
#define __vtkVectorGlyph_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataSource.h"

//  Modifications:
//    Jeremy Meredith, Fri Nov 21 11:25:27 PST 2003
//    Added offset to allow glyphs to originate/terminate at the point.
//
//    Jeremy Meredith, Mon Mar 19 14:33:15 EDT 2007
//    Added "HighQuality", which ups the number of facets, "LineStem"
//    which determines if the stem is a line or a cylinder(3d)/rectangle(2d),
//    "CapEnds" which caps the cone's base (and the cylinder's if
//    we're in 3D, i.e. if ConeHead is true), and the "StemWidth" which
//    defines the width of the stem if we're drawing is as a cyl/rect.
//


class VISIT_VTK_API vtkVectorGlyph : public vtkPolyDataSource
{
public:
  vtkTypeMacro(vtkVectorGlyph, vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(HighQuality,int);
  vtkBooleanMacro(HighQuality,int);
  vtkGetMacro(HighQuality,int);

  vtkSetMacro(CapEnds,int);
  vtkBooleanMacro(CapEnds,int);
  vtkGetMacro(CapEnds,int);

  vtkSetMacro(LineStem,int);
  vtkBooleanMacro(LineStem,int);
  vtkGetMacro(LineStem,int);

  vtkSetClampMacro(StemWidth,float,0.0,0.5);
  vtkGetMacro(StemWidth,float);

  vtkSetMacro(MakeHead,int);
  vtkBooleanMacro(MakeHead,int);
  vtkGetMacro(MakeHead,int);

  vtkSetClampMacro(HeadSize,float,0.0,1.0);
  vtkGetMacro(HeadSize,float);

  vtkSetClampMacro(OriginOffset,float,-.5,+.5);
  vtkGetMacro(OriginOffset,float);

  vtkSetMacro(ConeHead,int);
  vtkGetMacro(ConeHead,int);

  // Description:
  // Instantiate a stride filter that throws away nine of every ten elements.
  static vtkVectorGlyph *New();

protected:
  vtkVectorGlyph();
  ~vtkVectorGlyph() {};

  void Execute();

  int HighQuality;
  int CapEnds;
  int LineStem;
  float StemWidth;
  int ConeHead;
  int MakeHead;
  float HeadSize;
  float OriginOffset;

private:
  vtkVectorGlyph(const vtkVectorGlyph&);
  void operator=(const vtkVectorGlyph&);
};

#endif


