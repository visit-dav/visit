// ************************************************************************* //
//                           avtPseudocolorFilter.C                          //
// ************************************************************************* //

#include <avtPseudocolorFilter.h>


// ****************************************************************************
//  Method: avtPseudocolorFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPseudocolorFilter::avtPseudocolorFilter()
{
    keepNodeZone = false;
}


// ****************************************************************************
//  Method: avtPseudocolorFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPseudocolorFilter::~avtPseudocolorFilter()
{
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::ExecuteData
//
//  Purpose:
//      Returns input. 
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtPseudocolorFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    return inDS;
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::RefashionDataObjectInfo
//
//  Purpose:  Sets flags in the pipeline.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPseudocolorFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::PerformRestriction
//
//  Purpose:  Turns on Node/Zone numbers when appropriate. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   October 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtPseudocolorFilter::PerformRestriction(avtPipelineSpecification_p pspec)
{
    avtPipelineSpecification_p rv = pspec;

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    if (pspec->GetDataSpecification()->MayRequireZones())
    {
        keepNodeZone = true;
        if (data.GetCentering() == AVT_NODECENT)
        {
            rv->GetDataSpecification()->TurnNodeNumbersOn();
        }
        else if (data.GetCentering() == AVT_ZONECENT)
        {
            rv->GetDataSpecification()->TurnZoneNumbersOn();
        }
    }
    else
    {
        keepNodeZone = false;
    }

    return rv;
}
