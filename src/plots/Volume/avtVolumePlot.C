/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtVolumePlot.C                             //
// ************************************************************************* //

#include "avtVolumePlot.h"

#include <limits.h>
#include <math.h>

#include <avtCallback.h>
#include <avtVolumeRenderer.h>
#include <avtLookupTable.h>
#include <avtResampleFilter.h>
#include <avtShiftCenteringFilter.h>
#include <avtUserDefinedMapper.h>
#include <avtVolumeFilter.h>

#include <VolumeAttributes.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <LostConnectionException.h>


// ****************************************************************************
//  Method: avtVolumePlot constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 19 15:10:55 PST 2001
//    Added a legend.
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    Added avtLUT. 
//
//    Hank Childs, Tue Nov 20 17:45:22 PST 2001
//    Initialized volume filter.
//
//    Hank Childs, Fri Feb  8 19:38:57 PST 2002
//    Initialized shift centering filter.
//
//    Hank Childs, Wed Apr 24 09:38:36 PDT 2002
//    Make use of instantiation method that insulate us from graphics libs.
//
//    Hank Childs, Mon May 20 10:47:18 PDT 2002
//    Added reference pointer construct because renderers are now reference
//    counted.
//
//    Eric Brugger, Wed Jul 16 11:35:47 PDT 2003
//    Modified to work with the new way legends are managed.
//
//    Hank Childs, Wed Nov 24 17:03:44 PST 2004
//    Removed hacks involved with previous software volume rendering mode.
//
// ****************************************************************************

avtVolumePlot::avtVolumePlot() : avtVolumeDataPlot()
{
    volumeFilter = NULL;
    resampleFilter = NULL;
    shiftCentering = NULL;
    renderer = avtVolumeRenderer::New();

    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    mapper = new avtUserDefinedMapper(cr);

    avtLUT = new avtLookupTable();

    varLegend = new avtVolumeVariableLegend;
    varLegend->SetTitle("Volume");

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a 
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;
}

// ****************************************************************************
//  Method: avtVolumePlot destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001A
//    Added avtLUT.
//
//    Hank Childs, Tue Nov 20 17:50:55 PST 2001
//    Delete volume filter.
//
//    Hank Childs, Fri Feb  8 19:38:57 PST 2002
//    Delete shift centering filter.
//
//    Hank Childs, Mon May 20 10:47:18 PDT 2002
//    Don't delete the renderer, since it is now reference counted.
//
//    Hank Childs, Wed Nov 24 17:03:44 PST 2004
//    Removed hacks involved with previous software volume rendering mode.
//
// ****************************************************************************

avtVolumePlot::~avtVolumePlot()
{
    delete mapper;
    delete shiftCentering;
    delete avtLUT;

    if (volumeFilter != NULL)
        delete volumeFilter;
    if (resampleFilter != NULL)
        delete resampleFilter;
    renderer = NULL;

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}


// ****************************************************************************
//  Method: avtVolumePlot::PlotIsImageBased
//
//  Purpose:
//      Determines if the plot is image based, meaning that it can't run as a
//      standard SR plot.
//
//  Programmer: Hank Childs
//  Creation:   November 24, 2004
//
//  Modifications:
//
//    Brad Whitlock, Wed Dec 15 09:33:40 PDT 2004
//    I made it use the renderer type instead.
//
//    Hank Childs, Mon Sep 11 14:50:28 PDT 2006
//    Add support for the integration ray function.
//
// ****************************************************************************

bool
avtVolumePlot::PlotIsImageBased(void)
{
    return (atts.GetRendererType() == VolumeAttributes::RayCasting ||
            atts.GetRendererType() == VolumeAttributes::RayCastingIntegration);
}


// ****************************************************************************
//  Method:  avtVolumePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 27, 2001
//
// ****************************************************************************

avtPlot*
avtVolumePlot::Create()
{
    return new avtVolumePlot;
}

// ****************************************************************************
//  Method: avtVolumePlot::SetAtts
//
//  Purpose:
//    Sets the plot with the attributes.
//
//  Arguments:
//    atts : The attributes for this isoVolume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 19 16:06:43 PST 2001
//    Added code to set the legend.
//
//    Jeremy Meredith, Tue Jun  5 20:33:13 PDT 2001
//    Added code to set a flag if the plot needs recalculation.
//
//    Hank Childs, Fri Dec 14 09:02:05 PST 2001
//    Set the legend range correctly using artificial limits if appropriate.
//
//    Eric Brugger, Wed Jul 16 11:35:47 PDT 2003
//    Modified to work with the new way legends are managed.
//
// ****************************************************************************

void
avtVolumePlot::SetAtts(const AttributeGroup *a)
{
    renderer->SetAtts(a);
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const VolumeAttributes*)a);
    atts = *(const VolumeAttributes*)a;

    SetLegendOpacities();

    double min, max;
    varLegend->GetRange(min, max);
    if (atts.GetUseColorVarMin())
    {
        min = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        max = atts.GetColorVarMax();
    }
    varLegend->SetRange(min, max);

    SetLegend(atts.GetLegendFlag());
}

// ****************************************************************************
// Method: avtVolumePlot::SetLegendOpacities
//
// Purpose: 
//   Copies the opacity values from the attributes into the legend.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 20 12:14:18 PDT 2001
//
// Modifications:
//   
//   Hank Childs, Wed Aug 15 10:13:58 PDT 2001
//   Also set attributes for color table with the legend.
//
//   Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001 
//   Set colors in avtLUT instead of legend.  Use avtLUT to set the
//   legend's lut. 
//
//   Brad Whitlock, Thu Sep 6 11:25:26 PDT 2001
//   Modified to account for changes in VolumeAttributes.
//
//   Brad Whitlock, Thu Nov 21 15:15:03 PST 2002
//   The GetColors method moved into the color control point list.
//
// ****************************************************************************

void
avtVolumePlot::SetLegendOpacities()
{
    unsigned char opacity[256];

    // Get the opacity values from the attributes.
    atts.GetOpacities(opacity);
    varLegend->SetLegendOpacities(opacity);

    unsigned char rgb[256*3];
    atts.GetColorControlPoints().GetColors(rgb, 256);
    avtLUT->SetLUTColors(rgb, 256);
    varLegend->SetLookupTable(avtLUT->GetLookupTable());
}


// ****************************************************************************
//  Method: avtVolumePlot::GetNumberOfStagesForImageBasedPlot
//
//  Purpose:
//      Determines the number of stages for this plot.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
// ****************************************************************************

int
avtVolumePlot::GetNumberOfStagesForImageBasedPlot(const WindowAttributes &a)
{
    return volumeFilter->GetNumberOfStages(a);
}

// ****************************************************************************
//  Method: avtVolumePlot::ImageExecute
//
//  Purpose:
//      Takes an input image and adds a volume rendering to it.
//
//  Programmer: Hank Childs
//  Creation:   November 24, 2004
//
// ****************************************************************************

avtImage_p
avtVolumePlot::ImageExecute(avtImage_p input,
                            const WindowAttributes &window_atts)
{
    avtImage_p rv = input;

    if (volumeFilter != NULL)
    {
        volumeFilter->SetAttributes(atts);
        rv = volumeFilter->RenderImage(input, window_atts);
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtVolumePlot::SetLegend
//
//  Purpose:
//    Turns the legend on or off.
//
//  Arguments:
//    legendOn : true if the legend should be turned on, false otherwise.
//
//  Programmer: Jeremy Meredith 
//  Creation:   March 27, 2001
//
// ****************************************************************************

void
avtVolumePlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        varLegend->LegendOn();
    }
    else
    {
        varLegend->LegendOff();
    }
}

// ****************************************************************************
//  Method: avtVolumePlot::GetMapper
//
//  Purpose:
//      Gets the mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
// ****************************************************************************

avtMapper *
avtVolumePlot::GetMapper(void)
{
    return mapper;
}

// ****************************************************************************
//  Method: avtVolumePlot::ApplyOperators
//
//  Purpose:
//      Performs the implied operators for an isoVolume plot, namely,
//      an avtVolumeFilter. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the volume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Fri Apr  6 10:56:18 PDT 2001
//    Made it use a 50k target point value so we are guaranteed square voxels.
//
//    Hank Childs, Fri Jun 15 09:16:54 PDT 2001
//    Use more general data objects as input and output.
//
//    Jeremy Meredith, Tue Nov 13 11:45:10 PST 2001
//    Added setting of target value from the plot attributes.
//
//    Hank Childs, Tue Nov 20 17:45:22 PST 2001
//    Add volume filter as an implied operator.
//
//    Hank Childs, Fri Feb  8 19:35:50 PST 2002
//    Add shift centering as an implied operator.
//
//    Hank Childs, Wed Nov 24 17:03:44 PST 2004
//    No longer apply the volume filter as an operator.
//
// ****************************************************************************

avtDataObject_p
avtVolumePlot::ApplyOperators(avtDataObject_p input)
{
    avtDataObject_p dob = input;

    //
    // Clean up any old filters.
    //
    if (shiftCentering != NULL)
    {
        delete shiftCentering;
        shiftCentering = NULL;
    }

    //
    // Only shift the centering if the toggle is set.
    //
    if (atts.GetSmoothData())
    {
        shiftCentering = new avtShiftCenteringFilter(1);
        shiftCentering->SetInput(dob);
        dob = shiftCentering->GetOutput();
    }

    return dob;
}

// ****************************************************************************
//  Method: avtVolumePlot::ApplyRenderingTransformation
//
//  Purpose:
//      Performs the rendering transforamtion for a Volume plot.
//
//  Notes:
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
//    Hank Childs, Wed Nov 24 17:03:44 PST 2004
//    Apply the rendering transformation -- resampling in hardware, volume
//    filter for ray tracing.
//
//    Brad Whitlock, Wed Dec 15 09:34:33 PDT 2004
//    I made it use the renderer type instead.
//
//    Hank Childs, Mon Sep 11 14:50:28 PDT 2006
//    Added support for the integration ray function.
//
// ****************************************************************************

avtDataObject_p
avtVolumePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    //
    // Clean up any old filters.
    //
    if (volumeFilter != NULL)
    {
        delete volumeFilter;
        volumeFilter = NULL;
    }
    if (resampleFilter != NULL)
    {
        delete resampleFilter;
        resampleFilter = NULL;
    }

    avtDataObject_p dob = input;
    if (atts.GetRendererType() == VolumeAttributes::RayCasting ||
        atts.GetRendererType() == VolumeAttributes::RayCastingIntegration)
    {
        volumeFilter = new avtVolumeFilter();
        volumeFilter->SetAttributes(atts);
        volumeFilter->SetInput(input);
        dob = volumeFilter->GetOutput();
    }
    else
    {
        ResampleAttributes resampleAtts;
        resampleAtts.SetTargetVal(atts.GetResampleTarget());
        resampleAtts.SetUseTargetVal(true);
        resampleAtts.SetPrefersPowersOfTwo(
                    atts.GetRendererType() == VolumeAttributes::Texture3D);
        resampleFilter = new avtResampleFilter(&resampleAtts);

        resampleFilter->SetInput(input);
        dob = resampleFilter->GetOutput();
    }

    return dob;
}


// ****************************************************************************
//  Method: avtVolumePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.  
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 19 16:15:23 PST 2001
//    Added a legend.
//
//    Kathleen Bonnell, Thu Sep 18 13:40:26 PDT 2003 
//    Set anti-aliased render order to be the same as normal.
//
//    Hank Childs, Tue Nov 30 09:03:00 PST 2004
//    Make sure legend always reflects artificial ranges.
//
// ****************************************************************************

void
avtVolumePlot::CustomizeBehavior(void)
{
    // make it go last
    behavior->SetRenderOrder(MUST_GO_LAST);
    behavior->SetAntialiasedRenderOrder(MUST_GO_LAST);

    // Add a legend.
    double min, max;
    mapper->GetRange(min, max);
    varLegend->SetVarRange(min, max);
    if (atts.GetUseColorVarMin())
    {
        min = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        max = atts.GetColorVarMax();
    }
    varLegend->SetRange(min, max);
    behavior->SetLegend(varLegendRefPtr);
}


// ****************************************************************************
//  Method: avtVolumePlot::EnhanceSpecification
//
//  Purpose:
//      This is a hook from the base class that allows the plot to add to the
//      specification.  It is needed for this derived type because we want
//      to (potentially) specify that another variable gets read in for the
//      opacity.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2001
//
// ****************************************************************************

avtPipelineSpecification_p
avtVolumePlot::EnhanceSpecification(avtPipelineSpecification_p spec)
{
    string ov = atts.GetOpacityVariable();
    if (ov == "default")
    {
        return spec;
    }
    avtDataSpecification_p ds = spec->GetDataSpecification();
    const char *primaryVariable = ds->GetVariable();
    if (ov == primaryVariable)
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
    avtDataSpecification_p nds = new avtDataSpecification(primaryVariable,
                                      ds->GetTimestep(), ds->GetRestriction());
    nds->AddSecondaryVariable(ov.c_str());
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec, nds);

    return rv;
}


// ****************************************************************************
//  Method: avtVolumePlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
// ****************************************************************************
 
void
avtVolumePlot::ReleaseData(void)
{
    avtVolumeDataPlot::ReleaseData();
 
    if (volumeFilter != NULL)
    {
        volumeFilter->ReleaseData();
    }
    if (shiftCentering != NULL)
    {
        shiftCentering->ReleaseData();
    }
}


// ****************************************************************************
//  Method:  avtVolumePlot::Equivalent
//
//  Purpose:
//    Returns true if the plots are equivalent.
//
//  Arguments:
//    a          the atts to compare
//
//  Programmer:  Hank Childs
//  Creation:    November 20, 2001
//
// ****************************************************************************

bool
avtVolumePlot::Equivalent(const AttributeGroup *a)
{
    const VolumeAttributes *objAtts = (const VolumeAttributes *)a;
    // Almost the inverse of changes require recalculation -- doSoftware being
    // different is okay!
    if (atts.GetOpacityVariable() != objAtts->GetOpacityVariable())
        return false;
    if (atts.GetResampleTarget() != objAtts->GetResampleTarget())
        return false;
    return true;
}


