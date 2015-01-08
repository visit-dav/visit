/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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


using     std::vector;

bool sortImgMetaDataByDepth(imgMetaData const& before, imgMetaData const& after){ return before.avg_z > after.avg_z; }

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
//    Pascal Grosset & Manasa Prasad, Fri Sep 20 2013
//    Add the ray casting SLIVR code
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
    if (trilinearInterpolation || rayCastingSLIVR){
      extractor.SetTrilinear(trilinearInterpolation);
        extractor.SetRayCastingSLIVR(rayCastingSLIVR);
        extractor.SetLighting(lighting);
        extractor.SetLightDirection(lightDirection);
        extractor.SetMatProperties(materialProperties);
        extractor.SetModelViewMatrix(modelViewMatrix);
        extractor.SetTransferFn(transferFn1D);
        extractor.SetViewDirection(view_direction);
        extractor.SetViewUp(view_up);
        extractor.SetTransferFn(transferFn1D);
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
    int  timingVolToImg = 0;
    if (rayCastingSLIVR == true && parallelOn)
        timingVolToImg = visitTimer->StartTimer();

    avtDataObject_p samples = extractor.GetOutput();

    //
    // Ray casting: SLIVR
    //
    if (rayCastingSLIVR == true){
        avtRayCompositer rc(rayfoo);                // only required to force an update - Need to find a way to get rid of that!!!!
        rc.SetInput(samples);
        avtImage_p image  = rc.GetTypedOutput();
        image->Update(GetGeneralContract());

        //
        // Single Processor
        //
        if (parallelOn == false){
            
            //
            // Get the metadata
            //
            std::vector<imgMetaData> allImgMetaData;          // contains the metadata to composite the image
            int numPatches = extractor.getImgPatchSize();     // get the number of patches
        
            for (int i=0; i<numPatches; i++){
                imgMetaData temp;
                temp = extractor.getImgMetaPatch(i);
                allImgMetaData.push_back(temp);
            }
            debug5 << "Number of patches: " << numPatches << std::endl;


            //
            // Sort with the largest z first
            //
            std::sort(allImgMetaData.begin(), allImgMetaData.end(), &sortImgMetaDataByDepth);


            //
            // Composite the images
            //
            
            // Creates a buffer to store the composited image
            float *composedData = new float[screen[0] * screen[1] * 4];

            for (int i=0; i<(screen[0] * screen[1] * 4); i+=4){
                composedData[i+0] = background[0]/255.0;   
                composedData[i+1] = background[1]/255.0; 
                composedData[i+2] = background[2]/255.0; 
                composedData[i+3] = 1.0;
            }

            for (int i=0; i<numPatches; i++){
                imgMetaData currentPatch = allImgMetaData[i];

                imgData tempImgData;
                tempImgData.imagePatch = new float[currentPatch.dims[0] * currentPatch.dims[1] * 4];
                extractor.getnDelImgData(currentPatch.patchNumber, tempImgData);

                for (int j=0; j<currentPatch.dims[1]; j++){
                    for (int k=0; k<currentPatch.dims[0]; k++){

                        int startingX = currentPatch.screen_ll[0];
                        int startingY = currentPatch.screen_ll[1]; 

                        if ((startingX + k) > screen[0])
                            continue;

                        if ((startingY + j) > screen[1])
                            continue;
                        
                        int subImgIndex = currentPatch.dims[0]*j*4 + k*4;                                   // index in the subimage 
                        int bufferIndex = (startingY*screen[0]*4 + j*screen[0]*4) + (startingX*4 + k*4);    // index in the big buffer

                        // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
                        composedData[bufferIndex+0] = imgComm.clamp((composedData[bufferIndex+0] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+0]);
                        composedData[bufferIndex+1] = imgComm.clamp((composedData[bufferIndex+1] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+1]);
                        composedData[bufferIndex+2] = imgComm.clamp((composedData[bufferIndex+2] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+2]);
                        composedData[bufferIndex+3] = imgComm.clamp((composedData[bufferIndex+3] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+3]);
                    }
                }

                if (tempImgData.imagePatch != NULL)
                    delete []tempImgData.imagePatch;
                tempImgData.imagePatch = NULL;
            }
            allImgMetaData.clear();


            // Creates an image structure to hold the image
            avtImage_p whole_image;
            whole_image = new avtImage(this);

            float *zbuffer = new float[screen[0] * screen[1]];
            unsigned char *imgTest = NULL;

            vtkImageData *img = avtImageRepresentation::NewImage(screen[0], screen[1]);
            whole_image->GetImage() = img;

            imgTest = new unsigned char[screen[0] * screen[1] * 3];
            imgTest = whole_image->GetImage().GetRGBBuffer();

            zbuffer = new float[screen[0] * screen[1]]();
            for (int s=0; s<screen[0] * screen[1]; s++)
                zbuffer[s] = 20.0;
            zbuffer = whole_image->GetImage().GetZBuffer();


            // Get the composited image
            for (int i=0; i< screen[1]; i++)
                for (int j=0; j<screen[0]; j++){
                    int bufferIndex = (screen[0]*4*i) + (j*4);
                    int wholeImgIndex = (screen[0]*3*i) + (j*3);

                    imgTest[wholeImgIndex+0] = (composedData[bufferIndex+0] ) * 255;
                    imgTest[wholeImgIndex+1] = (composedData[bufferIndex+1] ) * 255;
                    imgTest[wholeImgIndex+2] = (composedData[bufferIndex+2] ) * 255;
                }

            img->Delete();

            if (zbuffer != NULL)
                delete []zbuffer;

            SetOutput(whole_image);
            return;
        }


        //
        // Parallel
        //

        //
        // -- -- -- Timing -- 

        // imgComm.syncAllProcs();     // only required for time testing purposes

        visitTimer->StopTimer(timingVolToImg, "VolToImg");
        visitTimer->DumpTimings();
        
        int  timingComm = visitTimer->StartTimer();
        int  timingCommMeta = visitTimer->StartTimer();

       
        //
        // Getting the patches & send/receive the number of patches that each has to 0
        //
        int numPatches = extractor.getImgPatchSize();     // get the number of patches
        imgComm.gatherNumPatches(numPatches);

        debug5 << PAR_Rank() << "   avtRayTracer::Execute  - Getting the patches -    numPatches: " << numPatches << "   total assigned: " << extractor.getTotalAssignedPatches() << endl;


        //
        // Send/Receive the patches iota metadata to proc 0
        //

        float *tempSendBuffer = NULL;
        tempSendBuffer = new float[numPatches*7];

        std::multimap<int, imgMetaData> imgMetaDataMultiMap;
        imgMetaDataMultiMap.clear();
        for (int i=0; i<numPatches; i++){
            imgMetaData temp;
            temp = extractor.getImgMetaPatch(i);
            imgMetaDataMultiMap.insert(  std::pair<int, imgMetaData>   (temp.patchNumber, temp));

            tempSendBuffer[i*7 + 0] = temp.procId;
            tempSendBuffer[i*7 + 1] = temp.patchNumber;
            tempSendBuffer[i*7 + 2] = temp.dims[0];
            tempSendBuffer[i*7 + 3] = temp.dims[1];
            tempSendBuffer[i*7 + 4] = temp.screen_ll[0];
            tempSendBuffer[i*7 + 5] = temp.screen_ll[1];
            tempSendBuffer[i*7 + 6] = temp.avg_z;

            debug5 << PAR_Rank() << "   ~ has patch #: " << temp.patchNumber << "   avg_z: " << temp.avg_z << endl;
        }

        imgComm.gatherIotaMetaData(numPatches*7, tempSendBuffer); 

        delete []tempSendBuffer;
        tempSendBuffer = NULL;


        //
        // -- -- Timing --
        visitTimer->StopTimer(timingCommMeta, "Communicating metadata");
        visitTimer->DumpTimings();
        debug5 << PAR_Rank() << "   avtRayTracer::Execute  - Send/Receive the patches iota " << endl;

        int  timingCommDecision = visitTimer->StartTimer();


        //
        // Patch allocation Logic & inform other procs about logic
        // 
        if (PAR_Rank() == 0)
            imgComm.patchAllocationLogic();    

        debug5 << PAR_Rank() << "   avtRayTracer::Execute  - imgComm.patchAllocationLogic() " << endl;

        // informationToRecvArray:   (procId, numPatches)           (procId, numPatches)         (procId, numPatches)  ...
        // informationToSendArray:   (patchNumber, destProcId)     (patchNumber, destProcId)    (patchNumber, destProcId)  ...
        int totalSendData, totalRecvData, numZDivisions, totalPatchesToCompositeLocally;
        int *informationToRecvArray, *informationToSendArray, *patchesToCompositeLocally;
        float *divisionsArray = NULL;
        informationToRecvArray = informationToSendArray = patchesToCompositeLocally = NULL;
        totalSendData = totalRecvData = numZDivisions = totalPatchesToCompositeLocally = 0;

        //
        // Send info about which patch to receive and which patches to send & receive
        imgComm.scatterNumDataToCompose(totalSendData, totalRecvData, numZDivisions, totalPatchesToCompositeLocally);

        debug5 << PAR_Rank() << " ~  num patches to send: " << totalSendData/2 << " num processors to recv from: " << totalRecvData/2 << "    numZDivisions: " << numZDivisions << "   totalPatchesToCompositeLocally: " <<   totalPatchesToCompositeLocally << endl;


        //
        //receive the information about how many patches to receive from other processors
        informationToRecvArray = new int[totalRecvData];
        informationToSendArray = new int[totalSendData];
        divisionsArray = new float[numZDivisions];


        if (totalPatchesToCompositeLocally > 0)
            patchesToCompositeLocally = new int[totalPatchesToCompositeLocally];

        imgComm.scatterDataToCompose(totalSendData, informationToSendArray, totalRecvData, informationToRecvArray, numZDivisions, divisionsArray, totalPatchesToCompositeLocally, patchesToCompositeLocally);
        numZDivisions /= 2;   // halve it as it's the size of the array that contains start n end

        
        //
        // --- Timing -- //
        
        visitTimer->StopTimer(timingCommDecision, "Decision making and informing procs");
        visitTimer->DumpTimings();
        
        int  timingLocalCompositing = visitTimer->StartTimer();
        
        
        //
        // Local compositing if any is required
        // 
        int index = 1;
        int start, end;
        start = end = index;

        std::vector<imgData> compositedDataVec;
        debug5 << PAR_Rank() << " ~  totalPatchesToCompositeLocally: " << totalPatchesToCompositeLocally << endl;

        while (index <= totalPatchesToCompositeLocally){
            if (patchesToCompositeLocally[index] == -1 || index == totalPatchesToCompositeLocally){
                end = index-1;

                imgMetaData composedPatch = imgMetaDataMultiMap.find(patchesToCompositeLocally[start])->second;
                imgMetaData lastPatch = imgMetaDataMultiMap.find(patchesToCompositeLocally[end])->second;

                imgData composedData;
                composedData.patchNumber    = composedPatch.patchNumber;
                composedData.procId         = composedPatch.procId;

                int imgBufferWidth = abs(lastPatch.screen_ll[0] + lastPatch.dims[0] - composedPatch.screen_ll[0]);
                int imgBufferHeight = abs(lastPatch.screen_ll[1] + lastPatch.dims[1] - composedPatch.screen_ll[1]);

                composedData.imagePatch = new float[((imgBufferWidth * imgBufferHeight * 4)) ]();  //size: imgBufferWidth * imgBufferHeight * 4, initialized to 0

                composedPatch.dims[0]   = imgBufferWidth;
                composedPatch.dims[1]   = imgBufferHeight;
                composedPatch.inUse     = false;


                for (int patchIndex=start; patchIndex<=end; patchIndex++){
                    imgMetaData currentPatch = imgMetaDataMultiMap.find(patchesToCompositeLocally[patchIndex])->second;

                    int startingX = currentPatch.screen_ll[0] - composedPatch.screen_ll[0];
                    int startingY = currentPatch.screen_ll[1] - composedPatch.screen_ll[1]; 

                    imgData tempImgData;
                    tempImgData.imagePatch = new float[currentPatch.dims[0] * currentPatch.dims[1] * 4];
                    extractor.getnDelImgData(currentPatch.patchNumber, tempImgData);

                    // Assemble the idivisions into 1 layer
                    for (int j=0; j<currentPatch.dims[1]; j++){
                        for (int k=0; k<currentPatch.dims[0]; k++){

                            if ((startingX + k) > imgBufferWidth)
                                continue;

                            if ((startingY + j) > imgBufferHeight)
                                continue;
                            
                            int subImgIndex = currentPatch.dims[0]*j*4 + k*4;                             // index in the subimage 
                            int bufferIndex = (startingY*imgBufferWidth*4 + j*imgBufferWidth*4) + (startingX*4 + k*4);  // index in the big buffer

                            // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
                            composedData.imagePatch[bufferIndex+0] = (composedData.imagePatch[bufferIndex+0] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+0];
                            composedData.imagePatch[bufferIndex+1] = (composedData.imagePatch[bufferIndex+1] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+1];
                            composedData.imagePatch[bufferIndex+2] = (composedData.imagePatch[bufferIndex+2] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+2];
                            composedData.imagePatch[bufferIndex+3] = (composedData.imagePatch[bufferIndex+3] * (1.0 - tempImgData.imagePatch[subImgIndex+3])) + tempImgData.imagePatch[subImgIndex+3];
                        }
                    }

                    if (tempImgData.imagePatch != NULL)
                        delete []tempImgData.imagePatch;
                    tempImgData.imagePatch = NULL;

                    imgMetaDataMultiMap.erase(patchesToCompositeLocally[patchIndex]);
                }
                start = index+1;

                imgMetaDataMultiMap.insert(std::pair<int,imgMetaData>(composedPatch.patchNumber, composedPatch));
                compositedDataVec.push_back(composedData);
            }
            index++;
        }

        //
        // --- Timing -- //

        visitTimer->StopTimer(timingLocalCompositing, "Local compositing");
        visitTimer->DumpTimings();
        
        int  timingLocalWork = visitTimer->StartTimer();
        

        //
        // Set the destination for each patch
        //

        for (int k = 0; k < totalSendData; k+=2){
            std::multimap<int,imgMetaData>::iterator it = imgMetaDataMultiMap.find(informationToSendArray[k]);
            (it->second).destProcId = informationToSendArray[k+1];
        }

        //
        // storage initialization
        //        
        int remainingPatches = 0;
        std::vector<imgMetaData> allImgMetaData;                        // to contain the metadata to composite
        std::multimap< std::pair<int,int>, imgData> imgDataToCompose;   // to contain the data to composite

        //std::multimap<int, imgMetaData> imgMetaDataMultiMap;          // contained all the patches it produced

        allImgMetaData.clear();
        imgDataToCompose.clear();


        //
        // Copying the patches that it will need
        //
        std::multimap<int,imgMetaData>::iterator it;
        for (it = imgMetaDataMultiMap.begin(); it != imgMetaDataMultiMap.end(); ++it ){

            imgMetaData tempImgMetaData = it->second;

            if (tempImgMetaData.destProcId == tempImgMetaData.procId ){
                imgData tempImgData;
                tempImgData.imagePatch = new float[it->second.dims[0] * it->second.dims[1] * 4];
                extractor.getnDelImgData(tempImgMetaData.patchNumber, tempImgData);

                allImgMetaData.push_back(tempImgMetaData);
                imgDataToCompose.insert( std::pair< std::pair<int,int>, imgData> (  std::pair<int,int>(tempImgMetaData.procId, tempImgMetaData.patchNumber), tempImgData)   );

                remainingPatches++;
            }    
        }


        //
        // --- Timing -- //
        visitTimer->StopTimer(timingLocalWork, "Local compositing patches to send");
        visitTimer->DumpTimings();
        
        int  timingSendReceive = visitTimer->StartTimer();
        

        //
        // Sending and receiving from other patches (does a kind of binary swap - half send, half receive and each list gets subdivided)
        // 
        int startingProc = 0;
        int endingProc = PAR_Size() - 1;

        while (startingProc != endingProc){
            int newStartingProc, newEndingProc, numInOtherHalf;
            int *procsInOtherList = NULL;

            int numProcessors = endingProc - startingProc + 1;
            int half = numProcessors/2;
            int middleProc = startingProc + (half-1);
            int doFirst = SEND;     //1: send 2: receive
            
            if (PAR_Rank() <= middleProc){
                numInOtherHalf = numProcessors - half;
                procsInOtherList = new int[numInOtherHalf];
                
                for (int i=0; i<numInOtherHalf; i++)
                    procsInOtherList[i] = startingProc+half+i;
                
                doFirst = SEND;
                newStartingProc = startingProc;
                newEndingProc = middleProc; 
            }
            else{
                numInOtherHalf = half;
                procsInOtherList = new int[numInOtherHalf];
                
                for (int i=0; i<half; i++)
                    procsInOtherList[i] = startingProc+i;
                
                doFirst = RECEIVE;
                newStartingProc = middleProc+1;
                newEndingProc = endingProc;
            }

            if (doFirst == SEND){
                //
                // Send
                std::set<int> senderListSet;     senderListSet.clear();

                for (int i=0; i<numInOtherHalf; i++)
                    for(int j = 0; j < totalSendData; j+=2) 
                        if (informationToSendArray[j+1] == procsInOtherList[i])
                            senderListSet.insert(informationToSendArray[j+1]);
                    
                std::multimap<int,imgMetaData>::iterator it;
                for (it = imgMetaDataMultiMap.begin(); it != imgMetaDataMultiMap.end(); ++it ){

                    const bool is_in = ((std::find(senderListSet.begin(), senderListSet.end(), it->second.destProcId)) != (senderListSet.end()));
                    if (is_in){
                        imgMetaData tempImgMetaData = it->second;
                        imgData tempImgData;
                        tempImgData.patchNumber = tempImgMetaData.patchNumber;
                        tempImgData.procId = tempImgMetaData.procId;
                        tempImgData.imagePatch = new float[tempImgMetaData.dims[0] * tempImgMetaData.dims[1] * 4];
                        

                        if(tempImgMetaData.inUse)
                            extractor.getnDelImgData(tempImgMetaData.patchNumber, tempImgData);
                        else{
                            const bool is_inC = (std::find(compositedDataVec.begin(), compositedDataVec.end(), tempImgData)) != compositedDataVec.end();  
                            if(is_inC) 
                                tempImgData = *(std::find(compositedDataVec.begin(), compositedDataVec.end(), tempImgData));
                            else 
                                debug5 << PAR_Rank() << " Ray casting: SLIVR uuuuuuuh it didn't find the patch" << endl;
                        }

                        imgComm.sendPointToPoint(tempImgMetaData,tempImgData, numProcessors);

                        if (tempImgData.imagePatch != NULL)
                            delete []tempImgData.imagePatch;
                        tempImgData.imagePatch = NULL;
                    }
                }

                senderListSet.clear();

                //
                // Receive

                //
                // counting how many to receive
                int numToReceive = 0;
                for (int i=0; i<totalRecvData/2; i++)
                    for (int j=0; j<numInOtherHalf; j++)
                        if ((informationToRecvArray[i*2] == procsInOtherList[j]) && (informationToRecvArray[i*2 + 1] > 0))
                            numToReceive+=informationToRecvArray[i*2 + 1];

                
                // does the receive
                for (int i=0; i<numToReceive; i++){
                    imgData tempImgData;
                    imgMetaData tempImgMetaData;

                    imgComm.recvPointToPointMetaData(tempImgMetaData, numProcessors);

                    tempImgData.procId = tempImgMetaData.procId;
                    tempImgData.patchNumber = tempImgMetaData.patchNumber;
                    tempImgData.imagePatch = new float[tempImgMetaData.dims[0]*tempImgMetaData.dims[1] * 4];
                    imgComm.recvPointToPointImgData(tempImgMetaData, tempImgData, numProcessors);

                    allImgMetaData.push_back(tempImgMetaData);
                    imgDataToCompose.insert( std::pair< std::pair<int,int>, imgData> (std::pair<int,int>(tempImgMetaData.procId, tempImgMetaData.patchNumber), tempImgData));
                }
            }else{
                //
                // Receive
        
                // counting how many to receive
                int numToReceive = 0;
                for (int i=0; i<totalRecvData/2; i++)
                  for (int j=0; j<numInOtherHalf; j++)
                        if ((informationToRecvArray[i*2] == procsInOtherList[j]) && (informationToRecvArray[i*2 + 1] > 0))
                            numToReceive+=informationToRecvArray[i*2 + 1];

                // does the receive
                for (int i=0; i<numToReceive; i++){
                    imgData tempImgData;
                    imgMetaData tempImgMetaData;

                    imgComm.recvPointToPointMetaData(tempImgMetaData, numProcessors);

                    tempImgData.procId = tempImgMetaData.procId;
                    tempImgData.patchNumber = tempImgMetaData.patchNumber;
                    tempImgData.imagePatch = new float[tempImgMetaData.dims[0]*tempImgMetaData.dims[1] * 4];
                    imgComm.recvPointToPointImgData(tempImgMetaData, tempImgData, numProcessors);

                    allImgMetaData.push_back(tempImgMetaData);
                    imgDataToCompose.insert( std::pair< std::pair<int,int>, imgData> (std::pair<int,int>(tempImgMetaData.procId, tempImgMetaData.patchNumber), tempImgData));
                }


                //
                // Send
                std::set<int> senderListSet;    senderListSet.clear();

                for (int i=0; i<numInOtherHalf; i++)
                    for(int j = 0; j < totalSendData; j+=2) 
                        if (informationToSendArray[j+1] == procsInOtherList[i])
                            senderListSet.insert(informationToSendArray[j+1]);
                    
                
                std::multimap<int,imgMetaData>::iterator it;
                for (it = imgMetaDataMultiMap.begin(); it != imgMetaDataMultiMap.end(); ++it ){

                    const bool is_in = ((std::find(senderListSet.begin(), senderListSet.end(), it->second.destProcId)) != (senderListSet.end()));
                    if (is_in){
                        imgMetaData tempImgMetaData = it->second;
                        imgData tempImgData;
                        tempImgData.patchNumber = tempImgMetaData.patchNumber;
                        tempImgData.procId = tempImgMetaData.procId;
                        tempImgData.imagePatch = new float[it->second.dims[0] * it->second.dims[1] * 4];

                        if(tempImgMetaData.inUse)
                            extractor.getnDelImgData(tempImgMetaData.patchNumber, tempImgData);
                        else{
                            const bool is_inC = (std::find(compositedDataVec.begin(), compositedDataVec.end(), tempImgData)) != compositedDataVec.end();  
                            if(is_inC) 
                                tempImgData = *(std::find(compositedDataVec.begin(), compositedDataVec.end(), tempImgData));
                            else 
                                debug5 << PAR_Rank() << "Ray casting: SLIVR uuuuuuuh it didn't find the patch" << endl;
                        }
                        

                        imgComm.sendPointToPoint(tempImgMetaData,tempImgData, numProcessors);

                        if (tempImgData.imagePatch != NULL)
                            delete []tempImgData.imagePatch;
                        tempImgData.imagePatch = NULL;
                    }
                }

                senderListSet.clear();
            }
  
            if (procsInOtherList != NULL)
                delete []procsInOtherList;
            procsInOtherList = NULL;
    
            startingProc = newStartingProc;
            endingProc = newEndingProc;
        }

        if (informationToRecvArray != NULL)
            delete []informationToRecvArray;
        informationToRecvArray = NULL;

        if (informationToSendArray != NULL)
            delete []informationToSendArray;
        informationToSendArray = NULL;



        //
        // --- Timing -- 
        
        visitTimer->StopTimer(timingSendReceive, "Send Receive");
        visitTimer->DumpTimings();
        
        int  timingLocalCompositing2 = visitTimer->StartTimer();
        


        //
        // Each proc does local compositing and then sends
        //
        int imgBufferWidth = screen[0];
        int imgBufferHeight = screen[1];
        float *buffer = new float[((imgBufferWidth * imgBufferHeight * 4)) * numZDivisions]();  //size: imgBufferWidth * imgBufferHeight * 4, initialized to 0

        std::sort(allImgMetaData.begin(), allImgMetaData.end(), &sortImgMetaDataByDepth);

        std::multimap< std::pair<int,int>, imgData>::iterator itImgData;
        int bufferDivisionIndex = 0;
        int divIndex = 0;
        int totalSize = allImgMetaData.size();

        debug5 << PAR_Rank() << "   ~   totalSize to compose: " << totalSize << "    numZDivisions: " << numZDivisions << endl;

        for (int k=0; k<numZDivisions; k++){
            debug5 << PAR_Rank() << "   ~   division boundaries: " << divisionsArray[k*2 + 0] << " to " << divisionsArray[k*2 + 1] << endl;
        }

        for (int patchIndex=0; patchIndex<totalSize; patchIndex++){
            if (allImgMetaData[patchIndex].avg_z >= divisionsArray[divIndex*2] && allImgMetaData[patchIndex].avg_z <= divisionsArray[divIndex*2+1]){  //new index
            }else{
                for (int z=0; z<numZDivisions; z++){
                    if (allImgMetaData[patchIndex].avg_z >= divisionsArray[z*2] && allImgMetaData[patchIndex].avg_z <= divisionsArray[z*2+1]) {
                        divIndex = z;
                        break;
                    }  
                }
            }
            bufferDivisionIndex = (imgBufferWidth * imgBufferHeight * 4) * divIndex; 

            int startingX = allImgMetaData[patchIndex].screen_ll[0];
            int startingY = allImgMetaData[patchIndex].screen_ll[1]; 

            debug5 << PAR_Rank() << "   ~   divIndex: " << divIndex << "    composing patch #: " << allImgMetaData[patchIndex].procId << " ,  " << allImgMetaData[patchIndex].patchNumber << "   size: " << allImgMetaData[patchIndex].dims[0] << " x  " << allImgMetaData[patchIndex].dims[1] << "   avg_z: " << allImgMetaData[patchIndex].avg_z << endl;


            itImgData = imgDataToCompose.find( std::pair<int,int>(allImgMetaData[patchIndex].procId, allImgMetaData[patchIndex].patchNumber) );
            if (itImgData == imgDataToCompose.end()){
                debug5 << "Error in local compositing - shouldn't happen!!!  " << allImgMetaData[patchIndex].patchNumber << endl;
                continue;
            }
            
            // Assemble the idivisions into 1 layer
            for (int j=0; j<allImgMetaData[patchIndex].dims[1]; j++){
                for (int k=0; k<allImgMetaData[patchIndex].dims[0]; k++){

                    if ((startingX + k) > imgBufferWidth)
                        continue;

                    if ((startingY + j) > imgBufferHeight)
                        continue;
                    
                    int subImgIndex = allImgMetaData[patchIndex].dims[0]*j*4 + k*4;                             // index in the subimage 
                    int bufferIndex = (startingY*imgBufferWidth*4 + j*imgBufferWidth*4) + (startingX*4 + k*4);  // index in the big buffer

                    // back to Front compositing: composited_i = composited_i-1 * (1.0 - alpha_i) + incoming; alpha = alpha_i-1 * (1- alpha_i)
                    buffer[bufferDivisionIndex + bufferIndex+0] = (buffer[bufferDivisionIndex + bufferIndex+0] * (1.0 - itImgData->second.imagePatch[subImgIndex+3])) + itImgData->second.imagePatch[subImgIndex+0];
                    buffer[bufferDivisionIndex + bufferIndex+1] = (buffer[bufferDivisionIndex + bufferIndex+1] * (1.0 - itImgData->second.imagePatch[subImgIndex+3])) + itImgData->second.imagePatch[subImgIndex+1];
                    buffer[bufferDivisionIndex + bufferIndex+2] = (buffer[bufferDivisionIndex + bufferIndex+2] * (1.0 - itImgData->second.imagePatch[subImgIndex+3])) + itImgData->second.imagePatch[subImgIndex+2];
                    buffer[bufferDivisionIndex + bufferIndex+3] = (buffer[bufferDivisionIndex + bufferIndex+3] * (1.0 - itImgData->second.imagePatch[subImgIndex+3])) + itImgData->second.imagePatch[subImgIndex+3];
                }
            }

            if (itImgData->second.imagePatch != NULL)
                delete []itImgData->second.imagePatch;
            itImgData->second.imagePatch = NULL;
        }

        allImgMetaData.clear();
        imgDataToCompose.clear();

        debug5 << PAR_Rank() << "   ~ composing patch done: " << endl;


        //
        // --- Timing -- 
        visitTimer->StopTimer(timingLocalCompositing2, "Local Comnpositing of received patches");
        visitTimer->DumpTimings();
        
        int  timingRLE = visitTimer->StartTimer();
        


        //
        // RLE Encoding
        //

        float *encoding = NULL;
        int *sizeEncoding = NULL;

        int totalEncodingSize = imgComm.rleEncodeAll(imgBufferWidth,imgBufferHeight, numZDivisions,buffer,  encoding,sizeEncoding);

        if (buffer != NULL)
            delete []buffer;
        buffer = NULL;
        
        debug5 << PAR_Rank() << "   ~ encoding done!  "<< endl;

        //
        // --- Timing -- 
        visitTimer->StopTimer(timingRLE, "RLE ");
        visitTimer->DumpTimings();
        
        int  finalSend = visitTimer->StartTimer();
        

        //
        // Proc 0 recieves and does the final assmebly
        //
        if (PAR_Rank() == 0)
            imgComm.setBackground(background);

        // Gather all the images
        imgComm.gatherEncodingSizes(sizeEncoding, numZDivisions);                                                       // size of images
        imgComm.gatherAndAssembleEncodedImages(screen[0], screen[1], totalEncodingSize*5, encoding, numZDivisions);     // data from each processor

        debug5 << PAR_Rank() << "   ~ gatherEncodingSizes " << endl;


        if (encoding != NULL)
            delete []encoding;
        encoding = NULL;

        if (sizeEncoding != NULL)
            delete []sizeEncoding;
        sizeEncoding = NULL;

        if (divisionsArray != NULL)
            delete []divisionsArray;
        divisionsArray = NULL;
       

        //
        // --- Timing -- 
        visitTimer->StopTimer(finalSend, "Final send ");
        visitTimer->DumpTimings();
        
        visitTimer->StopTimer(timingComm, "Communicating");
        visitTimer->DumpTimings();

        int  timingCompositinig = visitTimer->StartTimer();


        //
        // Compositing
        //

        // create images structures to hold these
        avtImage_p whole_image, tempImage;
    
        tempImage = new avtImage(this);     // for processors other than proc 0 ; a dummy
        
        // Processor 0 does a special compositing
        if (PAR_Rank() == 0)
        {
            whole_image = new avtImage(this);

            float *zbuffer = new float[screen[0] * screen[1]];
            unsigned char *imgTest = NULL;

            // creates input for the
            vtkImageData *img = avtImageRepresentation::NewImage(screen[0], screen[1]);
            whole_image->GetImage() = img;


            imgTest = new unsigned char[screen[0] * screen[1] * 3];
            imgTest = whole_image->GetImage().GetRGBBuffer();

            zbuffer = new float[screen[0] * screen[1]]();
            for (int s=0; s<screen[0] * screen[1]; s++)
                zbuffer[s] = 20.0;
            zbuffer = whole_image->GetImage().GetZBuffer();

            
            // Get the composited image
            imgComm.getcompositedImage(screen[0], screen[1], imgTest); 
            img->Delete();

            debug5 << PAR_Rank() << "   ~ final: " << endl;

            if (zbuffer != NULL)
                delete []zbuffer;
        }
        imgComm.syncAllProcs();

        if (PAR_Rank() == 0)
            tempImage->Copy(*whole_image);
        SetOutput(tempImage);

        visitTimer->StopTimer(timingCompositinig, "Compositing");
        visitTimer->DumpTimings();

        visitTimer->StopTimer(timingIndex, "Ray Tracing");
        visitTimer->DumpTimings();

        extractor.delImgPatches();

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


