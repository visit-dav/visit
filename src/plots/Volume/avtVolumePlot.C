// ************************************************************************* //
//                               avtVolumePlot.C                             //
// ************************************************************************* //

#include "avtVolumePlot.h"

#include <limits.h>
#include <math.h>

#include <avtCallback.h>
#include <avtVolumeRenderer.h>
#include <avtLookupTable.h>
#include <avtShiftCenteringFilter.h>
#include <avtUserDefinedMapper.h>
#include <avtVolumeFilter.h>

#include <VolumeAttributes.h>

#include <DebugStream.h>
#include <LostConnectionException.h>


static void OverrideWithSoftwareImageCallback(void *, avtDataObject_p &);


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
// ****************************************************************************

avtVolumePlot::avtVolumePlot() : avtVolumeDataPlot()
{
    volumeFilter = NULL;
    shiftCentering = NULL;
    renderer = avtVolumeRenderer::New();
    renderer->RegisterOverrideRenderCallback(OverrideWithSoftwareImageCallback,
                                             this);

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

    lastImage = NULL;

    //
    // Remove this HACK when the software rendering route from the viewer
    // to the engine gets better.
    //
    id = "Volume_1.1";
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
// ****************************************************************************

avtVolumePlot::~avtVolumePlot()
{
    delete mapper;
    delete volumeFilter;
    delete shiftCentering;
    delete avtLUT;

    //
    // The renderer may still try to draw, so let it know that we are deleted.
    //
    renderer->RegisterOverrideRenderCallback(OverrideWithSoftwareImageCallback,
                                             NULL);
    renderer = NULL;

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
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

    float min, max;
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
// ****************************************************************************

avtDataObject_p
avtVolumePlot::ApplyOperators(avtDataObject_p input)
{
    avtDataObject_p dob = input;

    //
    // Clean up any old filters.
    //
    if (volumeFilter != NULL)
    {
        delete volumeFilter;
        volumeFilter = NULL;
    }
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

    volumeFilter = new avtVolumeFilter();
    volumeFilter->SetAttributes(atts);
    volumeFilter->SetInput(dob);
    return volumeFilter->GetOutput();
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
//
// ****************************************************************************

avtDataObject_p
avtVolumePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
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
// ****************************************************************************

void
avtVolumePlot::CustomizeBehavior(void)
{
    // make it go last
    behavior->SetRenderOrder(MUST_GO_LAST);

    // Add a legend.
    float min, max;
    mapper->GetRange(min, max);
    varLegend->SetRange(min, max);
    varLegend->SetVarRange(min, max);
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
//  Method: avtVolumePlot::OverrideWithSoftwareImage
//
//  Purpose:
//      Determines if we should override the normal hardware rendering with
//      a software rendering.
//
//  Arguments:
//      dob     A place to store the image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Aug 10 19:39:52 PDT 2003
//    Add support for software volume rendering crashing the engine (meaning
//    make sure the engine doesn't crash as well.)
//
// ****************************************************************************

void
avtVolumePlot::OverrideWithSoftwareImage(avtDataObject_p &dob)
{
    //
    // Check to see if we can re-use the last image.  This comes up a lot with
    // redraws, etc.
    //
    const WindowAttributes &curWin = avtCallback::GetCurrentWindowAtts();
    if (lastWindowAtts == curWin && lastAtts == atts && *lastImage != NULL)
    {
        dob = lastImage;
        return;
    }

    if (atts.GetDoSoftware())
    {
        TRY
        {
            avtCallback::UpdatePlotAttributes(id, index, &atts);
            avtCallback::GetImage(index, dob);

            lastWindowAtts = curWin;
            lastAtts = atts;
            lastImage = dob;
        }
        CATCH(LostConnectionException)
        {
            lastImage = NULL;
        }
    }

    atts.SetDoSoftware(false);
}


// ****************************************************************************
//  Function: OverrideWithSoftwareImageCallback
//
//  Purpose:
//      A normal C-function used for callbacks to override rendering with
//      software images.
//
//  Arguments:
//      vp      A void pointer.  This should be a ptr to a volume plot.
//      dob     A place to store the image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon May 20 16:31:08 PDT 2002
//    Made more of an effort not to access free'd memory.
//
// ****************************************************************************

void
OverrideWithSoftwareImageCallback(void *vp, avtDataObject_p &dob)
{
    if (vp != NULL)
    {
        avtVolumePlot *plot = (avtVolumePlot *) vp;
        plot->OverrideWithSoftwareImage(dob);
    }
    else
    {
        debug3 << "Could not draw a volume plot correctly because it "
               << "was deleted.  This happens when a volume is being "
               << "updated on the engine and the viewer deletes the current "
               << "one." << endl;
    }
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


// THESE METHODS ARE HACKS THAT WERE ADDED FOR DOING SOFTWARE RENDERING.
// WHEN BETTER SOLUTIONS ARISE, THEY SHOULD BE REMOVED.

void
avtVolumePlot::SetId(const string &id_)
{
    id = id_;
}


