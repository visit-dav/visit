// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtExpExpression.C                          //
// ************************************************************************* //

#include <avtExpExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtExpExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 27 15:14:49 EDT 2008
//
// ****************************************************************************

avtExpExpression::avtExpExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtExpExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 27 15:14:58 EDT 2008
//
// ****************************************************************************

avtExpExpression::~avtExpExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtExpExpression::DoOperation
//
//  Purpose:
//      Performs the exp function to each component, tuple of a data array.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 27 15:15:11 EDT 2008
//
//  Modifications:
//
// ****************************************************************************
 
void
avtExpExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            double val = in->GetComponent(i, j);
            out->SetComponent(i, j, exp(std::move(val)));
        }
    }
}
