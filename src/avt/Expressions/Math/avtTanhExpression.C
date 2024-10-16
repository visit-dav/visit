// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtTanhExpression.C                              //
// ************************************************************************* //

#include <avtTanhExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtTanhExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 27, 2009
//
// ****************************************************************************

avtTanhExpression::avtTanhExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTanhExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 27, 2009
//
// ****************************************************************************

avtTanhExpression::~avtTanhExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTanhExpression::DoOperation
//
//  Purpose:
//      Take the hyperbolic tangent of each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 27, 2009
//
//  Modifications:
//
// ****************************************************************************
 
void
avtTanhExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val = in->GetComponent(i, j);
            out->SetComponent(i, j, tanh(val));
        }
    }
}


