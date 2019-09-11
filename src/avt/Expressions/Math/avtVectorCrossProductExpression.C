// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtVectorCrossProductExpression.C                    //
// ************************************************************************* //

#include <avtVectorCrossProductExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorCrossProductExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorCrossProductExpression::avtVectorCrossProductExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorCrossProductExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorCrossProductExpression::~avtVectorCrossProductExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorCrossProductExpression::DoOperation
//
//  Purpose:
//      Takes the cross product of the two inputs
//
//  Arguments:
//      in1           The first input vector
//      in2           The second input vector
//      out           The output vector.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)  Must be 3 for this operation.
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Notes: a x b = (a2 b3 - a3 b2)i + (a3 b1 - a1 b3)j + (a1 b2 - a2 b1)k
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 11 13:32:55 PDT 2003
//
//  Modifications:
//
//    Hank Childs, Wed Dec 21 14:51:43 CST 2011
//    Add support for 2D vectors creating scalars.
//
// ****************************************************************************

void
avtVectorCrossProductExpression::DoOperation(vtkDataArray *in1,
    vtkDataArray *in2, vtkDataArray *out, int ncomponents, int ntuples)
{
    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    if ((ndims == 3 && ncomponents != 3) || (ndims < 3 && ncomponents != 1))
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "you cannot take the cross product of data which are not 3-component vectors.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        double a1 = in1->GetComponent(i, 0);
        double a2 = in1->GetComponent(i, 1);
        double a3 = in1->GetComponent(i, 2);
        double b1 = in2->GetComponent(i, 0);
        double b2 = in2->GetComponent(i, 1);
        double b3 = in2->GetComponent(i, 2);

        if (ndims == 3)
        {
            out->SetComponent(i, 0, a2*b3 - a3*b2);
            out->SetComponent(i, 1, a3*b1 - a1*b3);
            out->SetComponent(i, 2, a1*b2 - a2*b1);
        }
        else
        {
            out->SetComponent(i, 0, a1*b2 - a2*b1);
        }
    }
}


// ****************************************************************************
//  Method: avtVectorCrossProductExpression::GetVariableDimension
//
//  Purpose:
//      Declares the variable dimension: 3 for 3D, 1 for 2D.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2011
//
// ****************************************************************************

int
avtVectorCrossProductExpression::GetVariableDimension(void)
{
    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    return (ndims == 3 ? 3 : 1);
}


