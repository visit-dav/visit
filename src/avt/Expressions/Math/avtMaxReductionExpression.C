// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMaxReductionExpression.C                          //
// ************************************************************************* //

#include <avtMaxReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

#include <ImproperUseException.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtMaxReductionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtMaxReductionExpression::avtMaxReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMaxReductionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtMaxReductionExpression::~avtMaxReductionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMaxReductionExpression::DoOperation
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
avtMaxReductionExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    vtkDataArray *ghost_zones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghost_nodes = in_ds->GetPointData()->GetArray("avtGhostNodes");

    if (AVT_ZONECENT == centering)
    {
        if (ghost_zones)
        {
            for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
            {
                bool init_max_set = false;
                double comp_max;
                int start_tuple_id = 0;
                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    const int ghost = ghost_zones->GetComponent(tuple_id, 0);
                    if (0 == ghost)
                    {
                        comp_max = in->GetComponent(tuple_id, comp_id);
                        start_tuple_id = tuple_id + 1;
                        init_max_set = true;
                        break;
                    }
                }

                if (!init_max_set)
                {
                    EXCEPTION2(ExpressionException, outputVariableName,
                         "BAD");
                }

                for (int tuple_id = start_tuple_id; tuple_id < ntuples; tuple_id ++)
                {
                    const int ghost = ghost_zones->GetComponent(tuple_id, 0);
                    if (0 == ghost)
                    {
                        const double val = in->GetComponent(tuple_id, comp_id);
                        if (val > comp_max)
                        {
                            comp_max = val;
                        }
                    }
                }

                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    out->SetComponent(tuple_id, comp_id, comp_max);
                }
            }
        }
        else // no ghosts
        {
            for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
            {
                double comp_max = in->GetComponent(0, comp_id);
                for (int tuple_id = 1; tuple_id < ntuples; tuple_id ++)
                {
                    const double val = in->GetComponent(tuple_id, comp_id);
                    if (val > comp_max)
                    {
                        comp_max = val;
                    }
                }

                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    out->SetComponent(tuple_id, comp_id, comp_max);
                }
            }
        }
    }
    else // AVT_NODECENT == centering
    {
        if (ghost_zones)
        {
            const int nCells = in_ds->GetNumberOfCells();
            const int nPoints = in_ds->GetNumberOfPoints();
            
            // we create an array to track if this point should be counted
            std::vector<int> pointShouldBeCounted(nPoints, false);

            // iterate through the cells and mark points that are touching non-ghosts
            // as points that should be counted
            for (int cellId = 0; cellId < nCells; cellId ++)
            {
                // if this zone is not a ghost zone
                if (0 == ghost_zones->GetComponent(cellId, 0))
                {
                    vtkIdType numCellPoints = 0;
#if LIB_VERSION_LE(VTK,8,1,0)
                    vtkIdType *cellPoints = NULL;
#else
                    const vtkIdType *cellPoints = nullptr;
#endif
                    vtkIdList *ptIds = vtkIdList::New();
                    // we get the points for this zone
                    in_ds->GetCellPoints(cellId, numCellPoints, cellPoints, ptIds);

                    // and mark them as valid points
                    if (numCellPoints && cellPoints)
                    {
                        for (int cellPointId = 0; cellPointId < numCellPoints; cellPointId ++)
                        {
                            const int pointId = cellPoints[cellPointId];
                            pointShouldBeCounted[pointId] = true;
                        }
                    }
                    ptIds->Delete();
                }
            }

            if (ghost_nodes)
            {
                // iterate through all points and make sure points marked as ghost
                // nodes are not counted
                for (int pointId = 0; pointId < nPoints; pointId ++)
                {
                    // if this node is a ghost node
                    if (0 != ghost_nodes->GetComponent(pointId, 0))
                    {
                        pointShouldBeCounted[pointId] = false;
                    }
                }
            }

            // now we can compute our max
            for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
            {
                bool init_max_set = false;
                double comp_max;
                int start_tuple_id = 0;
                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    if (pointShouldBeCounted[tuple_id])
                    {
                        comp_max = in->GetComponent(tuple_id, comp_id);
                        start_tuple_id = tuple_id + 1;
                        init_max_set = true;
                        break;
                    }
                }

                if (!init_max_set)
                {
                    EXCEPTION2(ExpressionException, outputVariableName,
                         "BAD");
                }

                for (int tuple_id = start_tuple_id; tuple_id < ntuples; tuple_id ++)
                {
                    if (pointShouldBeCounted[tuple_id])
                    {
                        const double val = in->GetComponent(tuple_id, comp_id);
                        if (val > comp_max)
                        {
                            comp_max = val;
                        }
                    }
                }

                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    out->SetComponent(tuple_id, comp_id, comp_max);
                }
            }
        }
        else if (!ghost_zones && ghost_nodes)
        {
            const int nCells = in_ds->GetNumberOfCells();
            const int nPoints = in_ds->GetNumberOfPoints();
            
            // we create an array to track if this point should be counted
            std::vector<int> pointShouldBeCounted(nPoints, true);

            // iterate through all points and make sure points marked as ghost
            // nodes are not counted
            for (int pointId = 0; pointId < nPoints; pointId ++)
            {
                // if this node is a ghost node
                if (0 != ghost_nodes->GetComponent(pointId, 0))
                {
                    pointShouldBeCounted[pointId] = false;
                }
            }

            // now we can compute our max
            for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
            {
                bool init_max_set = false;
                double comp_max;
                int start_tuple_id = 0;
                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    if (pointShouldBeCounted[tuple_id])
                    {
                        comp_max = in->GetComponent(tuple_id, comp_id);
                        start_tuple_id = tuple_id + 1;
                        init_max_set = true;
                        break;
                    }
                }

                if (!init_max_set)
                {
                    EXCEPTION2(ExpressionException, outputVariableName,
                         "BAD");
                }

                for (int tuple_id = start_tuple_id; tuple_id < ntuples; tuple_id ++)
                {
                    if (pointShouldBeCounted[tuple_id])
                    {
                        const double val = in->GetComponent(tuple_id, comp_id);
                        if (val > comp_max)
                        {
                            comp_max = val;
                        }
                    }
                }

                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    out->SetComponent(tuple_id, comp_id, comp_max);
                }
            }
        }
        else // !ghost_zones && !ghost_nodes
        {
            for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
            {
                double comp_max = in->GetComponent(0, comp_id);
                for (int tuple_id = 1; tuple_id < ntuples; tuple_id ++)
                {
                    const double val = in->GetComponent(tuple_id, comp_id);
                    if (val > comp_max)
                    {
                        comp_max = val;
                    }
                }

                for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                {
                    out->SetComponent(tuple_id, comp_id, comp_max);
                }
            }
        }
    }
}
