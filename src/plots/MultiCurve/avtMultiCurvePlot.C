/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                          avtMultiCurvePlot.C                              //
// ************************************************************************* //

#include <avtMultiCurvePlot.h>

#include <avtColorTables.h>
#include <avtLevelsLegend.h>
#include <avtLevelsMapper.h>
#include <avtLookupTable.h>
#include <avtMultiCurveFilter.h>
#include <avtMultiCurveLabelMapper.h>

#include <ColorAttribute.h>
#include <InvalidColortableException.h>


// ****************************************************************************
//  Method: avtMultiCurvePlot constructor
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Fri Feb 20 16:21:57 PST 2009
//    I added a legend to the plot.
//
// ****************************************************************************

avtMultiCurvePlot::avtMultiCurvePlot()
{
    MultiCurveFilter = new avtMultiCurveFilter(atts);

    levelsMapper     = new avtLevelsMapper;
    levelsLegend     = new avtLevelsLegend;
    decoMapper       = new avtMultiCurveLabelMapper();
    avtLUT           = new avtLookupTable;

    //
    // Set some legend properties.
    //
    levelsLegend->SetColorBarVisibility(0);
    levelsLegend->SetTitleVisibility(1);
    levelsLegend->SetLookupTable(avtLUT->GetLookupTable());

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a
    // levelsLegend.
    //
    levelsLegendRefPtr = levelsLegend;
}


// ****************************************************************************
//  Method: avtMultiCurvePlot destructor
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Fri Feb 20 16:21:57 PST 2009
//    I added a legend to the plot.
//
// ****************************************************************************

avtMultiCurvePlot::~avtMultiCurvePlot()
{
    if (levelsMapper != NULL)
    {
        delete levelsMapper;
        levelsMapper = NULL;
    }
    if (decoMapper != NULL)
    {
        delete decoMapper;
        decoMapper = NULL;
    }
    if (MultiCurveFilter != NULL)
    {
        delete MultiCurveFilter;
        MultiCurveFilter = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }

    //
    // Do not delete the levelsLegend since it is being held by
    // levelsLegendRefPtr.
    //
}


// ****************************************************************************
//  Method:  avtMultiCurvePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

avtPlot*
avtMultiCurvePlot::Create()
{
    return new avtMultiCurvePlot;
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

avtMapper *
avtMultiCurvePlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::GetDecorationsMapper
//
//  Purpose:
//      Gets a decorations mapper for this plot, in this case the
//      labeled curve mapper.
//
//  Returns:    The labeled curve mapper typed as its base class mapper.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

avtDecorationsMapper *
avtMultiCurvePlot::GetDecorationsMapper(void)
{
    return decoMapper;
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a MultiCurve plot.  
//      The output from this method is a query-able object.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the MultiCurve plot has been applied.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

avtDataObject_p
avtMultiCurvePlot::ApplyOperators(avtDataObject_p input)
{
    MultiCurveFilter->SetInput(input);
    return MultiCurveFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation associated with a MultiCurve plot.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the MultiCurve plot has been applied.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

avtDataObject_p
avtMultiCurvePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a MultiCurve plot.  This
//      includes behavior like shifting towards or away from the screen.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Fri Feb 20 16:21:57 PST 2009
//    I added a legend to the plot.
//
// ****************************************************************************

void
avtMultiCurvePlot::CustomizeBehavior(void)
{
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_AXISPARALLEL);

    //
    // Create the legend, which consists of the tick scale and any legend
    // text from the operators.
    //
    char msg[80];
    sprintf(msg, "Each tick is %g", atts.GetYAxisRange() / 4.);
    levelsLegend->SetTitle(msg);

    const MapNode *mNode = behavior->GetInfo().GetAttributes().GetPlotInformation().GetData().GetEntry("LegendMessage");
    if (mNode != NULL)
    {
        const string message = mNode->GetEntry("message")->AsString();
        levelsLegend->SetMessage(message.c_str());
    }

    behavior->SetShiftFactor(0.0);
    behavior->SetLegend(levelsLegendRefPtr);
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::CustomizeMapper
//
//  Purpose:
//      A hook from the base class that allows the plot to change its mapper
//      based on the dataset input. 
//
//  Arguments:
//      doi     The data object information.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

void
avtMultiCurvePlot::CustomizeMapper(avtDataObjectInformation &doi)
{
    return;
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::SetAtts
//
//  Purpose:
//      Sets the atts for the MultiCurve plot.
//
//  Arguments:
//      atts    The attributes for this MultiCurve plot.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Wed Feb 18 12:13:03 PST 2009
//    I added the ability to display identifiers at each of the points.
//
//    Eric Brugger, Fri Feb 20 16:21:57 PST 2009
//    I added a legend to the plot.
//
// ****************************************************************************

void
avtMultiCurvePlot::SetAtts(const AttributeGroup *a)
{
    const MultiCurveAttributes *newAtts = (const MultiCurveAttributes *)a;

    needsRecalculation = atts.ChangesRequireRecalculation(*newAtts);

    atts = *newAtts;

    if (MultiCurveFilter != NULL)
        MultiCurveFilter->SetAttributes(*newAtts);

    if (atts.GetColorType() == MultiCurveAttributes::ColorBySingleColor)
    {
        const ColorAttribute ca = atts.GetSingleColor();
        ColorAttributeList cal;
        cal.AddColors(atts.GetSingleColor());
        avtLUT->SetLUTColorsWithOpacity(ca.GetColor(), 1);
        levelsMapper->SetColors(cal);
    }
    else // ColorByMultipleColors
    {
        const ColorAttributeList &cal = atts.GetMultiColor();
        unsigned char *colors = new unsigned char[cal.GetNumColors() * 4];
        unsigned char *cptr = colors;
        for(int i = 0; i < cal.GetNumColors(); i++)
        {
            *cptr++ = (char)cal[i].Red();
            *cptr++ = (char)cal[i].Green();
            *cptr++ = (char)cal[i].Blue();
            *cptr++ = (char)cal[i].Alpha();
        }

        avtLUT->SetLUTColorsWithOpacity(colors, cal.GetNumColors());
        levelsMapper->SetColors(cal);

        delete [] colors;
    }

    SetLineWidth(atts.GetLineWidth());
    SetLineStyle(atts.GetLineStyle());
    SetLegend(atts.GetLegendFlag());

    //
    // Create the decorations.
    //
    decoMapper->SetMarkerVisibility(atts.GetDisplayMarkers());
    decoMapper->SetIdVisibility(atts.GetDisplayIds());
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::SetLineWidth
//
//  Purpose:
//      Sets the line width for the MultiCurve plot.
//
//  Arguments:
//      lw      The line width for this MultiCurve plot.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

void
avtMultiCurvePlot::SetLineWidth(int lw)
{
    levelsMapper->SetLineWidth(Int2LineWidth(lw));
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::SetLineStyle
//
//  Purpose:
//      Sets the line style for the MultiCurve plot.
//
//  Arguments:
//      ls      The line style for this MultiCurve plot.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

void
avtMultiCurvePlot::SetLineStyle(int ls)
{
    levelsMapper->SetLineStyle(Int2LineStyle(ls));
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      on      True if the legend should be turned on, false otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   February 20, 2009
//
// ****************************************************************************

void
avtMultiCurvePlot::SetLegend(bool on)
{
    if (on)
    {
        levelsLegend->LegendOn();
    }
    else
    {
        levelsLegend->LegendOff();
    }
}


// ****************************************************************************
//  Method: avtMultiCurvePlot::EnhanceSpecification
//
//  Purpose:
//      Enhance the contract to request the secondary variable.
//
//  Returns:    The new contract.
//
//  Arguments:
//      spec    The existing.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

avtContract_p
avtMultiCurvePlot::EnhanceSpecification(avtContract_p spec)
{
    string mv = atts.GetMarkerVariable();
    if (mv == "default")
    {
        return spec;
    }
    avtDataRequest_p ds = spec->GetDataRequest();
    const char *primaryVariable = ds->GetVariable();
    if (mv == primaryVariable)
    {
        //
        // They didn't leave it as "default", but it is the same variable, so
        // don't read it in again.
        //
        return spec;
    }

    //
    // The pipeline specification should really be const -- it is used
    // elsewhere, so we can't modify it and return it.  Make a copy and in
    // the new copy, add a secondary variable.
    //
    avtDataRequest_p nds = new avtDataRequest(primaryVariable,
                                      ds->GetTimestep(), ds->GetRestriction());
    nds->AddSecondaryVariable(mv.c_str());
    avtContract_p rv = new avtContract(spec, nds);

    return rv;
}
