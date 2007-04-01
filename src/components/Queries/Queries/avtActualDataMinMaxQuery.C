// ************************************************************************* //
//                        avtActualDataMinMaxQuery.C                         //
// ************************************************************************* //

#include <avtActualDataMinMaxQuery.h>

#include <avtCondenseDatasetFilter.h>
#include <avtTerminatingSource.h>
#include <avtSourceFromAVTDataset.h>


// ****************************************************************************
//  Method: avtActualDataMinMaxQuery::avtActualDataMinMaxQuery
//
//  Purpose:
//      Construct an avtActualDataMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     February 10, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Add min/max arguments, passed to parent class.
//
//    Kathleen Bonnell, Wed Apr 14 18:00:49 PDT 2004 
//    Tell condense filter to bypass its heuristic and force execution of
//    relevant points filter. 
//
// ****************************************************************************

avtActualDataMinMaxQuery::avtActualDataMinMaxQuery(bool min, bool max)
    : avtMinMaxQuery(min, max)
{
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    condense->BypassHeuristic(true);
}


// ****************************************************************************
//  Method: avtActualDataMinMaxQuery::~avtActualDataMinMaxQuery
//
//  Purpose:
//      Destruct an avtActualDataMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     February 10, 2040 
//
//  Modifications:
//
// ****************************************************************************

avtActualDataMinMaxQuery::~avtActualDataMinMaxQuery()
{
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
    }
}


// ****************************************************************************
//  Method: avtActualDataMinMaxQuery::ApplyFilters
//
//  Purpose:
//    Applies the condense filter to input and returns the filter's output. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:07:50 PST 2004
//    Added code for time-varying case.
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
// ****************************************************************************

avtDataObject_p
avtActualDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    if (!timeVarying)
    {
        avtPipelineSpecification_p pspec = 
            inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

        avtDataset_p ds;
        CopyTo(ds, inData);
        avtSourceFromAVTDataset termsrc(ds);
        avtDataObject_p obj = termsrc.GetOutput();
        condense->SetInput(obj);
        avtDataObject_p retObj = condense->GetOutput();
        retObj->Update(pspec);
        return retObj;
    }
    else 
    {
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec->GetVariable(), queryAtts.GetTimeStep(), 
                                 querySILR);

        avtPipelineSpecification_p pspec = 
            new avtPipelineSpecification(newDS, queryAtts.GetPipeIndex());

        avtDataObject_p temp;
        CopyTo(temp, inData);
        condense->SetInput(temp);
        avtDataObject_p retObj = condense->GetOutput();
        retObj->Update(pspec);
        return retObj;
    }
}
