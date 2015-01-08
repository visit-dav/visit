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
//                         avtEdgeNormalExpression.C                         //
// ************************************************************************* //

#include <avtEdgeNormalExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItPolyDataNormals2D.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtEdgeNormalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
// ****************************************************************************

avtEdgeNormalExpression::avtEdgeNormalExpression()
{
    isPoint = true;
}


// ****************************************************************************
//  Method: avtEdgeNormalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
// ****************************************************************************

avtEdgeNormalExpression::~avtEdgeNormalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEdgeNormalExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:   Jeremy Meredith
//  Creation:     March 10, 2014
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtEdgeNormalExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        return RectilinearDeriveVariable((vtkRectilinearGrid *) in_ds);
    }

    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The Edge normal expression "
                   "can only be calculated on edges.");
    }

    vtkPolyData *pd = (vtkPolyData *) in_ds;

    vtkVisItPolyDataNormals2D *n = vtkVisItPolyDataNormals2D::New();
    if (isPoint)
        n->SetNormalTypeToPoint();
    else
        n->SetNormalTypeToCell();
    n->SetInputData(pd);
    n->Update();
    vtkPolyData *out = n->GetOutput();

    vtkDataArray *arr = NULL;
    if (isPoint)
        arr = out->GetPointData()->GetNormals();
    else
        arr = out->GetCellData()->GetNormals();
    

    if (arr == NULL)
    {
        n->Delete();
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "An internal error occurred where "
                   "the edge normals could not be calculated.  Please "
                   "contact a VisIt developer.");
    }

    arr->Register(NULL);
    n->Delete();

    return arr;
}



// ****************************************************************************
//  Method: avtEdgeNormalExpression::RectilinearDeriveVariable
//
//  Purpose:
//      A method that does special handling for rectilinear generation.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
// ****************************************************************************

vtkDataArray *
avtEdgeNormalExpression::RectilinearDeriveVariable(vtkRectilinearGrid *rgrid)
{
    int dims[3];
    rgrid->GetDimensions(dims);
    int nMatch = 0;
    bool doX = (dims[0] == 1);
    if (doX)
        nMatch++;
    bool doY = (dims[1] == 1);
    if (doY)
        nMatch++;
    bool doZ = (dims[2] == 1);
    if (doZ)
        nMatch++;
    if (nMatch == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "edge normals for a 3D data set.");
    }
    if (nMatch == 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "edge normals for polygons.");
    }
    if (nMatch == 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "edge normals for vertices.");
    }
    vtkDataArray *n = CreateArrayFromMesh(rgrid);
    n->SetNumberOfComponents(3);
    vtkIdType ntuples = (isPoint ? rgrid->GetNumberOfPoints() 
                           : rgrid->GetNumberOfCells());
    n->SetNumberOfTuples(ntuples);
    double norm[3] = { 0, 0, 0 };
    if (doY)
        norm[1] = 1.0;
    else
        norm[0] = 1.0;
    for (vtkIdType i = 0 ; i < ntuples ; i++)
    {
        n->SetTuple(i, norm);
    }

    return n;
}


