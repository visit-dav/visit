// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtCountReductionExpression.C                          //
// ************************************************************************* //

#include <avtCountReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtCountReductionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtCountReductionExpression::avtCountReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCountReductionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtCountReductionExpression::~avtCountReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCountReductionExpression::DoOperation
//
//  Purpose:
//      TODO
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************
 
void
avtCountReductionExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, ntuples);
        }
    }
}
