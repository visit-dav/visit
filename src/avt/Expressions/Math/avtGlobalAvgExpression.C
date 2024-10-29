// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalAvgExpression.C                    //
// ************************************************************************* //

#include <avtGlobalAvgExpression.h>

#include <vtkDataArray.h>


// ****************************************************************************
//  Method: avtGlobalAvgExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtGlobalAvgExpression::avtGlobalAvgExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGlobalAvgExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtGlobalAvgExpression::~avtGlobalAvgExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGlobalAvgExpression::CalculateWithoutGhosts
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
avtGlobalAvgExpression::CalculateWithoutGhosts(vtkDataArray *in, 
                                               vtkDataArray *out,
                                               int ncomponents,
                                               int ntuples)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double sum = 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            sum += val;
        }

        const double comp_avg = (ntuples > 0) ? sum / static_cast<double>(ntuples) : 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_avg);
        }
    }
}


// ****************************************************************************
//  Method: avtGlobalAvgExpression::CalculateWithGhosts
//
//  Purpose:
//      We provide a more complicated calculation that takes ghost data into account.
//      The way this works is it takes a function called getNodeOrCellValid() that is 
//      defined based on if we are working with zonal or nodal data. getNodeOrCellValid()
//      itself takes two pointers and an index called tuple_id.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//      getNodeOrCellValid A function that takes a vtkDataArray*, and int*, and
//                    an int, and returns an int. This function is used to test
//                    if a given zone or node is ghosted out. A value of zero
//                    means it should be counted, which was chosen because
//                    a ghost value of zero means no the zone or node is not
//                    ghosted. For zonal variables, the caller passes a 
//                    getNodeOrCellValid function that examines the ghost zones
//                    vtkDataArray array. For nodal variables, the caller 
//                    passes a getNodeOrCellValid function that examines the
//                    nodeShouldBeIgnoredPtr, which has an entry for each node.
//      ghostZones    A pointer to the ghost zones array. Could be null.
//      nodeShouldBeIgnoredPtr A pointer to an array we constructed that 
//                    records if each node is ghosted out or not.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

void
avtGlobalAvgExpression::CalculateWithGhosts(vtkDataArray *in,
                                            vtkDataArray *out,
                                            int ncomponents,
                                            int ntuples,
                                            int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                            vtkDataArray *ghostZones,
                                            int *nodeShouldBeIgnoredPtr)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double sum = 0;
        int num_valid_tuples = 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                sum += val;
                num_valid_tuples ++;
            }
        }

        const double comp_avg = (num_valid_tuples > 0) ? sum / static_cast<double>(num_valid_tuples) : 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_avg);
        }
    }
}

