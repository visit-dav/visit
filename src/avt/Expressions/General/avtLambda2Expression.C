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
//                     avtLambda2Expression.C                          //
// ************************************************************************* //

#include <avtLambda2Expression.h>
#include <avtVariableCache.h>

#include <math.h>

#include <vtkMath.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkDataSet.h>
#include <vtkInformation.h>
#include <vtkInformationDoubleVectorKey.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtLambda2Expression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Kevin Griffin
//  Creation:   Mon Aug  4 15:15:38 PDT 2014
//
// ****************************************************************************

avtLambda2Expression::avtLambda2Expression()
{
    ;
}


// ****************************************************************************
//  Method: avtLambda2Expression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kevin Griffin
//  Creation:   Tue Jul 29 10:08:40 PDT 2014
//
// ****************************************************************************

avtLambda2Expression::~avtLambda2Expression()
{
    ;
}

// ****************************************************************************
//  Method: avtLambda2Expression::EigenValues
//
//  Purpose:
//      Jacobi iteration for the solution of eigenvalues of a 3x3 matrix.
//      The resulting eigenvalues are sorted in decreasing order. The calling
//      class must free this memory.
//
//  Arguments:
//      matrix    3x3 matrix, row-major order
//
//  Returns:      Pointer to an array of eigenvalues
//
//  Programmer:   Kevin Griffin
//  Creation:     Mon Aug  4 15:15:38 PDT 2014
//
//  Modifications:
//
// ****************************************************************************
double *
avtLambda2Expression::EigenValues(double **matrix)
{
    // Init eigenvectors storage
    double *eigenVectors[3];
    eigenVectors[0] = new double[3];
    eigenVectors[1] = new double[3];
    eigenVectors[2] = new double[3];
    
    // Init eigenvalues storage
    double *eigenValues = new double[3];
    vtkMath::Jacobi(matrix, eigenValues, eigenVectors);
    
    // Cleanup
    for(int i=0; i<3; i++)
    {
        delete [] eigenVectors[i];
    }
    
    return eigenValues;
}

// ****************************************************************************
//  Method: avtLambda2Expression::Lambda2
//
//  Purpose:
//      Generates the Lambda-2 criterion. It is based on the observation that,
//      in regions where Lambda-2 is less than zero, rotation exceeds strain
//      and, in conjuction with a pressure min, indicates the presence of a
//      vortex.
//
//  Arguments:
//      du      The gradient vector of X
//      dv      The gradient vector of Y
//      dw      The gradient vector of Z
//
//  Returns:      The Lambda-2 value
//
//  Programmer:   Kevin Griffin
//  Creation:     Mon Aug  4 15:15:38 PDT 2014
//
//  Modifications:
//
// ****************************************************************************
double
avtLambda2Expression::Lambda2(const double * du, const double * dv, const double * dw)
{
    double l11 = du[2]*du[2] + du[1]*dv[2] + du[0]*dw[2];
    double l12 = ((du[1] + dv[2]) * (du[2] + dv[1]) + dv[0]*dw[2] + du[0]*dw[1])/2.0;
    double l13 = (dv[2]*dv[0] + du[1]*dw[1] + (du[0]+dw[2])*(du[2]+dw[0]))/2.0;
    double l22 = du[1]*dv[2] + dv[1]*dv[1] + dv[0]*dw[1];
    double l23 = (du[0]*dv[2]+du[1]*dw[2] + (dv[0]+dw[1])*(dv[1]+ dw[0]))/2.0;
    double l33 = du[0]*dw[2] + dv[0]*dw[1] + dw[0]*dw[0];
    
    double **matrix = new double*[3];
    
    double row1[3] = {l11, l12, l13};
    double row2[3] = {l12, l22, l23};
    double row3[3] = {l13, l23, l33};
    
    matrix[0] = row1;
    matrix[1] = row2;
    matrix[2] = row3;
    
    double *lambda = avtLambda2Expression::EigenValues(matrix);
    double l2 = lambda[1];
    
    // Clean-Up
    delete [] matrix;
    delete [] lambda;
    
    return l2;
}

// ****************************************************************************
//  Method: avtLambda2Expression::DeriveVariable
//
//  Purpose:
//      Creates a scalar variable from velocity gradients.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived lambda-2 scalar values.  The calling class must
//                free this memory.
//
//  Programmer:   Kevin Griffin
//  Creation:     Mon Aug  4 15:15:38 PDT 2014
//
//  Modifications:
//
// ****************************************************************************
vtkDataArray *
avtLambda2Expression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if(varnames.size() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Expecting 3 gradient vector inputs");
    }
    
    vtkDataArray *var1 = NULL;  /* du */
    vtkDataArray *var2 = NULL;  /* dv */
    vtkDataArray *var3 = NULL;  /* dw */
    
    var1 = in_ds->GetPointData()->GetArray(varnames[0]);
    
    if(var1 != NULL)
    {
        var2 = in_ds->GetPointData()->GetArray(varnames[1]);
        var3 = in_ds->GetPointData()->GetArray(varnames[2]);
    }
    else
    {
        var1 = in_ds->GetCellData()->GetArray(varnames[0]);
        var2 = in_ds->GetCellData()->GetArray(varnames[1]);
        var3 = in_ds->GetCellData()->GetArray(varnames[2]);
    }
    
    // Check that the variable(s) exist and that they have the same centering
    if(var1 == NULL || var2 == NULL || var3 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Variable for lambda2 must exist and have the same centering");
    }
    
    // Check that the input variables are vectors
    if(var1->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "lambda2 only runs on vectors");
    }
    
    int numTuples = var1->GetNumberOfTuples();
    
    // Q-Criterion Results
    vtkDataArray *results = vtkDoubleArray::New();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(numTuples);
    
    for(int i=0; i<numTuples; i++)
    {
        results->SetTuple1(i, avtLambda2Expression::Lambda2(var1->GetTuple(i), var2->GetTuple(i), var3->GetTuple(i)));
    }
    
    return results;
}
