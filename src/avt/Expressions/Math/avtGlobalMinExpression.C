// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalMinExpression.C                    //
// ************************************************************************* //

#include <avtGlobalMinExpression.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtGlobalMinExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtGlobalMinExpression::avtGlobalMinExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGlobalMinExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtGlobalMinExpression::~avtGlobalMinExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGlobalMinExpression::CalculateWithoutGhosts
//
//  Purpose:
//      We provide a simple calculation in the case that we don't need to worry
//      about ghosts.
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
avtGlobalMinExpression::CalculateWithoutGhosts(vtkDataArray *in, 
                                               vtkDataArray *out,
                                               int ncomponents,
                                               int ntuples)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double comp_min = in->GetComponent(0, comp_id);
        // start at 1 since we already looked at the 0th element
        for (int tuple_id = 1; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            if (val < comp_min)
            {
                comp_min = val;
            }
        }

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_min);
        }
    }
}


// ****************************************************************************
//  Method: avtGlobalMinExpression::CalculateWithGhosts
//
//  Purpose:
//      We provide a more complicated calculation that takes ghost data into account.
//      The way this works is it takes a function called getPointValid() that is 
//      defined based on if we are working with zonal or nodal data. getPointValid()
//      itself takes two pointers and an index called tuple_id.
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
avtGlobalMinExpression::CalculateWithGhosts(vtkDataArray *in,
                                            vtkDataArray *out,
                                            int ncomponents,
                                            int ntuples,
                                            int (getPointValid)(vtkDataArray *, int *, int),
                                            vtkDataArray *ghostZones,
                                            int *nodeShouldBeIgnoredPtr)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        int start_tuple_id = 0;
        double comp_min = [&]() -> double
        {
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                if (0 == getPointValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
                {
                    start_tuple_id = tuple_id + 1;
                    return in->GetComponent(tuple_id, comp_id);
                }
            }
            EXCEPTION2(ExpressionException, outputVariableName,
                 "Everything is ghosted so the global_min expression is not valid.");
            return 0; // return so the compiler is happy
        }();

        // start at start_tuple_id since it is the second non-ghosted tuple and we
        // have already looked at the first.
        for (int tuple_id = start_tuple_id; tuple_id < ntuples; tuple_id ++)
        {
            if (0 == getPointValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                if (val < comp_min)
                {
                    comp_min = val;
                }
            }
        }

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_min);
        }
    }
}

