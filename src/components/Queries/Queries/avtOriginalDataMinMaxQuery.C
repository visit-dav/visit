// ************************************************************************* //
//                      avtOriginalDataMinMaxQuery.C                         //
// ************************************************************************* //

#include <avtOriginalDataMinMaxQuery.h>

#include <avtExpressionEvaluatorFilter.h>
#include <avtTerminatingSource.h>



// ****************************************************************************
//  Method: avtOriginalDataMinMaxQuery::avtOriginalDataMinMaxQuery
//
//  Purpose:
//      Construct an avtOriginalDataMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     February 10, 2004
//
//  Modifications:
//
// ****************************************************************************

avtOriginalDataMinMaxQuery::avtOriginalDataMinMaxQuery()
{
    eef = new avtExpressionEvaluatorFilter;
}


// ****************************************************************************
//  Method: avtOriginalDataMinMaxQuery::~avtOriginalDataMinMaxQuery
//
//  Purpose:
//      Destruct an avtOriginalDataMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     February 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtOriginalDataMinMaxQuery::~avtOriginalDataMinMaxQuery()
{
    if (eef != NULL)
    {
        delete eef;
        eef = NULL;
    }
}


// ****************************************************************************
//  Method: avtOriginalDataMinMaxQuery::ApplyFilters
//
//  Purpose:
//    Applies avtExpressionEvaluatorFilter to the input data object and returns
//    the output from the filter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb 20 17:03:48 PST 2004
//    Create new pipeline spec so that input can be load balanced.
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    avtDataSpecification_p dspec = inData->GetTerminatingSource()->
        GetGeneralPipelineSpecification()->GetDataSpecification();
    avtPipelineSpecification_p pspec = 
        new avtPipelineSpecification(dspec, queryAtts.GetPipeIndex()); 

    avtDataObject_p temp;
    CopyTo(temp, inData);
    eef->SetInput(temp);
    avtDataObject_p retObj = eef->GetOutput();
    retObj->Update(pspec);
    return retObj;
}

