// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPosCMFEExpression.C                          //
// ************************************************************************* //

#include <avtPosCMFEExpression.h>

#include <avtPosCMFEAlgorithm.h>


// ****************************************************************************
//  Method: avtPosCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::avtPosCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPosCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::~avtPosCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPosCMFEExpression::PerformCMFE
//
//  Purpose:
//      Performs a cross-mesh field evaluation based on position-based 
//      differencing.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
// 
//  Modifications:
//
//    Hank Childs, Sat Jan 21 12:56:57 PST 2006
//    Re-wrote to use avtPosCMFEAlgorithm.
//
//    Hank Childs, Tue Mar 13 11:16:24 PDT 2012
//    Return early if we were given a SIL with no data.  
//
//    Hank Childs, Wed Mar 14 08:47:56 PDT 2012
//    Determine if we should run in a "serial-only" mode.
//
//    David Camp, Wed Mar 15 08:47:56 PDT 2012
//    Change the emtpy data to return an avtDataTree_p object.
//
// ****************************************************************************

avtDataTree_p
avtPosCMFEExpression::PerformCMFE(avtDataTree_p output_mesh,
                                  avtDataTree_p mesh_to_be_sampled,
                                  const std::string &invar,
                                  const std::string &outvar)
{
    if (!onDemandProcessing && !initialSILHasData)
    {
        // This is empty data, just return an empty tree.
        avtDataTree_p rv = new avtDataTree();
        return rv;
    }

    bool serialOnly = onDemandProcessing;
    return avtPosCMFEAlgorithm::PerformCMFE(output_mesh, mesh_to_be_sampled,
                                            invar, varnames[1], outvar,
                                            serialOnly);
}


// ****************************************************************************
//  Method:  avtPosCMFEAlgorithm::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
//  Note:
//    Report explicitly that this filter does NOT understand transformed
//    rectilinear grids.  This method should default to returning false
//    anyway, but there are specific reasons this filter cannot yet be
//    optimized in this fashion, so ensure that even if other CMFE's change
//    to default to true, this one remains false until it can be fixed.
//
// ****************************************************************************

bool
avtPosCMFEExpression::FilterUnderstandsTransformedRectMesh()
{
    // This avtPosCMFEAlgorithm is not optimized for transformed
    // rectilinear grids.  Furthermore, it calls a form of
    // avtDatasetExaminer::GetSpatialExtents that also does not
    // understand these transformed rectilinear grids.  These
    // both must be fixed before this can return 'true'.
    return false;
}
