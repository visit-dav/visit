// ************************************************************************* //
//                     avtWeightedVariableSummationQuery.C                   //
// ************************************************************************* //

#include <avtWeightedVariableSummationQuery.h>

#include <avtBinaryMultiplyFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>
#include <avtVMetricArea.h>
#include <avtVMetricVolume.h>


using     std::string;


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Aug 10 14:05:07 PDT 2005
//    Force only positive volumes, but allow negative values from var.
//
// ****************************************************************************

avtWeightedVariableSummationQuery::avtWeightedVariableSummationQuery() 
    : avtSummationQuery()
{
    area = new avtVMetricArea;
    area->SetOutputVariableName("avt_weights");

    multiply = new avtBinaryMultiplyFilter;
    multiply->SetOutputVariableName("avt_sum");

    volume = new avtVMetricVolume;
    volume->SetOutputVariableName("avt_weights");
    volume->UseOnlyPositiveVolumes(true);

    string vname = "avt_sum";
    SetVariableName(vname);
    SumGhostValues(false);
    SumOnlyPositiveValues(false);
}


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
// ****************************************************************************

avtWeightedVariableSummationQuery::~avtWeightedVariableSummationQuery()
{
    delete area;
    delete multiply;
    delete volume;
}


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Mar 31 16:20:39 PST 2004
//    Added logic for time-varying case.
//
//    Hank Childs, Thu Apr  8 08:24:12 PDT 2004
//    Allow time varying data to access database, so it can consider more than
//    one timestep.
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
//    Kathleen Bonnell, Fri Jan  7 15:15:32 PST 2005 
//    Rework so that both time-varying and non use artificial pipeline. 
//    Only difference is the pipeline spec used. 
//
// ****************************************************************************

avtDataObject_p
avtWeightedVariableSummationQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    //
    // Set up our base class so it is ready to sum.
    //
    avtDataSpecification_p dspec = GetInput()->GetTerminatingSource()
                                     ->GetFullDataSpecification();
    string varname = dspec->GetVariable();
    SetSumType(varname);

    int topo = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if (topo == 2)
    {
        area->SetInput(dob);
        dob = area->GetOutput();
    }
    else
    {
        volume->SetInput(dob);
        dob = volume->GetOutput();
    }

    multiply->SetInput(dob);
    multiply->ClearInputVariableNames();
    multiply->AddInputVariableName("avt_weights");
    multiply->AddInputVariableName(varname.c_str());

    //
    // Cause our artificial pipeline to execute.
    //
    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    if (timeVarying) 
    { 
        avtDataSpecification_p newDS = new 
            avtDataSpecification(dspec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        pspec = new avtPipelineSpecification(newDS, pspec->GetPipelineIndex());
    }

    multiply->GetOutput()->Update(pspec);
    return multiply->GetOutput();
}


// ****************************************************************************
//  Method: avtWeightedVariableSummationQuery::VerifyInput
//
//  Purpose:
//      Now that we have an input, we can determine what the variable units
//      are and tell the base class about it.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 28, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005 
//    Remove TRY-CATCH block in favor of testing for ValidActiveVariable. 
//
// ****************************************************************************

void
avtWeightedVariableSummationQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtSummationQuery::VerifyInput();

    if (GetInput()->GetInfo().GetAttributes().ValidActiveVariable()) 
    {
        //
        // Set the base class units to be used in output.
        //
        SetUnits(GetInput()->GetInfo().GetAttributes().GetVariableUnits());
    }
}

