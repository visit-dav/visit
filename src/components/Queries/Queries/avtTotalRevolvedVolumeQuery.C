// ************************************************************************* //
//                    avtTotalRevolvedVolumeQuery.C                          //
// ************************************************************************* //

#include <avtTotalRevolvedVolumeQuery.h>

#include <avtRevolvedVolume.h>
#include <avtSourceFromAVTDataset.h>


using     std::string;

// ****************************************************************************
//  Method: avtTotalRevolvedVolumeQuery constructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalRevolvedVolumeFilter, 
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
// ****************************************************************************

avtTotalRevolvedVolumeQuery::avtTotalRevolvedVolumeQuery() : avtSummationQuery()
{
    string      varname = "revolved_volume";
    string      sum_type = "RevolvedVolume";    

    volume      = new avtRevolvedVolume;
    volume->SetOutputVariableName(varname.c_str());

    SetVariableName(varname);
    SetSumType(sum_type);
    SumGhostValues(false);
}


// ****************************************************************************
//  Method: avtTotalRevolvedVolumeQuery destructor
//
//  Notes:  Taken mostly from Hank Childs' avtTotalRevolvedVolumeFilter, 
//          now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtTotalRevolvedVolumeQuery::~avtTotalRevolvedVolumeQuery()
{
    if (volume != NULL)
    {
        delete volume;
        volume = NULL;
    }
}


// ****************************************************************************
//  Method: avtTotalRevolvedVolumeQuery::ApplyFilters 
//
//  Notes:  Taken mostly from Hank Childs' avtTotalRevolvedVolumeFilter, 
//          now deprecated.
//
//  Purpose:
//    Apply any avt filters necessary for the completion of this query.
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
avtTotalRevolvedVolumeQuery::ApplyFilters(avtDataObject_p inData)
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


