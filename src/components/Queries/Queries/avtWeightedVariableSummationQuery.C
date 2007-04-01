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

    string vname = "avt_sum";
    SetVariableName(vname);
    SumGhostValues(false);
    SumOnlyPositiveValues(true);
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
// ****************************************************************************

avtDataObject_p
avtWeightedVariableSummationQuery::ApplyFilters(avtDataObject_p inData)
{
    if (!timeVarying)
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
        avtPipelineSpecification_p pspec = inData->GetTerminatingSource()
                                          ->GetGeneralPipelineSpecification();
        multiply->GetOutput()->Update(pspec);

        return multiply->GetOutput();
    }
    else
    {
        avtDataObject_p dob = inData;

        //
        // Set up our base class so it is ready to sum.
        //
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new
                           avtDataSpecification(oldSpec->GetVariable(),
                           queryAtts.GetTimeStep(), querySILR);

        avtPipelineSpecification_p pspec =
            new avtPipelineSpecification(newDS, queryAtts.GetPipeIndex());

        string varname = newDS->GetVariable();
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
        multiply->GetOutput()->Update(pspec);

        return multiply->GetOutput();
    }
}


