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

bool sortImgMetaDataByDepth(imgMetaData const& before, imgMetaData const& after){ return before.avg_z > after.avg_z; }
bool sortImgMetaDataByEyeSpaceDepth(imgMetaData const& before, imgMetaData const& after){ return before.eye_z > after.eye_z; }

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
    kernelBasedSampling = false;
    trilinearInterpolation = false;
    rayCastingSLIVR = false;
    convexHullOnRCSLIVR = false;

    lighting = false;
    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0;   lightPosition[3] = 1.0;
    materialProperties[0] = 0.4; materialProperties[1] = 0.75; materialProperties[3] = 0.0; materialProperties[3] = 15.0;
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
//  Function:
//
//  Purpose:
//      Blend images
//
//  Programmer: August 14, 2016
//  Creation:   Pascal Grosset
//
//  Modifications:
//
// ****************************************************************************

void
avtRayTracer::blendImages(float *src, int dimsSrc[2], int posSrc[2], float *dst, int dimsDst[2], int posDst[2])
{
    for (int _y=0; _y<dimsSrc[1]; _y++)
        for (int _x=0; _x<dimsSrc[0]; _x++)
        {
            int startingX = posSrc[0];
            int startingY = posSrc[1];

            if ((startingX + _x) > (posDst[0]+dimsDst[0]))
                continue;

            if ((startingY + _y) > (posDst[1]+dimsDst[1]))
                continue;

            int subImgIndex = dimsSrc[0]*_y*4 + _x*4;                                     // index in the subimage
            int bufferIndex = ( (startingY+_y - posDst[1])*dimsDst[0]*4  + (startingX+_x - posDst[0])*4 );    // index in the big buffer

            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
            dst[bufferIndex+0] = imgComm.clamp( (dst[bufferIndex+0] * (1.0 - src[subImgIndex+3])) + src[subImgIndex+0] );
            dst[bufferIndex+1] = imgComm.clamp( (dst[bufferIndex+1] * (1.0 - src[subImgIndex+3])) + src[subImgIndex+1] );
            dst[bufferIndex+2] = imgComm.clamp( (dst[bufferIndex+2] * (1.0 - src[subImgIndex+3])) + src[subImgIndex+2] );
            dst[bufferIndex+3] = imgComm.clamp( (dst[bufferIndex+3] * (1.0 - src[subImgIndex+3])) + src[subImgIndex+3] );
        }
}



// ****************************************************************************
//  Method: avtRayTracer::unProject
//
//  Purpose:
//      Convert from screen coordinates to world coordinates
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtRayTracer::unProject(int _x, int _y, float _z, double _worldCoordinates[3], int _width, int _height, vtkMatrix4x4 *invModelViewProj)
{
    // remove panning
    _x -= round(_width * panPercentage[0]);
    _y -= round(_height * panPercentage[1]); 

    double worldCoordinates[4] = {0,0,0,1};
    double in[4] = {0,0,0,1};
    in[0] = (_x - _width/2. )/(_width/2.);
    in[1] = (_y - _height/2.)/(_height/2.);
    in[2] = _z;

    invModelViewProj->MultiplyPoint(in, worldCoordinates);

    if (worldCoordinates[3] == 0)
        debug5 << "avtMassVoxelExtractor::unProject division by 0 error!" << endl;

    worldCoordinates[0] = worldCoordinates[0]/worldCoordinates[3];
    worldCoordinates[1] = worldCoordinates[1]/worldCoordinates[3];
    worldCoordinates[2] = worldCoordinates[2]/worldCoordinates[3];
    worldCoordinates[3] = worldCoordinates[3]/worldCoordinates[3];

    _worldCoordinates[0] = worldCoordinates[0];
    _worldCoordinates[1] = worldCoordinates[1];
    _worldCoordinates[2] = worldCoordinates[2];
}



// ****************************************************************************
//  Method: avtRayTracer::project
//
//  Purpose:
//      Convert world coordinates to screen coordinates
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************
double
avtRayTracer::project(double _worldCoordinates[3], int pos2D[2], int _width, int _height, vtkMatrix4x4 *modelViewProj)
{
    double normDevCoord[4];
    double worldCoordinates[4] = {0,0,0,1};
    worldCoordinates[0] = _worldCoordinates[0];
    worldCoordinates[1] = _worldCoordinates[1];
    worldCoordinates[2] = _worldCoordinates[2];

    // World to Clip space (-1 - 1)
    modelViewProj->MultiplyPoint(worldCoordinates, normDevCoord);

    if (normDevCoord[3] == 0)
    {
        debug5 << "avtMassVoxelExtractor::project division by 0 error!" << endl;
        debug5 << "worldCoordinates: " << worldCoordinates[0] << ", " << worldCoordinates[1] << ", " << worldCoordinates[2] << "   " << normDevCoord[0] << ", " << normDevCoord[1] << ", " << normDevCoord[2] << endl;
        debug5 << "Matrix: " << *modelViewProj << endl;
    }

    normDevCoord[0] = normDevCoord[0]/normDevCoord[3];
    normDevCoord[1] = normDevCoord[1]/normDevCoord[3];
    normDevCoord[2] = normDevCoord[2]/normDevCoord[3];
    normDevCoord[3] = normDevCoord[3]/normDevCoord[3];

    pos2D[0] = round( normDevCoord[0]*(_width/2.)  + (_width/2.)  );
    pos2D[1] = round( normDevCoord[1]*(_height/2.) + (_height/2.) );

    pos2D[0] += round(_width * panPercentage[0]);
    pos2D[1] += round(_height * panPercentage[1]);

    return normDevCoord[2];
}



// ****************************************************************************
//  Method: avtRayTracer::project3Dto2D
//
//  Purpose:
//          Compute the extents of a volume
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************
void
avtRayTracer::project3Dto2D(double _3Dextents[6], int width, int height, vtkMatrix4x4 *modelViewProj, int _2DExtents[4], double depthExtents[2])
{
    double _world[3];
    int _xMin, _xMax, _yMin, _yMax;
    double _zMin, _zMax;
    _xMin = _yMin = std::numeric_limits<int>::max();
    _xMax = _yMax = std::numeric_limits<int>::min();

    _zMin = std::numeric_limits<double>::max();
    _zMax = std::numeric_limits<double>::min();

    float coordinates[8][3];
    coordinates[0][0] = _3Dextents[0];   coordinates[0][1] = _3Dextents[2];   coordinates[0][2] = _3Dextents[4];
    coordinates[1][0] = _3Dextents[1];   coordinates[1][1] = _3Dextents[2];   coordinates[1][2] = _3Dextents[4];
    coordinates[2][0] = _3Dextents[1];   coordinates[2][1] = _3Dextents[3];   coordinates[2][2] = _3Dextents[4];
    coordinates[3][0] = _3Dextents[0];   coordinates[3][1] = _3Dextents[3];   coordinates[3][2] = _3Dextents[4];

    coordinates[4][0] = _3Dextents[0];   coordinates[4][1] = _3Dextents[2];   coordinates[4][2] = _3Dextents[5];
    coordinates[5][0] = _3Dextents[1];   coordinates[5][1] = _3Dextents[2];   coordinates[5][2] = _3Dextents[5];
    coordinates[6][0] = _3Dextents[1];   coordinates[6][1] = _3Dextents[3];   coordinates[6][2] = _3Dextents[5];
    coordinates[7][0] = _3Dextents[0];   coordinates[7][1] = _3Dextents[3];   coordinates[7][2] = _3Dextents[5];

    int pos2D[2];
    double _z;
    for (int i=0; i<8; i++)
    {
        _world[0] = coordinates[i][0];
        _world[1] = coordinates[i][1];
        _world[2] = coordinates[i][2];
        _z = project(_world, pos2D, width, height, modelViewProj);

        // Get min max
        _2DExtents[0] = _xMin = std::min(_xMin, pos2D[0]);
        _2DExtents[1] = _xMax = std::max(_xMax, pos2D[0]);
        _2DExtents[2] = _yMin = std::min(_yMin, pos2D[1]);
        _2DExtents[3] = _yMax = std::max(_yMax, pos2D[1]);

        depthExtents[0] = _zMin = std::min(_zMin, _z);
        depthExtents[1] = _zMax = std::max(_zMax, _z);
    }


    debug5 << "_2DExtents " << _2DExtents[0] << ", " << _2DExtents[1] << "   "  << _2DExtents[2] << ", "  << _2DExtents[3] << "     z: " << depthExtents[0] << ", " << depthExtents[1] << endl;
}



// ****************************************************************************
//  Method: avtRayTracer::checkInBounds
//
//  Purpose:
//       Checks whether a coordinate value (coord) falls into a volume (volBounds)
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
bool
avtRayTracer::checkInBounds(double volBounds[6], double coord[3])
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
// ****************************************************************************

void
avtRayTracer::Execute(void)
{
    int  timingIndex = visitTimer->StartTimer();
    bool parallelOn = (imgComm.GetNumProcs() == 1)?false:true;

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
    oldNearPlane = view.nearPlane;  oldFarPlane  = view.farPlane;
    view.nearPlane = newNearPlane;  view.farPlane  = newFarPlane;
    transform->Delete();

    avtWorldSpaceToImageSpaceTransform trans(view, aspect);
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
    extractor.SetInput(trans.GetOutput());


    if (trilinearInterpolation)
        extractor.SetTrilinear(true);


    //
    // Ray casting: SLIVR ~ Before Rendering
    //
    double dbounds[6];  // Extents of the volume in world coordinates
    vtkMatrix4x4 *pvm = vtkMatrix4x4::New();

    vtkImageData  *__opaqueImageVTK = NULL;
    unsigned char *__opaqueImageData = NULL;
    float         *__opaqueImageZB = NULL;

    int fullImageExtents[4];


    //
    // Ray casting: SLIVR ~ Setup
    //
    if (rayCastingSLIVR)
    {
        extractor.SetRayCastingSLIVR(true);

        //
        // Camera Settings
        vtkCamera *sceneCam = vtkCamera::New();
        sceneCam->SetPosition(view.camera[0],view.camera[1],view.camera[2]);
        sceneCam->SetFocalPoint(view.focus[0],view.focus[1],view.focus[2]);
        sceneCam->SetViewUp(view.viewUp[0],view.viewUp[1],view.viewUp[2]);
        sceneCam->SetViewAngle(view.viewAngle);
        sceneCam->SetClippingRange(oldNearPlane, oldFarPlane);
        if (view.orthographic)
            sceneCam->ParallelProjectionOn();
        else
            sceneCam->ParallelProjectionOff();
        sceneCam->SetParallelScale(view.parallelScale);

        debug5 << "RT View settings: " << endl;
        debug5 << "camera: "       << view.camera[0]      << ", " << view.camera[1]     << ", " << view.camera[2] << std::endl;
        debug5 << "focus: "     << view.focus[0]          << ", " << view.focus[1]      << ", " << view.focus[2] << std::endl;
        debug5 << "viewUp: "    << view.viewUp[0]         << ", " << view.viewUp[1]     << ", " << view.viewUp[2] << std::endl;
        debug5 << "viewAngle: "  << view.viewAngle  << std::endl;
        debug5 << "eyeAngle: "  << view.eyeAngle  << std::endl;
        debug5 << "parallelScale: "  << view.parallelScale  << std::endl;
        debug5 << "setScale: "  << view.setScale  << std::endl;
        debug5 << "nearPlane: " << view.nearPlane   << std::endl;
        debug5 << "farPlane: " << view.farPlane     << std::endl;
        debug5 << "imagePan[0]: " << view.imagePan[0]     << std::endl;     // this is a freaking fraction!!!
        debug5 << "imagePan[1]: " << view.imagePan[1]     << std::endl;
        debug5 << "imageZoom: " << view.imageZoom     << std::endl;
        debug5 << "orthographic: " << view.orthographic     << std::endl;
        debug5 << "shear[0]: " << view.shear[0]     << std::endl;
        debug5 << "shear[1]: " << view.shear[1]     << std::endl;
        debug5 << "shear[2]: " << view.shear[2]     << std::endl;
        debug5 << "oldNearPlane: " << oldNearPlane  << std::endl;
        debug5 << "oldFarPlane: " <<oldFarPlane     << std::endl;
        debug5 << "aspect: " << aspect << std::endl << std::endl;

        double _clip[2];
        _clip[0]=oldNearPlane;  _clip[1]=oldFarPlane;

        panPercentage[0] = view.imagePan[0] * view.imageZoom;
        panPercentage[1] = view.imagePan[1] * view.imageZoom;


        // Scaling
        vtkMatrix4x4 *scaletrans = vtkMatrix4x4::New();
        scaletrans->Identity();
        scaletrans->SetElement(0, 0, scale[0]);
        scaletrans->SetElement(1, 1, scale[1]);
        scaletrans->SetElement(2, 2, scale[2]);

        // Zoom and pan portions
        vtkMatrix4x4 *imageZoomAndPan = vtkMatrix4x4::New();
        imageZoomAndPan->Identity();
        imageZoomAndPan->SetElement(0, 0, view.imageZoom);
        imageZoomAndPan->SetElement(1, 1, view.imageZoom);
        //imageZoomAndPan->SetElement(0, 3, 2*view.imagePan[0]*view.imageZoom);
        //imageZoomAndPan->SetElement(1, 3, 2*view.imagePan[1]*view.imageZoom);

        // View
        vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
        vtkMatrix4x4 *vm = vtkMatrix4x4::New();
        vtkMatrix4x4 *vmInit = sceneCam->GetModelViewTransformMatrix();

        //
        //? why we use zoom&pan * scale * vmInit here ?
        // vmInit->Transpose();
        // imageZoomAndPan->Transpose();
        // vtkMatrix4x4::Multiply4x4(vmInit, scaletrans, tmp);
        // vtkMatrix4x4::Multiply4x4(tmp, imageZoomAndPan, vm);
        // vm->Transpose();
        //
        vtkMatrix4x4::Multiply4x4(vmInit, scaletrans, tmp);
        vtkMatrix4x4::Multiply4x4(imageZoomAndPan, tmp, vm);

        // Projection: http://www.codinglabs.net/article_world_view_projection_matrix.aspx
        vtkMatrix4x4 *p = sceneCam->GetProjectionTransformMatrix(aspect,oldNearPlane, oldFarPlane);

        // The Z buffer that is passed from visit is in clip scape with z limits of -1 and 1
        // (http://www.codinglabs.net/article_world_view_projection_matrix.aspx). However, using VTK, the
        // z limits are withing nearz and farz.
        // (https://fossies.org/dox/VTK-7.0.0/classvtkCamera.html#a77e5d3a6e753ae4068f9a3d91267d0eb)
        // So, the projection matrix from VTK is hijacked here and adjusted to be within -1 and 1 too
        // Same as in avtWorldSpaceToImageSpaceTransform::CalculatePerspectiveTransform
        if (!view.orthographic)
        {
            p = sceneCam->GetProjectionTransformMatrix(aspect,oldNearPlane, oldFarPlane);
            p->SetElement(2, 2, -(oldFarPlane+oldNearPlane)   / (oldFarPlane-oldNearPlane));
            p->SetElement(2, 3, -(2*oldFarPlane*oldNearPlane) / (oldFarPlane-oldNearPlane));
        }
        else
        {
            p = sceneCam->GetProjectionTransformMatrix(aspect,oldNearPlane, oldFarPlane);
            p->SetElement(2, 2, -2.0 / (oldFarPlane-oldNearPlane));
            p->SetElement(2, 3, -(oldFarPlane + oldNearPlane) / (oldFarPlane-oldNearPlane));
        }


        // pan
        vtkMatrix4x4 *pantrans = vtkMatrix4x4::New();
        pantrans->Identity();
        pantrans->SetElement(0, 3, 2*view.imagePan[0]);
        pantrans->SetElement(1, 3, 2*view.imagePan[1]);

        vtkMatrix4x4::Multiply4x4(p,vm,pvm);

        debug5 << "pvm: " << *pvm << std::endl;

        //
        // Cleanup
        scaletrans->Delete();
        imageZoomAndPan->Delete();
        vmInit->Delete();
        tmp->Delete();
        vm->Delete();
        p->Delete();

        //
        // Get the full image extents of the volume
        double depthExtents[2];

        GetSpatialExtents(dbounds);
        project3Dto2D(dbounds, screen[0], screen[1], pvm,  fullImageExtents, depthExtents);

        //debug5 << "Full data extents: " << dbounds[0] << ", " << dbounds[1] << "    " << dbounds[2] << ", " << dbounds[3] << "    " << dbounds[4] << ", " << dbounds[5] << std::endl;
        //debug5 << "fullImageExtents: " << fullImageExtents[0] << ", " << fullImageExtents[1] << "     " << fullImageExtents[2] << ", " << fullImageExtents[3] << std::endl;

        if (parallelOn == false)
            extractor.SetRayCastingSLIVRParallel(true);

        extractor.SetJittering(false);
        extractor.SetLighting(lighting);
        extractor.SetLightDirection(lightDirection);
        extractor.SetMatProperties(materialProperties);
        extractor.SetViewDirection(view_direction);
        extractor.SetTransferFn(transferFn1D);
        extractor.SetClipPlanes(_clip);
        extractor.SetPanPercentages(panPercentage);
        extractor.SetDepthExtents(depthExtents);
        extractor.SetMVPMatrix(pvm);

        //
        // Capture background
        __opaqueImageVTK = opaqueImage->GetImage().GetImageVTK();
        __opaqueImageData = (unsigned char *)__opaqueImageVTK->GetScalarPointer(0, 0, 0);
        __opaqueImageZB  = opaqueImage->GetImage().GetZBuffer();

        //createColorPPM("/home/pascal/Desktop/background", __opaqueImageData, screen[0], screen[1]);
        //writeOutputToFileByLine("/home/pascal/Desktop/debugImages/RCSLV_depth_1_", __opaqueImageZB, screen[0], screen[1]);
        //writeDepthBufferToPPM("/home/pascal/Desktop/depthBuffer", __opaqueImageZB, screen[0], screen[1]);

        extractor.setDepthBuffer(__opaqueImageZB, screen[0]*screen[1]);
        extractor.setRGBBuffer(__opaqueImageData, screen[0], screen[1]);

        int _bufExtents[4] = {0,0,0,0};
        _bufExtents[1] = screen[0]; _bufExtents[3] = screen[1];
        extractor.setBufferExtents(_bufExtents);
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


    int timingVolToImg = 0;
    if (rayCastingSLIVR == true && parallelOn)
        timingVolToImg = visitTimer->StartTimer();

    debug5 << "Raytracing setup done! " << std::endl;

    // Execute raytracer
    avtDataObject_p samples = extractor.GetOutput();

    debug5 << "Raytracing rendering done! " << std::endl;

    //
    // Ray casting: SLIVR ~ After Rendering
    //
    if (rayCastingSLIVR == true)
    {
        debug5 << "Start compositing" << std::endl;

        avtRayCompositer rc(rayfoo);                            // only required to force an update - Need to find a way to get rid of that!!!!
        rc.SetInput(samples);
        avtImage_p image  = rc.GetTypedOutput();
        image->Update(GetGeneralContract());

        //
        // SERIAL : Single Processor
        //
        if (parallelOn == false)
        {
            debug5 << "Serial compositing!" << std::endl;

            //
            // Get the metadata for all patches
            std::vector<imgMetaData> allImgMetaData;          // contains the metadata to composite the image
            int numPatches = extractor.getImgPatchSize();     // get the number of patches

            for (int i=0; i<numPatches; i++)
            {
                imgMetaData temp;
                temp = extractor.getImgMetaPatch(i);
                allImgMetaData.push_back(temp);
            }

            //debug5 << "Number of patches: " << numPatches << std::endl;


            //
            // Sort with the largest z first
            std::sort(allImgMetaData.begin(), allImgMetaData.end(), &sortImgMetaDataByEyeSpaceDepth);


            //
            // Blend images
            //
            int renderedWidth = fullImageExtents[1] - fullImageExtents[0];
            int renderedHeight = fullImageExtents[3] - fullImageExtents[2];
            float *composedData = new float[renderedWidth * renderedHeight * 4]();

            for (int i=0; i<numPatches; i++)
            {
                imgMetaData currentPatch = allImgMetaData[i];

                imgData tempImgData;
                tempImgData.imagePatch = NULL;
                tempImgData.imagePatch = new float[currentPatch.dims[0] * currentPatch.dims[1] * 4];
                extractor.getnDelImgData(currentPatch.patchNumber, tempImgData);

                for (int _y=0; _y<currentPatch.dims[1]; _y++)
                    for (int _x=0; _x<currentPatch.dims[0]; _x++)
                    {
                        int startingX = currentPatch.screen_ll[0];
                        int startingY = currentPatch.screen_ll[1];

                        if ((startingX + _x) > fullImageExtents[1])
                            continue;

                        if ((startingY + _y) > fullImageExtents[3])
                            continue;

                        int subImgIndex = (_y*currentPatch.dims[0] + _x) * 4;                                                           // index in the subimage
                        int bufferIndex = ( (((startingY+_y)-fullImageExtents[2]) * renderedWidth)  +  ((startingX+_x)-fullImageExtents[0]) ) * 4;  // index in the big buffer

                        if (composedData[bufferIndex+3] < 1.0)
                        {
                            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
                            float alpha = (1.0 - tempImgData.imagePatch[subImgIndex+3]);
                            composedData[bufferIndex+0] = imgComm.clamp( (composedData[bufferIndex+0] * alpha) + tempImgData.imagePatch[subImgIndex+0] );
                            composedData[bufferIndex+1] = imgComm.clamp( (composedData[bufferIndex+1] * alpha) + tempImgData.imagePatch[subImgIndex+1] );
                            composedData[bufferIndex+2] = imgComm.clamp( (composedData[bufferIndex+2] * alpha) + tempImgData.imagePatch[subImgIndex+2] );
                            composedData[bufferIndex+3] = imgComm.clamp( (composedData[bufferIndex+3] * alpha) + tempImgData.imagePatch[subImgIndex+3] );
                        }
                    }

                //
                // Clean up data
                if (tempImgData.imagePatch != NULL)
                    delete []tempImgData.imagePatch;
                tempImgData.imagePatch = NULL;
            }
            allImgMetaData.clear();


            debug5 << "Serial compositing done!" << std::endl;

            //
            // Create image for visit to display
            avtImage_p whole_image;
            whole_image = new avtImage(this);

            vtkImageData *img = avtImageRepresentation::NewImage(screen[0], screen[1]);
            whole_image->GetImage() = img;

            unsigned char *imgFinal = NULL;
            imgFinal = new unsigned char[screen[0] * screen[1] * 3];
            imgFinal = whole_image->GetImage().GetRGBBuffer();


            //
            // Blend in with bounding box and other visit plots
            vtkMatrix4x4 *Inversepvm = vtkMatrix4x4::New();
            vtkMatrix4x4::Invert(pvm,Inversepvm);

            int compositedImageWidth = fullImageExtents[1] - fullImageExtents[0];
            int compositedImageHeight = fullImageExtents[3] - fullImageExtents[2];

            // Having to adjust the dataset bounds by a arbitrary magic number here. Needs to be sorted out at some point!
            dbounds[5] = dbounds[5]-0.025;

            //debug5 << "Place in image ~ screen " <<  screen[0] << ", " << screen[1] << "   compositedImageWidth: " << compositedImageWidth << "  compositedImageHeight: " << compositedImageHeight
            //     << "  fullImageExtents: " << fullImageExtents[0] << ", " << fullImageExtents[1] << ", " << fullImageExtents[2] << ", " << fullImageExtents[3] << std::endl;
            //writeArrayToPPM("/home/pascal/Desktop/debugImages/final_", composedData, compositedImageWidth, compositedImageHeight);


            for (int _y=0; _y<screen[1]; _y++)
                for (int _x=0; _x<screen[0]; _x++)
                {

                    int index = _y*screen[0] + _x;
                    int indexComposited = (_y-fullImageExtents[2])*compositedImageWidth + (_x-fullImageExtents[0]);

                    bool insideComposited = false;
                    if (_x >= fullImageExtents[0] && _x < fullImageExtents[1])
                         if (_y >= fullImageExtents[2] && _y < fullImageExtents[3])
                            insideComposited = true;


                    if ( insideComposited )
                    {
                        if (composedData[indexComposited*4 + 3] == 0)
                        {
                            // No data from rendering here!
                            imgFinal[index*3 + 0] = __opaqueImageData[index*3 + 0];
                            imgFinal[index*3 + 1] = __opaqueImageData[index*3 + 1];
                            imgFinal[index*3 + 2] = __opaqueImageData[index*3 + 2];
                        }
                        else
                        {
                            if (__opaqueImageZB[index] != 1)
                            {
                                // Might need to do some blending
                                double worldCoordinates[3];
                                float _tempZ = __opaqueImageZB[index] * 2 - 1;
                                unProject(_x, _y, _tempZ, worldCoordinates, screen[0], screen[1], Inversepvm);

                                //debug5 << "x,y,z: " << _x << ", " << _y << ", " << _tempZ << "   wordld: " << worldCoordinates[0] << ", " << worldCoordinates[1] << ", " << worldCoordinates[2];
                                if ( checkInBounds(dbounds, worldCoordinates) )
                                {
                                    // Completely inside bounding box
                                    float alpha = composedData[indexComposited*4+3];
                                    float oneMinusAlpha = (1.0 - composedData[indexComposited*4+3]);
                                    imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 0] ), 1.0) * 255;
                                    imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 1] ), 1.0) * 255;
                                    imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 2] ), 1.0) * 255;
                                }
                                else
                                {
                                    // Intersect inside with bounding box

                                    double ray[3], tMin, tMax;
                                    computeRay( view.camera, worldCoordinates, ray);
                                    if ( intersect(dbounds, ray, view.camera, tMin, tMax) )
                                    {
                                        double tIntersect = std::min( (worldCoordinates[0]-view.camera[0])/ray[0],
                                                            std::min( (worldCoordinates[1]-view.camera[1])/ray[1], (worldCoordinates[2]-view.camera[2])/ray[2] ) );

                                        if (tMin <= tIntersect)
                                        {
                                            float alpha = composedData[indexComposited*4+3];
                                            float oneMinusAlpha = (1.0 - composedData[indexComposited*4+3]);
                                            imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 0] ), 1.0) * 255;
                                            imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 1] ), 1.0) * 255;
                                            imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 2] ), 1.0) * 255;
                                            // volume infront
                                        }
                                        else
                                        {
                                            // box infront
                                            imgFinal[index*3 + 0] = __opaqueImageData[index*3 + 0];
                                            imgFinal[index*3 + 1] = __opaqueImageData[index*3 + 1];
                                            imgFinal[index*3 + 2] = __opaqueImageData[index*3 + 2];
                                            //debug5 << "  intersection - box infront!" << endl;
                                        }
                                    }
                                    else
                                    {
                                        imgFinal[index*3 + 0] = (  composedData[indexComposited*4 + 0] ) * 255;
                                        imgFinal[index*3 + 1] = (  composedData[indexComposited*4 + 1] ) * 255;
                                        imgFinal[index*3 + 2] = (  composedData[indexComposited*4 + 2] ) * 255;
                                    }
                                }
                            }
                            else
                            {
                                // Inside bounding box but only background - Good
                                float alpha = composedData[indexComposited*4+3];
                                float oneMinusAlpha = (1.0 - composedData[indexComposited*4+3]);
                                imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 0] ), 1.0) * 255;
                                imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 1] ), 1.0) * 255;
                                imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * oneMinusAlpha  +  composedData[indexComposited*4 + 2] ), 1.0) * 255;
                            }
                        }

                    }
                    else
                    {
                        // Outside bounding box: Use the background - Good
                        imgFinal[index*3 + 0] = __opaqueImageData[index*3 + 0];
                        imgFinal[index*3 + 1] = __opaqueImageData[index*3 + 1];
                        imgFinal[index*3 + 2] = __opaqueImageData[index*3 + 2];
                    }

                }

            img->Delete();
            SetOutput(whole_image);

            //
            // Cleanup
            delete []composedData;

            return;
        }

        //
        // Parallel
        //

        debug5 << "Parallel compositing" << std::endl;
        int  timingCompositinig = visitTimer->StartTimer();

        //
        // Get the metadata for all patches
        std::vector<imgMetaData> allImgMetaData;          // contains the metadata to composite the image
        int numPatches = extractor.getImgPatchSize();     // get the number of patches

        int imgExtents[4] = {0,0,0,0}; //minX, maxX,  minY, maxY
        int imgSize[2];                 // x, y
        float *composedData = NULL;
        float *localPatchesDepth = NULL;


        debug5 << "Number of patches: " << numPatches << std::endl;
        for (int i=0; i<numPatches; i++)
        {
            imgMetaData temp;
            temp = extractor.getImgMetaPatch(i);

            imgExtents[0]=temp.screen_ll[0];   // minX
            imgExtents[1]=temp.screen_ur[0];   // maxX

            imgExtents[2]=temp.screen_ll[1];   // minY
            imgExtents[3]=temp.screen_ur[1];   // maxY

            imgSize[0] = imgExtents[1]-imgExtents[0];
            imgSize[1] = imgExtents[3]-imgExtents[2];

            debug5 << "i: " << i << " image (minX, maxX   minY , maxY): " << imgExtents[0] << ", " << imgExtents[1] << "    " << imgExtents[2] << ", " << imgExtents[3] <<
                                "  size: " << imgSize[0] << " x " << imgSize[1] << std::endl;
        }


        //
        // Compositing
        //

        // //
        // // Serial Direct Send
        //imgComm.serialDirectSend(1, localPatchesDepth, imgExtents, composedData, backgroundColor, screen[0], screen[1]);


        //
        // Parallel Direct Send
        int tags[2] = {1081, 1681};
        int tagGather = 2681;
        int numMPIRanks = imgComm.GetNumProcs();
        int *regions =  new int[numMPIRanks]();

        imgComm.regionAllocation(numMPIRanks, regions);
        debug5 << "regionAllocation done!" << std::endl;

        // // 
        // // Parallel Direct Send
        //imgComm.parallelDirectSend(composedData, imgExtents, regions, numMPIRanks, tags, fullImageExtents);


        debug5 << "Starting parallel compositing!" << std::endl;
        int myRegionHeight = imgComm.parallelDirectSendManyPatches(extractor.imgDataHashMap, extractor.imageMetaPatchVector, numPatches, regions, numMPIRanks, tags, fullImageExtents);
        imgComm.gatherImages(regions, numMPIRanks, imgComm.intermediateImage, imgComm.intermediateImageExtents, imgComm.intermediateImageExtents, tagGather, fullImageExtents, myRegionHeight);
        debug5 << imgComm.GetMyId() << " gather done! " << std::endl;


        //
        // Some cleanup

        if (regions != NULL)
            delete []regions;
        regions = NULL;

        if (imgComm.intermediateImage != NULL)
            delete []imgComm.intermediateImage;
        imgComm.intermediateImage = NULL;
        

        imgComm.barrier();
        debug5 << "Global compositing done!" << std::endl;
        

        //
        // Blend with VisIt background at root!
        //

        if (PAR_Rank() == 0)
        {
            //
            // Create image for visit to display
            avtImage_p whole_image;
            whole_image = new avtImage(this);

            vtkImageData *img = avtImageRepresentation::NewImage(screen[0], screen[1]);
            whole_image->GetImage() = img;

            unsigned char *imgFinal = NULL;
            imgFinal = new unsigned char[screen[0] * screen[1] * 3]();
            imgFinal = whole_image->GetImage().GetRGBBuffer();


            //
            // Blend in with bounding box and other visit plots
            vtkMatrix4x4 *Inversepvm = vtkMatrix4x4::New();
            vtkMatrix4x4::Invert(pvm,Inversepvm);

            int compositedImageWidth  = imgComm.finalImageExtents[1] - imgComm.finalImageExtents[0];
            int compositedImageHeight = imgComm.finalImageExtents[3] - imgComm.finalImageExtents[2];

            //debug5 << "Place in image ~ screen " <<  screen[0] << ", " << screen[1] << "   compositedImageWidth: " << compositedImageWidth << "  compositedImageHeight: " << compositedImageHeight
            //     << "  fullImageExtents: " << fullImageExtents[0] << ", " << fullImageExtents[1] << ", " << fullImageExtents[2] << ", " << fullImageExtents[3] << std::endl;
            //writeArrayToPPM("/home/pascal/Desktop/debugImages/final_", imgComm.imgBuffer, compositedImageWidth, compositedImageHeight);

            // Having to adjust the dataset bounds by a arbitrary (magic) number here. Needs to be sorted out at some point!
            //dbounds[5] = dbounds[5]-0.025;

            debug5 << "dbounds: "   << dbounds[0] << ", " << dbounds[1] << "    " << dbounds[2] << ", " << dbounds[3] << "  " << dbounds[4] << ", "<< dbounds[5]  << std::endl;

            dbounds[0] = dbounds[0]+0.00;
            dbounds[1] = dbounds[1]-0.00;

            dbounds[2] = dbounds[2]+0.00;
            dbounds[3] = dbounds[3]-0.00;

            dbounds[4] = dbounds[4]+0.00;
            dbounds[5] = dbounds[5]-0.00;

            


            for (int _y=0; _y<screen[1]; _y++)
                for (int _x=0; _x<screen[0]; _x++)
                {
                    int index = _y*screen[0] + _x;
                    int indexComposited = (_y-imgComm.finalImageExtents[2])*compositedImageWidth + (_x-imgComm.finalImageExtents[0]);

                    bool insideComposited = false;
                    if (_x >= imgComm.finalImageExtents[0] && _x < imgComm.finalImageExtents[1])
                         if (_y >= imgComm.finalImageExtents[2] && _y < imgComm.finalImageExtents[3])
                            insideComposited = true;


                    if ( insideComposited )
                    {
                        if (imgComm.imgBuffer[indexComposited*4 + 3] == 0)
                        {
                            // No data from rendering here! - Good
                            imgFinal[index*3 + 0] = __opaqueImageData[index*3 + 0];
                            imgFinal[index*3 + 1] = __opaqueImageData[index*3 + 1];
                            imgFinal[index*3 + 2] = __opaqueImageData[index*3 + 2];
                        }
                        else
                        {
                            if (__opaqueImageZB[index] != 1)
                            {
                                // Might need to do some blending
                                double worldCoordinates[3];
                                float _tempZ = __opaqueImageZB[index] * 2 - 1;
                                unProject(_x, _y, _tempZ, worldCoordinates, screen[0], screen[1], Inversepvm);

                                //debug5 << "x,y,z: " << _x << ", " << _y << ", " << _tempZ << "   wordld: " << worldCoordinates[0] << ", " << worldCoordinates[1] << ", " << worldCoordinates[2];
                                if ( checkInBounds(dbounds, worldCoordinates) )
                                {
                                    // Completely inside bounding box
                                    float alpha = (1.0 - imgComm.imgBuffer[indexComposited*4+3]);
                                    imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 0] ),1.0) * 255;
                                    imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 1] ),1.0) * 255;
                                    imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 2] ),1.0) * 255;
                                }
                                else
                                {
                                    // Intersect inside with bounding box
                                    double ray[3], tMin, tMax;
                                    computeRay( view.camera, worldCoordinates, ray);
                                    if ( intersect(dbounds, ray, view.camera, tMin, tMax) )
                                    {
                                        double tIntersect = std::min( (worldCoordinates[0]-view.camera[0])/ray[0],
                                                            std::min( (worldCoordinates[1]-view.camera[1])/ray[1], 
                                                                      (worldCoordinates[2]-view.camera[2])/ray[2] ) );

                                        if (tMin <= tIntersect)
                                        {
                                            float alpha = (1.0 - imgComm.imgBuffer[indexComposited*4+3]);
                                            imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 0] ), 1.0) * 255;
                                            imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 1] ), 1.0) * 255;
                                            imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 2] ), 1.0) * 255;
                                            // volume infront
                                        }
                                        else
                                        {
                                            // box infront
                                            imgFinal[index*3 + 0] = __opaqueImageData[index*3 + 0];
                                            imgFinal[index*3 + 1] = __opaqueImageData[index*3 + 1];
                                            imgFinal[index*3 + 2] = __opaqueImageData[index*3 + 2];
                                            //debug5 << "  intersection - box infront!" << endl;
                                        }
                                    }
                                    else
                                    {
                                        float alpha = (1.0 - imgComm.imgBuffer[indexComposited*4+3]);
                                        imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 0] ), 1.0) * 255;
                                        imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 1] ), 1.0) * 255;
                                        imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 2] ), 1.0) * 255;
                                    }
                                }
                            }
                            else
                            {
                                //__opaqueImageZB[index] == 1 - Only data, no background : Good
                                float alpha = (1.0 - imgComm.imgBuffer[indexComposited*4+3]);
                                imgFinal[index*3 + 0] = std::min( ( ((float)__opaqueImageData[index*3 + 0]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 0] ), 1.0) * 255;
                                imgFinal[index*3 + 1] = std::min( ( ((float)__opaqueImageData[index*3 + 1]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 1] ), 1.0) * 255;
                                imgFinal[index*3 + 2] = std::min( ( ((float)__opaqueImageData[index*3 + 2]/255.0) * alpha  +  imgComm.imgBuffer[indexComposited*4 + 2] ), 1.0) * 255;
                            }
                        }

                    }
                    else
                    {
                        // Outside bounding box: Use the background : Good
                        imgFinal[index*3 + 0] = __opaqueImageData[index*3 + 0];
                        imgFinal[index*3 + 1] = __opaqueImageData[index*3 + 1];
                        imgFinal[index*3 + 2] = __opaqueImageData[index*3 + 2];
                    }

                }

            img->Delete();
            SetOutput(whole_image);
        }

        debug5 << "RC SLIVR: Done!" << std::endl;


        //
        // Cleanup
        if (composedData != NULL)
            delete []composedData;

        if (localPatchesDepth != NULL)
            delete []localPatchesDepth;

        pvm->Delete();


        visitTimer->StopTimer(timingCompositinig, "Compositing");
        visitTimer->DumpTimings();

        visitTimer->StopTimer(timingIndex, "Ray Tracing");
        visitTimer->DumpTimings();

        return;
    }


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

            vtkImageData  *_opaqueImageVTK = opaqueImage->GetImage().GetImageVTK();
            unsigned char *_opaqueImageData = (unsigned char *)_opaqueImageVTK->GetScalarPointer(0, 0, 0);

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
avtRayTracer::computeRay(double camera[3], double position[3], double ray[3])
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
avtRayTracer::intersect(double bounds[6], double ray[3], double cameraPos[3], double &tMin, double &tMax)
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
