// ************************************************************************* //
//                      avtOriginalDataNumZonesQuery.C                       //
// ************************************************************************* //

#include <avtOriginalDataNumZonesQuery.h>

#include <avtTerminatingSource.h>


// ****************************************************************************
//  Method: avtOriginalDataNumZonesQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtOriginalDataNumZonesQuery::avtOriginalDataNumZonesQuery() : avtNumZonesQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataNumZonesQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
avtOriginalDataNumZonesQuery::~avtOriginalDataNumZonesQuery() 
{
}


// ****************************************************************************
//  Method: avtOriginalDataNumZonesQuery::ApplyFilters
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
avtOriginalDataNumZonesQuery::ApplyFilters(avtDataObject_p inData)
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

