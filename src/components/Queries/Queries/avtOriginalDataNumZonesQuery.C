// ************************************************************************* //
//                      avtOriginalDataNumZonesQuery.C                       //
// ************************************************************************* //

#include <avtOriginalDataNumZonesQuery.h>

#include <avtTerminatingSource.h>
#include <ParsingExprList.h>


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
//    Kathleen Bonnell, Mon Mar  7 18:09:14 PST 2005
//    Ensure that the correct DB variable is used.
//
//    Hank Childs, Fri Sep 23 10:53:37 PDT 2005
//    Data spec's DBVariable has been deprecated.  Use a different path.
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataNumZonesQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataSpecification_p dspec = inData->GetTerminatingSource()->
        GetGeneralPipelineSpecification()->GetDataSpecification();

    string dbVar = ParsingExprList::GetRealVariable(
                       queryAtts.GetVariables()[0]);
    avtDataSpecification_p new_dspec = new avtDataSpecification(dspec,
                                                                dbVar.c_str());

    avtPipelineSpecification_p pspec = 
        new avtPipelineSpecification(new_dspec, queryAtts.GetPipeIndex()); 

    avtDataObject_p retObj;
    CopyTo(retObj, inData);
    retObj->Update(pspec);
    return retObj;
}

