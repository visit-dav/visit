// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtVarianceReductionExpression.C                          //
// ************************************************************************* //

#include <avtVarianceReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtVarianceReductionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtVarianceReductionExpression::avtVarianceReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVarianceReductionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtVarianceReductionExpression::~avtVarianceReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVarianceReductionExpression::DoOperation
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
avtVarianceReductionExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        const double mean = [&]()
        {
            double sum = 0;
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                sum += val;
            }
            return (ntuples > 0) ? sum / static_cast<double>(ntuples) : 0;
        }();
        
        const double intermediate_sum = [&]()
        {
            double intermediate_sum = 0;
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                intermediate_sum += pow(val - mean, 2);
            }
            return intermediate_sum;
        }();

        const double variance = intermediate_sum / static_cast<double>(ntuples);

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, variance);
        }
    }
}
