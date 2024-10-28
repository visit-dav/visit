// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtGhostAwareUnaryMathExpression.C              //
// ************************************************************************* //

#include <avtGhostAwareUnaryMathExpression.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

// ****************************************************************************
//  Method: avtGhostAwareUnaryMathExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   10/25/24
//
//  Modifications:
//
// ****************************************************************************

avtGhostAwareUnaryMathExpression::avtGhostAwareUnaryMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGhostAwareUnaryMathExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   10/25/24
//
// ****************************************************************************

avtGhostAwareUnaryMathExpression::~avtGhostAwareUnaryMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGhostAwareUnaryMathExpression::IdentifyGhostedNodes
//
//  Purpose:
//      This function determines which nodes ought to be used if we are
//      taking ghosts into account. We mark all nodes touching non-ghosted 
//      zones as good to count, and mark all nodes that are ghost nodes
//      as nodes that should not be counted.
// 
//  Returns:
//      A vector that is number of nodes long containing true when the node
//      should be ignored in calculations and false when the node should
//      not be ignored.
//
//  Programmer: Justin Privitera
//  Creation:   10/24/24
//
// ****************************************************************************

std::vector<int>
avtGhostAwareUnaryMathExpression::IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                       vtkDataArray *ghostZones,
                                                       vtkDataArray *ghostNodes)
{
    const int nPoints = in_ds->GetNumberOfPoints();

    // we create an array to track if this point should be counted
    std::vector<int> nodeShouldBeIgnored(nPoints);
    if (ghostZones)
    {
        // if there are ghost zones, we want to initialize all points to not being counted
        fill(nodeShouldBeIgnored.begin(), nodeShouldBeIgnored.end(), true);
    }
    else
    {
        // If there are ghost nodes and NOT ghost zones, we want to initialize all points
        // to being counted
        // Alternatively, we will hit this case if there are neither ghost zones nor ghost
        // nodes. In that case, we shouldn't even be in this function, but if we are here
        // we might as well say all the points are valid since they are.
        fill(nodeShouldBeIgnored.begin(), nodeShouldBeIgnored.end(), false);
    }

    if (ghostZones)
    {
        const int nCells = in_ds->GetNumberOfCells();
        // iterate through the cells and mark points that are touching non-ghosts
        // as points that should be counted
        for (int cellId = 0; cellId < nCells; cellId ++)
        {
            // if this zone is not a ghost zone
            if (0 == ghostZones->GetComponent(cellId, 0))
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
                        nodeShouldBeIgnored[pointId] = false;
                    }
                }
                ptIds->Delete();
            }
        }
    }

    if (ghostNodes)
    {
        // iterate through all points and make sure points marked as ghost
        // nodes are not counted
        for (int pointId = 0; pointId < nPoints; pointId ++)
        {
            // if this node is a ghost node
            if (0 != ghostNodes->GetComponent(pointId, 0))
            {
                nodeShouldBeIgnored[pointId] = true;
            }
        }
    }

    return nodeShouldBeIgnored;
}


// ****************************************************************************
//  Method: avtGhostAwareUnaryMathExpression::DoOperation
//
//  Purpose:
//      All ghost-aware unary expressions have the same underlying logic, which
//      we provide here. If we are operating on zonal data, we need to check
//      for ghost zones and only calculate using the non-ghosted zones. If we 
//      are operating on nodal data, we need to use IdentifyGhostedNodes() to
//      determine which nodes we can use. We then call CalculateWithGhosts() or
//      CalculateWithoutGhosts() which are defined in classes that inherit from
//      avtGhostAwareUnaryMathExpression. CalculateWithoutGhosts() is an 
//      optimized path that we can use when there are no ghosts we need to 
//      worry about.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//      in_ds         The input dataset.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

void
avtGhostAwareUnaryMathExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    vtkDataArray *ghostZones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghostNodes = in_ds->GetPointData()->GetArray("avtGhostNodes");
    int *nodeShouldBeIgnoredPtr = nullptr;

    if (AVT_ZONECENT == centering)
    {
        if (ghostZones)
        {
            // we pass a lambda to CalculateWithGhosts() that
            // looks at the ghostZones to determine if a cell
            // is valid and ignores the nodeShouldBeIgnoredPtr.
            CalculateWithGhosts(in, out, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return ghostZones->GetComponent(tuple_id, 0); },
                                ghostZones,
                                nodeShouldBeIgnoredPtr);
        }
        else // no ghosts or just ghost nodes
        {
            CalculateWithoutGhosts(in, out, ncomponents, ntuples);
        }
    }
    else // AVT_NODECENT == centering
    {
        // if we have any kind of ghosts
        if (ghostZones || ghostNodes)
        {
            // we need to identify which nodes should be ignored
            std::vector<int> nodeShouldBeIgnored = IdentifyGhostedNodes(
                in_ds, ghostZones, ghostNodes);
            nodeShouldBeIgnoredPtr = nodeShouldBeIgnored.data();

            // we pass a lambda to CalculateWithGhosts() that
            // looks at the nodeShouldBeIgnoredPtr to determine 
            // if a node is valid and ignores the ghostZones.
            CalculateWithGhosts(in, out, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return nodeShouldBeIgnoredPtr[tuple_id]; },
                                ghostZones,
                                nodeShouldBeIgnoredPtr);
        }
        else // no ghosts
        {
            CalculateWithoutGhosts(in, out, ncomponents, ntuples);
        }
    }
}


