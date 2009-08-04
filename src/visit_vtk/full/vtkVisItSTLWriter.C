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

// TAKEN FROM THE 7/25 VTK SOURCE
//
// Modifications:
//   Jeremy Meredith, Wed Jul 31 17:40:32 PDT 2002
//   Renamed to vtkVisItSTLWriter.
//   Made the number of cells in the binary writer
//   be an int instead of a long.  It was trying to write
//   4 bytes of a long, which failed on 64-bit platforms
//   (e.g. wrote all zeros).

#include "vtkVisItSTLWriter.h"

#include <vtkByteSwap.h>
#include <vtkCellArray.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkTriangle.h>

vtkCxxRevisionMacro(vtkVisItSTLWriter, "$Revision: 1.45 $");
vtkStandardNewMacro(vtkVisItSTLWriter);

vtkVisItSTLWriter::vtkVisItSTLWriter()
{
  this->FileType = VTK_ASCII;
}

void vtkVisItSTLWriter::WriteData()
{
  vtkPoints *pts;
  vtkCellArray *polys;
  vtkPolyData *input = this->GetInput();

  polys = input->GetPolys();
  pts = input->GetPoints();
  if (pts == NULL || polys == NULL )
    {
    vtkErrorMacro(<<"No data to write!");
    return;
    }

  if ( this->FileName == NULL)
    {
    vtkErrorMacro(<< "Please specify FileName to write");
    return;
    }

  if ( this->FileType == VTK_BINARY )
    {
    this->WriteBinarySTL(pts,polys);
    }
  else
    {
    this->WriteAsciiSTL(pts,polys);
    }
}

static char header[]="Visualization Toolkit generated SLA File                                        ";

void vtkVisItSTLWriter::WriteAsciiSTL(vtkPoints *pts, vtkCellArray *polys)
{
  FILE *fp;
  double n[3], v1[3], v2[3], v3[3];
  vtkIdType npts = 0;
  vtkIdType *indx = 0;
  
  if ((fp = fopen(this->FileName, "w")) == NULL)
    {
    vtkErrorMacro(<< "Couldn't open file: " << this->FileName);
    return;
    }
//
//  Write header
//
  vtkDebugMacro("Writing ASCII sla file");
  fprintf (fp, "solid ascii\n");
//
//  Write out triangle polygons.  In not a triangle polygon, only first 
//  three vertices are written.
//
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {
    pts->GetPoint(indx[0], v1);
    pts->GetPoint(indx[1], v2);
    pts->GetPoint(indx[2], v3);

    vtkTriangle::ComputeNormal(pts, npts, indx, n);

    fprintf (fp, " facet normal %.6g %.6g %.6g\n  outer loop\n",
            n[0], n[1], n[2]);

    fprintf (fp, "   vertex %.6g %.6g %.6g\n", v1[0], v1[1], v1[2]);
    fprintf (fp, "   vertex %.6g %.6g %.6g\n", v2[0], v2[1], v2[2]);
    fprintf (fp, "   vertex %.6g %.6g %.6g\n", v3[0], v3[1], v3[2]);

    fprintf (fp, "  endloop\n endfacet\n");
    }
  fprintf (fp, "endsolid\n");
  fclose (fp);
}

void vtkVisItSTLWriter::WriteBinarySTL(vtkPoints *pts, vtkCellArray *polys)
{
  FILE *fp;
  double dn[3], v1[3], v2[3], v3[3];
  vtkIdType npts = 0;
  vtkIdType *indx = 0;
  int ncells;
  unsigned short ibuff2=0;

  if ((fp = fopen(this->FileName, "wb")) == NULL)
    {
    vtkErrorMacro(<< "Couldn't open file: " << this->FileName);
    return;
    }
  
  //  Write header
  //
  vtkDebugMacro("Writing Binary STL file");
  fwrite (header, 1, 80, fp);

  ncells = (int) polys->GetNumberOfCells();
  vtkByteSwap::Swap4LE(&ncells);
  fwrite (&ncells, 1, 4, fp);

  //  Write out triangle polygons.  In not a triangle polygon, only first 
  //  three vertices are written.
  //
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {
    pts->GetPoint(indx[0], v1);
    pts->GetPoint(indx[1], v2);
    pts->GetPoint(indx[2], v3);

    vtkTriangle::ComputeNormal(pts, npts, indx, dn);
    float n[3] = {(float)dn[0], (float)dn[1], (float)dn[2]};
    vtkByteSwap::Swap4LE(n); 
    vtkByteSwap::Swap4LE(n+1); 
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = (float)v1[0];  n[1] = (float)v1[1];  n[2] = (float)v1[2]; 
    vtkByteSwap::Swap4LE(n); 
    vtkByteSwap::Swap4LE(n+1); 
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = (float)v2[0];  n[1] = (float)v2[1];  n[2] = (float)v2[2]; 
    vtkByteSwap::Swap4LE(n); 
    vtkByteSwap::Swap4LE(n+1); 
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = (float)v3[0];  n[1] = (float)v3[1];  n[2] = (float)v3[2]; 
    vtkByteSwap::Swap4LE(n); 
    vtkByteSwap::Swap4LE(n+1); 
    vtkByteSwap::Swap4LE(n+2);
    fwrite (n, 4, 3, fp);

    fwrite (&ibuff2, 2, 1, fp);
    }
  fclose (fp);
}

