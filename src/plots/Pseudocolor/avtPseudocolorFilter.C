// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPseudocolorFilter.C                          //
// ************************************************************************* //

#include <avtPseudocolorFilter.h>

#include <string>

// ****************************************************************************
//  Method: avtPseudocolorFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
//    Alister Maguire, Tue Jul 16 14:12:20 PDT 2019
//    Added instantiation of mustRemoveFacesBeforeGhosts.
//
// ****************************************************************************

avtPseudocolorFilter::avtPseudocolorFilter()
{
    keepNodeZone = false;
    mustRemoveFacesBeforeGhosts = false;
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
//  Creation:   November 10, 2004
//
//  Modifications:
//
//    Alister Maguire, Tue Jul 16 14:12:20 PDT 2019
//    Check if we've entered or exited an opacity mode that allows
//    for transparency. If so, we need to update the ghost/face removal
//    flag.
//
// ****************************************************************************

void
avtPseudocolorFilter::SetPlotAtts(const PseudocolorAttributes *atts)
{
    PseudocolorAttributes::OpacityType newOpacType = atts->GetOpacityType();
    PseudocolorAttributes::OpacityType oldOpacType = plotAtts.GetOpacityType();

    if (newOpacType != PseudocolorAttributes::FullyOpaque &&
        oldOpacType == PseudocolorAttributes::FullyOpaque)
    {
        //
        // The user has turned on transparency. We must remove faces
        // before ghosts to avoid rendering processor boundaries.
        //
        mustRemoveFacesBeforeGhosts = true;
    }
    else if (newOpacType == PseudocolorAttributes::FullyOpaque &&
             oldOpacType != PseudocolorAttributes::FullyOpaque)
    {
        //
        // Transparency is now off, so we can go back to the normal
        // ghost/face removal process.
        //
        mustRemoveFacesBeforeGhosts = false;
    }

    plotAtts = *atts;
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::ExecuteData
//
//  Purpose:
//      Returns input.
//
//  Arguments:
//      inDR      The input data representation.
//
//  Returns:      The output data representation.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     October 29, 2004
//
//  Modifications:
//    Eric Brugger, Tue Aug 19 11:11:13 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtPseudocolorFilter::ExecuteData(avtDataRepresentation *inDR)
{
    return inDR;
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::UpdateDataObjectInfo
//
//  Purpose:  Sets flags in the pipeline.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//    Kathleen Biagas, Fri Nov  2 10:23:11 PDT 2012
//    Ensure primaryVariable is still the active var, use of expression for
//    pointVar may have changed the active var.
//
//    Alister Maguire, Tue Jul 16 14:12:20 PDT 2019
//    Added a call to SetRemoveFacesBeforeGhosts. When the user allows
//    or disallows transparency, we need to toggle this flag so that
//    processor boundaries are not rendered.
//
// ****************************************************************************

void
avtPseudocolorFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

    int topoDim = inAtts.GetTopologicalDimension();

    outAtts.SetTopologicalDimension(topoDim);

    if( topoDim == 0 )
    {
      outAtts.SetKeepNodeZoneArrays(keepNodeZone);

      if (!primaryVar.empty() && outAtts.ValidActiveVariable())
      {
        if (outAtts.GetVariableName() != primaryVar)
            outAtts.SetActiveVariable(primaryVar.c_str());
      }
    }

    outAtts.SetForceRemoveFacesBeforeGhosts(mustRemoveFacesBeforeGhosts);
}


// ****************************************************************************
//  Method: avtPseudocolorFilter::ModifyContract
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
//    Kathleen Bonnell, Tue Jul 14 13:42:37 PDT 2009
//    Added test for MayRequireNodes for turning Node numbers on.
//
//    Kathleen Biagas, Fri Nov  2 10:24:21 PDT 2012
//    Retrieve the active variable.
//
//    Eric Brugger, Wed Oct 26 09:23:35 PDT 2016
//    I modified the plot to support independently setting the point style
//    for the two end points of lines.
//
//    Kathleen Biagas, Wed Nov  6 15:19:01 PST 2019
//    Cannot use topological dimension test for point/line settings.
//    Dataset may be of mixed topology.
//
// ****************************************************************************

avtContract_p
avtPseudocolorFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = contract;

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    int topoDim = data.GetTopologicalDimension();

    std::string pointVar           = plotAtts.GetPointSizeVar();
    std::string tubeRadiusVar      = plotAtts.GetTubeRadiusVar();
    std::string endPointRadiusVar  = plotAtts.GetEndPointRadiusVar();
    std::string opacityVar         = plotAtts.GetOpacityVariable();

    avtDataRequest_p dataRequest = new avtDataRequest(
                                       contract->GetDataRequest());

    primaryVar = dataRequest->GetVariable();

    //
    // Find out if we need to add a secondary variable.
    //
    if (plotAtts.GetOpacityType() == PseudocolorAttributes::VariableRange &&
        opacityVar != "default" &&
        opacityVar != "\0" &&
        opacityVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(opacityVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(opacityVar.c_str());
        rv->SetCalculateVariableExtents(opacityVar, true);
    }

    // Point scaling by a secondary variable
    if( plotAtts.GetPointType() != Point &&
        plotAtts.GetPointType() != Sphere &&
        plotAtts.GetPointSizeVarEnabled() &&
        pointVar != "default" &&
        pointVar != "\0" &&
        pointVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(pointVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(pointVar.c_str());
        rv->SetCalculateVariableExtents(pointVar, true);
    }

    // Tube/Ribbon scaling by a secondary variable
    if( (plotAtts.GetLineType() == PseudocolorAttributes::Tube ||
         plotAtts.GetLineType() == PseudocolorAttributes::Ribbon) &&
        plotAtts.GetTubeRadiusVarEnabled() &&
        tubeRadiusVar != "default" &&
        tubeRadiusVar != "\0" &&
        tubeRadiusVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(tubeRadiusVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(tubeRadiusVar.c_str());
        rv->SetCalculateVariableExtents(tubeRadiusVar, true);

        std::string key =
          rv->SetAttribute( &plotAtts, PseudocolorAttributes::ID_tubeRadiusVar,
                            tubeRadiusVar );
    }

    // End Point scaling by a secondary variable
    if( (plotAtts.GetTailStyle() != PseudocolorAttributes::None ||
         plotAtts.GetHeadStyle() != PseudocolorAttributes::None) &&
        plotAtts.GetEndPointRadiusVarEnabled() &&
        endPointRadiusVar != "default" &&
        endPointRadiusVar != "\0" &&
        endPointRadiusVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(endPointRadiusVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(endPointRadiusVar.c_str());
        rv->SetCalculateVariableExtents(endPointRadiusVar, true);

        std::string key =
          rv->SetAttribute( &plotAtts, PseudocolorAttributes::ID_endPointRadiusVar,
                            endPointRadiusVar );
    }

    // Note the line type so that upstream operators can obtain the
    // needed data for displaying ribbons or tubes.
    std::string key =
      rv->SetAttribute( &plotAtts, PseudocolorAttributes::ID_lineType,
                        PseudocolorAttributes::LineType_ToString(plotAtts.GetLineType()) );


    if( topoDim == 0 )
    {
      if (contract->GetDataRequest()->MayRequireZones() ||
          contract->GetDataRequest()->MayRequireNodes())
      {
        keepNodeZone = true;

        if (data.ValidActiveVariable())
          {
            if (data.GetCentering() == AVT_NODECENT)
            {
                rv->GetDataRequest()->TurnNodeNumbersOn();
            }
            else if (data.GetCentering() == AVT_ZONECENT)
            {
                rv->GetDataRequest()->TurnZoneNumbersOn();
            }
        }
        else
        {
            // canot determine variable centering, so turn on both
            // node numbers and zone numbers.
            rv->GetDataRequest()->TurnNodeNumbersOn();
            rv->GetDataRequest()->TurnZoneNumbersOn();
        }
      }
      else
      {
        keepNodeZone = false;
      }
    }

    return rv;
}
