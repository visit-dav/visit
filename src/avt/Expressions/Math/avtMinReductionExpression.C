// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMinReductionExpression.C                          //
// ************************************************************************* //

#include <avtMinReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtMinReductionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtMinReductionExpression::avtMinReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMinReductionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtMinReductionExpression::~avtMinReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMinReductionExpression::DoOperation
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
avtMinReductionExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    std::vector<double> comp_mins(ncomponents);
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double comp_min = in->GetComponent(0, comp_id);
        for (int tuple_id = 1; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            if (val < comp_min)
            {
                comp_min = val;
            }
        }
        comp_mins[comp_id] = comp_min;
    }

    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double &comp_min = comp_mins[comp_id];
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_min);
        }
    }
}


