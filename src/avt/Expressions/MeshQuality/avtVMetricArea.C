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
//                              avtVMetricArea.C                             //
// ************************************************************************* //

#include "avtVMetricArea.h"

#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <verdict.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtVMetricArea::Metric
//
//  Purpose:
//      Inspect an element and calculate the Area.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The Area of the cell, or defaultValue if not supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
// ****************************************************************************

double avtVMetricArea::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT
    switch(type)
    {
        case VTK_TRIANGLE:
            return v_tri_area(3, coords);
        
        case VTK_QUAD:
            return v_quad_area(4, coords);
    }
#endif
    return -1;
}


// ****************************************************************************
//  Method: avtVMetricArea::OperateDirectlyOnMesh
//
//  Purpose:
//      Determines if we want to speed up the operation by operating directly
//      on the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

bool
avtVMetricArea::OperateDirectlyOnMesh(vtkDataSet *ds)
{
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int dims[3];
        ((vtkRectilinearGrid *) ds)->GetDimensions(dims);
        if (dims[0] > 1 && dims[1] > 1 && dims[2] == 1)
            return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtVMetricArea::MetricForWholeMesh
//
//  Purpose:
//      Determines the area for each cell in the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtVMetricArea::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    int  i, j;

    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    float *Xdist = new float[dims[0]-1];
    for (i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    float *Ydist = new float[dims[1]-1];
    for (i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);

    for (j = 0 ; j < dims[1]-1 ; j++)
        for (i = 0 ; i < dims[0]-1 ; i++)
        {
            int idx = j*(dims[0]-1) + i;
            float area = Xdist[i]*Ydist[j];
            rv->SetTuple1(idx, area);
        }

    delete [] Xdist;
    delete [] Ydist;
}


