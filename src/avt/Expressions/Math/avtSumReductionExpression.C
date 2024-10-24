// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSumReductionExpression.C                          //
// ************************************************************************* //

#include <avtSumReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>


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
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    vtkDataArray *ghost_zones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghost_nodes = in_ds->GetPointData()->GetArray("avtGhostNodes");
    int *pointShouldBeIgnoredPtr = nullptr;

    auto calculate_with_ghosts = [&](int (get_point_valid)(vtkDataArray *, int *, int))
    {
        for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
        {
            double sum = 0;
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                if (0 == get_point_valid(ghost_zones, pointShouldBeIgnoredPtr, tuple_id))
                {
                    const double val = in->GetComponent(tuple_id, comp_id);
                    sum += val;
                }
            }

            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                out->SetComponent(tuple_id, comp_id, sum);
            }
        }
    };

    auto calculate_without_ghosts = [&]()
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
    };

    if (AVT_ZONECENT == centering)
    {
        if (ghost_zones)
        {
            calculate_with_ghosts([](vtkDataArray *ghost_zones,
                                     int *pointShouldBeIgnoredPtr,
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
        if (ghost_zones || ghost_nodes)
        {
            std::vector<int> pointShouldBeIgnored = IdentifyGhostedNodes(
                in_ds, ghost_zones, ghost_nodes);
            pointShouldBeIgnoredPtr = pointShouldBeIgnored.data();
            calculate_with_ghosts([](vtkDataArray *ghost_zones,
                                     int *pointShouldBeIgnoredPtr,
                                     int tuple_id) -> int 
                { return pointShouldBeIgnoredPtr[tuple_id]; });
        }
        else // !ghost_zones && !ghost_nodes
        {
            calculate_without_ghosts();
        }
    }
}
