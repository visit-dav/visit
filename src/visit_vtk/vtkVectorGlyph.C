/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVectorGlyph.cxx,v $
  Language:  C++
  Date:      $Date: 2001/03/20 14:10:58 $
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

#include "vtkVectorGlyph.h"

#include <vtkCellArray.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:10:03 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API
// ****************************************************************************

vtkStandardNewMacro(vtkVectorGlyph);

// ****************************************************************************
//  Modifications:
//    Jeremy Meredith, Fri Nov 21 12:31:16 PST 2003
//    Added origin offset to the x position.  This lets the glyphs originate
//    or terminate at the nodes (instead of always being centered on them).
// ****************************************************************************

vtkVectorGlyph::vtkVectorGlyph()
{
  MakeHead = 1;
  HeadSize = 0.25;
  ConeHead = 1;
  OriginOffset = 0.;
}


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make pt of type vtkIdType to match VTK 4.0 API.
//
//    Jeremy Meredith, Fri Nov 21 12:31:16 PST 2003
//    Added origin offset to the x position.  This lets the glyphs originate
//    or terminate at the nodes (instead of always being centered on them).
//
// ****************************************************************************

void vtkVectorGlyph::Execute(void)
{
  vtkPolyData *output = this->GetOutput();

  vtkPoints *pts = vtkPoints::New();

  //
  // The vector will be with the bounding box x,y,z=-0.5,0.5.
  // The head will start at (0.5, 0., 0.) and the tail will end at (-0.5,0.,0.)
  // 
  float endOfHead = 0.5 - this->HeadSize;
  float proportion = this->HeadSize;

  // Add all the points.  If the head is turned off they just won't get used.
  // Pt 0 is the tip of the head.  Pt 7 is the end of the tail.  All of the
  // other points are along the rim of the head.
  pts->SetNumberOfPoints(8);
  pts->SetPoint(0, OriginOffset + 0.5, 0., 0.);
  pts->SetPoint(1, OriginOffset + endOfHead, 0.5*proportion, 0.*proportion);
  pts->SetPoint(2, OriginOffset + endOfHead, 0.25*proportion, 0.433013*proportion);
  pts->SetPoint(3, OriginOffset + endOfHead, -0.25*proportion, 0.433013*proportion);
  pts->SetPoint(4, OriginOffset + endOfHead, -0.5*proportion, 0.*proportion);
  pts->SetPoint(5, OriginOffset + endOfHead, -0.25*proportion, -0.433013*proportion);
  pts->SetPoint(6, OriginOffset + endOfHead, 0.25*proportion, -0.433013*proportion);
  pts->SetPoint(7, OriginOffset + -0.5, 0., 0.);

  output->SetPoints(pts);
  pts->Delete();

  vtkCellArray *lines = vtkCellArray::New();
  lines->InsertNextCell(2);
  lines->InsertCellPoint(0);
  lines->InsertCellPoint(7);
  output->SetLines(lines);
  lines->Delete();

  if (this->MakeHead)
    {
    vtkCellArray *polys = vtkCellArray::New();
    vtkIdType pt[3];

    pt[0] = 0;
    if (this->ConeHead)
      {
      for (int i = 0 ; i < 6 ; i++)
        {
        pt[1] = i+1;
        pt[2] = (i+1)%6+1;
        polys->InsertNextCell(3, pt);
        }
      }
    else
      {
      // Make a flat head.
      pt[1] = 1;
      pt[2] = 4;
      polys->InsertNextCell(3, pt);
      }
      output->SetPolys(polys);
      polys->Delete();
    }
}

  
void vtkVectorGlyph::PrintSelf(ostream &os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os, indent);
   os << indent << "MakeHead: " << this->MakeHead << "\n";
   os << indent << "ConeHead: " << this->ConeHead << "\n";
   os << indent << "Relative Size of Heads: " << this->HeadSize << "\n";
}

