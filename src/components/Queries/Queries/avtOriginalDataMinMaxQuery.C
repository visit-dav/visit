// ************************************************************************* //
//                      avtOriginalDataMinMaxQuery.C                         //
// ************************************************************************* //

#include <avtOriginalDataMinMaxQuery.h>

#include <avtCondenseDatasetFilter.h>
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
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Added optional args.
//
//    Kathleen Bonnell, Wed Apr 14 18:05:08 PDT 2004 
//    Added condense filter. 
//
// ****************************************************************************

avtOriginalDataMinMaxQuery::avtOriginalDataMinMaxQuery(bool min, bool max)
    : avtMinMaxQuery(min, max)
{
    eef = new avtExpressionEvaluatorFilter;
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    condense->BypassHeuristic(true);
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
//    Kathleen Bonnell, Wed Apr 14 18:05:08 PDT 2004 
//    Added condense filter. 
//
// ****************************************************************************

avtOriginalDataMinMaxQuery::~avtOriginalDataMinMaxQuery()
{
    if (eef != NULL)
    {
        delete eef;
        eef = NULL;
    }
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
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
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Added logic for time-varying case.
//
//    Kathleen Bonnell, Wed Apr 14 18:05:08 PDT 2004 
//    Added condense filter. 
//
// ****************************************************************************

avtDataObject_p
avtOriginalDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    avtDataSpecification_p dspec;
    if (!timeVarying)
    {
        dspec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();
    }
    else 
    {
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        dspec = new avtDataSpecification(oldSpec->GetVariable(), 
                                         queryAtts.GetTimeStep(), 
                                         oldSpec->GetRestriction());
    }
    avtPipelineSpecification_p pspec =
        new avtPipelineSpecification(dspec, queryAtts.GetPipeIndex()); 
    avtDataObject_p temp;
    CopyTo(temp, inData);
    eef->SetInput(temp);
    condense->SetInput(eef->GetOutput());
    avtDataObject_p retObj = condense->GetOutput();
    retObj->Update(pspec);
    return retObj;
}

