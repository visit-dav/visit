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

// Heavily borrowed from vtkOBJExporter.
//
// Written by Hank Childs, May 27, 2002.

#include "vtkOBJWriter.h"
#include <vtkAssemblyNode.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkGeometryFilter.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

vtkCxxRevisionMacro(vtkOBJWriter, "$Revision: 1.42 $");
vtkStandardNewMacro(vtkOBJWriter);

vtkOBJWriter::vtkOBJWriter()
{
  this->Label = NULL;
}

vtkOBJWriter::~vtkOBJWriter()
{
  if (this->Label != NULL)
    {
    delete [] this->Label;
    }
}

void vtkOBJWriter::WriteData()
{
  FILE *fpObj;
  int idStart = 1;
  vtkPolyData *pd = this->GetInput();
  vtkPointData *pntData;
  vtkPoints *points = NULL;
  vtkDataArray *normals = NULL;
  vtkDataArray *tcoords = NULL;
  int i, idNext;
  double *p;
  vtkCellArray *cells;
  vtkIdType npts;
  vtkIdType *indx;
  
  if (pd == NULL)
    {
    vtkErrorMacro(<< "No input to writer");
    return;
    }

  if (this->FileName == NULL)
    {
    vtkErrorMacro(<< "Please specify FileName to write");
    return;
    }

  fpObj = fopen(this->FileName,"w");
  if (!fpObj)
    {
    vtkErrorMacro(<< "unable to open output file");
    return;
    }
  
  //
  //  Write header
  //
  vtkDebugMacro("Writing wavefront files");
  fprintf(fpObj, "# wavefront obj file written by VisIt\n");
  if (this->Label)
    {
    fprintf(fpObj, "# Description: %s\n", this->Label);
    }
  fprintf(fpObj, "\n");
  
  // write out the points
  points = pd->GetPoints();
  for (i = 0; i < points->GetNumberOfPoints(); i++)
    {
    p = points->GetPoint(i);
    fprintf (fpObj, "v %g %g %g\n", p[0], p[1], p[2]);
    }
  idNext = idStart + (int)(points->GetNumberOfPoints());
  
  // write out the point data
  pntData = pd->GetPointData();
  if (pntData->GetNormals())
    {
    normals = pntData->GetNormals();
    for (i = 0; i < normals->GetNumberOfTuples(); i++)
      {
      p = normals->GetTuple(i);
      fprintf (fpObj, "vn %g %g %g\n", p[0], p[1], p[2]);
      }
    }
  
  tcoords = pntData->GetTCoords();
  if (tcoords)
    {
    for (i = 0; i < tcoords->GetNumberOfTuples(); i++)
      {
      p = tcoords->GetTuple(i);
      fprintf (fpObj, "vt %g %g\n", p[0], p[1]);
      }
    }
  
  // write out polys if any
  if (pd->GetNumberOfPolys() > 0)
    {
    cells = pd->GetPolys();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
      {
      fprintf(fpObj,"f ");
      for (i = 0; i < npts; i++)
        {
        if (normals)
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i/%i/%i ", ((int)indx[i])+idStart, 
                    ((int)indx[i]) + idStart, ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i//%i ",((int)indx[i])+idStart,
                    ((int)indx[i]) + idStart);
            }
          }
        else
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i/%i ", ((int)indx[i])+idStart, 
                    ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i ", ((int)indx[i])+idStart);
            }
          }
        }
      fprintf(fpObj,"\n");
      }
    }

  idStart = idNext;

  fclose(fpObj);
}

