// ************************************************************************* //
//                             avtRayTracer.C                                //
// ************************************************************************* //

#include <avtRayTracer.h>

#include <vector>

#include <visit-config.h>

#include <vtkImageData.h>

#include <avtCommonDataFunctions.h>
#include <avtDataset.h>
#include <avtDatasetExaminer.h>
#include <avtImage.h>
#include <avtImagePartition.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtRayCompositer.h>
#include <avtSamplePointExtractor.h>
#include <avtSamplePoints.h>
#include <avtVolume.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#ifdef PARALLEL
#include <avtImageCommunicator.h>
#include <avtSamplePointCommunicator.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>


using     std::vector;


// ****************************************************************************
//  Method: avtRayTracer constructor
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Dec 5 11:19:29 PDT 2001
//    Added gradient backgrounds.
//
//    Hank Childs, Mon Jan 16 11:11:47 PST 2006
//    Added kernel based sampling.
//
// ****************************************************************************

avtRayTracer::avtRayTracer()
{
    view.camera[0] = -5.;
    view.camera[1] = 10.;
    view.camera[2] = -15.;
    view.focus[0]  = 0.;
    view.focus[1]  = 0.;
    view.focus[2]  = 0.;
    view.viewAngle = 70.;
    view.viewUp[0] = 0.;
    view.viewUp[1] = 0.;
    view.viewUp[2] = 1.;
    view.nearPlane = 5.;
    view.farPlane  = 30.;
    view.parallelScale = 10;
    view.orthographic = true;

    rayfoo         = NULL;

    opaqueImage    = NULL;

    background[0]  = 255;
    background[1]  = 255;
    background[2]  = 255;
    backgroundMode = BACKGROUND_SOLID;
    gradBG1[0] = 0.;
    gradBG1[1] = 0.;
    gradBG1[2] = 1.;
    gradBG2[0] = 0.;
    gradBG2[1] = 0.;
    gradBG2[2] = 0.;

    screen[0] = screen[1] = 400;
    samplesPerRay  = 40;
    kernelBasedSampling = false;
}


// ****************************************************************************
//  Method: avtRayTracer destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRayTracer::~avtRayTracer()
{
    ;
}


// ****************************************************************************
//  Method: avtRayTracer::SetBackgroundColor
//
//  Purpose:
//      Allows the background color to be set.
//
//  Arguments:
//      b       The new background color.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2001
//
// ****************************************************************************

void
avtRayTracer::SetBackgroundColor(const unsigned char b[3])
{
    background[0] = b[0];
    background[1] = b[1];
    background[2] = b[2];
}


// ****************************************************************************
//  Method: avtRayTracer::SetBackgroundMode
//
//  Purpose: 
//      Sets the background mode.
//
//  Arguments:
//      mode    The new background mode.
//
//  Programmer: Brad Whitlock
//  Creation:   December 5, 2001
//
// ****************************************************************************

void
avtRayTracer::SetBackgroundMode(int mode)
{
    if (mode >= BACKGROUND_SOLID && mode <= BACKGROUND_GRADIENT_RADIAL)
    {
        backgroundMode = mode;
    }
}


// ****************************************************************************
//  Method: avtRayTracer::SetGradientBackgroundColors
//
//  Purpose: 
//      Sets the gradient background colors.
//
//  Arguments:
//      bg1     An array containing the first gradient background color.
//      bg2     An array containing the second gradient background color.
//
//  Programmer: Brad Whitlock
//  Creation:   December 5, 2001
//
// ****************************************************************************

void
avtRayTracer::SetGradientBackgroundColors(const double bg1[3],
                                          const double bg2[3])
{
    gradBG1[0] = bg1[0];
    gradBG1[1] = bg1[1];
    gradBG1[2] = bg1[2];
    gradBG2[0] = bg2[0];
    gradBG2[1] = bg2[1];
    gradBG2[2] = bg2[2];
}


// ****************************************************************************
//  Function: GetNumberOfStages
//
//  Purpose:
//      Determines how many stages the ray tracer will take.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
// ****************************************************************************

int
avtRayTracer::GetNumberOfStages(int screenX, int screenY, int screenZ)
{
    int nD = GetNumberOfDivisions(screenX, screenY, screenZ);
    int numPerTile = 3;
#ifdef PARALLEL
    numPerTile = 5;
#endif
    return 3*nD*nD;
}


// ****************************************************************************
//  Function: GetNumberOfDivisions
//
//  Purpose:
//      Determines how many divisions of screen space we should use.  That is,
//      how many tiles should we use.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2005
//
// ****************************************************************************

int
avtRayTracer::GetNumberOfDivisions(int screenX, int screenY, int screenZ)
{
    VISIT_LONG_LONG numSamps = screenX*screenY*screenZ;
    int sampLimitPerProc = 25000000; // 25M
    numSamps /= PAR_Size();
    int numTiles = numSamps/sampLimitPerProc;
    int numDivisions = (int) sqrt((double) numTiles);
    if (numDivisions < 1)
        numDivisions = 1;
    int altNumDiv = (int)(screenX / 700.) + 1;
    if (altNumDiv > numDivisions)
        numDivisions = altNumDiv;
    altNumDiv = (int)(screenY / 700.) + 1;
    if (altNumDiv > numDivisions)
        numDivisions = altNumDiv;

    return numDivisions;
}


// ****************************************************************************
//  Method: avtRayTracer::Execute
//
//  Purpose:
//      Executes the ray tracer.  
//      This means:
//      - Put the input mesh through a transform so it is in camera space.
//      - Get the sample points.
//      - Communicate the sample points (parallel only).
//      - Composite the sample points along rays.
//      - Communicate the pixels from each ray (parallel only).
//      - Output the image.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jan 26 17:00:07 PST 2001
//    Added support for parallel ray tracing.
//
//    Hank Childs, Sat Jan 27 15:09:34 PST 2001
//    Added support for sending cells when doing parallel ray tracing.
//
//    Hank Childs, Sun Mar  4 21:50:36 PST 2001
//    Added image partition to aid parallel modules in minimizing
//    communication by dynamically assigning the partitions.
//
//    Eric Brugger, Mon Nov  5 13:47:19 PST 2001
//    Modified to always compile the timing code.
//
//    Brad Whitlock, Wed Dec 5 10:57:56 PDT 2001
//    Added code to set the gradient background colors, background mode in
//    the ray compositer.
//
//    Kathleen Bonnell, Tue Apr 30 12:34:54 PDT 2002  
//    Use new version of CopyTo. 
//
//    Hank Childs, Fri Sep 13 12:04:04 PDT 2002
//    Reverse arguments for CopyTo (we weren't sending in the input correctly).
//
//    Hank Childs, Fri Nov 19 13:47:20 PST 2004
//    Added option to have sampling of rectilinear grids done efficiently
//    by sample point extractor by not converting grid into image space.
//
//    Hank Childs, Thu Dec  2 09:26:28 PST 2004
//    No longer tighten clipping planes ['5699].
//
//    Hank Childs, Thu Dec  9 17:15:44 PST 2004
//    Cast rays in tiles for big images ['1948].
//
//    Hank Childs, Sun Mar 13 11:16:20 PST 2005
//    Fix memory leak.
//
//    Hank Childs, Tue Mar 29 16:19:19 PST 2005
//    If the image is large, force divisions of 512x512, even if we should
//    theoretically have enough memory to cover it.
//
//    Hank Childs, Mon Jan 16 11:11:47 PST 2006
//    Add support for kernel based sampling.
//
// ****************************************************************************

void
avtRayTracer::Execute(void)
{
    int  timingIndex = visitTimer->StartTimer();

    if (rayfoo == NULL)
    {
        debug1 << "Never set ray function for ray tracer." << endl;
        EXCEPTION0(ImproperUseException);
    }

    //
    // First we need to transform all of domains into camera space.
    //
    double aspect = 1.;
    if (screen[1] > 0)
    {
        aspect = (double)screen[0] / (double)screen[1];
    }
    avtWorldSpaceToImageSpaceTransform trans(view, aspect);
    trans.SetInput(GetInput());

    //
    // Extract all of the samples from the dataset.
    //
    avtSamplePointExtractor extractor(screen[0], screen[1], samplesPerRay);
    extractor.SetKernelBasedSampling(kernelBasedSampling);
    extractor.RegisterRayFunction(rayfoo);
    extractor.SetInput(trans.GetOutput());

    //
    // For curvilinear and unstructured meshes, it makes sense to convert the
    // cells to image space.  But for rectilinear meshes, it is not the
    // most efficient strategy.  So set some flags here that allow the 
    // extractor to do the extraction in world space.
    //
    if (!kernelBasedSampling)
    {
        trans.SetPassThruRectilinearGrids(true);
        extractor.SetRectilinearGridsAreInWorldSpace(true, view, aspect);
    }

    avtDataObject_p samples = extractor.GetOutput();

#ifdef PARALLEL
    //
    // Tell the sample point extractor that we would like to send cells
    // instead of sample points when appropriate.
    //
    extractor.SendCellsMode(true);

    //
    // If we have more processors than domains, we have to handle that
    // gracefully.  Communicate how many variables there are so that those
    // that don't have data can play well.
    //
    VarList vl;
    vl.nvars = -1;
    avtDataset_p ds=GetTypedInput();
    avtDatasetExaminer::GetVariableList(ds, vl);

    int effectiveVars = UnifyMaximumValue(vl.nvars);
    extractor.GetTypedOutput()->SetNumberOfVariables(effectiveVars);

    //
    // Communicate the samples to the other processors.
    //
    avtSamplePointCommunicator sampleCommunicator;
    sampleCommunicator.SetInput(extractor.GetOutput());

    samples = sampleCommunicator.GetOutput();
#endif
 
    //
    // Perform compositing on the rays to get the final image.
    //
    avtRayCompositer rc(rayfoo);
    rc.SetBackgroundColor(background);
    rc.SetBackgroundMode(backgroundMode);
    rc.SetGradientBackgroundColors(gradBG1, gradBG2);
    if (*opaqueImage != NULL)
    {
        rc.InsertOpaqueImage(opaqueImage);
    }
    rc.SetInput(samples);
    avtImage_p image = rc.GetTypedOutput();

#ifdef PARALLEL
    //
    // Communicate the screen to the root processor.
    //
    avtImageCommunicator imageCommunicator;
    avtDataObject_p dob;
    CopyTo(dob, image);
    imageCommunicator.SetInput(dob);
    image = imageCommunicator.GetTypedOutput();
#endif

    //
    // Update the pipeline several times, once for each tile.
    // The tiles are important to make sure that we never need too much
    // memory.
    //
    int numDivisions = GetNumberOfDivisions(screen[0],screen[1],samplesPerRay);

    int IStep = screen[0] / numDivisions;
    int JStep = screen[1] / numDivisions;
    avtImage_p whole_image;
    if (PAR_Rank() == 0)
    {
        whole_image = new avtImage(this);
        vtkImageData *img = avtImageRepresentation::NewImage(screen[0], 
                                                             screen[1]);
        whole_image->GetImage() = img;
        img->Delete();
    }
    for (int i = 0 ; i < numDivisions ; i++)
        for (int j = 0 ; j < numDivisions ; j++)
        {
            int IStart = i*IStep;
            int IEnd = (i == (numDivisions-1) ? screen[0] : (i+1)*IStep);
            int JStart = j*JStep;
            int JEnd = (j == (numDivisions-1) ? screen[1] : (j+1)*JStep);
    
#ifdef PARALLEL
            //
            // Create an image partition that will be passed around between
            // parallel modules in an effort to minimize communication.
            //
            avtImagePartition imagePartition(screen[0], screen[1]);
            imagePartition.RestrictToTile(IStart, IEnd, JStart, JEnd);
            sampleCommunicator.SetImagePartition(&imagePartition);
            imageCommunicator.SetImagePartition(&imagePartition);
#endif
            extractor.RestrictToTile(IStart, IEnd, JStart, JEnd);
            image->Update(GetGeneralPipelineSpecification());
            if (PAR_Rank() == 0)
            {
                unsigned char *whole_rgb = 
                                        whole_image->GetImage().GetRGBBuffer();
                unsigned char *tile = image->GetImage().GetRGBBuffer();
   
                for (int jj = JStart ; jj < JEnd ; jj++)
                    for (int ii = IStart ; ii < IEnd ; ii++)
                    {
                        int index = screen[0]*jj + ii;
                        int index2 = (IEnd-IStart)*(jj-JStart) + (ii-IStart);
                        whole_rgb[3*index+0] = tile[3*index2+0];
                        whole_rgb[3*index+1] = tile[3*index2+1];
                        whole_rgb[3*index+2] = tile[3*index2+2];
                    }
            }
        }
    if (PAR_Rank() == 0)
        image->Copy(*whole_image);

    //
    // Make our output image look the same as the ray compositer's.
    //
    SetOutput(image);

    visitTimer->StopTimer(timingIndex, "Ray Tracing");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtRayTracer::SetView
//
//  Purpose:
//      Sets the viewing matrix of the camera.
//
//  Arguments:
//      v       The view info.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
// ****************************************************************************

void
avtRayTracer::SetView(const avtViewInfo &v)
{
    view = v;
    modified = true;
}


// ****************************************************************************
//  Method: avtRayTracer::SetRayFunction
//
//  Purpose:
//      Sets the ray function for the ray tracer.
//
//  Arguments:
//      rf      The new ray function.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:37:01 PST 2001
//    Removed pixelizer from its coupling with a ray function.
//
// ****************************************************************************

void
avtRayTracer::SetRayFunction(avtRayFunction *rf)
{
    rayfoo    = rf;
}


// ****************************************************************************
//  Method: avtRayTracer::SetScreen
//
//  Purpose:
//      Tells the ray tracer the size of the screen and thus how many rays
//      to send out.
//
//  Arguments:
//      screenX       The number of pixels in width.
//      screenY       The number of pixels in height.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2000
//
// ****************************************************************************

void
avtRayTracer::SetScreen(int screenX, int screenY)
{
    if (screenX > 0 && screenY > 0)
    {
        screen[0] = screenX;
        screen[1] = screenY;
        modified  = true;
    }
}


// ****************************************************************************
//  Method: avtRayTracer::SetSamplesPerRay
//
//  Purpose:
//      Sets the number of samples to be taken along each ray.
//
//  Arguments:
//      samps   The number of samples that should be taken along each ray.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2000
//
// ****************************************************************************

void
avtRayTracer::SetSamplesPerRay(int samps)
{
    if (samps > 0)
    {
        samplesPerRay = samps;
        modified      = true;
    }
}


// ****************************************************************************
//  Method: avtRayTracer::InsertOpaqueImage
//
//  Purpose:
//      Allows for an opaque image to be inserted into the middle of the
//      rendering.
//
//  Arguments:
//      img     The image to be used in the ray tracer.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
// ****************************************************************************

void
avtRayTracer::InsertOpaqueImage(avtImage_p img)
{
    opaqueImage = img;
}


// ****************************************************************************
//  Method: avtRayTracer::ReleaseData
//
//  Purpose:
//      Releases the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 24 08:25:28 PDT 2002
//    I forgot the call to the base class's release data.
//
// ****************************************************************************

void
avtRayTracer::ReleaseData(void)
{
    avtDatasetToImageFilter::ReleaseData();
    if (*opaqueImage != NULL)
    {
        opaqueImage->ReleaseData();
    }
}


// ****************************************************************************
//  Method: avtRayTracer::PerformRestriction
//
//  Purpose:
//      Restricts the data of interest.  Does this by getting the spatial
//      extents and culling around the view.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 13:59:45 PST 2001
//    Put in check to make sure the spatial extents still apply before trying
//    use them for culling.
//
//    Hank Childs, Sun Mar 25 11:44:12 PST 2001
//    Account for new interface with data object information.
//
//    Hank Childs, Wed Nov 21 11:53:21 PST 2001
//    Don't assume we can get an interval tree.
//
// ****************************************************************************

avtPipelineSpecification_p
avtRayTracer::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = spec;
    if (GetInput()->GetInfo().GetValidity().GetSpatialMetaDataPreserved())
    {
        vector<int> domains;
        avtIntervalTree *tree = GetMetaData()->GetSpatialExtents();
        if (tree != NULL)
        {
            avtWorldSpaceToImageSpaceTransform::GetDomainsList(view,
                                                               domains, tree);
            rv = new avtPipelineSpecification(spec);
            rv->GetDataSpecification()->GetRestriction()
                                                    ->RestrictDomains(domains);
        }
    }

    rv->NoDynamicLoadBalancing();
    return rv;
}


