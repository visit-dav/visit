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
//
// ****************************************************************************

avtActualDataMinMaxQuery::avtActualDataMinMaxQuery()
{
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
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
//
// ****************************************************************************

avtDataObject_p
avtActualDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
    avtDataSpecification_p dspec = pspec->GetDataSpecification();

    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p obj = termsrc.GetOutput();
    condense->SetInput(obj);
    avtDataObject_p retObj = condense->GetOutput();
    retObj->Update(pspec);
    return retObj;
}
