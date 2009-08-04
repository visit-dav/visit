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

// ************************************************************************* //
//                             avtFacePlanarity.C                             //
// ************************************************************************* //

#include <avtFacePlanarity.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

static double
GetFacePlanarityForFace(vtkCell *face, bool takeRel)
{
    int i, j;
    int nPoints = face->GetNumberOfPoints();

    if (nPoints <= 3)
        return 0;

    //
    // Compute average edge length first if necessary.
    //
    double len_sum = 1;
    if (takeRel)
    {
        len_sum = 0;
        for (i = 0; i < nPoints; i++)
        {
            int ptids[2];
            double ptx[2][3];
            for (j = 0; j < 2; j++)
            {
                ptids[j] = (i+j) % nPoints;
                face->GetPoints()->GetPoint(ptids[j],ptx[j]);
            }
            double n[3] = {ptx[0][0] - ptx[1][0], ptx[0][1] - ptx[1][1], ptx[0][2] - ptx[1][2]};
            double n_len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
            len_sum += n_len;
        }
        len_sum /= nPoints;
    }

    double distMax = 0;
    for (i = 0; i < nPoints; i++)
    {
        int ptids[4];
        double ptx[4][3];
        for (j = 0; j < 4; j++)
        {
            ptids[j] = (i+j) % nPoints;
            face->GetPoints()->GetPoint(ptids[j],ptx[j]);
        }

        //
        // Compute a vector normal to plane of first 3 points
        // n1 = p0 - p1, n2 = p2 - p1
        //
        double n1[3] = {ptx[0][0] - ptx[1][0], ptx[0][1] - ptx[1][1], ptx[0][2] - ptx[1][2]};
        double n2[3] = {ptx[2][0] - ptx[1][0], ptx[2][1] - ptx[1][1], ptx[2][2] - ptx[1][2]};
        double n1Xn2[3] = {  n1[1]*n2[2] - n1[2]*n2[1],
                           -(n1[0]*n2[2] - n1[2]*n2[0]),
                             n1[0]*n2[1] - n1[1]*n2[0]};

        //
        // Normalize it.
        //
        double n1Xn2_len = sqrt(n1Xn2[0]*n1Xn2[0]+n1Xn2[1]*n1Xn2[1]+n1Xn2[2]*n1Xn2[2]);
        if (n1Xn2_len > 0)
        {
            n1Xn2[0] /= n1Xn2_len;
            n1Xn2[1] /= n1Xn2_len;
            n1Xn2[2] /= n1Xn2_len;
        }

        //
        // Compute distance to 4th point via dot-product with normal computed above.
        //
        double p4[3] = {ptx[3][0] - ptx[1][0], ptx[3][1] - ptx[1][1], ptx[3][2] - ptx[1][2]};
        double dist = p4[0] * n1Xn2[0] + p4[1] * n1Xn2[1] + p4[2] * n1Xn2[2];
        if (dist < 0) dist = -dist;
        if (takeRel)
            dist /= len_sum;
        if (dist > distMax)
            distMax = dist;
    }
    return distMax;
}

// ****************************************************************************
//  Funcion: GetFacePlanarityForCell
//
//  Purpose: Compute Face planarity measure over whole cell.
//
//  Programmer: Mark C. Miller 
//  Creation:   June 3, 2009 
//
// ****************************************************************************
 
static double
GetFacePlanarityForCell(vtkCell *cell, bool takeRel)
{
    int celltype = cell->GetCellType();
    if (celltype == VTK_EMPTY_CELL ||
        celltype == VTK_VERTEX ||
        celltype == VTK_POLY_VERTEX ||
        celltype == VTK_LINE ||
        celltype == VTK_POLY_LINE ||
        celltype == VTK_TRIANGLE ||
        celltype == VTK_TRIANGLE_STRIP ||
        celltype == VTK_POLYGON || // assumed 2d
        celltype == VTK_PIXEL ||
        celltype == VTK_TETRA)
    {
        return 0;
    }
    else if (celltype == VTK_QUAD)
    {
        return GetFacePlanarityForFace(cell, takeRel);
    }
    else
    {
        int nFaces = cell->GetNumberOfFaces();
        if (nFaces == 0)
            return 0;
        double distMax = 0;
        for (int f = 0; f < nFaces; f++)
        {
            double dist = GetFacePlanarityForFace(cell->GetFace(f), takeRel);
            if (dist > distMax)
                distMax = dist;
        }
        return distMax;
    }
}

// ****************************************************************************
//  Method: avtFacePlanarity constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************

avtFacePlanarity::avtFacePlanarity()
{
    takeRel = false;
}

// ****************************************************************************
//  Method: avtFacePlanarity::DeriveVariable
//
//  Purpose:
//      Find the minimum or maximum corner angle.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************

vtkDataArray *
avtFacePlanarity::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        float vol = (float) GetFacePlanarityForCell(cell, takeRel);
        arr->SetTuple(i, &vol);
    }

    return arr;
}
