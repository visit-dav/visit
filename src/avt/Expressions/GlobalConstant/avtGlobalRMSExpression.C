// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalRMSExpression.C                    //
// ************************************************************************* //

#include <avtGlobalRMSExpression.h>
#include <avtParallel.h>

#include <vtkDataArray.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif


// ****************************************************************************
//  Method: avtGlobalRMSExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtGlobalRMSExpression::avtGlobalRMSExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGlobalRMSExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
// ****************************************************************************

avtGlobalRMSExpression::~avtGlobalRMSExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtGlobalRMSExpression::CalculateWithoutGhosts
//
//  Purpose:
//      We provide a simple calculation in the case that we don't need to worry
//      about ghosts.
//
//  Arguments:
//      in                     The input data array.
//      ncomponents            The number of components ('1' for scalar, '2' or 
//                             '3' for vectors, etc.)
//      ntuples                The number of tuples (ie 'npoints' or 'ncells')
//      constant_results       A vector to store per-component results.
//      extra_constant_results A vector to store additional per-component
//                             results.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//      Justin Privitera, Wed Oct  8 14:13:21 PDT 2025
//      Removed the output data array parameter.
//      Added constant_results and extra_constant_results parameters.
//      We now store the sum(x^2) result in the constant_results vector.
//
// ****************************************************************************

void
avtGlobalRMSExpression::CalculateWithoutGhosts(vtkDataArray *in, 
                                               const int ncomponents,
                                               const int ntuples,
                                               std::vector<double> &constant_results,
                                               std::vector<double> &extra_constant_results)
{
    (void) extra_constant_results;

    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double sum_of_squares = 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            sum_of_squares += val * val;
        }

        constant_results[comp_id] = sum_of_squares;
    }
}


// ****************************************************************************
//  Method: avtGlobalRMSExpression::CalculateWithGhosts
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
avtGlobalRMSExpression::CalculateWithGhosts(vtkDataArray *in,
                                            const int ncomponents,
                                            const int ntuples,
                                            int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                            vtkDataArray *ghostZones,
                                            int *nodeShouldBeIgnoredPtr,
                                            std::vector<double> &constant_results,
                                            std::vector<double> &extra_constant_results)
{
    (void) extra_constant_results;

    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double sum_of_squares = 0;
        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                sum_of_squares += val * val;
            }
        }

        constant_results[comp_id] = sum_of_squares;
    }
}


// ****************************************************************************
//  Method: avtGlobalRMSExpression::LocalIntermediateReduction
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
double
avtGlobalRMSExpression::LocalIntermediateReduction(const double running_reduction,
                                                   const double intermediate_value)
{
    return running_reduction + intermediate_value;
}


// ****************************************************************************
//  Method: avtGlobalRMSExpression::GlobalIntermediateReduction
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
void
avtGlobalRMSExpression::GlobalIntermediateReduction(std::vector<double> &local_constant_results,
                                                    std::vector<double> &global_constant_results,
                                                    const int ncomps)
{
#ifdef PARALLEL
    MPI_Allreduce(local_constant_results.data(), global_constant_results.data(),
                  ncomps, MPI_DOUBLE, MPI_SUM, VISIT_MPI_COMM);
#else
    (void) local_constant_results;
    (void) global_constant_results;
    (void) ncomps;
#endif
}


// ****************************************************************************
//  Method: avtGlobalRMSExpression::CalculateFinalResults
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************

void
avtGlobalRMSExpression::CalculateFinalResults(const std::vector<double> &global_constant_results,
                                              const std::vector<double> &global_extra_constant_results,
                                              const int global_ncomps,
                                              const int global_ntuples,
                                              std::vector<double> &final_results)
{
    // we didn't use this to get our final answer
    (void) global_extra_constant_results;

    // we need to divide each component sum of squares by the global number of 
    // non ghosted tuples and take the square root
    final_results.resize(global_ncomps);
    // TODO this case doesn't make any sense
    if (global_ntuples == 0)
    {
        std::fill(final_results.begin(), final_results.end(), 0.0);
    }
    else
    {
        const double ntuples_double = static_cast<double>(global_ntuples);
        for (int comp_id = 0; comp_id < global_ncomps; comp_id ++)
        {
            final_results[comp_id] = sqrt(global_constant_results[comp_id] / ntuples_double);
        }
    }
}



