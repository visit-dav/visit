// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMaxReductionExpression.C                          //
// ************************************************************************* //

#include <avtMaxReductionExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


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
    // const int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
    // bool has_ghosts = false;
    // for (int cellArrayId = 0; cellArrayId < ncellArray; cellArrayId ++)
    // {
    //     vtkDataArray *currArray = in_ds->GetCellData()->GetArray(cellArrayId);
    //     const std::string arrayName = currArray->GetName();
    //     if (arrayName == "avtGhostZones")
    //     {
    //         has_ghosts = true;
    //         break;
    //     }
    // }

    // vtkDataSet *filtered_ds = nullptr;
    // vtkDataSetRemoveGhostCells *ghost_filters = nullptr;
    
    // // filter out any ghost cells
    // if (has_ghosts)
    // {
    //     ghost_filters = vtkDataSetRemoveGhostCells::New();
    //     ghost_filters->SetInputData(in_ds);
    //     ghost_filters->Update();
    //     filtered_ds = ghost_filters->GetOutput();
    // }
    // else
    // {
    //     filtered_ds = in_ds;
    // }

    // if (has_ghosts)
    // {
    //     ghost_filters->Delete();
    // }



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

        std::cout << comp_max << std::endl;

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_max);
        }
    }
}
