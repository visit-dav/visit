// ************************************************************************* //
//                              avtVolumeFilter.C                            //
// ************************************************************************* //

#include <avtVolumeFilter.h>

#include <avtCallback.h>
#include <avtCommonDataFunctions.h>
#include <avtCompositeRF.h>
#include <avtDatasetExaminer.h>
#include <avtFlatLighting.h>
#include <avtOpacityMap.h>
#include <avtParallel.h>
#include <avtRay.h>
#include <avtPhong.h>
#include <avtRayTracer.h>
#include <avtResampleFilter.h>
#include <avtSamplePointArbitrator.h>
#include <avtSourceFromAVTDataset.h>
#include <avtView3D.h>
#include <avtViewInfo.h>

#include <DebugStream.h>
#include <InvalidDimensionsException.h>
#include <InvalidVariableException.h>


//
// Function Prototypes
//

static void CreateViewInfoFromViewAttributes(avtViewInfo &,
                                             const ViewAttributes &);


// ****************************************************************************
//  Method: avtVolumeFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

avtVolumeFilter::avtVolumeFilter()
{
    image     = new avtImage(this);
    dataset   = new avtDataset(this);
    software  = NULL;
    resampler = NULL;
    primaryVariable = NULL;
}


// ****************************************************************************
//  Method: avtVolumeFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

avtVolumeFilter::~avtVolumeFilter()
{
    if (software != NULL)
    {
        delete software;
        software = NULL;
    }
    if (resampler != NULL)
    {
        delete resampler;
        resampler = NULL;
    }
    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
        primaryVariable = NULL;
    }
}


// ****************************************************************************
//  Method: avtVolumeFilter::SetAttributes
//
//  Purpose:
//      Sets the attributes of the software override filter.
//
//  Arguments:
//      a       The attributes for the filter.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtVolumeFilter::SetAttributes(const VolumeAttributes &a)
{
    atts = a;
}


// ****************************************************************************
//  Method: avtVolumeFilter::DoSoftwareRender
//
//  Purpose:
//      Determines if we should do a software render.
//
//  Returns:    true if we should do a software render, false if we should do
//              hardware.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
// ****************************************************************************

bool
avtVolumeFilter::DoSoftwareRender(void)
{
    bool doSoftware = atts.GetDoSoftware();
    if (doSoftware)
    {
        if (software == NULL)
        {
            software = new avtRayTracer;
        }
    }
    else
    {
        if (resampler == NULL)
        {
            ResampleAttributes resampleAtts;
            resampleAtts.SetTargetVal(atts.GetResampleTarget());
            resampleAtts.SetUseTargetVal(true);
            resampler = new avtResampleFilter(&resampleAtts);
        }
    }
    
    return doSoftware;
}


// ****************************************************************************
//  Method: avtVolumeFilter::GetOutput
//
//  Purpose:
//      Gets the correct output based on whether or not we are doing software
//      rendering.
//
//  Returns:    Either the image or dataset depending on the mode.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Apr 10 17:23:51 PST 2002
//    Changed the CopyTo method to be an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtVolumeFilter::GetOutput(void)
{
    avtDataObject_p dob;
    if (DoSoftwareRender())
    {
        CopyTo(dob, image);
    }
    else
    {
        CopyTo(dob, dataset);
    }
    return dob;
}


// ****************************************************************************
//  Method: avtVolumeFilter::Execute
//
//  Purpose:
//      If we are supposed to do software rendering, then go ahead and do a
//      volume plot.  If we are supposed to do the hardware accelerated then
//      resample onto a rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec  5 09:13:09 PST 2001
//    Get extents from the actual data.  Make sure those extents are from
//    the right variable.
//
//    Brad Whitlock, Wed Dec 5 11:16:38 PDT 2001
//    Added code to set the window's background mode and color.
//
//    Hank Childs, Wed Dec 12 10:54:58 PST 2001
//    Allow for extents to be set artificially.
//
//    Hank Childs, Fri Dec 21 07:52:45 PST 2001
//    Do a better job of using extents.
//
//    Hank Childs, Wed Jan 23 11:20:50 PST 2002
//    Add support for small cells.
//
//    Hank Childs, Wed Feb  6 09:22:21 PST 2002
//    Add support for running in parallel with more processors than domains.
//
//    Hank Childs, Fri Feb  8 19:03:49 PST 2002
//    Add support for setting the number of samples per ray.
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Add support for dataset examiner.
//
//    Kathleen Bonnell, Wed Oct 23 13:27:56 PDT 2002  
//    Set queryable to false for the image and dataset object's validity. 
//
//    Hank Childs, Mon Jul  7 22:24:26 PDT 2003
//    If an error occurred, pass that message on.
//
// ****************************************************************************

void
avtVolumeFilter::Execute(void)
{
    if (DoSoftwareRender())
    {
        //
        // We need to create a dummy pipeline with the volume renderer that we
        // can force to execute within our "Execute".  Start with the source.
        //
        avtSourceFromAVTDataset termsrc(GetTypedInput());

        //
        // Set up the volume renderer.
        //
        software->SetInput(termsrc.GetOutput());
       
        unsigned char vtf[4*256];
        atts.GetTransferFunction(vtf);
        avtOpacityMap om(256);
        om.SetTable(vtf, 256);
        double actualRange[2];
        bool artificialMin = atts.GetUseColorVarMin();
        bool artificialMax = atts.GetUseColorVarMax();
        if (!artificialMin || !artificialMax)
        {
            GetDataExtents(actualRange, primaryVariable);
            UnifyMinMax(actualRange, 2);
        }
        double range[2];
        range[0] = (artificialMin ? atts.GetColorVarMin() : actualRange[0]);
        range[1] = (artificialMax ? atts.GetColorVarMax() : actualRange[1]);
        om.SetMin(range[0]);
        om.SetMax(range[1]);

        avtFlatLighting fl;
        avtLightingModel *lm = &fl;
/*
        avtPhong phong;
        if (atts.GetLightingFlag())
        {
            lm = &phong;
        }
        else
        {
            lm = &fl;
        }
 */
        
        //
        // Determine which variables to use and tell the ray function.
        //
        VarList vl;
        avtDataset_p input = GetTypedInput();
        avtDatasetExaminer::GetVariableList(input, vl);
        int primIndex = -1;
        int opacIndex = -1;
        for (int i = 0 ; i < vl.nvars ; i++)
        {
            if (vl.varnames[i] == primaryVariable)
            {
                primIndex = i;
            }
            if (vl.varnames[i] == atts.GetOpacityVariable())
            {
                opacIndex = i;
            }
        }
        if (primIndex == -1)
        {
            if (vl.nvars <= 0)
            {
                debug1 << "Could not locate primary variable "
                       << primaryVariable << ", assuming that we are running "
                       << "in parallel and have more processors than domains."
                       << endl;
            }
            else
            {
                EXCEPTION1(InvalidVariableException, primaryVariable);
            }
        }
        if (opacIndex == -1)
        {
            if (atts.GetOpacityVariable() == "default")
            {
                opacIndex = primIndex;
            }
            else if (vl.nvars <= 0)
            {
                debug1 << "Could not locate opacity variable "
                       << atts.GetOpacityVariable().c_str() << ", assuming that we "
                       << "are running in parallel and have more processors "
                       << "than domains." << endl;
            }
            else
            {
                EXCEPTION1(InvalidVariableException,atts.GetOpacityVariable());
            }
        }

        int newPrimIndex = UnifyMaximumValue(primIndex);
        if (primIndex >= 0 && newPrimIndex != primIndex)
        {
            //
            // We shouldn't ever have different orderings for our variables.
            //
            EXCEPTION1(InvalidVariableException, primaryVariable);
        }
        primIndex = newPrimIndex;

        int newOpacIndex = UnifyMaximumValue(opacIndex);
        if (opacIndex >= 0 && newOpacIndex != opacIndex)
        {
            //
            // We shouldn't ever have different orderings for our variables.
            //
            EXCEPTION1(InvalidVariableException, atts.GetOpacityVariable());
        }
        opacIndex = newOpacIndex;

        avtOpacityMap *om2 = NULL;
        if (primIndex == opacIndex)
        {
            // Note that we are forcing the color variables range onto the
            // opacity variable.
            om2 = &om;
        }
        else
        {
            om2 = new avtOpacityMap(256);
            om2->SetTable(vtf, 256);
            double range[2];

            bool artificialMin = atts.GetUseOpacityVarMin();
            bool artificialMax = atts.GetUseOpacityVarMax();
            if (!artificialMin || !artificialMax)
            {
                InputSetActiveVariable(atts.GetOpacityVariable().c_str());
                avtDatasetExaminer::GetDataExtents(input, range);
                UnifyMinMax(range, 2);
                InputSetActiveVariable(primaryVariable);
            }
            range[0] = (artificialMin ? atts.GetOpacityVarMin() : range[0]);
            range[1] = (artificialMax ? atts.GetOpacityVarMax() : range[1]);
            om2->SetMin(range[0]);
            om2->SetMax(range[1]);
            // LEAK!!
        }
        avtCompositeRF rayfoo(lm, &om, om2);
        rayfoo.SetColorVariableIndex(primIndex);
        rayfoo.SetOpacityVariableIndex(opacIndex);
        
        software->SetRayFunction(&rayfoo);
        software->SetSamplesPerRay(atts.GetSamplesPerRay());

        const WindowAttributes &window = avtCallback::GetCurrentWindowAtts();
        const int *size = window.GetSize();
        software->SetScreen(size[0], size[1]);

        const ViewAttributes &view = window.GetView();
        avtViewInfo vi;
        CreateViewInfoFromViewAttributes(vi, view);
        software->SetView(vi);

        //
        // Set the volume renderer's background color and mode from the
        // window attributes.
        //
        software->SetBackgroundMode(window.GetBackgroundMode());
        software->SetBackgroundColor(window.GetBackground());
        software->SetGradientBackgroundColors(window.GetGradBG1(),
                                              window.GetGradBG2());

        //
        // We have to set up a sample point "arbitrator" to allow small cells
        // to be included in the final picture.
        //
        avtSamplePointArbitrator arb(om2, opacIndex);
        avtRay::SetArbitrator(&arb);

        //
        // Do the funny business to force an update.
        //
        avtDataObject_p dob = software->GetOutput();
        dob->Update(GetGeneralPipelineSpecification());

        //
        // Free up some memory and clean up.
        //
        delete software;
        software = NULL;
        avtRay::SetArbitrator(NULL);

        //
        // Copy the output of the volume renderer to our output.
        //
        dob->SetSource(NULL);
        avtDataObject *dobp = *dob;
        image->Copy(dobp);
        image->GetInfo().GetValidity().SetQueryable(false);
    }
    else
    {
        //
        // We need to create a dummy pipeline with the resampler that we can
        // force to execute within our "Execute".  Start with the source.
        //
        avtSourceFromAVTDataset termsrc(GetTypedInput());

        //
        // Set up the resample filter.
        //
        resampler->SetInput(termsrc.GetOutput());

        //
        // Do the funny business to force an update.
        //
        avtDataObject_p dob = resampler->GetOutput();
        dob->Update(GetGeneralPipelineSpecification());

        //
        // Free up some memory.
        //
        delete resampler;
        resampler = NULL;

        //
        // Copy the output of the resampler to our output.
        //
        dob->SetSource(NULL);
        avtDataObject *dobp = *dob;
        dataset->Copy(dobp);
        if (dob->GetInfo().GetValidity().HasErrorOccurred())
        {
            dataset->GetInfo().GetValidity().ErrorOccurred();
            dataset->GetInfo().GetValidity().SetErrorMessage(
                               dob->GetInfo().GetValidity().GetErrorMessage());
        }
            
        dataset->GetInfo().GetValidity().SetQueryable(false);
    }
}


// ****************************************************************************
//  Function: CreateViewInfoFromViewAttributes
//
//  Purpose:
//      It appears that we have three view holders around: ViewAttributes,
//      avtViewInfo, and avtViewXD.  I'm not sure why this is.  We have
//      ViewAttributes and we want avtViewInfo.  Conversion routines exist
//      for an intermediate form, avtView3D.  Convert to that intermediate
//      form and use that.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
//  Modifications:
//    Eric Brugger, Tue Jun 10 15:59:11 PDT 2003
//    I renamed camera to view normal in the view attributes.
//
//    Hank Childs, Tue Jul  8 22:43:39 PDT 2003
//    Copy over image zoom, pan.
//
// ****************************************************************************

void
CreateViewInfoFromViewAttributes(avtViewInfo &vi, const ViewAttributes &view)
{
    //
    // Conversion routines are already established for converting to 3D.
    //
    avtView3D view3d;
    view3d.normal[0] = view.GetViewNormal()[0];
    view3d.normal[1] = view.GetViewNormal()[1];
    view3d.normal[2] = view.GetViewNormal()[2];
    view3d.focus[0] = view.GetFocus()[0];
    view3d.focus[1] = view.GetFocus()[1];
    view3d.focus[2] = view.GetFocus()[2];
    view3d.viewUp[0] = view.GetViewUp()[0];
    view3d.viewUp[1] = view.GetViewUp()[1];
    view3d.viewUp[2] = view.GetViewUp()[2];
    view3d.viewAngle = view.GetViewAngle();
    view3d.parallelScale = view.GetParallelScale();
    view3d.nearPlane = view.GetNearPlane();
    view3d.farPlane = view.GetFarPlane();
    view3d.perspective = view.GetPerspective();
    view3d.imagePan[0] = view.GetImagePan()[0];
    view3d.imagePan[1] = view.GetImagePan()[1];
    view3d.imageZoom = view.GetImageZoom();

    //
    // Now View3D can be converted directly into avtViewInfo.
    //
    view3d.SetViewInfoFromView(vi);
}


// ****************************************************************************
//  Method: avtVolumeFilter::AdditionalPipelineFilters
//
//  Purpose:
//      Determines how many additional pipeline filters will execute.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
// ****************************************************************************

int
avtVolumeFilter::AdditionalPipelineFilters(void)
{
    int rv = 0;
    if (DoSoftwareRender())
    {
        //rv = software->AdditionalPipelineFilters();
        // HANK -- HELP ME
        rv = 3;
    }
    else
    {
        //rv = resampler->AdditionalPipelineFilters();
        // HANK -- HELP ME
        rv = 2;
    }

    //
    // The resamplers and softwares volume renderers don't account for
    // themselves, hence add 1.
    //
    return rv + 1;
}


// ****************************************************************************
//  Method: avtVolumeFilter::ReleaseData
//
//  Purpose:
//      Releases the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 16, 2002
//
// ****************************************************************************

void
avtVolumeFilter::ReleaseData(void)
{
    if (*dataset != NULL)
    {
        dataset->ReleaseData();
    }
    if (*image != NULL)
    {
        image->ReleaseData();
    }
    if (software != NULL)
    {
        software->ReleaseData();
    }
    if (resampler != NULL)
    {
        resampler->ReleaseData();
    }
}


// ****************************************************************************
//  Method: avtVolumeFilter::PerformRestriction
//
//  Purpose:
//      Performs a restriction based on which filter it is using underneath.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 14:15:20 PST 2002
//    Clean up memory leak.
//
// ****************************************************************************

avtPipelineSpecification_p
avtVolumeFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p newspec = NULL;
/* HANK -- HELP ME
    if (DoSoftwareRender())
    {
        newspec = software->PerformRestriction(spec);
    }
    else
    {
        newspec = resampler->PerformRestriction(spec);
    }
 */
    newspec = spec;

    newspec->GetDataSpecification()->NoGhostZones();
    newspec->NoDynamicLoadBalancing();

    if (primaryVariable != NULL)
    {
        delete [] primaryVariable;
    }

    const char *var = spec->GetDataSpecification()->GetVariable();
    primaryVariable = new char[strlen(var)+1];
    strcpy(primaryVariable, var);

    return newspec;
}


// ****************************************************************************
//  Method: avtVolumeFilter::VerifyInput
//
//  Purpose:
//      Verifies that the input is 3D data, throws an exception if not.
//
//  Programmer: Hank Childs
//  Creation:   September 26, 2002
//
// ****************************************************************************
 
void
avtVolumeFilter::VerifyInput(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "Volume", "3D");
    }
}

