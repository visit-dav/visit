// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtActualDataNumNodesQuery.C                        //
// ************************************************************************* //

#include <avtActualDataNumNodesQuery.h>
#include <avtCondenseDatasetFilter.h>
#include <avtOriginatingSource.h>
#include <avtSourceFromAVTDataset.h>


// ****************************************************************************
//  Method: avtActualDataNumNodesQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 13:37:48 PDT 2008
//    Initialize condense filter.
//
// ****************************************************************************

avtActualDataNumNodesQuery::avtActualDataNumNodesQuery() : avtNumNodesQuery() 
{
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    condense->BypassHeuristic(true);
}


// ****************************************************************************
//  Method: avtActualDataNumNodesQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 13:37:48 PDT 2008
//    Delete condense filter.
//
// ****************************************************************************
avtActualDataNumNodesQuery::~avtActualDataNumNodesQuery() 
{
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
    }
}


// ****************************************************************************
//  Method: avtActualDataNumNodesQuery::ApplyFilters
//
//  Purpose:
//    Applies the condense filter to input and returns the filter's output. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 23, 2008 
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtActualDataNumNodesQuery::ApplyFilters(avtDataObject_p inData)
{
    avtContract_p contract = 
        inData->GetOriginatingSource()->GetGeneralContract();

    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p obj = termsrc.GetOutput();
    condense->SetInput(obj);
    avtDataObject_p retObj = condense->GetOutput();
    retObj->Update(contract);
    return retObj;
}
