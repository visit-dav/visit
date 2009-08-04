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
//                   avtRectilinearLaplacianExpression.C                     //
// ************************************************************************* //

#include <avtRectilinearLaplacianExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <ExpressionException.h>


static double SecondDerivative(double, double, double, double, double, double);

// ****************************************************************************
//  Method: avtRectilinearLaplacianExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2008
//
// ****************************************************************************

avtRectilinearLaplacianExpression::avtRectilinearLaplacianExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtRectilinearLaplacianExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2008
//
// ****************************************************************************

avtRectilinearLaplacianExpression::~avtRectilinearLaplacianExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtRectilinearLaplacianExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     May 8, 2008
//
// ****************************************************************************

vtkDataArray *
avtRectilinearLaplacianExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The rectilinear Laplacian expression can only be applied"
                   " to rectilinear grids.");
    }

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;
    int dims[3];
    rg->GetDimensions(dims);

    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;

    vtkDataArray *field = in_ds->GetPointData()->GetArray(varname);
    
    if (field == NULL)
    {
        field = in_ds->GetCellData()->GetArray(varname);
        dims[0] = (dims[0] > 1 ? dims[0]-1 : dims[0]);
        dims[1] = (dims[1] > 1 ? dims[1]-1 : dims[1]);
        dims[2] = (dims[2] > 1 ? dims[2]-1 : dims[2]);
    }
    if (field == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "Unable to locate variable for Laplace operator");
    }

    if (field->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Can only take Laplacian of scalars.");
    }

    int ntuples = field->GetNumberOfTuples();
    vtkDataArray *results = field->NewInstance();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(ntuples);

    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    vtkDataArray *Z = rg->GetZCoordinates();
    for (int i = 0 ; i < dims[0] ; i++)
        for (int j = 0 ; j < dims[1] ; j++)
            for (int k = 0 ; k < dims[2] ; k++)
            {
                double lp = 0.;
                int idx = k*dims[1]*dims[0] + j*dims[0] + i;
                if (dims[0] > 2)
                {
                    // Note that our point locations may be translated
                    // for cell data.  This is okay because the derivative
                    // really just cares about relative position.
                    if (i > 0 && i < (dims[0]-1))
                       lp += SecondDerivative(field->GetTuple1(idx-1),
                                              field->GetTuple1(idx),
                                              field->GetTuple1(idx+1), 
                                              X->GetTuple1(i-1),
                                              X->GetTuple1(i),
                                              X->GetTuple1(i+1));
                    else if (i == 0)
                       lp += SecondDerivative(field->GetTuple1(idx),
                                              field->GetTuple1(idx+1),
                                              field->GetTuple1(idx+2),
                                              X->GetTuple1(0),
                                              X->GetTuple1(1),
                                              X->GetTuple1(2));
                    else if (i == (dims[0]-1))
                       lp += SecondDerivative(field->GetTuple1(idx),
                                              field->GetTuple1(idx-1),
                                              field->GetTuple1(idx-2),
                                              X->GetTuple1(dims[0]-1),
                                              X->GetTuple1(dims[0]-2),
                                              X->GetTuple1(dims[0]-3));
                }
                if (dims[1] > 2)
                {
                    int one = dims[0];
                    if (j > 0 && j < (dims[1]-1))
                       lp += SecondDerivative(field->GetTuple1(idx-one),
                                              field->GetTuple1(idx),
                                              field->GetTuple1(idx+one),
                                              Y->GetTuple1(j-1),
                                              Y->GetTuple1(j),
                                              Y->GetTuple1(j+1));
                    else if (j == 0)
                       lp += SecondDerivative(field->GetTuple1(idx),
                                              field->GetTuple1(idx+one),
                                              field->GetTuple1(idx+2*one),
                                              Y->GetTuple1(0),
                                              Y->GetTuple1(1),
                                              Y->GetTuple1(2));
                    else if (j == (dims[1]-1))
                       lp += SecondDerivative(field->GetTuple1(idx),
                                              field->GetTuple1(idx-one),
                                              field->GetTuple1(idx-2*one),
                                              Y->GetTuple1(dims[1]-1),
                                              Y->GetTuple1(dims[1]-2),
                                              Y->GetTuple1(dims[1]-3));
                }
                if (dims[2] > 2)
                {
                    int one = dims[0]*dims[1];
                    if (k > 0 && k < (dims[2]-1))
                       lp += SecondDerivative(field->GetTuple1(idx-one),
                                              field->GetTuple1(idx),
                                              field->GetTuple1(idx+one),
                                              Z->GetTuple1(k-1),
                                              Z->GetTuple1(k),
                                              Z->GetTuple1(k+1));
                    else if (k == 0)
                       lp += SecondDerivative(field->GetTuple1(idx),
                                              field->GetTuple1(idx+one),
                                              field->GetTuple1(idx+2*one),
                                              Z->GetTuple1(0),
                                              Z->GetTuple1(1),
                                              Z->GetTuple1(2));
                    else if (k == (dims[2]-1))
                       lp += SecondDerivative(field->GetTuple1(idx),
                                              field->GetTuple1(idx-one),
                                              field->GetTuple1(idx-2*one),
                                              Z->GetTuple1(dims[2]-1),
                                              Z->GetTuple1(dims[2]-2),
                                              Z->GetTuple1(dims[2]-3));
                }

                results->SetTuple1(idx, lp);
            }

    return results;
}


// ****************************************************************************
//  Function: SecondDerivative
//
//  Purpose:
//      Calculates the second derivative of three points, by fitting a parabola
//      to the points and taking the second derivative at the middle point.
//
//      Note that the parabola has form y = ax^2 + bx + c, which has second 
//      derivative of 2a, so the X location of where we evaluate the derivative
//      doesn't really matter.
//
//  Arguments:
//      V1      The value of a point on the curve.
//      V2      The value of a point on the curve.
//      V3      The value of a point on the curve.
//      X1      The location of a point on the curve.
//      X2      The location of a point on the curve.
//      X3      The location of a point on the curve.

//  Programmer: Hank Childs
//  Creation:   May 8, 2008
//
// ****************************************************************************

double
SecondDerivative(double V1, double V2, double V3, 
                 double X1, double X2, double X3)
{
    //
    // We have three points on a curve.  The curve follows 
    // the form y = ax^2 + bx + c.  We can solve for a, b, and c and then we
    // will know the curve exactly.  3 equations, 3 unknowns, so we can solve
    // this with a matrix inverse.  Use the avtMatrix class, which is actually
    // for 4x4 matrices.  Just stick in a dummy equation for the fourth.
    //
    double elem[16];
    elem[0] = X1*X1; // *a
    elem[1] = X1;    // *b
    elem[2] = 1;     // *c
    elem[3] = 0;     // we're not using this
    elem[4] = X2*X2; // *a
    elem[5] = X2;    // *b
    elem[6] = 1;     // *c
    elem[7] = 0;     // we're not using this
    elem[8] = X3*X3; // *a
    elem[9] = X3;    // *b
    elem[10] = 1;    // *c
    elem[11] = 0;    // we're not using this
    elem[12] = 0;    // dummy
    elem[13] = 0;    // dummy
    elem[14] = 0;    // dummy
    elem[15] = 1;    // dummy

    //
    // Now we have the matrix:
    //
    //  (X1^2 X1 1 0)  (a)   (V1)
    //  (X2^2 X2 1 0)  (b) = (V2)
    //  (X3^2 X3 1 0)  (c) = (V3)
    //  (0    0  0 1)  (d)   (1)
    //
    // We can invert the matrix and left multiply it to both sides.
    //
    // This would give:
    //  (a)                         (V1)
    //  (b)  =  (inverse matrix) *  (V2)
    //  (c)  =                      (V3)
    //  (d)                         (1)
    //
    avtMatrix mat(elem);
    mat.Inverse();
    const double *inv = mat.GetElements();
    double a = inv[0]*V1 + inv[1]*V2 + inv[2]*V3;

    // The second derivative is just 2*a.
    return 2*a;
}


