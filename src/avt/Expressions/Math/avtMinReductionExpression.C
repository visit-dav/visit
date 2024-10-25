// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMinReductionExpression.C                 //
// ************************************************************************* //

#include <avtMinReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


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
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
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

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_min);
        }
    }

    vtkDataArray *ghost_zones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghost_nodes = in_ds->GetPointData()->GetArray("avtGhostNodes");
    int *nodeShouldBeIgnoredPtr = nullptr;

    // We provide a simple calculation in the case that we don't need to worry
    // about ghosts.
    auto calculate_without_ghosts = [&]()
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
    };

    // We provide a more complicated calculation that takes ghost data into account.
    // The way this works is it takes a function called get_point_valid() that is 
    // defined based on if we are working with zonal or nodal data. get_point_valid()
    // itself takes two pointers and an index called tuple_id.
    auto calculate_with_ghosts = [&](int (get_point_valid)(vtkDataArray *, int *, int))
    {
        for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
        {
            int start_tuple_id = 0;
            double comp_min = [&]() -> double
            {
                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    if (0 == get_point_valid(ghost_zones, nodeShouldBeIgnoredPtr, tuple_id))
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
                if (0 == get_point_valid(ghost_zones, nodeShouldBeIgnoredPtr, tuple_id))
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
    };

    if (AVT_ZONECENT == centering)
    {
        if (ghost_zones)
        {
            // we pass a lambda to calculate_with_ghosts() that
            // looks at the ghost_zones to determine if a cell
            // is valid and ignores the nodeShouldBeIgnoredPtr.
            calculate_with_ghosts([](vtkDataArray *ghost_zones,
                                     int *nodeShouldBeIgnoredPtr,
                                     int tuple_id) -> int 
                { return ghost_zones->GetComponent(tuple_id, 0); });
        }
        else // no ghosts or just ghost nodes
        {
            calculate_without_ghosts();
        }
    }
    else // AVT_NODECENT == centering
    {
        // if we have any kind of ghosts
        if (ghost_zones || ghost_nodes)
        {
            // we need to identify which nodes should be ignored
            std::vector<int> nodeShouldBeIgnored = IdentifyGhostedNodes(
                in_ds, ghost_zones, ghost_nodes);
            nodeShouldBeIgnoredPtr = nodeShouldBeIgnored.data();

            // we pass a lambda to calculate_with_ghosts() that
            // looks at the nodeShouldBeIgnoredPtr to determine 
            // if a node is valid and ignores the ghost_zones.
            calculate_with_ghosts([](vtkDataArray *ghost_zones,
                                     int *nodeShouldBeIgnoredPtr,
                                     int tuple_id) -> int 
                { return nodeShouldBeIgnoredPtr[tuple_id]; });
        }
        else // no ghosts
        {
            calculate_without_ghosts();
        }
    }
}
