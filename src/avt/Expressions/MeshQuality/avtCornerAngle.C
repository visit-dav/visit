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
//                              avtCornerAngle.C                             //
// ************************************************************************* //

#include <avtCornerAngle.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtCornerAngle constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************

avtCornerAngle::avtCornerAngle()
{
    takeMin = true;
}


// ****************************************************************************
//  Method: avtCornerAngle::DeriveVariable
//
//  Purpose:
//      Find the minimum or maximum corner angle.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************

vtkDataArray *
avtCornerAngle::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        float vol = (float) GetCornerAngle(cell);
        arr->SetTuple(i, &vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtCornerAngle::GetCornerAngle
//
//  Purpose:
//      Evaluate the corner angle and determine the minimum or
//      maximum.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The corner angle of the zone.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************
 
double
avtCornerAngle::GetCornerAngle(vtkCell *cell)
{
    int  i, j;

    int celltype = cell->GetCellType();
    if (celltype == VTK_VERTEX || celltype == VTK_LINE)
        return 0.;

    //
    // Calculate the value of each edge and then return the minimum or maximum.
    //
    int nPts = cell->GetNumberOfPoints();
    int nEdges = cell->GetNumberOfEdges();
    double rv = +FLT_MAX;
    if (!takeMin)
        rv = -FLT_MAX;

    for (i = 0 ; i < nPts ; i++)
    {
        int ID = cell->GetPointId(i);
        double ptV[3];
        cell->GetPoints()->GetPoint(i, ptV);
        double A[3][3];

        // Identify the edges adjacent to this vertex.
        int numEdges = 0;
        for (j = 0 ; j < nEdges ; j++)
        {
            vtkCell *edge = cell->GetEdge(j);
            int match = -1;
            if (edge->GetPointId(0) == ID)
                match = 1;
            if (edge->GetPointId(1) == ID)
                match = 0;
            if (match < 0)
                continue;

            // Guard against 4 edges from a vertex.
            if (numEdges > 3)
                return 0.;

            double ptA[3];
            edge->GetPoints()->GetPoint(match, ptA);
            A[numEdges][0] = ptA[0] - ptV[0];
            A[numEdges][1] = ptA[1] - ptV[1];
            A[numEdges][2] = ptA[2] - ptV[2];
            double amag = sqrt(A[numEdges][0]*A[numEdges][0] +
                               A[numEdges][1]*A[numEdges][1] + 
                               A[numEdges][2]*A[numEdges][2]);
            if (amag != 0.)
            {
                A[numEdges][0] /= amag;
                A[numEdges][1] /= amag;
                A[numEdges][2] /= amag;
            }

            numEdges++;
        }

        double angle = 0.;
        if (numEdges == 2)
        {
            double dot = A[0][0]*A[1][0] + A[0][1]*A[1][1] + A[0][2]*A[1][2];
            angle = acos(dot);
        }
        else if (numEdges == 3)
        {
            // According to Tony De Groot, we want this to be asin(sqrt(AxB.C)).
            double cross[3];
            cross[0] = A[0][1]*A[1][2] - A[0][2]*A[1][1];
            cross[1] = A[0][2]*A[1][0] - A[0][0]*A[1][2];
            cross[2] = A[0][0]*A[1][1] - A[0][1]*A[1][0];
            double dot = A[2][0]*cross[0] + A[2][1]*cross[1] + A[2][2]*cross[2];
            double root = sqrt(dot);
            angle = asin(root);
        }

        if (takeMin)
            rv = (rv < angle ? rv : angle);
        else
            rv = (rv > angle ? rv : angle);
    }

    return rv;
}


