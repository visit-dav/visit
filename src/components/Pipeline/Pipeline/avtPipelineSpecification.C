// ************************************************************************* //
//                          avtPipelineSpecification.C                       //
// ************************************************************************* //

#include <avtPipelineSpecification.h>


// ****************************************************************************
//  Method: avtPipelineSpecification constructor
//
//  Arguments:
//      d        The data specification for this pipeline.
//      pi       The index of the pipeline.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 17:18:19 PDT 2001
//    Initialize nFilters.
//
//    Hank Childs, Thu Mar  3 16:36:50 PST 2005
//    Don't share a reference to the data specification, because we may modify
//    it.
//
//    Hank Childs, Sun Mar 13 09:49:16 PST 2005
//    Initialize haveStructuredMeshOptimizations.
//
// ****************************************************************************

avtPipelineSpecification::avtPipelineSpecification(avtDataSpecification_p d,
                                                   int pi)
{
    data             = new avtDataSpecification(d);
    pipelineIndex    = pi;
    canDoDynamic     = true;
    useLoadBalancing = true;
    nFilters         = 0;
    haveCurvilinearMeshOptimizations = false;
    haveRectilinearMeshOptimizations = false;
}


// ****************************************************************************
//  Method: avtPipelineSpecification copy constructor
//
//  Arguments:
//      ps       A pipeline specification to copy.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
// ****************************************************************************

avtPipelineSpecification::avtPipelineSpecification(
                                                 avtPipelineSpecification_p ps)
{
    *this = **ps;
}


// ****************************************************************************
//  Method: avtPipelineSpecification copy constructor
//
//  Arguments:
//      ps       A pipeline specification to copy.
//
//  Programmer:  Hank Childs
//  Creation:    June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Mar  3 16:36:50 PST 2005
//    Don't share a reference to the data specification, because we may modify
//    it.
//
// ****************************************************************************

avtPipelineSpecification::avtPipelineSpecification(
                      avtPipelineSpecification_p ps, avtDataSpecification_p ds)
{
    *this = **ps;
    data  = new avtDataSpecification(ds);
}


// ****************************************************************************
//  Method: avtPipelineSpecification destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPipelineSpecification::~avtPipelineSpecification()
{
    ;
}


// ****************************************************************************
//  Method: avtPipelineSpecification assignment operator
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 17:30:28 PDT 2001
//    Add nFilters.
//
//    Hank Childs, Thu Mar  3 16:36:50 PST 2005
//    Don't share a reference to the data specification, because we may modify
//    it.
//
//    Hank Childs, Sun Mar 13 09:49:16 PST 2005
//    Add haveStructuredMeshOptimizations.
//
// ****************************************************************************

avtPipelineSpecification &
avtPipelineSpecification::operator=(avtPipelineSpecification &ps)
{
    data             = new avtDataSpecification(ps.data);
    pipelineIndex    = ps.pipelineIndex;
    canDoDynamic     = ps.canDoDynamic;
    useLoadBalancing = ps.useLoadBalancing;
    nFilters         = ps.nFilters;
    haveCurvilinearMeshOptimizations = ps.haveCurvilinearMeshOptimizations;
    haveRectilinearMeshOptimizations = ps.haveRectilinearMeshOptimizations;

    return *this;
}


// ****************************************************************************
//  Method: avtPipelineSpecification::UseLoadBalancing
//
//  Purpose:
//      Allows load balancing to be turned off for situations like when you
//      know you are in serial (on the viewer), or if you are in the Execute
//      portion of a filter and want to set up a pipeline inside of it and
//      don't want any further load balancing to take place.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

void
avtPipelineSpecification::UseLoadBalancing(bool newVal)
{
    useLoadBalancing = newVal;
}


