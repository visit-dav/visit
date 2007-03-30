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
// ****************************************************************************

avtTotalVolumeQuery::avtTotalVolumeQuery() : avtSummationQuery()
{
    string      varname = "volume";
    string      sum_type = "Volume";    

    volume      = new avtVMetricVolume;
    volume->SetOutputVariableName(varname.c_str());
    volume->UseOnlyPositiveVolumes(true);

    SetVariableName(varname);
    SetSumType(sum_type);
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
// ****************************************************************************

avtDataObject_p 
avtTotalVolumeQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

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

