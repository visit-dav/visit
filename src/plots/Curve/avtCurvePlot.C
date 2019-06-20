/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                             avtCurvePlot.C                                //
// ************************************************************************* //



#include <avtCurvePlot.h>
#include <avtCurveMapper.h>

#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtCompactTreeFilter.h>
#include <avtCurveFilter.h>
#include <avtCurveLegend.h>
#include <avtLabeledCurveMapper.h>
#include <avtWarpFilter.h>
#include <avtPolarToCartesianFilter.h>

#include <LineAttributes.h>

#include <string>

// ****************************************************************************
//  Method: avtCurvePlot constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//
//    Hank Childs, Wed May 22 15:52:30 PDT 2002
//    Use a reference counted mapper to avoid problems when the plot has been
//    deleted but the actor is still part of a network.
//
//    Kathleen Bonnell, Fri Jul 12 16:53:11 PDT 2002  
//    Add a labeled curve mapper for decorations.  
//    
//    Kathleen Bonnell, Thu Oct 27 15:12:13 PDT 2005 
//    Added a legend.
//    
//    Kathleen Bonnell, Wed Jul 12 08:30:04 PDT 2006 
//    Added warp filter. 
//
//    Brad Whitlock, Mon Nov 20 10:16:08 PDT 2006
//    Changed to a curve renderer.
//
//    Kathleen Biagas, Wed Sep 11 17:14:48 PDT 2013
//    Added PolarFilter.
//
// ****************************************************************************

avtCurvePlot::avtCurvePlot()
{
    curveLegend = new avtCurveLegend;
    curveLegend->SetTitle("Curve");

    CurveFilter = new avtCurveFilter();
    WarpFilter = new avtWarpFilter();
    PolarFilter = new avtPolarToCartesianFilter();
    mapper = new avtCurveMapper();

    decoMapper = new avtLabeledCurveMapper();

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // CurveLegend.
    //
    curveLegendRefPtr = curveLegend;
}


// ****************************************************************************
//  Method: avtCurvePlot destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//
//    Hank Childs, Wed May 22 15:52:30 PDT 2002
//    Do not delete the renderer since it is now reference counted.
//
//    Kathleen Bonnell, Fri Jul 12 16:53:11 PDT 2002  
//    Delete the decorations mapper. 
//    
//    Kathleen Bonnell, Wed Jul 12 08:30:04 PDT 2006 
//    Added warp filter. 
//
//    Brad Whitlock, Mon Nov 20 10:16:25 PDT 2006
//    Removed property.
//
//    Kathleen Biagas, Wed Sep 11 17:15:06 PDT 2013
//    Added PolarFilter.
//
// ****************************************************************************

avtCurvePlot::~avtCurvePlot()
{
    if (mapper != NULL)
    {
        delete mapper;
        mapper = NULL;
    }
    if (CurveFilter != NULL)
    {
        delete CurveFilter;
        CurveFilter = NULL;
    }
    if (decoMapper != NULL)
    {
        delete decoMapper;
        decoMapper = NULL;
    }
    if (WarpFilter != NULL)
    {
        delete WarpFilter;
        WarpFilter = NULL;
    }
    if (PolarFilter != NULL)
    {
        delete PolarFilter;
        PolarFilter = NULL;
    }
}


// ****************************************************************************
//  Method:  avtCurvePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
// ****************************************************************************

avtPlot*
avtCurvePlot::Create()
{
    return new avtCurvePlot;
}


// ****************************************************************************
//  Method: avtCurvePlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
// ****************************************************************************

avtMapperBase *
avtCurvePlot::GetMapper(void)
{
    return mapper;
}


// ****************************************************************************
//  Method: avtCurvePlot::GetDecorationsMapper
//
//  Purpose:
//      Gets a decorations mapper for this plot, in this case the
//      labeled curve mapper. 
//
//  Returns:    The labeled curve mapper typed as its base class mapper.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

avtDecorationsMapper *
avtCurvePlot::GetDecorationsMapper(void)
{
    return decoMapper;
}

// ****************************************************************************
//  Method: avtCurvePlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Curve plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Curve plot has been applied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//
//    Hank Childs, Fri Oct  3 11:27:49 PDT 2003
//    No longer set attributes with curve filter.
//
// ****************************************************************************

avtDataObject_p
avtCurvePlot::ApplyOperators(avtDataObject_p input)
{
    CurveFilter->SetInput(input);
    return CurveFilter->GetOutput();
}


// ****************************************************************************
//  Method: avtCurvePlot::ApplyRenderingTransformation
//
//  Purpose:
//      Applies the rendering transformation associated with a Curve plot.  
//      Nothing for now.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The input data object. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 12 08:30:04 PDT 2006 
//    Added warp filter. 
//    
//    Kathleen Biagas, Wed Sep 11 17:15:21 PDT 2013
//    Use PolarFilter if user has requested the conversion.
//
// ****************************************************************************

avtDataObject_p
avtCurvePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    WarpFilter->SetInput(input);
    if (!atts.GetPolarToCartesian())
        return WarpFilter->GetOutput();
    else
    {
        PolarFilter->SetSwapCoords(
            atts.GetPolarCoordinateOrder()==CurveAttributes::Theta_R);
        PolarFilter->SetUseDegrees(
            atts.GetAngleUnits()==CurveAttributes::Degrees);
        PolarFilter->SetInput(WarpFilter->GetOutput());
        return PolarFilter->GetOutput();
    }
}


// ****************************************************************************
//  Method: avtCurvePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Curve plot.  This includes
//      behavior like shifting towards or away from the screen.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 10:23:51 PDT 2003
//    I added code to set the window mode to curve.
//
//    Kathleen Bonnell, Thu Oct 27 15:12:13 PDT 2005 
//    Set the legend.
//    
//    Mark C. Miller, Thu Jun 21 00:12:28 PDT 2007
//    Added support to overlay curve plots on 2D plots.
//    Ensure that curve plots are plotted on top of 2D plots
//
//    Sean Ahern, Wed Aug  6 16:50:41 EDT 2008
//    If we have no designator, set the label from the varname.  This makes
//    labeling of curves loaded from files work.
//
//    Kathleen Bonnell, Thu Feb 17 10:06:20 PST 2011
//    Use multiple labels if present in the atts.
//
// ****************************************************************************

void
avtCurvePlot::CustomizeBehavior(void)
{
    behavior->SetLegend(curveLegendRefPtr);
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_CURVE);
    behavior->SetRenderOrder(MUST_GO_LAST);

    stringVector labels;
    behavior->GetInfo().GetAttributes().GetLabels(labels);
    if (atts.GetDesignator() != "")
    {
        decoMapper->SetLabel(atts.GetDesignator());
    }
    else if (labels.size() > 0)
    {
        decoMapper->SetLabels(labels);
    }
    else if (varname != NULL)
    {
        // No designator set, so set it to the varname.
        std::string s(varname);
        decoMapper->SetLabel(s);
    }
}


// ****************************************************************************
//  Method: avtCurvePlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Curve plot.
//
//  Arguments:
//      atts    The attributes for this Curve plot.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002 
//    Set attributes for the decorations mapper.
//
//    Kathleen Bonnell, Tue Jul 23 14:12:47 PDT 2002       
//    LabelSize no longer an attribute. 
//
//    Kathleen Bonnell, Tue Dec 23 11:08:38 PST 2003 
//    Added ShowPoints and PointSize. 
//
//    Kathleen Bonnell, Thu Oct 27 15:12:13 PDT 2005 
//    Set the legend's color.
//
//    Brad Whitlock, Mon Nov 20 10:17:14 PDT 2006
//    Changed so it uses a curve renderer.
//
//    Kathleen Bonnell, Thu May  7 17:56:28 PDT 2009
//    Advertise window-mode here, since it never changes and may need
//    to be known before plot executes.
//
//    Alister Maguire, Mon Jun 11 10:24:59 PDT 2018
//    Set attributes for the time cue options. 
//
// ****************************************************************************

void
avtCurvePlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const CurveAttributes*)a);
    atts = *(const CurveAttributes*)a;

    double curveRgb[4];
    if (atts.GetCurveColorSource() == CurveAttributes::Cycle)
    {
        unsigned char rgb[3] = {0,0,0};
        unsigned char bg[3] = {static_cast<unsigned char>(bgColor[0]*255),
                               static_cast<unsigned char>(bgColor[1]*255),
                               static_cast<unsigned char>(bgColor[2]*255)};
        avtColorTables *ct = avtColorTables::Instance();
        if (! ct->GetJNDControlPointColor(ct->GetDefaultDiscreteColorTable(),
                                          this->instanceIndex, bg, rgb))
            ct->GetJNDControlPointColor("distinct", this->instanceIndex, bg, rgb);
        double drgb[4] = {rgb[0]/255.0,rgb[1]/255.0,rgb[2]/255.0,255.0};
        std::copy(drgb,drgb+4,curveRgb);
    }
    else
    {
        atts.GetCurveColor().GetRgb(curveRgb);
    }

    curveLegend->SetColor(curveRgb);
    mapper->SetColor(curveRgb);
    mapper->SetDrawCurve(atts.GetShowLines());
    mapper->SetDrawPoints(atts.GetShowPoints());
    mapper->SetPointSize(atts.GetPointSize());
    mapper->SetPointStride(atts.GetPointStride());
    mapper->SetSymbolType(atts.GetSymbol());
    mapper->SetStaticPoints(atts.GetPointFillMode() == CurveAttributes::Static);
    mapper->SetPointDensity(atts.GetSymbolDensity());
    mapper->SetTimeForTimeCue(atts.GetTimeForTimeCue());
    mapper->SetDoBallTimeCue(atts.GetDoBallTimeCue());
    mapper->SetTimeCueBallSize(atts.GetTimeCueBallSize());
    double ballRgb[3];
    atts.GetBallTimeCueColor().GetRgb(ballRgb);
    mapper->SetTimeCueBallColor(ballRgb);
    mapper->SetDoLineTimeCue(atts.GetDoLineTimeCue());
    mapper->SetTimeCueLineWidth(atts.GetLineTimeCueWidth());
    double lineRgb[3];
    atts.GetLineTimeCueColor().GetRgb(lineRgb);
    mapper->SetTimeCueLineColor(lineRgb);
    mapper->SetDoCropTimeCue(atts.GetDoCropTimeCue());
   
    if (atts.GetShowLegend())
    {
        curveLegend->LegendOn();
    }
    else
    {
        curveLegend->LegendOff();
    }

    SetLineWidth(atts.GetLineWidth());

    decoMapper->SetLabelColor(curveRgb);
    decoMapper->SetLabelVisibility(atts.GetShowLabels());

    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_CURVE);
}

// ****************************************************************************
//  Method: avtCurvePlot::SetLineWidth
//
//  Purpose:
//      Sets the line width.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 24, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 27 15:12:13 PDT 2005 
//    Set the legend's line width.
//
//    Brad Whitlock, Mon Nov 20 10:13:58 PDT 2006
//    Removed property.
//
// ****************************************************************************
 
void
avtCurvePlot::SetLineWidth(int lw)
{
    curveLegend->SetLineWidth(Int2LineWidth(lw));
    mapper->SetLineWidth(Int2LineWidth(lw));
}

// ****************************************************************************
//  Method: avtCurvePlot::SetBackgroundColor
//
//  Purpose: Sets the background color.
//
//  Returns: True if using this color will require the plot to be redrawn.
//
//  Mark C. Miller, Wed Jun 19 11:05:40 PDT 2019
// ****************************************************************************

bool
avtCurvePlot::SetBackgroundColor(const double *bg)
{
    bool retVal = false;

    if (bgColor[0] != bg[0] || bgColor[1] != bg[1] || bgColor[2] != bg[2])
    {
        bgColor[0] = bg[0];
        bgColor[1] = bg[1];
        bgColor[2] = bg[2];
        retVal = true;
    }

    return retVal;
}
 
// ****************************************************************************
//  Method: avtCurvePlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 12 08:30:04 PDT 2006 
//    Added warp filter. 
//    
// ****************************************************************************
 
void
avtCurvePlot::ReleaseData(void)
{
    avtLineDataPlot::ReleaseData();
 
    if (CurveFilter != NULL)
    {
        CurveFilter->ReleaseData();
    }
    if (WarpFilter != NULL)
    {
        WarpFilter->ReleaseData();
    }
}
