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

#include "vtkVisItStructuredGridNormals.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>


vtkCxxRevisionMacro(vtkVisItStructuredGridNormals, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkVisItStructuredGridNormals);

// ****************************************************************************
//  Constructor:  vtkVisItStructuredGridNormals::vtkVisItStructuredGridNormals
//
//  Arguments:
//    none
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2006
//
// ****************************************************************************
vtkVisItStructuredGridNormals::vtkVisItStructuredGridNormals()
{
    ComputePointNormals = true;
}


// ****************************************************************************
//  Method:  vtkVisItStructuredGridNormals::Execute
//
//  Purpose:
//    main update function; decide which type of algorithm to perform
//
//  Arguments:
//    none
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2006
//
// ****************************************************************************
int
vtkVisItStructuredGridNormals::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and output
    vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int dims[3];
    input->GetDimensions(dims);
    if (dims[0] > 1 && dims[1] > 1 && dims[2] > 1)
    {
        vtkErrorMacro("This filter only operates on topologically "
                      "two-dimensional structured grids");
        output->ShallowCopy(input);
        return 0;
    }

    if (ComputePointNormals)
    {
        ExecutePoint(input, output);
    }
    else
    {
        // Cell normals
        ExecuteCell(input, output);
    }
    return 1;
}


// ****************************************************************************
//  Method:  vtkVisItStructuredGridNormals::ExecutePoint
//
//  Purpose:
//    Create normals at the points by averaging from neighboring cells.
//
//  Arguments:
//      input    The input grid
//      output   The output grid
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2006
//
// ****************************************************************************

void
vtkVisItStructuredGridNormals::ExecutePoint(vtkStructuredGrid *input, 
                                            vtkStructuredGrid *output)
{
    int i, j;

    int dims[3];
    input->GetDimensions(dims);
    int nPoints = input->GetNumberOfPoints();

    output->ShallowCopy(input);
    vtkFloatArray *normals = vtkFloatArray::New();
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(nPoints);
    normals->SetName("Normals");
    output->GetPointData()->SetNormals(normals);
    normals->Delete();

    int fastDim, slowDim;
    if (dims[0] <= 1)
    {
        fastDim = dims[1];
        slowDim = dims[2];
    }
    else if (dims[1] <= 1)
    {
        fastDim = dims[0];
        slowDim = dims[2];
    }
    else
    {
        fastDim = dims[0];
        slowDim = dims[1];
    }

    for (j = 0 ; j < slowDim ; j++)
    {
        for (i = 0 ; i < fastDim ; i++)
        {
            float normal[3] = { 0, 0, 0 };
            int nQuads = 0;

            double anchor[3];
            int idx = j*fastDim + i;
            input->GetPoint(idx, anchor);
  
            double compass[4][3];
            int cIdx = 0;
            if (i-1 >= 0)
            {
                int idx = j*fastDim + i-1;
                input->GetPoint(idx, compass[cIdx++]);
            }
            if (j-1 >= 0)
            {
                int idx = (j-1)*fastDim + i;
                input->GetPoint(idx, compass[cIdx++]);
            }
            if (i+1 < fastDim)
            {
                int idx = (j)*fastDim + i+1;
                input->GetPoint(idx, compass[cIdx++]);
            }
            if (j+1 < slowDim)
            {
                int idx = (j+1)*fastDim + i;
                input->GetPoint(idx, compass[cIdx++]);
            }
            // It we are at the corner, then we only have two points.
            // The loop below would consider their cross product twice
            // but coming from opposite directions.  So change number of loop
            // iterations to consider it once.
            int iter = (cIdx == 2 ? 1 : cIdx);
            for (int k = 0 ; k < iter ; k++)
            {
                int idx1 = k;
                int idx2 = (k+1) % cIdx;
                double vec1[3];
                vec1[0] = compass[idx1][0] - anchor[0];
                vec1[1] = compass[idx1][1] - anchor[1];
                vec1[2] = compass[idx1][2] - anchor[2];
                double vec2[3];
                vec2[0] = compass[idx2][0] - anchor[0];
                vec2[1] = compass[idx2][1] - anchor[1];
                vec2[2] = compass[idx2][2] - anchor[2];

                // The normal is their cross product.
                normal[0] += vec1[1]*vec2[2] - vec1[2]*vec2[1];
                normal[1] += vec1[2]*vec2[0] - vec1[0]*vec2[2];
                normal[2] += vec1[0]*vec2[1] - vec1[1]*vec2[0];
            }

            // If we are at a corner, there is a way our indexing
            // could be reversed (see ordering above for creation of compass).
            if (cIdx == 2 && (i-1 >= 0) && (j+1 < slowDim))
            {
                normal[0] *= -1;
                normal[1] *= -1;
                normal[2] *= -1;
            }

            double mag = sqrt(normal[0]*normal[0] + normal[1]*normal[1] +
                              normal[2]*normal[2]);
            if (mag != 0.)
            {
                normal[0] /= mag;
                normal[1] /= mag;
                normal[2] /= mag;
            }
            normals->SetTuple(idx, normal);
        }
    }
}


// ****************************************************************************
//  Method:  vtkVisItStructuredGridNormals::ExecuteCell
//
//  Purpose:
//    Create normals for each cell.
//
//  Arguments:
//    input      The input structured grid
//    output     The output structured grid
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2006
//
// ****************************************************************************
void
vtkVisItStructuredGridNormals::ExecuteCell(vtkStructuredGrid *input, 
                                           vtkStructuredGrid *output)
{
    int  i, j;

    // Get all the input and output objects we'll need to reference
    output->ShallowCopy(input);

    vtkPoints    *inPts = input->GetPoints();

    int dims[3];
    input->GetDimensions(dims);
    int nCells  = input->GetNumberOfCells();

    // Create the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nCells);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetPointer(0);

    int fastDim, slowDim;
    if (dims[0] <= 1)
    {
        fastDim = dims[1];
        slowDim = dims[2];
    }
    else if (dims[1] <= 1)
    {
        fastDim = dims[0];
        slowDim = dims[2];
    }
    else
    {
        fastDim = dims[0];
        slowDim = dims[1];
    }

    for (j = 0 ; j < slowDim-1 ; j++)
    {
        for (i = 0 ; i < fastDim-1 ; i++)
        {
            double normal[3] = {0, 0, 0};

            //
            // Technically, we can always use only the first three vertices, but
            // but it is not a big hit to do the quads better, and it
            // accomodates for degenerate quads directly.  The code is the same
            // algorithm as vtkPolygon::ComputeNormal, but changed to 
            // make it work better.
            //
            double v0[3], v1[3], v2[3];
            int idx1 = j*fastDim + i;
            input->GetPoint(idx1, v0);
            int idx2 = j*fastDim + i+1;
            input->GetPoint(idx2, v1);
            int idx3 = (j+1)*fastDim + i+1;
            input->GetPoint(idx3, v2);

            double vec1[3];
            vec1[0] = v1[0] - v0[0];
            vec1[1] = v1[1] - v0[1];
            vec1[2] = v1[2] - v0[2];
            double vec2[3];
            vec2[0] = v2[0] - v0[0];
            vec2[1] = v2[1] - v0[1];
            vec2[2] = v2[2] - v0[2];

            // The normal is their cross product.
            normal[0] += vec1[1]*vec2[2] - vec1[2]*vec2[1];
            normal[1] += vec1[2]*vec2[0] - vec1[0]*vec2[2];
            normal[2] += vec1[0]*vec2[1] - vec1[1]*vec2[0];

            idx1 = (j+1)*fastDim + i+1;
            input->GetPoint(idx1, v0);

            vec1[0] = v1[0] - v0[0];
            vec1[1] = v1[1] - v0[1];
            vec1[2] = v1[2] - v0[2];
            vec2[0] = v2[0] - v0[0];
            vec2[1] = v2[1] - v0[1];
            vec2[2] = v2[2] - v0[2];

            // The normal is their cross product.
            normal[0] += vec1[1]*vec2[2] - vec1[2]*vec2[1];
            normal[1] += vec1[2]*vec2[0] - vec1[0]*vec2[2];
            normal[2] += vec1[0]*vec2[1] - vec1[1]*vec2[0];

            // Calculate the length, and throw out degenerate cases
            double nx = normal[0];
            double ny = normal[1];
            double nz = normal[2];
            double length = sqrt(nx*nx + ny*ny + nz*nz);
    
            if (length != 0)
            {
                normal[0] = (float)(nx/length);
                normal[1] = (float)(ny/length);
                normal[2] = (float)(nz/length);
            }
            else
            {
                normal[0] = 0.f;
                normal[1] = 0.f;
                normal[2] = 1.f;
            }
       
            int idx = j*(fastDim-1) + i;
            newNormals->SetTuple(idx, normal);
        }
    }
        
    output->GetCellData()->SetNormals(newNormals);
    newNormals->Delete();
}


