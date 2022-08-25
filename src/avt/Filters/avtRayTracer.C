// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtRayTracer.C                                //
// ************************************************************************* //

#include <avtRayTracer.h>

#include <visit-config.h>

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

#include <avtImage.h>
#include <avtImagePartition.h>
#include <avtParallel.h>
#include <avtRayCompositer.h>
#include <avtSamplePointExtractor.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#ifdef PARALLEL
#include <avtImageCommunicator.h>
#include <avtSamplePointCommunicator.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>


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
//    Pascal Grosset, Fri Sep 20 2013
//    Added ray casting slivr & trilinear interpolation
//
// ****************************************************************************

avtRayTracer::avtRayTracer() : avtRayTracerBase()
{
    backgroundMode = BACKGROUND_SOLID;
    gradBG1[0] = 0.;
    gradBG1[1] = 0.;
    gradBG1[2] = 1.;
    gradBG2[0] = 0.;
    gradBG2[1] = 0.;
    gradBG2[2] = 0.;

    kernelBasedSampling = false;
    trilinearInterpolation = false;
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
//    Hank Childs, Thu May 31 22:36:32 PDT 2007
//    Remove code for setting number of variables for processors that don't
//    have data, since that is now handled by the avtSamplePointExtractor's
//    base class.
//
//    Hank Childs, Wed Dec 24 09:46:56 PST 2008
//    Add code to convert the z-buffer of the background to the w-buffer.
//    This is being done so the samples lie more evenly.
//
//    Hank Childs, Wed Dec 24 14:16:04 PST 2008
//    Automatically tighten the clipping planes so we utilize our samples
//    more effectively.
//
//    Hank Childs, Fri Jan  9 14:17:10 PST 2009
//    Have each ray be randomly jittered forward or backwards along its
//    direction.
//
//    Hank Childs, Tue Jan 13 14:26:44 PST 2009
//    Fix oversight where parallel volume rendering was not being jittered.
//
//    Hank Childs, Sun Jan 24 15:35:50 PST 2010
//    Automatically use the kernel based resampling for point data.
//
//    Pascal Grosset & Manasa Prasad, Fri Aug 20 2016
//    Add the ray casting SLIVR code
//
//    Qi WU, Tue Aug 8 2017
//    Fix camera matrices multiplication order for ray casting SLIVR
//    Also fixed panning for ray casting SLIVR
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9: view changed to
//    viewInfo.
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

    double scale[3] = {1,1,1};
    vtkMatrix4x4 *transform = vtkMatrix4x4::New();
    avtWorldSpaceToImageSpaceTransform::CalculateTransform(viewInfo, transform,
                                                           scale, aspect);
    double newNearPlane, newFarPlane, oldNearPlane, oldFarPlane;
    TightenClippingPlanes(viewInfo, transform, newNearPlane, newFarPlane);
    oldNearPlane = viewInfo.nearPlane;  oldFarPlane  = viewInfo.farPlane;
    viewInfo.nearPlane = newNearPlane;  viewInfo.farPlane  = newFarPlane;
    transform->Delete();

    avtWorldSpaceToImageSpaceTransform trans(viewInfo, aspect);
    trans.SetInput(GetInput());


    //
    // Extract all of the samples from the dataset.
    //
    avtSamplePointExtractor extractor(screen[0], screen[1], samplesPerRay);
    bool doKernel = kernelBasedSampling;
    if (trans.GetOutput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
        doKernel = true;

    extractor.SetKernelBasedSampling(doKernel);
    extractor.RegisterRayFunction(rayfoo);
    extractor.SetJittering(true);
    extractor.SetTransferFn(transferFn1D);
    extractor.SetInput(trans.GetOutput());

    if (trilinearInterpolation)
        extractor.SetTrilinear(true);

    //
    // For curvilinear and unstructured meshes, it makes sense to convert the
    // cells to image space.  But for rectilinear meshes, it is not the
    // most efficient strategy.  So set some flags here that allow the
    // extractor to do the extraction in world space.
    //
    if (!kernelBasedSampling)
    {
        trans.SetPassThruRectilinearGrids(true);
        extractor.SetRectilinearGridsAreInWorldSpace(true, viewInfo, aspect);
    }

    debug5 << "Raytracing setup done! " << std::endl;

    // Execute raytracer
    avtDataObject_p samples = extractor.GetOutput();

    debug5 << "Raytracing rendering done! " << std::endl;

  #ifdef PARALLEL
    //
    // Tell the sample point extractor that we would like to send cells
    // instead of sample points when appropriate.
    //
    extractor.SendCellsMode(true);

    //
    // Communicate the samples to the other processors.
    //
    avtSamplePointCommunicator sampleCommunicator;
    sampleCommunicator.SetInput(extractor.GetOutput());
    sampleCommunicator.SetJittering(true);

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
        bool convertToWBuffer = !viewInfo.orthographic;
        if (convertToWBuffer)
        {
            float *opaqueImageZB  = opaqueImage->GetImage().GetZBuffer();
            const int numpixels = screen[0]*screen[1];

            for (int p = 0 ; p < numpixels ; p++)
            {
                // The z value in clip space in the depth buifer is between 0 and 1 while it is normal for that
                // value to be between -1 and 1 instead. This is corrected here.
                double val = 2*opaqueImageZB[p]-1.0;

                // Map to actual distance from camera.
                val = (-2*oldFarPlane*oldNearPlane)
                         / ( (val*(oldFarPlane-oldNearPlane)) -
                             (oldFarPlane+oldNearPlane) );

                // Now normalize based on near and far.
                val = (val - newNearPlane) / (newFarPlane-newNearPlane);
                opaqueImageZB[p] = val;
            }
        }
        else // orthographic and need to adjust for tightened clipping planes
        {
            float *opaqueImageZB  = opaqueImage->GetImage().GetZBuffer();
            const int numpixels = screen[0]*screen[1];
            for (int p = 0 ; p < numpixels ; p++)
            {
                double val = oldNearPlane +
                             (oldFarPlane-oldNearPlane)*opaqueImageZB[p];
                opaqueImageZB[p] = (val-newNearPlane)
                                 / (newFarPlane-newNearPlane);
            }
        }
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
            image->Update(GetGeneralContract());
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

