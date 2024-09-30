// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSumReductionExpression.C                          //
// ************************************************************************* //

#include <avtSumReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtSumReductionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtSumReductionExpression::avtSumReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSumReductionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtSumReductionExpression::~avtSumReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSumReductionExpression::DoOperation
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
avtSumReductionExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double sum = 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            sum += val;
        }
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, sum);
        }
    }
}
