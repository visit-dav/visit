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
    bool has_ghosts = false;
    vtkDataArray *ghost_data = nullptr;

    if (centering == AVT_ZONECENT)
    {
        const int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
        for (int cellArrayId = 0; cellArrayId < ncellArray; cellArrayId ++)
        {
            vtkDataArray *currArray = in_ds->GetCellData()->GetArray(cellArrayId);
            const std::string arrayName = currArray->GetName();
            if (arrayName == "avtGhostZones")
            {
                has_ghosts = true;
                ghost_data = currArray;
                break;
            }
        }
    }
    else // node centered
    {
        const int npointArray = in_ds->GetPointData()->GetNumberOfArrays();
        for (int pointArrayId = 0; pointArrayId < npointArray; pointArrayId ++)
        {
            vtkDataArray *currArray = in_ds->GetPointData()->GetArray(pointArrayId);
            const std::string arrayName = currArray->GetName();
            if (arrayName == "avtGhostNodes")
            {
                has_ghosts = true;
                ghost_data = currArray;
                break;
            }
        }
    }

    // TODO
    // iterate through all cells and mark cells that are touching non-ghost zones as good to count
    // need to figure out which zone a node belongs to
    //  |----> how to do this? -> rv->GetCellPoints(i, nCellPts, cellPts);
    //      cellpts are indices into node arrays
    // if that node belongs to only zones that are ghosted then do not count it
    // also ignore things in ghost node array

    if (has_ghosts)
    {
        for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
        {
            bool init_max_set = false;
            double comp_max;
            int start_tuple_id = 0;
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                const int ghost = ghost_data->GetComponent(tuple_id, 0);
                if (0 == ghost)
                {
                    comp_max = in->GetComponent(tuple_id, comp_id);
                    start_tuple_id = tuple_id;
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
                const int ghost = ghost_data->GetComponent(tuple_id, 0);
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
    else
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
