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
//  Method: avtPseudocolorFilter::SetPlotAtts
//
//  Purpose:    Sets the PseudcolorAttributes needed for this filter.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 10, 2040 
//
// ****************************************************************************

void
avtPseudocolorFilter::SetPlotAtts(const PseudocolorAttributes *atts)
{
    plotAtts = *atts;
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
//    Kathleen Bonnell, Fri Jun 10 13:37:09 PDT 2005
//    Verify the existence of a valid variable before attempting to retrieve
//    its centering.
//
// ****************************************************************************

avtPipelineSpecification_p
avtPseudocolorFilter::PerformRestriction(avtPipelineSpecification_p pspec)
{
    avtPipelineSpecification_p rv = pspec;

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    string pointVar = plotAtts.GetPointSizeVar();
    avtDataSpecification_p dspec = new avtDataSpecification(
                                       pspec->GetDataSpecification());

    //
    // Find out if we need to add a secondary variable.
    //
    if (plotAtts.GetPointSizeVarEnabled() && 
        pointVar != "default" &&
        pointVar != "\0" &&
        pointVar != dspec->GetVariable() &&
        !dspec->HasSecondaryVariable(pointVar.c_str()))
    {
        rv->GetDataSpecification()->AddSecondaryVariable(pointVar.c_str());
    }


    if (pspec->GetDataSpecification()->MayRequireZones())
    {
        keepNodeZone = true;
        if (data.ValidActiveVariable())
        {
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
            // canot determine variable centering, so turn on both
            // node numbers and zone numbers.
            rv->GetDataSpecification()->TurnNodeNumbersOn();
            rv->GetDataSpecification()->TurnZoneNumbersOn();
        }
    }
    else
    {
        keepNodeZone = false;
    }

    return rv;
}
