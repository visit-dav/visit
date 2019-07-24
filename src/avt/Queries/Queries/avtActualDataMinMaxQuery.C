// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtActualDataMinMaxQuery.C                         //
// ************************************************************************* //

#include <avtActualDataMinMaxQuery.h>

#include <avtCondenseDatasetFilter.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
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
//    Kathleen Bonnell, Thu May  6 17:36:43 PDT 2004
//    Request OriginalCellsArray if zones have not been preserved.
//
//    Kathleen Biagas, Fri Feb 10 11:30:54 PST 2012
//    ZonesPreserved may not be the same on all processor, so make sure if
//    any processor reports zones are not preserved, they all take that value
//    so the same path through the code is used.
//
//    Burlen Loring, Wed Aug  5 12:34:55 PDT 2015
//    Explicitly set the variable name in the data request. This is needed
//    when operators are creating a new variable (eg. databinning)
//
// ****************************************************************************

avtDataObject_p
avtActualDataMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    Preparation(inData);

    bool zonesPreserved  = GetInput()->GetInfo().GetValidity().GetZonesPreserved();
    zonesPreserved = (bool)UnifyMinimumValue((int)zonesPreserved);

    if (!timeVarying && zonesPreserved)
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
    else
    {
        avtDataRequest_p oldSpec = inData->GetOriginatingSource()->
            GetGeneralContract()->GetDataRequest();

        avtDataRequest_p newDS = new
            avtDataRequest(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());
        newDS->SetVariable(queryAtts.GetVariables()[0].c_str());

        if (!zonesPreserved)
            newDS->TurnZoneNumbersOn();

        avtContract_p contract =
            new avtContract(newDS, queryAtts.GetPipeIndex());

        avtDataObject_p temp;
        CopyTo(temp, inData);
        condense->SetInput(temp);
        avtDataObject_p retObj = condense->GetOutput();
        retObj->Update(contract);
        return retObj;
    }
}
