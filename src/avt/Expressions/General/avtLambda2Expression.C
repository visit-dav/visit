// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

#include <DebugStream.h>
#include <ExpressionException.h>
#include <StackTimer.h>

#ifdef _OPENMP
# include <omp.h>
#endif

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
//    Brad Whitlock, Thu Jan 11 17:43:28 PST 2018
//    Remove dynamic allocation.
//
// ****************************************************************************

void
avtLambda2Expression::EigenValues(double **matrix, double eigenValues[3])
{
    double __ev0[3], __ev1[3], __ev2[3];
    double *eigenVectors[3] = {__ev0, __ev1, __ev2};
    vtkMath::Jacobi(matrix, eigenValues, eigenVectors);
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
//    Brad Whitlock, Thu Jan 11 17:43:28 PST 2018
//    Remove dynamic allocation.
//
// ****************************************************************************

#define COMPUTE_LAMBDA2 \
    double l11 = du[2]*du[2] + du[1]*dv[2] + du[0]*dw[2]; \
    double l12 = ((du[1] + dv[2]) * (du[2] + dv[1]) + dv[0]*dw[2] + du[0]*dw[1])/2.0; \
    double l13 = (dv[2]*dv[0] + du[1]*dw[1] + (du[0]+dw[2])*(du[2]+dw[0]))/2.0; \
    double l22 = du[1]*dv[2] + dv[1]*dv[1] + dv[0]*dw[1]; \
    double l23 = (du[0]*dv[2]+du[1]*dw[2] + (dv[0]+dw[1])*(dv[1]+ dw[0]))/2.0; \
    double l33 = du[0]*dw[2] + dv[0]*dw[1] + dw[0]*dw[0]; \
 \
    double row1[3] = {l11, l12, l13}; \
    double row2[3] = {l12, l22, l23}; \
    double row3[3] = {l13, l23, l33}; \
    double *matrix[3]; \
    matrix[0] = row1; \
    matrix[1] = row2; \
    matrix[2] = row3; \
 \
    double lambda[3]; \
    avtLambda2Expression::EigenValues(matrix, lambda); \

double
avtLambda2Expression::Lambda2(const double * du, const double * dv, const double * dw)
{
    COMPUTE_LAMBDA2
    return lambda[1];
}

// ****************************************************************************
//  Method: CalculateLambda2
//
//  Purpose:
//
//  Arguments:
//      gradX   The gradient vector of X-velocity
//      gradY   The gradient vector of Y-velocity
//      gradZ   The gradient vector of Z-velocity
//      lambda2   The resulting middle eigenvalue
//
//  Returns:    None
//
//  Programmer:   Christopher P. Stone
//  Creation:     Sat Apr 01 13:00:00 EDT 2017
//
//  Modifications:
//
// ****************************************************************************
#ifdef _OPENMP
template <typename InputType>
void
avtLambda2Expression::CalculateLambda2(const InputType * gradX, const InputType * gradY, const InputType * gradZ,
    double *lambda2, const int numTuples)
{
    #pragma omp parallel for
    for (int i = 0; i < numTuples; ++i)
    {
        const int offset = 3*i;
        const double du[3] = { gradX[offset], gradX[offset+1], gradX[offset+2] };
        const double dv[3] = { gradY[offset], gradY[offset+1], gradY[offset+2] };
        const double dw[3] = { gradZ[offset], gradZ[offset+1], gradZ[offset+2] };

        COMPUTE_LAMBDA2
        lambda2[i] = lambda[1];
    }
    
    return;
}
#endif

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
    
    // Lambda2 Results
    vtkDataArray *results = vtkDoubleArray::New();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(numTuples);

#ifdef _OPENMP
#pragma message("Compiling for OpenMP.")
    bool hasValidData = (var1->GetDataType() == var2->GetDataType()) &&
                        (var2->GetDataType() == var3->GetDataType()) &&
                       ((var1->GetDataType() == VTK_DOUBLE) || (var1->GetDataType() == VTK_FLOAT)) &&
                        (var1->HasStandardMemoryLayout() && var2->HasStandardMemoryLayout() && var3->HasStandardMemoryLayout());

    if (hasValidData)
    {
        StackTimer t0("avtLambda2Expression OpenMP");
        if (var1->GetDataType() == VTK_DOUBLE)
        {
            CalculateLambda2( (const double *) var1->GetVoidPointer(0),
                              (const double *) var2->GetVoidPointer(0),
                              (const double *) var3->GetVoidPointer(0),
                              (double *) results->GetVoidPointer(0),
                              numTuples);
        }
        else// if (var1->GetDataType() == VTK_FLOAT)
        {
            CalculateLambda2( (const float *) var1->GetVoidPointer(0),
                              (const float *) var2->GetVoidPointer(0),
                              (const float *) var3->GetVoidPointer(0),
                              (double *) results->GetVoidPointer(0),
                               numTuples);
        }
    }
    else
#endif
    {
        StackTimer t1("avtLambda2Expression");
        for(int i=0; i<numTuples; i++)
        {
            results->SetTuple1(i, avtLambda2Expression::Lambda2(var1->GetTuple(i), var2->GetTuple(i), var3->GetTuple(i)));
        }
    }

    return results;
}
