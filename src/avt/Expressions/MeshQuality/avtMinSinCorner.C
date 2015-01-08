/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                              avtMinSinCorner.C                            //
// ************************************************************************* //

#include <avtMinSinCorner.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtMinSinCorner constructor
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//
// ****************************************************************************

avtMinSinCorner::avtMinSinCorner()
{
    orderCCW = true;
}


// ****************************************************************************
//  Method: avtMinSinCorner::DeriveVariable
//
//  Purpose:
//      Find the minimum Sin(corner-angle) for each zone of a 2D quad mesh.
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//
// ****************************************************************************

vtkDataArray *
avtMinSinCorner::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double msca = GetMinSinCorner(cell);
        arr->SetTuple1(i, msca);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtMinSinCorner::GetMinSinCorner
//
//  Purpose:
//      Evaluate the Sin(corner-angles) and determine the minimum.
//
//  Arguments:
//      cell    The input zone of a 2D mesh.
//
//  Returns:    The Minimum Sin(corner-angle) of the zone.
//              Require answer in the range 0.0-1.0
//              where 0.0 denotes poor cell quality and 1.0 is good.
//              Zero also returned for pathological cases.
//
//  Comments:   Evaluates Min(Sin(corner-angle)) using Areas & Cross Products.
//              Note that this does not determine a unique angle -
//              but is nevertheless a useful measure of cell quality.
//
//              This version assumes Nodes are supplied in a cyclic order -
//              Counter-ClockWise (VTK default) if flag orderCCW is True,
//              otherwise assumes ClockWise ordering.
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//
// ****************************************************************************

double
avtMinSinCorner::GetMinSinCorner(vtkCell *cell)
{
    double rv = 0.0;

    int celltype = cell->GetCellType();
    if (celltype != VTK_QUAD)
        return rv;

    //
    // Get the Cell parameters:
    //
    vtkIdType nPts = cell->GetNumberOfPoints();
    // Return zero for anything other than a quad
    if (nPts != 4 )
        return rv;

    double ptV[3];
    double ptX[4], ptY[4];

    if (orderCCW)
        // Counter-ClockWise ordering:
        for (vtkIdType i = 0 ; i < nPts ; i++)
        {
            cell->GetPoints()->GetPoint(i, ptV);
            ptX[i] = ptV[0];
            ptY[i] = ptV[1];
        }
    else
        // ClockWise ordering:
        for (vtkIdType i = 0 ; i < nPts ; i++)
        {
            cell->GetPoints()->GetPoint((3-i), ptV);
            ptX[i] = ptV[0];
            ptY[i] = ptV[1];
        }


    // Intermediates
    double dx01, dy01, dx12, dy12, dx23, dy23, dx30, dy30;

    dx01 = ptX[0] - ptX[1];
    dy01 = ptY[0] - ptY[1];

    dx12 = ptX[1] - ptX[2];
    dy12 = ptY[1] - ptY[2];

    dx23 = ptX[2] - ptX[3];
    dy23 = ptY[2] - ptY[3];

    dx30 = ptX[3] - ptX[0];
    dy30 = ptY[3] - ptY[0];

    // Corner Angle Areas:
    double AA0, AA1, AA2, AA3;
    AA0 = dx30*dy01 - dx01*dy30;
    AA1 = dx01*dy12 - dx12*dy01;
    AA2 = dx12*dy23 - dx23*dy12;
    AA3 = dx23*dy30 - dx30*dy23;

    // return 0.0 if Corner Angle Area is practically zero
    if (AA0 <= FLT_MIN || AA1 <= FLT_MIN || AA2 <= FLT_MIN || AA3 <= FLT_MIN)
        return rv;


    // Sin(Corner-Angles)
    double mag01s, mag03s, mag12s, mag23s;
    double SCA[4], SCAMin;

    // Corner 0:
    mag01s = dx01*dx01 + dy01*dy01;
    mag03s = dx30*dx30 + dy30*dy30;
    SCA[0] = AA0/sqrt(mag01s*mag03s);

    // Corner 1:
    mag12s = dx12*dx12 + dy12*dy12;
    SCA[1] = AA1/sqrt(mag12s*mag01s);

    // Corner 2:
    mag23s = dx23*dx23 + dy23*dy23;
    SCA[2] = AA2/sqrt(mag23s*mag12s);

    // Corner 3:
    SCA[3] = AA3/sqrt(mag03s*mag23s);


    // What is the Minimum of these Sin(Corner-Angles)?

    SCAMin = SCA[0];
    for (int i = 1 ; i < 4 ; i++)
        if (SCA[i] < SCAMin)
            SCAMin = SCA[i];

    // Require answer in the range 0.0-1.0
    // Return 0.0 for anything negative.
    if (SCAMin > 0.0)
        rv = SCAMin;

    return rv;
}
