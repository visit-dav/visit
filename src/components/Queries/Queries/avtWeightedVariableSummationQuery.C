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
//    Kathleen Bonnell, Wed Mar 31 16:20:39 PST 2004
//    Added logic for time-varying case.
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
}


