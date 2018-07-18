/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
#include <avtMemory.h>
#include <avtCallback.h>

#include <avtSLIVRImgMetaData.h>
#include <avtSLIVRImgCommunicator.h>
#ifdef PARALLEL
#include <avtImageCommunicator.h>
#include <avtSamplePointCommunicator.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

#if defined (_MSC_VER) && (_MSC_VER < 1800) && !defined(round)
inline double round(double x) {return (x-floor(x)) > 0.5 ? ceil(x) : floor(x);}
#endif

using     std::vector;

bool sortImgMetaDataByDepth
(slivr::ImgMetaData const& before, slivr::ImgMetaData const& after)
{ return before.clip_z > after.clip_z; }
bool sortImgMetaDataByEyeSpaceDepth
(slivr::ImgMetaData const& before, slivr::ImgMetaData const& after)
{ return before.eye_z > after.eye_z; }


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
//    Qi WU, Wed Jun 20 2018
//    Add support for volume rendering using OSPRay
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

    panPercentage[0] = 0;
    panPercentage[1] = 0;

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
    // flags
    kernelBasedSampling = false;
    trilinearInterpolation = false;
    rayCastingSLIVR = false;
    convexHullOnRCSLIVR = false;
    // lighting properties
    lighting = false;
    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0;
    lightPosition[3] = 1.0;
    materialProperties[0] = 0.4; 
    materialProperties[1] = 0.75;
    materialProperties[2] = 0.0;
    materialProperties[3] = 15.0;
    // ospray
    ospray = NULL;
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
//  Method: avtRayTracer::CheckInBounds
//
//  Purpose:
//    Checks whether a coordinate value (coord) falls into a volume (volBounds)
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
bool
avtRayTracer::CheckInBounds(double volBounds[6], double coord[3])
{
    if (coord[0] > volBounds[0] && coord[0] < volBounds[1])
        if (coord[1] > volBounds[2] && coord[1] < volBounds[3])
            if (coord[2] > volBounds[4] && coord[2] < volBounds[5])
                return true;

    return false;
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
//    Qi WU, Wed Jun 20 2018
//    Add support for volume rendering using OSPRay
//
// ****************************************************************************

void
avtRayTracer::Execute(void)
{
    //=======================================================================//
    // Initialization and Debug
    //=======================================================================//
    // check memory in the beginning
    ospout << "[avrRayTracer] entering execute" << std::endl;
    slivr::CheckMemoryHere("[avtRayTracer] Execute", "ospout");    
    // initialize current time
    int timingIndex = visitTimer->StartTimer();

    //=======================================================================//
    // Start of original pipeline
    //=======================================================================//
    bool parallelOn = (imgComm.GetParSize() == 1) ? false : true;
    if (rayfoo == NULL)
    {
        debug1 << "Never set ray function for ray tracer." << endl;
        EXCEPTION0(ImproperUseException);
    }

    //
    // First we need to transform all of domains into camera space.
    //
    ospout << "[avrRayTracer] compute camera" << std::endl;
    double aspect = 1.;
    if (screen[1] > 0)
    {
        aspect = (double)screen[0] / (double)screen[1];
    }
    double scale[3] = {1,1,1};
    vtkMatrix4x4 *transform = vtkMatrix4x4::New();
    avtWorldSpaceToImageSpaceTransform::CalculateTransform
        (view, transform, scale, aspect);
    double newNearPlane, newFarPlane, oldNearPlane, oldFarPlane;
    TightenClippingPlanes(view, transform, newNearPlane, newFarPlane);
    oldNearPlane = view.nearPlane;  oldFarPlane  = view.farPlane;
    view.nearPlane = newNearPlane;  view.farPlane  = newFarPlane;
    transform->Delete();
    avtWorldSpaceToImageSpaceTransform trans(view, aspect);
    trans.SetInput(GetInput());

    //
    // Extract all of the samples from the dataset.
    //
    ospout << "[avrRayTracer] create extractor" << std::endl;
    avtSamplePointExtractor extractor(screen[0], screen[1], samplesPerRay);
    bool doKernel = kernelBasedSampling;
    if (trans.GetOutput()->
        GetInfo().GetAttributes().GetTopologicalDimension() == 0) 
    {
        doKernel = true;
    }
    extractor.SetKernelBasedSampling(doKernel);
    extractor.RegisterRayFunction(rayfoo);
    extractor.SetJittering(true);
    extractor.SetInput(trans.GetOutput());        
    extractor.SetTrilinear(trilinearInterpolation); 
        
    //
    // Before Rendering
    //
    double dbounds[6];  // Extents of the volume in world coordinates
    vtkMatrix4x4  *model_to_screen_transform = vtkMatrix4x4::New();
    vtkMatrix4x4  *screen_to_model_transform = vtkMatrix4x4::New();
    vtkMatrix4x4  *screen_to_camera_transform = vtkMatrix4x4::New();
    vtkImageData  *opaqueImageVTK = NULL;
    unsigned char *opaqueImageData = NULL;
    float         *opaqueImageZB = NULL;
    std::vector<float> opaqueImageDepth(screen[0] * screen[1], oldFarPlane);
    int            fullImageExtents[4];

    //
    // Ray casting: SLIVR ~ Setup
    //
    if (rayCastingSLIVR)
    {
        extractor.SetRayCastingSLIVR(true);
        //
        // Camera Settings
        //
        vtkCamera *sceneCam = vtkCamera::New();
        sceneCam->SetPosition(view.camera[0],view.camera[1],view.camera[2]);
        sceneCam->SetFocalPoint(view.focus[0],view.focus[1],view.focus[2]);
        sceneCam->SetViewUp(view.viewUp[0],view.viewUp[1],view.viewUp[2]);
        sceneCam->SetViewAngle(view.viewAngle);
        sceneCam->SetClippingRange(oldNearPlane, oldFarPlane);
        if (view.orthographic) { sceneCam->ParallelProjectionOn(); }
        else { sceneCam->ParallelProjectionOff(); }
        sceneCam->SetParallelScale(view.parallelScale); 
        // Clip planes
        double oldclip[2] = {oldNearPlane, oldFarPlane};
        panPercentage[0] = view.imagePan[0];
        panPercentage[1] = view.imagePan[1];
        // Scaling
        vtkMatrix4x4 *matScale = vtkMatrix4x4::New();
        matScale->Identity(); 
        if (avtCallback::UseOSPRay())
        { 
            // This is set in line 686 "ospray->SetScaling(scale)"
        }
        else 
        {
            matScale->SetElement(0, 0, scale[0]); 
            matScale->SetElement(1, 1, scale[1]);
            matScale->SetElement(2, 2, scale[2]);
        }
        // Scale + Model + View Matrix
        vtkMatrix4x4 *matViewModelScale = vtkMatrix4x4::New();
        vtkMatrix4x4 *matViewModel = sceneCam->GetModelViewTransformMatrix();
        vtkMatrix4x4::Multiply4x4(matViewModel, matScale, matViewModelScale);
        // Zooming
        vtkMatrix4x4 *matZoomViewModelScale = vtkMatrix4x4::New();
        vtkMatrix4x4 *matZoom = vtkMatrix4x4::New();
        matZoom->Identity(); 
        matZoom->SetElement(0, 0, view.imageZoom); 
        matZoom->SetElement(1, 1, view.imageZoom);
        vtkMatrix4x4::Multiply4x4(matZoom, matViewModelScale, 
                                  matZoomViewModelScale);
        // Projection:
        //
        // https://www.vtk.org/doc/release/6.1/html/classvtkCamera.html
        // HASH: #a4d9a509bf60f1555a70ecdee758c2753
        //
        // The Z buffer that is passed from visit is in clip scape with z 
        // limits of -1 and 1. However, using VTK 6.1.0, the z limits are 
        // wired. So, the projection matrix from VTK is hijacked here and
        // adjusted to be within -1 and 1 too
        //
        // Actually the correct way of using VTK GetProjectionTransformMatrix 
        // is to set near and far plane as -1 and 1
        //
        vtkMatrix4x4 *matProj = 
            sceneCam->GetProjectionTransformMatrix(aspect, -1, 1);
        double sceneSize[2];
        if (!view.orthographic)
        {
            sceneSize[0] = 2.0 * oldNearPlane / matProj->GetElement(0, 0);
            sceneSize[1] = 2.0 * oldNearPlane / matProj->GetElement(1, 1);
        }
        else
        {
            sceneSize[0] = 2.0 / matProj->GetElement(0, 0);
            sceneSize[1] = 2.0 / matProj->GetElement(1, 1);
        }
        // Compute model_to_screen_transform matrix
        vtkMatrix4x4::Multiply4x4(matProj,matZoomViewModelScale,
                                  model_to_screen_transform);
        vtkMatrix4x4::Invert(model_to_screen_transform,
                             screen_to_model_transform);
        vtkMatrix4x4::Invert(matProj,
                             screen_to_camera_transform);
        // Debug
        ospout << "[avrRayTracer] matZoom " << *matZoom << std::endl;
        ospout << "[avrRayTracer] matViewModel " << *matViewModel << std::endl;
        ospout << "[avrRayTracer] matScale " << *matScale << std::endl;
        ospout << "[avrRayTracer] matProj " << *matProj << std::endl;
        // Cleanup
        matScale->Delete();
        matViewModel->Delete();
        matViewModelScale->Delete();
        matZoom->Delete();
        matZoomViewModelScale->Delete();
        matProj->Delete();
        // Get the full image extents of the volume
        double depthExtents[2];
        GetSpatialExtents(dbounds);
        slivr::ProjectWorldToScreenCube(dbounds, screen[0], screen[1], 
                                        panPercentage, view.imageZoom,
                                        model_to_screen_transform,
                                        fullImageExtents, depthExtents);
        fullImageExtents[0] = std::max(fullImageExtents[0], 0);
        fullImageExtents[2] = std::max(fullImageExtents[2], 0);
        fullImageExtents[1] = std::min(1+fullImageExtents[1], screen[0]);
        fullImageExtents[3] = std::min(1+fullImageExtents[3], screen[1]);
        // Debug
        ospout << "[avrRayTracer] View settings: " << endl
               << "  inheriant view direction: "
               << viewDirection[0] << " "
               << viewDirection[1] << " "
               << viewDirection[2] << std::endl
               << "  camera: "       
               << view.camera[0] << ", " 
               << view.camera[1] << ", " 
               << view.camera[2] << std::endl
               << "  focus: "    
               << view.focus[0] << ", " 
               << view.focus[1] << ", " 
               << view.focus[2] << std::endl
               << "  viewUp: "    
               << view.viewUp[0] << ", " 
               << view.viewUp[1] << ", " 
               << view.viewUp[2] << std::endl
               << "  viewAngle: " << view.viewAngle << std::endl
               << "  eyeAngle:  " << view.eyeAngle  << std::endl
               << "  parallelScale: " << view.parallelScale  << std::endl
               << "  setScale: " << view.setScale << std::endl
               << "  scale:    " 
               << scale[0] << " " 
               << scale[1] << " " 
               << scale[2] << " " 
               << std::endl
               << "  nearPlane: " << view.nearPlane << std::endl
               << "  farPlane:  " << view.farPlane  << std::endl
               << "  imagePan[0]: " << view.imagePan[0] << std::endl 
               << "  imagePan[1]: " << view.imagePan[1] << std::endl
               << "  imageZoom:   " << view.imageZoom   << std::endl
               << "  orthographic: " << view.orthographic << std::endl
               << "  shear[0]: " << view.shear[0] << std::endl
               << "  shear[1]: " << view.shear[1] << std::endl
               << "  shear[2]: " << view.shear[2] << std::endl
               << "  oldNearPlane: " << oldNearPlane << std::endl
               << "  oldFarPlane:  " << oldFarPlane  << std::endl
               << "  aspect: " << aspect << std::endl
               << "[avrRayTracer] sceneSize: " 
               << sceneSize[0] << " " 
               << sceneSize[1] << std::endl
               << "[avrRayTracer] screen: " 
               << screen[0] << " " << screen[1] << std::endl
               << "[avrRayTracer] data bounds: " << std::endl
               << "\t" << dbounds[0] << " " << dbounds[1] << std::endl
               << "\t" << dbounds[2] << " " << dbounds[3] << std::endl
               << "\t" << dbounds[4] << " " << dbounds[5] << std::endl
               << "[avrRayTracer] full image extents: " << std::endl
               << "\t" << fullImageExtents[0] << " "
               << "\t" << fullImageExtents[1] << std::endl
               << "\t" << fullImageExtents[2] << " "
               << "\t" << fullImageExtents[3] << std::endl;
        ospout << "[avrRayTracer] model_to_screen_transform: " 
               << *model_to_screen_transform << std::endl;
        ospout << "[avrRayTracer] screen_to_model_transform: " 
               << *screen_to_model_transform << std::endl;
        ospout << "[avrRayTracer] screen_to_camera_transform: " 
               << *screen_to_camera_transform << std::endl;

        //===================================================================//
        // ospray stuffs
        //===================================================================//
        if (avtCallback::UseOSPRay()) {
            slivr::CheckMemoryHere("[avtRayTracer] Execute before ospray", 
                                   "ospout");
            // initialize ospray
            // -- multi-threading enabled
            ospray->InitOSP();
            // camera
            ospout << "[avrRayTracer] make ospray camera" << std::endl;
            if (!view.orthographic)
            {
                ospray->camera.Init(OSPVisItCamera::PERSPECTIVE);
            }
            else 
            {
                ospray->camera.Init(OSPVisItCamera::ORTHOGRAPHIC);
            }
            ospray->camera.Set(view.camera,
                               view.focus, 
                               view.viewUp, 
                               viewDirection,
                               sceneSize, 
                               aspect, 
                               view.viewAngle, 
                               view.imageZoom,
                               view.imagePan, 
                               fullImageExtents, 
                               screen);
            ospray->SetScaling(scale);
            // transfer function
            ospout  << "[avrRayTracer] make ospray transfer function" 
                    << std::endl;
            ospray->transferfcn.Init();
            ospray->transferfcn.Set
                ((OSPVisItColor*)transferFn1D->GetTableFloat(), 
                 transferFn1D->GetNumberOfTableEntries(),
                 (float)transferFn1D->GetMin(),
                 (float)transferFn1D->GetMax());
            // renderer
            ospout << "[avrRayTracer] make ospray renderer" << std::endl;
            ospray->renderer.Init();
            ospray->renderer.Set(materialProperties, viewDirection, lighting);
            ospray->SetDataBounds(dbounds);
            // check memory
            slivr::CheckMemoryHere("[avtRayTracer] Execute after ospray",
                                   "ospout");
        }

        // 
        // Continuation of previous pipeline
        //
        extractor.SetJittering(false);
        extractor.SetLighting(lighting);
        extractor.SetLightDirection(lightDirection);
        extractor.SetMatProperties(materialProperties);
        extractor.SetViewDirection(viewDirection);
        extractor.SetTransferFn(transferFn1D);
        extractor.SetClipPlanes(oldclip);
        extractor.SetPanPercentages(view.imagePan);
        extractor.SetImageZoom(view.imageZoom);
        extractor.SetRendererSampleRate(rendererSampleRate); 
        extractor.SetDepthExtents(depthExtents);
        extractor.SetMVPMatrix(model_to_screen_transform);
        extractor.SetFullImageExtents(fullImageExtents);
        // sending ospray
        extractor.SetOSPRay(ospray);

        //
        // Capture background
        //
        opaqueImageVTK  = opaqueImage->GetImage().GetImageVTK();
        opaqueImageData = 
            (unsigned char *)opaqueImageVTK->GetScalarPointer(0, 0, 0);
        opaqueImageZB   = opaqueImage->GetImage().GetZBuffer();
        int bufferScreenExtents[4] = {0,screen[0],0,screen[1]};
        extractor.SetDepthBuffer(opaqueImageZB,   screen[0]*screen[1]);
        extractor.SetRGBBuffer  (opaqueImageData, screen[0],screen[1]);
        extractor.SetBufferExtents(bufferScreenExtents);
        // Set the background to OSPRay
        if (avtCallback::UseOSPRay()) 
        {
            for (int y = 0; y < screen[1]; ++y)                     
            {
                for (int x = 0; x < screen[0]; ++x) 
                {
                    int index = x + y * screen[0];
                    int    screenCoord[2] = {x, y};
                    double screenDepth = opaqueImageZB[index] * 2 - 1;
                    double worldCoord[3];
                    slivr::ProjectScreenToCamera
                        (screenCoord, screenDepth, 
                         screen[0], screen[1],
                         screen_to_camera_transform, 
                         worldCoord);
                    opaqueImageDepth[index] = -worldCoord[2];
                }
            }
            ospray->SetBgBuffer(opaqueImageDepth.data(), 
                                bufferScreenExtents);
        }
        // TODO We cannot delete camera here, why ?
        //sceneCam->Delete();
    }

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

    // Qi debug
    slivr::CheckMemoryHere("[avtRayTracer] Execute raytracing setup done",
                           "ospout");

    // Execute raytracer
    avtDataObject_p samples = extractor.GetOutput();

    //
    // Ray casting: SLIVR ~ After Rendering
    //
    if (rayCastingSLIVR == true)
    {
        // Only required to force an update 
        // Need to find a way to get rid of that!!!!
        avtRayCompositer rc(rayfoo);
        rc.SetInput(samples);
        avtImage_p image  = rc.GetTypedOutput();

        // Start timing
        int timingVolToImg = visitTimer->StartTimer(); 
        
        // Execute rendering
        // This will call the execute function
        image->Update(GetGeneralContract()); 

        // Time rendering
        visitTimer->StopTimer(timingVolToImg, "AllPatchRendering");

        //
        // Image Compositing
        //
        // Timing
        int timingCompositinig = visitTimer->StartTimer();
        int timingOnlyCompositinig = visitTimer->StartTimer();
        int timingDetail;
        // Initialization
        float *compositedData = NULL;
        int compositedW, compositedH;
        int compositedExtents[4];
        // Debug
        int numPatches = extractor.GetImgPatchSize();
        ospout << "[avtRayTracer] Total num of patches " 
               << numPatches << std::endl;
        for (int i=0; i<numPatches; i++)
        {
            slivr::ImgMetaData currImgMeta = extractor.GetImgMetaPatch(i);
            ospout << "[avtRayTracer] Rank " << PAR_Rank() << " "
                   << "Idx " << i << " (" << currImgMeta.patchNumber << ") " 
                   << " depth " << currImgMeta.eye_z << std::endl
                   << "current patch size = " 
                   << currImgMeta.dims[0] << ", " 
                   << currImgMeta.dims[1] << std::endl
                   << "current patch starting" 
                   << " X = " << currImgMeta.screen_ll[0] 
                   << " Y = " << currImgMeta.screen_ll[1] << std::endl
                   << "current patch ending" 
                   << " X = " << currImgMeta.screen_ur[0] 
                   << " Y = " << currImgMeta.screen_ur[1] << std::endl;
        }
        //-------------------------------------------------------------------//
        // IceT: If each rank has only one patch, we use IceT to composite
        //-------------------------------------------------------------------//
        if (imgComm.IceTValid() && extractor.GetImgPatchSize() == 1)    
        {
            //---------------------------------------------------------------//
            // Setup Local Tile
            slivr::ImgMetaData currMeta = extractor.GetImgMetaPatch(0);
            slivr::ImgData     currData;
            currData.imagePatch = NULL;
            extractor.GetAndDelImgData /* do shallow copy inside */
                (currMeta.patchNumber, currData);
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // First Composition
            if (PAR_Size() > 1)
            { 
                compositedW = fullImageExtents[1] - fullImageExtents[0];
                compositedH = fullImageExtents[3] - fullImageExtents[2];
                compositedExtents[0] = fullImageExtents[0];
                compositedExtents[1] = fullImageExtents[1];
                compositedExtents[2] = fullImageExtents[2];
                compositedExtents[3] = fullImageExtents[3];
                if (PAR_Rank() == 0) {
                    compositedData = 
                        new float[4 * compositedW * compositedH]();
                }
                int currExtents[4] = 
                    {std::max(currMeta.screen_ll[0]-fullImageExtents[0], 0), 
                     std::min(currMeta.screen_ur[0]-fullImageExtents[0], 
                              compositedW), 
                     std::max(currMeta.screen_ll[1]-fullImageExtents[2], 0),
                     std::min(currMeta.screen_ur[1]-fullImageExtents[2],
                              compositedH)};
                imgComm.IceTInit(compositedW, compositedH);
                imgComm.IceTSetTile(currData.imagePatch, 
                                    currExtents,
                                    currMeta.eye_z);
                imgComm.IceTComposite(compositedData);
                if (currData.imagePatch != NULL) {
                    delete[] currData.imagePatch;
                    currData.imagePatch = NULL;
                }
            } else {
                compositedW = currMeta.dims[0];
                compositedH = currMeta.dims[1];
                compositedExtents[0] = fullImageExtents[0];
                compositedExtents[1] = fullImageExtents[0] + compositedW;
                compositedExtents[2] = fullImageExtents[2];
                compositedExtents[3] = fullImageExtents[2] + compositedH;
                compositedData = currData.imagePatch;
                currData.imagePatch = NULL;
            }
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Memory
            slivr::CheckMemoryHere("[avtRayTracer] Execute "
                                   "IceT Compositing Done", 
                                   "ospout");
            //---------------------------------------------------------------//
        }
        //-------------------------------------------------------------------//
        // SERIAL: Image Composition
        //-------------------------------------------------------------------//
        else if (parallelOn == false)
        {
            //---------------------------------------------------------------//
            // Get the Metadata for All Patches
            slivr::CheckSectionStart("avtRayTracer", "Execute", timingDetail,
                                     "Serial-Composite: Get the Metadata for "
                                     "All Patches");
            // contains the metadata to composite the image
            std::vector<slivr::ImgMetaData> allPatchMeta;
            std::vector<slivr::ImgData>     allPatchData;
            // get the number of patches
            int numPatches = extractor.GetImgPatchSize();
            for (int i=0; i<numPatches; i++)
            {
                allPatchMeta.push_back(extractor.GetImgMetaPatch(i));
            }
            slivr::CheckSectionStop("avtRayTracer", "Execute", timingDetail,
                                    "Serial-Composite: Get the Metadata for "
                                    "All Patches");
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Sort with the Largest z First
            slivr::CheckSectionStart("avtRayTracer", "Execute", timingDetail,
                                     "Serial-Composite: Sort with the Largest "
                                     "z First");
            std::sort(allPatchMeta.begin(), allPatchMeta.end(), 
                      &sortImgMetaDataByEyeSpaceDepth);
            slivr::CheckSectionStop("avtRayTracer", "Execute", timingDetail,
                                    "Serial-Composite: Sort with the Largest "
                                    "z First");
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Blend Images
            slivr::CheckSectionStart("avtRayTracer", "Execute", timingDetail,
                                     "Serial-Composite: Blend Images");
            compositedW = fullImageExtents[1] - fullImageExtents[0];
            compositedH = fullImageExtents[3] - fullImageExtents[2];
            compositedExtents[0] = fullImageExtents[0];
            compositedExtents[1] = fullImageExtents[0] + compositedW;
            compositedExtents[2] = fullImageExtents[2];
            compositedExtents[3] = fullImageExtents[2] + compositedH;       
            if (PAR_Rank() == 0) {
                compositedData = new float[compositedW * compositedH * 4]();
            }
            for (int i=0; i<numPatches; i++)
            {
                slivr::ImgMetaData currImgMeta = allPatchMeta[i];
                slivr::ImgData     currImgData;
                currImgData.imagePatch = NULL;
                extractor.GetAndDelImgData /* do shallow copy inside */
                    (currImgMeta.patchNumber, currImgData);
                const float* currData = currImgData.imagePatch;
                const int currExtents[4] = 
                    {currImgMeta.screen_ll[0], currImgMeta.screen_ur[0], 
                     currImgMeta.screen_ll[1], currImgMeta.screen_ur[1]};
                avtSLIVRImgCommunicator::BlendBackToFront(currData,
                                                          currExtents,
                                                          compositedData, 
                                                          compositedExtents);
                // Clean up data
                if (currImgData.imagePatch != NULL) {
                    delete[] currImgData.imagePatch;
                }
                currImgData.imagePatch = NULL;
            }
            allPatchMeta.clear();
            allPatchData.clear();
            slivr::CheckSectionStop("avtRayTracer", "Execute", timingDetail,
                                    "Serial-Composite: Blend Images");
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Memory
            slivr::CheckMemoryHere("[avtRayTracer] Execute "
                                   "Sequential Compositing Done", 
                                   "ospout");
            //---------------------------------------------------------------//
        } 
        //
        // PARALLEL: Image Composition
        //
        else
        { 
            //---------------------------------------------------------------//
            // Parallel Direct Send
            slivr::CheckSectionStart("avtRayTracer", "Execute", timingDetail,
                                     "Parallel-Composite: "
                                     "Parallel Direct Send");
            int tags[2] = {1081, 1681};
            int tagGather = 2681;
            int *regions = NULL;
            imgComm.RegionAllocation(regions);
            int myRegionHeight =
                imgComm.ParallelDirectSendManyPatches
                (extractor.imgDataHashMap, extractor.imageMetaPatchVector,
                 numPatches, regions, imgComm.GetParSize(), tags, 
                 fullImageExtents);
            imgComm.gatherImages(regions, imgComm.GetParSize(), 
                                 imgComm.intermediateImage, 
                                 imgComm.intermediateImageExtents, 
                                 imgComm.intermediateImageExtents, 
                                 tagGather, fullImageExtents, myRegionHeight);

            slivr::CheckSectionStop("avtRayTracer", "Execute", timingDetail,
                                    "Parallel-Composite: "
                                    "Parallel Direct Send");
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Some Cleanup
            slivr::CheckSectionStart("avtRayTracer", "Execute", timingDetail,
                                     "Parallel-Composite: Some Cleanup");
            if (regions != NULL)
                delete [] regions;
            regions = NULL;
            if (imgComm.intermediateImage != NULL)
                delete [] imgComm.intermediateImage;
            imgComm.intermediateImage = NULL;           
            imgComm.Barrier();
            slivr::CheckSectionStop("avtRayTracer", "Execute", timingDetail,
                                    "Parallel-Composite: Some Cleanup");
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Setup for Final Composition
            compositedW = 
                imgComm.finalImageExtents[1] -
                imgComm.finalImageExtents[0];
            compositedH = 
                imgComm.finalImageExtents[3] -
                imgComm.finalImageExtents[2];
            compositedExtents[0] = imgComm.finalImageExtents[0];
            compositedExtents[1] = imgComm.finalImageExtents[1];
            compositedExtents[2] = imgComm.finalImageExtents[2];
            compositedExtents[3] = imgComm.finalImageExtents[3];
            if (PAR_Rank() == 0) {
                compositedData = imgComm.GetFinalImageBuffer();
            }
            //---------------------------------------------------------------//
            //---------------------------------------------------------------//
            // Memory
            slivr::CheckMemoryHere("[avtRayTracer] Execute "
                                   "Parallel Compositing Done", 
                                   "ospout");
            //---------------------------------------------------------------//
        }       
        visitTimer->StopTimer(timingOnlyCompositinig, "Pure Compositing");

        ///////////////////////////////////////////////////////////////////
        //
        // Final Composition for Displaying
        //
        ///////////////////////////////////////////////////////////////////
        if (PAR_Rank() == 0) 
        {
            // Blend
            avtImage_p finalImage = new avtImage(this);
            vtkImageData *finalVTKImage = 
                avtImageRepresentation::NewImage(screen[0], screen[1]);
            finalImage->GetImage() = finalVTKImage;
            unsigned char *finalImageBuffer = 
                finalImage->GetImage().GetRGBBuffer();
            slivr::CompositeBackground(screen,
                                       compositedExtents,
                                       compositedW,
                                       compositedH,
                                       compositedData,
                                       opaqueImageData,
                                       opaqueImageZB,
                                       finalImageBuffer);
            // Cleanup
            finalVTKImage->Delete();
            SetOutput(finalImage);
        }
        if (compositedData != NULL) { 
            delete [] compositedData;
        }
        compositedData = NULL; 
        ospout << "[avtRayTracer] Raycasting SLIVR is Done !" << std::endl;
        
        //
        // time compositing
        //
        visitTimer->StopTimer(timingCompositinig, "Compositing");
        
    } else {

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
                const int numpixels = screen[0]*screen[1];
                float *opaqueImageZB  = opaqueImage->GetImage().GetZBuffer();
                vtkImageData  *opaqueImageVTK = 
                    opaqueImage->GetImage().GetImageVTK();
                unsigned char *opaqueImageData = 
                    (unsigned char*)opaqueImageVTK->GetScalarPointer(0, 0, 0);

                for (int p = 0 ; p < numpixels ; p++)
                {
                    // The z value in clip space in the depth buifer is
                    // between 0 and 1 while it is normal for that
                    // value to be between -1 and 1 instead. 
                    // This is corrected here.
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
            else 
            {
                // orthographic and need to adjust for tightened clipping
                // planes
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
        int numDivisions = 
            GetNumberOfDivisions(screen[0],screen[1],samplesPerRay);

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
                            int index2 = 
                                (IEnd-IStart)*(jj-JStart) + (ii-IStart);
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

    }

    //
    // Clean up
    //
    screen_to_model_transform->Delete();
    model_to_screen_transform->Delete();
    screen_to_camera_transform->Delete();
    
    //
    // Stop timer 
    //
    visitTimer->StopTimer(timingIndex, "Ray Tracing");

    //
    // Write timing to file
    // Postpone this
    //
    // visitTimer->DumpTimings();
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
//     Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//     Change extents names.
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
    avtExtents *exts = datts.GetDesiredSpatialExtents();
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


// ****************************************************************************
//  Method: avtRayTracer::computeRay
//
//  Purpose:
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
void
avtRayTracer::ComputeRay(double camera[3], double position[3], double ray[3])
{
        for (int i=0; i<3; i++)
                ray[i] = position[i] - camera[i];

        double mag = sqrt( ray[0]*ray[0] + ray[1]*ray[1] + ray[2]*ray[2] );

        for (int i=0; i<3; i++)
                ray[i] = ray[i]/mag;
}


// ****************************************************************************
//  Method: avtRayTracer::intersect
//
//  Purpose:
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
bool
avtRayTracer::Intersect(double bounds[6], double ray[3], double cameraPos[3],
                        double &tMin, double &tMax)
{
        double t1, t2, tXMin, tXMax, tYMin, tYMax, tZMin, tZMax;
        double invRay[3];

        for (int i=0; i<3; i++)
                invRay[i] = 1.0 / ray[i];

        // X
        t1 = (bounds[0] - cameraPos[0]) * invRay[0];
        t2 = (bounds[1] - cameraPos[0]) * invRay[0];

        tXMin = std::min(t1, t2);
        tXMax = std::max(t1, t2);


        // Y
        t1 = (bounds[2] - cameraPos[1]) * invRay[1];
        t2 = (bounds[3] - cameraPos[1]) * invRay[1];

        tYMin = std::min(t1, t2);
        tYMax = std::max(t1, t2);


        // Z
        t1 = (bounds[4] - cameraPos[2]) * invRay[2];
        t2 = (bounds[5] - cameraPos[2]) * invRay[2];

        tZMin = std::min(t1, t2);
        tZMax = std::max(t1, t2);


        // Comparing
        if ((tXMin > tYMax) || (tYMin > tXMax))
                return false;

        tMin = t1 = std::max(tXMin, tYMin);
        tMax = t2 = std::min(tXMax, tYMax);


        if ((t1 > tZMax) || (tZMin > t2))
                return false;

        tMin = std::max(t1, tZMin);
        tMax = std::min(t2, tYMax);

        return true;
}
