/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                             avtStreamlinePlot.C                           //
// ************************************************************************* //

#include <avtStreamlinePlot.h>

#include <avtShiftCenteringFilter.h>
#include <avtStreamlinePolyDataFilter.h>
#include <avtStreamlineRenderer.h>
#include <avtUserDefinedMapper.h>
#include <avtVariableLegend.h>
#include <avtVariableMapper.h>
#include <avtLookupTable.h>

#include <avtCallback.h>

#include <StreamlineAttributes.h>

// ****************************************************************************
//  Method: avtStreamlinePlot constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 11:14:52 PDT 2003
//    Modified to work with the new way legends are managed.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
// ****************************************************************************

avtStreamlinePlot::avtStreamlinePlot()
{
#ifdef ENGINE
    streamlineFilter = new avtStreamlinePolyDataFilter;
#endif
    shiftCenteringFilter = NULL;
    avtLUT = new avtLookupTable; 
    renderer = avtStreamlineRenderer::New();
    
    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    mapper  = new avtUserDefinedMapper(cr);

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Streamline");

    //
    // This is to allow the legend to be reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;
}

// ****************************************************************************
//  Method: avtStreamlinePlot destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
// ****************************************************************************

avtStreamlinePlot::~avtStreamlinePlot()
{
#ifdef ENGINE
    if (streamlineFilter != NULL)
    {
        delete streamlineFilter;
        streamlineFilter = NULL;
    }
#endif
    if (shiftCenteringFilter != NULL)
    {
        delete shiftCenteringFilter;
        shiftCenteringFilter = NULL;
    }
    if (mapper != NULL)
    {
        delete mapper;
        mapper = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }

    renderer = NULL;

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}

// ****************************************************************************
//  Method:  avtStreamlinePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
// ****************************************************************************

avtPlot*
avtStreamlinePlot::Create()
{
    return new avtStreamlinePlot;
}

// ****************************************************************************
//  Method: avtStreamlinePlot::GetMapper
//
//  Purpose:
//      Gets a mapper for this plot, it is actually a variable mapper.
//
//  Returns:    The variable mapper typed as its base class mapper.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
// ****************************************************************************

avtMapper *
avtStreamlinePlot::GetMapper(void)
{
    return mapper;
}

// ****************************************************************************
//  Method: avtStreamlinePlot::ApplyOperators
//
//  Purpose:
//      Applies the operators associated with a Streamline plot.  
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the Streamline plot has been applied.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
//    Hank Childs, Sat Mar  3 10:56:01 PST 2007
//    Test to make sure we have a valid variable before asking for its 
//    centering.
//
//    Dave Pugmire, Tue Mar 23 11:11:11 EDT 2010
//    Do zone-to-node centering, if needed.
//
// ****************************************************************************

avtDataObject_p
avtStreamlinePlot::ApplyOperators(avtDataObject_p input)
{
#ifdef ENGINE
    avtDataObject_p dob = input; 

    // Try to determine the centering.  If we have an expression, we won't
    // be able to.  So be conservative and assume the worst.
    avtCentering centering = AVT_ZONECENT;
    if (input->GetInfo().GetAttributes().ValidVariable(varname))
        centering = input->GetInfo().GetAttributes().GetCentering(varname);

    //Convert from zonal to nodal, if requested.
    if(centering == AVT_ZONECENT && atts.GetForceNodeCenteredData())
    {
        if(shiftCenteringFilter != NULL)
            delete shiftCenteringFilter;
        shiftCenteringFilter = new avtShiftCenteringFilter(AVT_NODECENT);
        shiftCenteringFilter->SetInput(input);
        dob = shiftCenteringFilter->GetOutput();
    }

    // Add the streamline filter.
    streamlineFilter->SetInput(dob);
    dob = streamlineFilter->GetOutput();

    return dob;
#else
    return input;
#endif
}

// ****************************************************************************
// Method: avtStreamlinePlot::ApplyRenderingTransformation
//
// Purpose: 
//   Performs the rendering transforamtion for a Streamline plot.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The input data object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 24 14:20:21 PST 2002
//
// Modifications:
//   
// ****************************************************************************

avtDataObject_p
avtStreamlinePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}

// ****************************************************************************
//  Method: avtStreamlinePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior as appropriate for a Streamline plot.  This 
//      includes behavior like shifting towards or away from the screen.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//
//    Hank Childs, Tue Aug 12 10:55:03 PDT 2008
//    Shift the streamline plot towards the front, so it will show up over 
//    Pseudocolor plots.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Hank Childs, Wed Sep 29 19:11:51 PDT 2010
//   Rename "None" to "FullyOpaque".
//
// ****************************************************************************

void
avtStreamlinePlot::CustomizeBehavior(void)
{
    UpdateMapperAndLegend();

    if (atts.GetOpacityType() != StreamlineAttributes::FullyOpaque)
    {
        behavior->SetRenderOrder(MUST_GO_LAST);
        behavior->SetAntialiasedRenderOrder(MUST_GO_LAST);
    }

    behavior->SetLegend(varLegendRefPtr);
    behavior->SetShiftFactor(0.3);
}


// ****************************************************************************
//  Method: avtStreamlinePlot::EnhanceSpecification
//
//  Purpose:
//      Modifies the contract to tell it we want the "colorVar" to be the 
//      primary variable for the pipeline.  If we don't do that, the primary
//      variable will be some vector variable and it will confuse our mapper.
//      The only reason that this works is that the streamline filter 
//      understands the colorVar trick and produces that variable.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2008
//
// ****************************************************************************

avtContract_p
avtStreamlinePlot::EnhanceSpecification(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    const char *var = in_dr->GetVariable();
    avtDataRequest_p out_dr = new avtDataRequest(in_dr, "colorVar");
    out_dr->AddSecondaryVariable(var);
    out_dr->SetOriginalVariable(var);
    avtContract_p out_contract = new avtContract(in_contract, out_dr);
    return out_contract;
}


// ****************************************************************************
//  Method: avtStreamlinePlot::SetAtts
//
//  Purpose:
//      Sets the atts for the Streamline plot.
//
//  Arguments:
//      atts    The attributes for this Streamline plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 13:02:51 PST 2004
//    Added support for coloring by vorticity. I also added support for
//    ribbons.
//
//    Hank Childs, Sat Mar  3 09:52:01 PST 2007
//    Set use whole box for the streamline filter.  Also don't show a 
//    color bar if the coloring is constant.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
//   Dave Pugmire, Wed Aug 6 15:16:23 EST 2008
//   Add accurate distance calculate option.
//
//   Dave Pugmire, Tue Aug 19 17:13:04EST 2008
//   Remove accurate distance calculate option.
//
//   Dave Pugmire, Thu Feb  5 12:23:33 EST 2009
//   Add workGroupSize for masterSlave algorithm.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Add pathline option.
//
//   Jeremy Meredith, Wed Apr  8 16:48:05 EDT 2009
//   Initial steps to unification with Poincare attributes.
//   Hank Childs, Sun May  3 12:32:13 CDT 2009
//   Added support for point list source types.
//
//   Dave Pugmire, Wed Jun 10 16:26:25 EDT 2009
//   Add color by variable.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Removed radius and showStart from the filter.
//
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Safer setting of attributes (more checks for enums).
//
//   Hank Childs, Sun Jun  6 11:53:33 CDT 2010
//   Change method name from streamline direction to integration direction.
//
//   Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//   New seed sources.
//
//   Hank Childs, Tue Sep  7 23:29:40 PDT 2010
//   Tell the legend what color table we're using.
//
//   Hank Childs, Wed Sep 29 20:52:04 PDT 2010
//   The DoPri option now has an explicit field for max time step.
//
//   Hank Childs, Fri Oct  1 20:35:21 PDT 2010
//   Add support for absolute tolerances that are a fraction of the bounding
//   box.
//
// ****************************************************************************

void
avtStreamlinePlot::SetAtts(const AttributeGroup *a)
{
    renderer->SetAtts(a);
    needsRecalculation = atts.ChangesRequireRecalculation(*(const StreamlineAttributes*)a);
    atts = *(const StreamlineAttributes*)a;

#ifdef ENGINE
    //
    // Set the filter's attributes based on the plot attributes.
    //

    switch (atts.GetSourceType())
    {
      case StreamlineAttributes::SpecifiedPoint:
        streamlineFilter->SetPointSource(atts.GetPointSource());
        break;
      case StreamlineAttributes::SpecifiedPointList:
        streamlineFilter->SetPointListSource(atts.GetPointList());
        break;

      case StreamlineAttributes::SpecifiedLine:
        streamlineFilter->SetLineSource(atts.GetLineStart(), atts.GetLineEnd(),
                                        atts.GetSampleDensity0(),
                                        atts.GetRandomSamples(), atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;
      case StreamlineAttributes::SpecifiedPlane:
        streamlineFilter->SetPlaneSource(atts.GetPlaneOrigin(),
                                         atts.GetPlaneNormal(),
                                         atts.GetPlaneUpAxis(),
                                         atts.GetSampleDensity0(), atts.GetSampleDensity1(),
                                         atts.GetSampleDistance0(), atts.GetSampleDistance1(),
                                         atts.GetFillInterior(),
                                         atts.GetRandomSamples(), atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;

      case StreamlineAttributes::SpecifiedCircle:
        streamlineFilter->SetCircleSource(atts.GetPlaneOrigin(),
                                          atts.GetPlaneNormal(),
                                          atts.GetPlaneUpAxis(),
                                          atts.GetRadius(),
                                          atts.GetSampleDensity0(), atts.GetSampleDensity1(),
                                          atts.GetFillInterior(),
                                          atts.GetRandomSamples(), atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;
      case StreamlineAttributes::SpecifiedSphere:
        streamlineFilter->SetSphereSource(atts.GetSphereOrigin(), atts.GetRadius(),
                                          atts.GetSampleDensity0(), atts.GetSampleDensity1(), atts.GetSampleDensity2(),
                                          atts.GetFillInterior(),
                                          atts.GetRandomSamples(), atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;

      case StreamlineAttributes::SpecifiedBox:
        streamlineFilter->SetBoxSource(atts.GetBoxExtents(),atts.GetUseWholeBox(),
                                       atts.GetSampleDensity0(), atts.GetSampleDensity1(), atts.GetSampleDensity2(),
                                       atts.GetFillInterior(),
                                       atts.GetRandomSamples(), atts.GetRandomSeed(), atts.GetNumberOfRandomSamples());
        break;
    }

    streamlineFilter->SetPathlines(atts.GetPathlines());

    streamlineFilter->SetIntegrationType(atts.GetIntegrationType());
    streamlineFilter->SetStreamlineAlgorithm(atts.GetStreamlineAlgorithmType(), 
                                             atts.GetMaxStreamlineProcessCount(),
                                             atts.GetMaxDomainCacheSize(),
                                             atts.GetWorkGroupSize());
    if (atts.GetIntegrationType() == StreamlineAttributes::DormandPrince)
    {
        // For DoPri, the max time step is sent in to the PICS filter as the max step length.
        double step = atts.GetMaxTimeStep();
        if (! atts.GetLimitMaximumTimestep())
            step = 0;
        streamlineFilter->SetMaxStepLength(step);
    }
    else
        streamlineFilter->SetMaxStepLength(atts.GetMaxStepLength());
    double absTol = 0.;
    bool doBBox = (atts.GetAbsTolSizeType() == StreamlineAttributes::FractionOfBBox);
    if (doBBox)
        absTol = atts.GetAbsTolBBox();
    else
        absTol = atts.GetAbsTolAbsolute();
    streamlineFilter->SetTolerances(atts.GetRelTol(), absTol, doBBox);

    streamlineFilter->SetTermination(atts.GetTerminationType(),
                                     atts.GetTermination());
    streamlineFilter->SetDisplayMethod(atts.GetDisplayMethod());

    streamlineFilter->SetIntegrationDirection(atts.GetStreamlineDirection());

    streamlineFilter->SetColoringMethod(int(atts.GetColoringMethod()),
                                        atts.GetColoringVariable());

    if (atts.GetOpacityType() == StreamlineAttributes::VariableRange)
        streamlineFilter->SetOpacityVariable(atts.GetOpacityVariable());
#endif

    UpdateMapperAndLegend();

    SetColorTable(atts.GetColorTableName().c_str());
    if (atts.GetLegendFlag())
    {
        varLegend->LegendOn();
        varLegend->SetLookupTable(avtLUT->GetLookupTable());
    }
    else
        varLegend->LegendOff();
    
    if (atts.GetColoringMethod() == StreamlineAttributes::Solid)
        avtLUT->SetLUTColors(atts.GetSingleColor().GetColor(), 1);
    else
        varLegend->SetLookupTable(avtLUT->GetLookupTable());

    SetLighting(atts.GetLightingFlag());
}

// ****************************************************************************
// Method: avtStreamlinePlot::SetColorTable
//
// Purpose: 
//   Sets the color table for the plot.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    If the color table changed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 15:40:41 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtStreamlinePlot::SetColorTable(const char *ctName)
{
    bool namesMatch = (atts.GetColorTableName() == std::string(ctName));

    if (atts.GetColorTableName() == "Default")
        return avtLUT->SetColorTable(NULL, namesMatch);
    else
        return avtLUT->SetColorTable(ctName, namesMatch);
}


// ****************************************************************************
//  Method: avtStreamlinePlot::SetLighting
//
//  Purpose:
//      Turns the lighting on or off.
//
//  Arguments:
//      lightingOn   true if the lighting should be turned on, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//   Hank Childs, Fri Oct 29 10:06:24 PDT 2004
//   Account for specular lighting.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Hank Childs, Sun Aug 29 21:08:30 PDT 2010
//   Lighting options were reversed ... this only showed up in SR mode.
//
// ****************************************************************************

void
avtStreamlinePlot::SetLighting(bool lightingOn)
{
    if (lightingOn)
    {
        mapper->GlobalLightingOn();
    }
    else
    {
        mapper->GlobalLightingOff();
    }
}

// ****************************************************************************
// Method: avtStreamlinePlot::UpdateMapperAndLegend
//
// Purpose: 
//   Sets the range to use for the legend.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 16:23:06 PST 2002
//
// Modifications:
//   
//    Hank Childs, Tue Aug 12 15:04:40 PDT 2008
//    If we don't have any streamlines, make sure something sensible shows up.
//
//   Dave Pugmire, Wed Jun 10 16:26:25 EDT 2009
//   Don't clamp max to 0.0. We can now color by a variable.
//
//   Dave Pugmire, Mon Nov 23 09:38:53 EST 2009
//   Add min/max options to color table.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Hank Childs, Thu Feb 18 11:29:22 PST 2010
//   Set up a reasonable range if there are no streamlines.
//
//   Hank Childs, Tue Sep  7 23:29:40 PDT 2010
//   Tell the legend what variable we're using.
//
// ****************************************************************************

void
avtStreamlinePlot::UpdateMapperAndLegend()
{
    double min = 0.0, max = 1.0;
    if (*(mapper->GetInput()) != NULL)
        mapper->GetRange(min, max);

    if (atts.GetLegendMinFlag())
        min = atts.GetLegendMin();
    if (atts.GetLegendMaxFlag())
        max = atts.GetLegendMax();

    if (max < -1e+30 && min > 1e+30)
    {
        min = 0;
        max = 1;
    }

    renderer->SetRange(min, max);
    
    // Set the range for the legend's text and colors.
    varLegend->SetScaling(0);
    varLegend->SetVarRange(min, max);
    varLegend->SetRange(min, max);
    switch (atts.GetColoringMethod())
    {
      case StreamlineAttributes::Solid:
        varLegend->SetVarName("Solid");
        break;
      case StreamlineAttributes::ColorBySpeed:
        varLegend->SetVarName("Speed");
        break;
      case StreamlineAttributes::ColorByVorticity:
        varLegend->SetVarName("Vorticity magnitude");
        break;
      case StreamlineAttributes::ColorByLength:
        varLegend->SetVarName("Arc length");
        break;
      case StreamlineAttributes::ColorByTime:
        varLegend->SetVarName("Time");
        break;
      case StreamlineAttributes::ColorBySeedPointID:
        varLegend->SetVarName("Seed point ID");
        break;
      case StreamlineAttributes::ColorByVariable:
        varLegend->SetVarName(atts.GetColoringVariable().c_str());
        break;
    }
}

// ****************************************************************************
// Method: avtStreamlinePlot::ReleaseData
//
// Purpose: 
//   Tells the the plot's filters to release their data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 6 15:52:07 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtStreamlinePlot::ReleaseData(void)
{
    avtLineDataPlot::ReleaseData();

#ifdef ENGINE
    if(streamlineFilter != NULL)
        streamlineFilter->ReleaseData();
#endif
    if(shiftCenteringFilter != NULL)
        shiftCenteringFilter->ReleaseData();
}
