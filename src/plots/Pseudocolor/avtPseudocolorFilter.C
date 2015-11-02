/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

void
avtPseudocolorFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(0);
    outAtts.SetKeepNodeZoneArrays(keepNodeZone);

    if (!primaryVar.empty() && outAtts.ValidActiveVariable())
    {
        if (outAtts.GetVariableName() != primaryVar)
            outAtts.SetActiveVariable(primaryVar.c_str());
    }
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
// ****************************************************************************

avtContract_p
avtPseudocolorFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = contract;

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    int topoDim = data.GetTopologicalDimension();

    std::string pointVar   = plotAtts.GetPointSizeVar();
    std::string radiusVar    = plotAtts.GetTubeRadiusVar();
    std::string opacityVar = plotAtts.GetOpacityVariable();

    avtDataRequest_p dataRequest = new avtDataRequest(
                                       contract->GetDataRequest());

    primaryVar = dataRequest->GetVariable();

    // ARS - FIX ME - Why AddSecondaryVariable here and not in
    // avtPseudocolorPlot::EnhanceSpecification

    //
    // Find out if we need to add a secondary variable.
    //
    if( (topoDim == 0 || (topoDim > 0 && plotAtts.GetRenderPoints())) &&
        plotAtts.GetPointType() != PseudocolorAttributes::Point &&
        plotAtts.GetPointType() != PseudocolorAttributes::Sphere &&
        plotAtts.GetPointSizeVarEnabled() && 
        pointVar != "default" &&
        pointVar != "\0" &&
        pointVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(pointVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(pointVar.c_str());
        rv->SetCalculateVariableExtents(pointVar, true);
    }

    if( (topoDim == 1 || (topoDim > 1 && plotAtts.GetRenderWireframe())) &&
        plotAtts.GetLineType() == PseudocolorAttributes::Tube && 
        plotAtts.GetTubeRadiusVarEnabled() &&
        radiusVar != "default" &&
        radiusVar != "\0" &&
        radiusVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(radiusVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(radiusVar.c_str());
        rv->SetCalculateVariableExtents(radiusVar, true);
    }

    if (plotAtts.GetOpacityType() == PseudocolorAttributes::VariableRange &&
        opacityVar != "default" &&
        opacityVar != "\0" &&
        opacityVar != primaryVar &&
        !dataRequest->HasSecondaryVariable(opacityVar.c_str()))
    {
        rv->GetDataRequest()->AddSecondaryVariable(opacityVar.c_str());
        rv->SetCalculateVariableExtents(opacityVar, true);
    }

    // Note the line type so that upstream operators can obtain the
    // needed data for displaying ribbons or tubes.
    std::string key =
      rv->SetAttribute( &plotAtts, PseudocolorAttributes::ID_lineType,
                        PseudocolorAttributes::LineType_ToString(plotAtts.GetLineType()) );

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

    return rv;
}
