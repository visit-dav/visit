// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalMinExpression.C                    //
// ************************************************************************* //

#include <avtGlobalMinExpression.h>
#include <avtParallel.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif


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
                                               const int ncomponents,
                                               const int ntuples,
                                               std::vector<double> &constant_results,
                                               std::vector<double> &extra_constant_results,
                                               std::vector<double> &sums)
{
    (void) extra_constant_results;
    (void) sums;

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

        constant_results[comp_id] = comp_min;
    }
}


// ****************************************************************************
//  Method: avtGlobalMinExpression::CalculateWithGhosts
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
avtGlobalMinExpression::CalculateWithGhosts(vtkDataArray *in,
                                            const int ncomponents,
                                            const int ntuples,
                                            int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                            vtkDataArray *ghostZones,
                                            int *nodeShouldBeIgnoredPtr,
                                            std::vector<double> &constant_results,
                                            std::vector<double> &extra_constant_results,
                                            std::vector<double> &sums)
{
    (void) extra_constant_results;
    (void) sums;

    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        int start_tuple_id = 0;
        double comp_min = [&]() -> double
        {
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
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
            if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                if (val < comp_min)
                {
                    comp_min = val;
                }
            }
        }

        constant_results[comp_id] = comp_min;
    }
}


// ****************************************************************************
//  Method: avtGlobalMinExpression::LocalIntermediateReduction
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
avtGlobalMinExpression::LocalIntermediateReduction(const double running_reduction,
                                                   const double intermediate_value)
{
    return std::min(running_reduction, intermediate_value);
}


// ****************************************************************************
//  Method: avtGlobalMinExpression::GlobalIntermediateReduction
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
avtGlobalMinExpression::GlobalIntermediateReduction(std::vector<double> &local_constant_results,
                                                    std::vector<double> &global_constant_results,
                                                    const int ncomps)
{
#ifdef PARALLEL
    MPI_Allreduce(local_constant_results.data(), global_constant_results.data(),
                  ncomps, MPI_DOUBLE, MPI_MIN, VISIT_MPI_COMM);
#else
    (void) local_constant_results;
    (void) global_constant_results;
    (void) ncomps;
#endif
}


// ****************************************************************************
//  Method: avtGlobalMinExpression::CalculateFinalResults
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
avtGlobalMinExpression::CalculateFinalResults(const std::vector<double> &global_constant_results,
                                              const std::vector<double> &global_extra_constant_results,
                                              const std::vector<double> &global_component_sums,
                                              const int global_ncomps,
                                              const int global_ntuples,
                                              std::vector<double> &final_results)
{
    // we didn't use any of these to get our final answer
    (void) global_extra_constant_results;
    (void) global_component_sums;
    (void) global_ntuples;
    (void) global_ncomps;

    // we want to avoid copying the vector
    final_results = std::move(global_constant_results);
}



