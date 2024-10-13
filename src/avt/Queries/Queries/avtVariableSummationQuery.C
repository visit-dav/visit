// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtVariableSummationQuery.C                       //
// ************************************************************************* //

#include <avtVariableSummationQuery.h>

#include <avtCondenseDatasetFilter.h>
#include <avtOriginatingSource.h>

using     std::string;

#include <avtParallel.h>

#ifdef PARALLEL
#include <mpi.h>
#endif


// ****************************************************************************
//  Method: avtVariableSummationQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Add condense filter.
//
// ****************************************************************************

avtVariableSummationQuery::avtVariableSummationQuery() : avtSummationQuery()
{
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    condense->BypassHeuristic(true);
}


// ****************************************************************************
//  Method: avtVariableSummationQuery destructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Add condense filter.
//
// ****************************************************************************

avtVariableSummationQuery::~avtVariableSummationQuery()
{
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
    }
}


// ****************************************************************************
//  Method: avtVariableSummationQuery::VerifyInput
//
//  Purpose:
//      Now that we have an input, we can determine what the variable name is
//      and tell the base class about it.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//    Retrieve variable's units, if available.
//
//    Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005
//    Remove TRY-CATCH block in favor of testing for ValidVariable.
//
//    Kathleen Bonnell, Wed Apr  2 10:20:27 PDT 2008
//    Retrieve the varname from the dataAtts instead of DataRequest, as
//    DataRequest may have the wrong value based on other pipelines sharing
//    the same source.
//
//    Kathleen Bonnell, Tue Jul 29 10:05:39 PDT 2008
//    Check for ValidActiveVariable before retrieving from DatAtts.  Revert
//    back to using varname from DataRequest if no valid active variable.
//
// ****************************************************************************

void
avtVariableSummationQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtSummationQuery::VerifyInput();

    avtDataRequest_p dataRequest = GetInput()->GetOriginatingSource()
                                     ->GetFullDataRequest();

    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();


    string varname;
    if (dataAtts.ValidActiveVariable())
    {
        varname = dataAtts.GetVariableName();
    }
    else
    {
        varname = dataRequest->GetVariable();
    }
    SetVariableName(varname);
    SumGhostValues(false);
    SetSumType(varname);
    if (dataAtts.ValidVariable(varname.c_str()))
    {
        //
        // Set the base class units to be used in output.
        //
        SetUnits(dataAtts.GetVariableUnits(varname.c_str()));
    }
}


// ****************************************************************************
//  Method: avtVariableSummationQuery::ApplyFilters
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 24, 2006
//
//  Modifications:
//    Kathleen Bonnell, Thu May 11 10:32:54 PDT 2006
//    Use condense filter for pointdata when the mesh has been mat selected.
//
// ****************************************************************************

avtDataObject_p
avtVariableSummationQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataValidity &dval = GetInput()->GetInfo().GetValidity();
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    bool cellData = false;

    avtCentering cent = AVT_UNKNOWN_CENT;
    if (datts.ValidVariable(variableName))
    {
        cent = datts.GetCentering(variableName.c_str());
        cellData = (cent != AVT_NODECENT);
    }
    else
    {
        // we can't determine the centering, assume zone-centered
        cellData = true;
    }

    int bDoCustomFiltering = dval.SubdivisionOccurred() ||
                             ( cellData && !dval.GetOriginalZonesIntact()) ||
                             (!cellData && !dval.GetZonesPreserved());
#ifdef PARALLEL
    int bAnyDoCustomFiltering;

    MPI_Allreduce(&bDoCustomFiltering, &bAnyDoCustomFiltering, 1,
                  MPI_INT, MPI_LOR, VISIT_MPI_COMM);
    bDoCustomFiltering = bAnyDoCustomFiltering;
#endif
    if (bDoCustomFiltering)
    {
        // This will work for time-varying data, too.

        // tell parent class to sum from original element values.
        // e.g. each 'original' cell/node constributes only once to
        // the sum.
        SumFromOriginalElement(true);

        // Need to request original cell and/or node numbers
        avtDataRequest_p oldSpec = inData->GetOriginatingSource()->
            GetGeneralContract()->GetDataRequest();

        avtDataRequest_p newDS = new
            avtDataRequest(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        if (cent == AVT_ZONECENT)
        {
            newDS->TurnZoneNumbersOn();
        }
        else if (cent == AVT_NODECENT)
        {
            newDS->TurnNodeNumbersOn();
        }
        else
        {
            newDS->TurnZoneNumbersOn();
            newDS->TurnNodeNumbersOn();
        }

        avtContract_p contract =
            new avtContract(newDS, queryAtts.GetPipeIndex());

        avtDataObject_p temp;
        CopyTo(temp, inData);
        condense->SetInput(temp);
        avtDataObject_p rv = condense->GetOutput();
        rv->Update(contract);
        return rv;
    }
    else
    {
        return avtSummationQuery::ApplyFilters(inData);
    }
}


// ****************************************************************************
//  Method: avtVariableSummationQuery::GetTimeCurveSpecs
//
//  Purpose:
//    Override default TimeCurveSpecs
//
//  Programmer:  Kathleen Bigags
//  Creation:    Sep 11, 2024
//
//  Modifications:
//
// ****************************************************************************

const MapNode&
avtVariableSummationQuery::GetTimeCurveSpecs(const QueryAttributes *qa)
{
    timeCurveSpecs["outputCurveLabel"] = "Sum_" + qa->GetVariables()[0];
    return timeCurveSpecs;
}

