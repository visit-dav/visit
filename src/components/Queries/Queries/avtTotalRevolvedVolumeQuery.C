// ************************************************************************* //
//                    avtTotalRevolvedVolumeQuery.C                          //
// ************************************************************************* //

#include <avtTotalRevolvedVolumeQuery.h>

#include <avtRevolvedVolume.h>
#include <avtSourceFromAVTDataset.h>
#include <NonQueryableInputException.h>


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
//  Modifications:
//    Kathleen Bonnell, Fri Jul 11 16:06:59 PDT 2003
//    Set units_append.
//
// ****************************************************************************

avtTotalRevolvedVolumeQuery::avtTotalRevolvedVolumeQuery() : avtSummationQuery()
{
    string      varname = "revolved_volume";
    string      sum_type = "RevolvedVolume";    
    string      units_append = "^3";    

    volume      = new avtRevolvedVolume;
    volume->SetOutputVariableName(varname.c_str());

    SetVariableName(varname);
    SetSumType(sum_type);
    SetUnitsAppend(units_append);
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


// ****************************************************************************
//  Method: avtTotalRevolvedVolumeQuery::VerifyInput
//
//  Purpose:
//    Now that we have an input, we can determine what the units are 
//    and tell the base class.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 27, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep  3 09:03:24 PDT 2004
//    Added test for topological dimension, as that is no longer performed by
//    base class.  
//
// ****************************************************************************

void
avtTotalRevolvedVolumeQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtDataObjectQuery::VerifyInput();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        EXCEPTION1(NonQueryableInputException,
            "Requires plot with topological dimension > 0.");
    }

    SetUnits(GetInput()->GetInfo().GetAttributes().GetXUnits());
}
