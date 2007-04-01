// ************************************************************************* //
//                      avtOriginalDataNumNodesQuery.C                       //
// ************************************************************************* //

#include <avtOriginalDataNumNodesQuery.h>
#include <avtTerminatingSource.h>
#include <ParsingExprList.h>


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
//    Kathleen Bonnell, Mon Mar  7 18:09:14 PST 2005
//    Ensure that the correct DB variable is used.
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataNumNodesQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataSpecification_p dspec = inData->GetTerminatingSource()->
        GetGeneralPipelineSpecification()->GetDataSpecification();

    string dbVar = ParsingExprList::GetRealVariable(
                       queryAtts.GetVariables()[0]);
    dspec->SetDBVariable(dbVar.c_str());

    avtPipelineSpecification_p pspec = 
        new avtPipelineSpecification(dspec, queryAtts.GetPipeIndex()); 

    avtDataObject_p retObj;
    CopyTo(retObj, inData);
    retObj->Update(pspec);
    return retObj;
}

