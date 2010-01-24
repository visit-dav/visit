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

#include <math.h>

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
//
//    Jeremy Meredith, Mon Mar 19 14:33:15 EDT 2007
//    Added settings to draw the stem as a cylinder (vs a line), and its
//    width, to increase the number of polygons, and to cap the ends of
//    the cone/cylinder.
// ****************************************************************************

vtkVectorGlyph::vtkVectorGlyph()
{
  LineStem = 1;
  HighQuality = 0;
  CapEnds = 0;
  StemWidth = 0.08;
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
//    Jeremy Meredith, Mon Mar 19 14:33:15 EDT 2007
//    Added settings to draw the stem as a cylinder (vs a line), and its
//    width, to increase the number of polygons, and to cap the ends of
//    the cone/cylinder.
//
//    Hank Childs, Sun Jan 24 10:26:07 PST 2010
//    Fix problem where glyph heads don't show up in 2D.
//
// ****************************************************************************

void vtkVectorGlyph::Execute(void)
{
    vtkPolyData *output = this->GetOutput();

    vtkPoints *pts = vtkPoints::New();
    output->SetPoints(pts);
    pts->Delete();
  
    vtkCellArray *lines = vtkCellArray::New();
    output->SetLines(lines);
    lines->Delete();

    vtkCellArray *polys = vtkCellArray::New();
    output->SetPolys(polys);
    polys->Delete();

    //
    // The vector will be with the bounding box x,y,z=-0.5,0.5.
    // The head will start at (0.5, 0., 0.) and
    // the tail will end at (-0.5,0.,0.)
    // 
    float realHeadSize = MakeHead ? this->HeadSize : 0;
    float endOfHead = 0.5 - realHeadSize;
    float proportion = realHeadSize;
    float cylThickness = this->StemWidth;

    vtkIdType pt[4];

    // Determine the number of points around the glyph cone/cylinder
    // (For a 2D glyph, 2 steps makes a triangle head & rectangle stem.)
    int nSteps = (!ConeHead) ? 2 : (HighQuality ? 16 : 6);

    // Determine the total number of points
    int nPts = 3 + nSteps + (LineStem ? 0 : nSteps*2);
    pts->SetNumberOfPoints(nPts);

    // Add the points along the central axis
    pts->SetPoint(0, OriginOffset + 0.5, 0., 0.);
    pts->SetPoint(1, OriginOffset - 0.5, 0., 0.);
    pts->SetPoint(2, OriginOffset + endOfHead, 0., 0.);

    // Add the points around the base of the cone
    for (int i=0; i<nSteps; i++)
    {
        float theta = 2*M_PI * float(i)/float(nSteps);
        float x = OriginOffset + endOfHead;
        float y = proportion * 0.5 * cos(theta);
        float z;
        if (ConeHead)
            z = proportion * 0.5 * sin(theta); 
        else
            z = 0;
        pts->SetPoint(3 + i, x, y, z);
    }

    // Draw the stem as a line or cylinder(3d)/rectangle(2d)
    if (LineStem)
    {
        lines->InsertNextCell(2);
        lines->InsertCellPoint(0);
        lines->InsertCellPoint(1);
    }
    else
    {
        float startx = OriginOffset - 0.5;
        float endx;
        // If we're in 3D and not capping the cone's base, then extend the
        // cylinder all the way to where it intersects the cone's facets.
        // Otherwise, just stop at the base of the cone.
        if (ConeHead && !CapEnds)
        {
            float len = (realHeadSize<cylThickness)?realHeadSize:cylThickness;
            endx = OriginOffset + .5 - len;
        }
        else
        {
            endx = OriginOffset + endOfHead;
        }

        for (int i=0; i<nSteps; i++)
        {
            float theta = 2*M_PI * float(i)/float(nSteps);
            float y = cylThickness * 0.5 * cos(theta);
            float z = cylThickness * 0.5 * sin(theta); 
            pts->SetPoint(3 + 1*nSteps + i, startx, y, z);
            pts->SetPoint(3 + 2*nSteps + i, endx,   y, z);
        }
        for (int i=0; i<nSteps; i++)
        {
            pt[0] = 3 + 1*nSteps + i;
            pt[1] = 3 + 1*nSteps + (i+1)%nSteps;
            pt[2] = 3 + 2*nSteps + (i+1)%nSteps;
            pt[3] = 3 + 2*nSteps + i;
            polys->InsertNextCell(4, pt);

            // If we're in 2D, no point in capping the end of a rectangle
            // even if "CapEnds" is true.
            if (CapEnds && ConeHead)
            {
                pt[0] = 1;
                pt[1] = 3 + 1*nSteps + i;
                pt[2] = 3 + 1*nSteps + (i+1)%nSteps;
                polys->InsertNextCell(3, pt);
                if (realHeadSize < cylThickness)
                {
                    pt[0] = 2;
                    pt[1] = 3 + 2*nSteps + (i+1)%nSteps;
                    pt[2] = 3 + 2*nSteps + i;
                    polys->InsertNextCell(3, pt);
                }
            }
        }
    }

    // If asked, draw a head as either a cone or a triangle
    if (MakeHead)
    {
        if (ConeHead)
        {
            for (int i=0; i<nSteps; i++)
            {
                pt[0] = 0;
                pt[1] = 3 + i;
                pt[2] = 3 + (i+1)%nSteps;
                polys->InsertNextCell(3, pt);
                if (CapEnds)
                {
                    pt[0] = 2;
                    pt[1] = 3 + (i+1)%nSteps;
                    pt[2] = 3 + i;
                    polys->InsertNextCell(3, pt);
                }
            }
        }
        else
        {
            pt[0] = 0;
            pt[1] = 3;
            pt[2] = 4;
            polys->InsertNextCell(3, pt);
        }
    }
}

  
// ****************************************************************************
//  Modifications:
//    Jeremy Meredith, Mon Mar 19 14:33:15 EDT 2007
//    Added settings to draw the stem as a cylinder (vs a line), and its
//    width, to increase the number of polygons, and to cap the ends of
//    the cone/cylinder.
// ****************************************************************************
void vtkVectorGlyph::PrintSelf(ostream &os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os, indent);
   os << indent << "MakeHead: " << this->MakeHead << "\n";
   os << indent << "ConeHead: " << this->ConeHead << "\n";
   os << indent << "LineStem: " << this->LineStem << "\n";
   os << indent << "StemWidth: " << this->StemWidth << "\n";
   os << indent << "CapEnds: " << this->CapEnds << "\n";
   os << indent << "HighQuality: " << this->HighQuality << "\n";
   os << indent << "Relative Size of Heads: " << this->HeadSize << "\n";
}

