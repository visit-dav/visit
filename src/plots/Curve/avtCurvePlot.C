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
//                             avtCurvePlot.C                                //
// ************************************************************************* //

#include <avtCurvePlot.h>

#include <avtCurveFilter.h>
#include <avtCurveLegend.h>
#include <avtCompactTreeFilter.h>
#include <avtSurfaceAndWireframeRenderer.h> 
#include <avtWarpFilter.h>

#include <avtCallback.h>
#ifdef VTK_USE_MANGLED_MESA
#include <avtMesaCurveRenderer.h>
#endif
#include <avtOpenGLCurveRenderer.h>

#include <LineAttributes.h>

#include <avtUserDefinedMapper.h>
#include <avtLabeledCurveMapper.h>

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
//    Brad Whitlock, Wed Jun 10 14:06:34 PST 2009
//    I made Mesa support be conditional.
//
// ****************************************************************************

avtCurvePlot::avtCurvePlot()
{
    curveLegend = new avtCurveLegend;
    curveLegend->SetTitle("Curve");

    CurveFilter = new avtCurveFilter();
    WarpFilter = new avtWarpFilter();
#ifdef VTK_USE_MANGLED_MESA
    if (avtCallback::GetSoftwareRendering())
        renderer = new avtMesaCurveRenderer;
    else
#endif
        renderer = new avtOpenGLCurveRenderer;
    avtCustomRenderer_p ren;
    CopyTo(ren, renderer);
    mapper = new avtUserDefinedMapper(ren);
 
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

avtMapper *
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
// ****************************************************************************

avtDataObject_p
avtCurvePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    WarpFilter->SetInput(input);
    return WarpFilter->GetOutput();
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
// ****************************************************************************

void
avtCurvePlot::CustomizeBehavior(void)
{
    behavior->SetLegend(curveLegendRefPtr);
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_CURVE);
    behavior->SetRenderOrder(MUST_GO_LAST);

    if (atts.GetDesignator() != "")
    {
        decoMapper->SetLabel(atts.GetDesignator());
    }
    else if (varname != NULL)
    {
        // No designator set, so set it to the varname.
        string s(varname);
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
// ****************************************************************************

void
avtCurvePlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const CurveAttributes*)a);
    atts = *(const CurveAttributes*)a;

    double rgba[4];
    atts.GetColor().GetRgba(rgba);
    curveLegend->SetColor(rgba);

    if (atts.GetShowLegend())
    {
        curveLegend->LegendOn();
    }
    else
    {
        curveLegend->LegendOff();
    }

    SetLineWidth(atts.GetLineWidth());
    SetLineStyle(atts.GetLineStyle());

    decoMapper->SetLabelColor(rgba);
    decoMapper->SetLabelVisibility(atts.GetShowLabels());

    renderer->SetAtts(atts);
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
}
 
 
// ****************************************************************************
//  Method: avtCurvePlot::SetLineStyle
//
//  Purpose:
//      Sets the line style.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 24, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 27 15:12:13 PDT 2005 
//    Set the legend's line style.
//    
//    Brad Whitlock, Mon Nov 20 10:13:58 PDT 2006
//    Removed property.
//
// ****************************************************************************
 
void
avtCurvePlot::SetLineStyle(int ls)
{
    curveLegend->SetLineStyle(Int2LineStyle(ls));
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
