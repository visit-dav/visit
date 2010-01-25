/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             avtRayTracer.C                                //
// ************************************************************************* //

#include <avtRayTracer.h>

#include <vector>

#include <visit-config.h>

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

#include <avtCommonDataFunctions.h>
#include <avtDataset.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
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
//  Modifications:
//
//    Hank Childs, Fri Jun  9 13:47:29 PDT 2006
//    Replace 3 with numPerTile (oversight that it wasn't coded that way
//    originally).
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
    return numPerTile*nD*nD;
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
//  Modifications:
//
//    Hank Childs, Sat Sep 26 20:43:55 CDT 2009
//    If we have more than 32 procs, then we have enough memory and don't need 
//    to tile.
//
// ****************************************************************************

int
avtRayTracer::GetNumberOfDivisions(int screenX, int screenY, int screenZ)
{
    if (PAR_Size() >= 32)
        return 1;

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
    avtWorldSpaceToImageSpaceTransform::CalculateTransform(view, transform, 
                                                           scale, aspect);
    double newNearPlane, newFarPlane, oldNearPlane, oldFarPlane;
    TightenClippingPlanes(view, transform, newNearPlane, newFarPlane);
    oldNearPlane = view.nearPlane;
    oldFarPlane  = view.farPlane;
    view.nearPlane = newNearPlane;
    view.farPlane  = newFarPlane;
    transform->Delete();

    avtWorldSpaceToImageSpaceTransform trans(view, aspect);
    trans.SetInput(GetInput());

    //
    // Extract all of the samples from the dataset.
    //
    avtSamplePointExtractor extractor(screen[0], screen[1], samplesPerRay);
    bool doKernel = kernelBasedSampling;
    if (trans.GetOutput()->GetInfo().GetAttributes().GetTopologicalDimension()
        == 0)
        doKernel = true;
    extractor.SetKernelBasedSampling(doKernel);
    extractor.RegisterRayFunction(rayfoo);
    extractor.SetJittering(true);
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
        bool convertToWBuffer = !view.orthographic;
        if (convertToWBuffer)
        {
            float *opaqueImageZB  = opaqueImage->GetImage().GetZBuffer();
            const int numpixels = screen[0]*screen[1];
            for (int p = 0 ; p < numpixels ; p++)
            {
                // We want the value to be between -1 and 1.
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
//  Method: avtRayTracer::ModifyContract
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
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Thu May 29 09:44:17 PDT 2008
//    No longer remove domains that cannot contribute to final picture, 
//    because that decision is made here one time for many renders.  If you
//    choose one set, it may change later for a different render.
//
// ****************************************************************************

avtContract_p
avtRayTracer::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);
    rv->NoStreaming();
    return rv;
}


// ****************************************************************************
//  Method:  avtRayTracer::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtRayTracer::FilterUnderstandsTransformedRectMesh()
{
    // There's nothing special about the raytracer that might
    // not understand these meshes.  At a lower level, filters
    // like the sample point extractor report this correctly.
    return true;
}


// ****************************************************************************
//   Method: avtRayTracer::TightenClippingPlanes
//
//   Purpose:
//       Tightens the clipping planes, so that more samples fall within
//       the view frustum.
//
//   Notes:      This code was originally in
//               avtWorldSpaceToImageSpaceTransform::PreExecute.
//
//   Programmer: Hank Childs
//   Creation:   December 24, 2008
//
//   Modifications:
//
//     Hank Childs, Sat Sep 26 20:43:55 CDT 2009
//     Fixed bug for tightening planes when the camera is inside the volume.
//
// ****************************************************************************

void
avtRayTracer::TightenClippingPlanes(const avtViewInfo &view,
                                    vtkMatrix4x4 *transform,
                                    double &newNearPlane, double &newFarPlane)
{
    newNearPlane = view.nearPlane;
    newFarPlane  = view.farPlane;

    double dbounds[6];
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    avtExtents *exts = datts.GetEffectiveSpatialExtents();
    if (exts->HasExtents())
    {
        exts->CopyTo(dbounds);
    }
    else
    {
        GetSpatialExtents(dbounds);
    }

    double vecFromCameraToPlaneX = view.focus[0] - view.camera[0];
    double vecFromCameraToPlaneY = view.focus[1] - view.camera[1];
    double vecFromCameraToPlaneZ = view.focus[2] - view.camera[2];
    double vecMag = (vecFromCameraToPlaneX*vecFromCameraToPlaneX)
                  + (vecFromCameraToPlaneY*vecFromCameraToPlaneY)
                  + (vecFromCameraToPlaneZ*vecFromCameraToPlaneZ);
    vecMag = sqrt(vecMag);

    double farthest = 0.;
    double nearest  = 0.;
    for (int i = 0 ; i < 8 ; i++)
    {
        double X = (i & 1 ? dbounds[1] : dbounds[0]);
        double Y = (i & 2 ? dbounds[3] : dbounds[2]);
        double Z = (i & 4 ? dbounds[5] : dbounds[4]);

        //
        // We are now constructing the dot product of our two vectors.  Note
        // That this will give us cosine of their angles times the magnitude
        // of the camera-to-plane vector times the magnitude of the
        // camera-to-farthest vector.  We want the magnitude of a new vector,
        // the camera-to-closest-point-on-plane-vector.  That vector will
        // lie along the same vector as the camera-to-plane and it forms
        // a triangle with the camera-to-farthest-vector.  Then we have the
        // same angle between them and we can re-use the cosine we calculate.
        //
        double vecFromCameraToX = X - view.camera[0];
        double vecFromCameraToY = Y - view.camera[1];
        double vecFromCameraToZ = Z - view.camera[2];

        //
        // dot = cos X * mag(A) * mag(B)
        // We know cos X = mag(C) / mag(A)   C = adjacent, A = hyp.
        // Then mag(C) = cos X * mag(A).
        // So mag(C) = dot / mag(B).
        //
        double dot = vecFromCameraToPlaneX*vecFromCameraToX
                   + vecFromCameraToPlaneY*vecFromCameraToY
                   + vecFromCameraToPlaneZ*vecFromCameraToZ;

        double dist = dot / vecMag;
        double newNearest  = dist - (view.farPlane-dist)*0.01; // fudge
        double newFarthest = dist + (dist-view.nearPlane)*0.01; // fudge
        if (i == 0)
        {
            farthest = newFarthest;
            nearest  = newNearest;
        }
        else
        {
            if (newNearest < nearest)
                nearest  = newNearest;
            if (newFarthest > farthest)
                farthest = newFarthest;
        }
    }

    if (nearest > view.nearPlane)
        newNearPlane = nearest;

    if (farthest < view.farPlane)
        newFarPlane = farthest;
}


