// ************************************************************************* //
//                      avtOriginalDataNumNodesQuery.C                       //
// ************************************************************************* //

#include <avtOriginalDataNumNodesQuery.h>
#include <avtTerminatingSource.h>


// ****************************************************************************
//  Method: avtOriginalDataNumNodesQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtOriginalDataNumNodesQuery::avtOriginalDataNumNodesQuery() : avtNumNodesQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataNumNodesQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
avtOriginalDataNumNodesQuery::~avtOriginalDataNumNodesQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataNumNodesQuery::ApplyFilters
//
//  Purpose:
//    Makes sure the entire SIL is used. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataNumNodesQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataSpecification_p dspec = inData->GetTerminatingSource()->
        GetGeneralPipelineSpecification()->GetDataSpecification();
    avtPipelineSpecification_p pspec = 
        new avtPipelineSpecification(dspec, queryAtts.GetPipeIndex()); 

    avtDataObject_p retObj;
    CopyTo(retObj, inData);
    retObj->Update(pspec);
    return retObj;
}

