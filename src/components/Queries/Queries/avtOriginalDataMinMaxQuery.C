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
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
    avtDataSpecification_p dspec = pspec->GetDataSpecification();

    avtDataObject_p temp;
    CopyTo(temp, inData);
    eef->SetInput(temp);
    avtDataObject_p retObj = eef->GetOutput();
    retObj->Update(pspec);
    return retObj;
}

