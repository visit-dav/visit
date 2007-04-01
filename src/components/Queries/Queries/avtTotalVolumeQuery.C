// ************************************************************************* //
//                        avtTotalVolumeQuery.C                              //
// ************************************************************************* //

#include <avtTotalVolumeQuery.h>

#include <avtVMetricVolume.h>
#include <avtSourceFromAVTDataset.h>
#include <InvalidDimensionsException.h>


using     std::string;

// ****************************************************************************
//  Method: avtTotalVolumeQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalVolumeFilter, 
//          now deprecated.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:06:59 PDT 2003
//    Set units_append.
//
// ****************************************************************************

avtTotalVolumeQuery::avtTotalVolumeQuery() : avtSummationQuery()
{
    string      varname = "volume";
    string      sum_type = "Volume";    
    string      units_append = "^3";    

    volume      = new avtVMetricVolume;
    volume->SetOutputVariableName(varname.c_str());
    volume->UseOnlyPositiveVolumes(true);

    SetVariableName(varname);
    SetSumType(sum_type);
    SetUnitsAppend(units_append);
    SumGhostValues(false);
}


// ****************************************************************************
//  Method: avtTotalVolumeQuery destructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalVolumeFilter, 
//          now deprecated.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtTotalVolumeQuery::~avtTotalVolumeQuery()
{
    if (volume != NULL)
    {
        delete volume;
        volume = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalVolumeQuery::VerifyInput 
//
//  Purpose:  Throw exception for invalid input. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//    Retrieve units from dataAttributes to be used in output.
//
// ****************************************************************************

void 
avtTotalVolumeQuery::VerifyInput()
{
    //
    //  Since base class performs general input verification for all queries,
    //  make sure to call it's verify method first.
    //
    avtDataObjectQuery::VerifyInput();

    int tDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    if  (tDim != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "Volume query", "3D surface");
    }
    SetUnits(GetInput()->GetInfo().GetAttributes().GetXUnits());
}


// ****************************************************************************
//  Method: avtTotalVolumeQuery::ApplyFilters 
//
//  Purpose:  Apply any avt filters necessary for completion of this query.
//
//  Notes:  Taken mostly from Hank Childs' avtTotalVolumeFilter, 
//          now deprecated.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Moved artificial pipeline creation from base class.
//
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004 
//    Added logic for time-varying case. 
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
//    Kathleen Bonnell, Thu Jan  6 11:06:29 PST 2005 
//    Rework so that both time-varying and non use artificial pipeline. 
//    Only difference is the pipeline spec used. 
//
// ****************************************************************************

avtDataObject_p 
avtTotalVolumeQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec =
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    if (timeVarying) 
    { 
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        avtDataSpecification_p newDS = new 
            avtDataSpecification(oldSpec, querySILR);
        newDS->SetTimestep(queryAtts.GetTimeStep());

        pspec = new avtPipelineSpecification(newDS, pspec->GetPipelineIndex());
    }


    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();
    volume->SetInput(dob);
    avtDataObject_p objOut = volume->GetOutput();
    objOut->Update(pspec);
    return objOut;
}

