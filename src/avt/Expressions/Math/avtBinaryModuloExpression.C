// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtBinaryModuloExpression.C                         //
// ************************************************************************* //

#include <avtBinaryModuloExpression.h>

#ifdef _WIN32
#include <math.h>
#else
#include <cmath>
#endif

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtBinaryModuloExpression constructor
//
//  Mark C. Miller, Fri Jul 26 00:06:58 PDT 2024
//  Copied from avtBinaryDivideExpression by Hank Childs
// ****************************************************************************

avtBinaryModuloExpression::avtBinaryModuloExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryModuloExpression destructor
//
//  Mark C. Miller, Fri Jul 26 00:07:09 PDT 2024
// ****************************************************************************

avtBinaryModuloExpression::~avtBinaryModuloExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryModuloExpression::DoOperation
//
//  Purpose:
//      Modulos the contents of the first array by the second array and puts
//      the output in a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
// Mark C. Miller, Fri Jul 26 00:07:43 PDT 2024
// ****************************************************************************

void
avtBinaryModuloExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                   vtkDataArray *out, int ncomponents,
                                   int ntuples)
{
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if ((in1ncomps == 1) && (in2ncomps == 1))
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            double val2 = in2->GetTuple1(tup2);
            if (val1 == 0. && val2 == 0.)
            {
                out->SetTuple1(i, 1.);
            }
            else if (val2 == 0. && val1 != 0.)
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "FPE: Divide by zero in mod operator");
            }
            else
                out->SetTuple1(i, fmod(val1,val2));
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val2 = in2->GetTuple1(tup2);
            if (val2 == 0)
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "FPE: Divide by zero in mod operator");
            }
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(tup1, j);
                out->SetComponent(i, j, fmod(val1,val2));
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                double val2 = in2->GetComponent(tup2, j);
                if (val2 == 0)
                {
                    EXCEPTION2(ExpressionException, outputVariableName, 
                               "FPE: Divide by zero in mod operator");
                }
                out->SetComponent(i, j, fmod(val1,val2));
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Division of vectors in undefined.");
    }
}
