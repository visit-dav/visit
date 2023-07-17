// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtVolumePlot.C                             //
// ************************************************************************* //

#include "avtVolumePlot.h"

#include <limits.h>
#include <math.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtGradientExpression.h>
#include <avtVolumeRenderer.h>
#include <avtLookupTable.h>
#include <avtResampleFilter.h>
#include <avtShiftCenteringFilter.h>
#include <avtUserDefinedMapper.h>
#include <avtVolumeFilter.h>
#include <avtVolumeResampleFilter.h>
#include <avtLowerResolutionVolumeFilter.h>
#include <avtTypes.h>

#include <VolumeAttributes.h>

#include <DebugStream.h>
#include <InvalidLimitsException.h>
#include <ImproperUseException.h>

#ifdef ENGINE
#include <avtParallel.h>
#endif

#include <string>

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
//    Brad Whitlock, Mon Dec 15 15:51:38 PST 2008
//    I added another filter.
//
// ****************************************************************************

avtVolumePlot::avtVolumePlot() : avtVolumeDataPlot()
{
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
//    Brad Whitlock, Mon Dec 15 15:52:01 PST 2008
//    I added another filter.
//
// ****************************************************************************

avtVolumePlot::~avtVolumePlot()
{
    delete mapper;
    delete shiftCentering;
    delete avtLUT;

    if (lowResVolumeFilter != nullptr)
        delete lowResVolumeFilter;
    if (volumeFilter != nullptr)
        delete volumeFilter;
    if (volumeResampleFilter != nullptr)
        delete volumeResampleFilter;
    if (gradientFilter != nullptr)
        delete gradientFilter;

    renderer = nullptr; // Deleting the mapper deletes the renderer?????

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}


// ****************************************************************************
//  Method: avtVolumePlot::ManagesOwnTransparency
//
//  Purpose:
//      Declares whether or not this plot manages its own transparency.
//      This is one factor in the decision of whether or not to use Ice-T.
//
//  Programmer: Hank Childs
//  Creation:   February 21, 2010
//
// ****************************************************************************

bool
avtVolumePlot::ManagesOwnTransparency(void)
{
    return !PlotIsImageBased();
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
    return (atts.GetRendererType() == VolumeAttributes::Composite ||
            atts.GetRendererType() == VolumeAttributes::Integration ||
            atts.GetRendererType() == VolumeAttributes::SLIVR ||
            atts.GetRendererType() == VolumeAttributes::Parallel);
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
//    Hank Childs, Tue Feb  6 15:12:04 PST 2007
//    Have the legend reflect the scaling (log, linear, skew).  Also make sure
//    that accurate numbers are put the legends range.
//
//    Kathleen Biagas, Mon Feb 7, 2022
//    Only set the legend range when mapper has input. Also added checks
//    for valid ranges.
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

    // SetAtts can be called before the mapper has data, so don't set
    // legend's range unless there is input, as invalid ranges can cause
    // vtkError messages
    if (*(mapper->GetInput()) != NULL)
    {
        double min = 0., max = 1.;
        mapper->GetRange(min, max);

        if (atts.GetUseColorVarMin())
        {
            min = atts.GetColorVarMin();
        }
        if (atts.GetUseColorVarMax())
        {
            max = atts.GetColorVarMax();
        }
        if (min > max)
        {
            EXCEPTION1(InvalidLimitsException, false);
        }
        varLegend->SetRange(min, max);
    }
    if (atts.GetScaling() == VolumeAttributes::Linear)
        varLegend->SetScaling(0);
    else if (atts.GetScaling() == VolumeAttributes::Log)
        varLegend->SetScaling(1);
    else if (atts.GetScaling() == VolumeAttributes::Skew)
        varLegend->SetScaling(2, atts.GetSkewFactor());

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
avtVolumePlot::GetNumberOfStagesForImageBasedPlot(const WindowAttributes &a) const
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

    if (volumeFilter != nullptr)
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
//      Gets the mapper as its base class (avtMapperBase) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
// ****************************************************************************

avtMapperBase *
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
//    Hank Childs, Fri Feb  8 19:35:50 PST 2002
//    Add shift centering as an implied operator.
//
//    Sean Ahern, Wed Sep 10 13:25:54 EDT 2008
//    For ease of code reading and maintenance, I forced the
//    avtShiftCenteringFilter to take avtCentering type, rather than
//    int.
//
//    Kathleen Biagas, Wed July 12, 2023
//    Add volumeResampleFilter.
//
// ****************************************************************************

avtDataObject_p
avtVolumePlot::ApplyOperators(avtDataObject_p input)
{
    avtDataObject_p dob = input;

    //
    // Clean up any old filters.
    //
    if (shiftCentering != nullptr)
    {
        delete shiftCentering;
        shiftCentering = nullptr;
    }

    //
    // Only shift the centering if the toggle is set.
    //
    if (atts.GetSmoothData())
    {
        shiftCentering = new avtShiftCenteringFilter(AVT_NODECENT);
        shiftCentering->SetInput(dob);
        dob = shiftCentering->GetOutput();
    }

#ifdef ENGINE
    if (volumeResampleFilter != nullptr)
    {
        delete volumeResampleFilter;
        volumeResampleFilter = nullptr;
    }
    volumeResampleFilter = new avtVolumeResampleFilter(atts);
    volumeResampleFilter->SetInput(dob);
    dob = volumeResampleFilter->GetOutput();
#endif

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
//    Brad Whitlock, Mon Dec 15 15:52:58 PST 2008
//    Added a filter for the HW accelerated case.
//
//    Hank Childs, Wed Dec 31 13:47:37 PST 2008
//    Renamed ResampleAtts to InternalResampleAtts.
//
//    Allen Harvey, Thurs Nov 3 7:21:13 EST 2011
//    Make resampling optional.
//
//    Brad Whitlock, Tue Jan 31 11:22:01 PST 2012
//    Add compact tree filter for non-resampling case.
//
//    Hank Childs, Tue Apr 10 17:03:40 PDT 2012
//    Create gradient as part of the volume plot (instead of cueing EEF to
//    do it).  This will allow for the volume plot to work on variables
//    that are created mid-pipeline.
//
//    Cyrus Harrison, Tue Mar 12 16:30:45 PDT 2013
//    Don't calc gradient for raycasting integration, lighting isn't applied
//    for this case.
//
//    Alister Maguire, Fri May 12 10:15:45 PDT 2017
//    Replaced the Texture3D renderer with the Default renderer.
//
//    Alister Maguire, Tue Dec 11 10:18:31 PST 2018
//    The new default renderer requires a single domain rectilinear dataset.
//    I've updated the logic to address this.
//
//    Kathleen Biagas, Wed July 12, 2023 
//    Resampling decisions moved to avtVolumeResampleFilter, utilized during
//    ApplyOperators method.
//
// ****************************************************************************

avtDataObject_p
avtVolumePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    //
    // Clean up any old filters.
    //
    if (lowResVolumeFilter != nullptr)
    {
        delete lowResVolumeFilter;
        lowResVolumeFilter = nullptr;
    }
    if (gradientFilter != nullptr)
    {
        delete gradientFilter;
        gradientFilter = nullptr;
    }
    if (volumeFilter != nullptr)
    {
        delete volumeFilter;
        volumeFilter = nullptr;
    }

    avtDataObject_p dob = input;
    const avtDataAttributes &datts = input->GetInfo().GetAttributes();
    std::string activeVariable = datts.GetVariableName();


#ifdef ENGINE
    // The gradient calc for ray casting composite is needed when
    // the lighting flag is on.
    if (atts.GetRendererType() == VolumeAttributes::Composite &&
            atts.GetLightingFlag())
    {
        char gradName[128], gradName2[128];
        const char *gradvar = atts.GetOpacityVariable().c_str();
        if (strcmp(gradvar, "default") == 0)
        {
            if (atts.GetScaling() == VolumeAttributes::Log ||
                atts.GetScaling() == VolumeAttributes::Skew)
            {
                snprintf(gradName2, 128, "_expr_%s", varname);
                gradvar = gradName2;
            }
            else
              gradvar = varname;
        }

        // The avtVolumeFilter uses this exact name downstream.
        snprintf(gradName, 128, "_%s_gradient", gradvar);

        gradientFilter = new avtGradientExpression();
        gradientFilter->SetInput(dob);
        gradientFilter->SetAlgorithm(FAST);
        gradientFilter->SetOutputVariableName(gradName);
        gradientFilter->AddInputVariableName(gradvar);

        // Prevent this intermediate object from getting cleared
        // out, so it is still there when rendering.
        gradientFilter->GetOutput()->SetTransientStatus(false);
        dob = gradientFilter->GetOutput();
    }
#endif

    if (atts.GetRendererType() == VolumeAttributes::Composite ||
        atts.GetRendererType() == VolumeAttributes::Integration ||
        atts.GetRendererType() == VolumeAttributes::SLIVR ||
        atts.GetRendererType() == VolumeAttributes::Parallel)
    {
        volumeFilter = new avtVolumeFilter();
        volumeFilter->SetAttributes(atts);
        volumeFilter->SetInput(dob);
        dob = volumeFilter->GetOutput();
    }
    // Serial rendering
    else //if (atts.GetRendererType() == VolumeAttributes::Serial)
    {
        // Apply a filter that will work on the combined data to make
        // histograms.
        lowResVolumeFilter = new avtLowerResolutionVolumeFilter();
        lowResVolumeFilter->SetAtts(&atts);
        lowResVolumeFilter->SetInput(dob);
        dob = lowResVolumeFilter->GetOutput();
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
//  Modifications:
//    Hank Childs, Thu Aug 26 17:44:13 PDT 2010
//    Calculate the extents for the opacity variable.
//
// ****************************************************************************

avtContract_p
avtVolumePlot::EnhanceSpecification(avtContract_p spec)
{
    std::string ov = atts.GetOpacityVariable();
    if (ov == "default")
    {
        if(atts.GetResampleType() == VolumeAttributes::SingleDomain ||
           atts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
           atts.GetResampleType() == VolumeAttributes::ParallelPerRank)
            return spec;
    }
    avtDataRequest_p ds = spec->GetDataRequest();
    std::string primaryVariable(ds->GetVariable());
    if (ov == primaryVariable)
    {
        if(atts.GetResampleType() == VolumeAttributes::SingleDomain ||
           atts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
           atts.GetResampleType() == VolumeAttributes::ParallelPerRank)
        {
            //
            // They didn't leave it as "default", but it is the same
            // variable, so don't read it in again.
            //
            return spec;
        }
    }

    //
    // The pipeline specification should really be const -- it is used
    // elsewhere, so we can't modify it and return it.  Make a copy and in
    // the new copy, add a secondary variable.
    //
    avtDataRequest_p nds = new avtDataRequest(primaryVariable.c_str(),
                                              ds->GetTimestep(), ds->GetRestriction());
    if(ov != "default")
    {
        debug5 << "Adding secondary variable: " << ov << endl;
        nds->AddSecondaryVariable(ov.c_str());
    }
    avtContract_p rv = new avtContract(spec, nds);
    rv->SetCalculateVariableExtents(ov, true);

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

    if (volumeFilter != nullptr)
    {
        volumeFilter->ReleaseData();
    }
    if (shiftCentering != nullptr)
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
//  Modifications:
//
// ****************************************************************************

bool
avtVolumePlot::Equivalent(const AttributeGroup *a)
{
    const VolumeAttributes *objAtts = (const VolumeAttributes *)a;
    // Almost the inverse of changes require recalculation -- doSoftware being
    // different is okay!
    if (atts.GetResampleType() != objAtts->GetResampleType())
        return false;
    if (atts.GetResampleTarget() != objAtts->GetResampleTarget())
        return false;
    if (atts.GetResampleCentering() != objAtts->GetResampleCentering())
        return false;
    if (atts.GetOpacityVariable() != objAtts->GetOpacityVariable())
        return false;
    return true;
}
