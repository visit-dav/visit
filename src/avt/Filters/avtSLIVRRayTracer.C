// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtSLIVRRayTracer.C                                //
// ************************************************************************* //

#include <avtSLIVRRayTracer.h>

#include <vector>

#include <visit-config.h>

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

#include <avtDataset.h>
#include <avtImage.h>
#include <avtParallel.h>
#include <avtRayCompositer.h>
#include <avtSLIVRSamplePointExtractor.h>
#include <avtWorldSpaceToImageSpaceTransform.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

using     std::vector;

bool SLIVRsortImgMetaDataByDepth(imgMetaData const& before, imgMetaData const& after){ return before.avg_z > after.avg_z; }
bool SLIVRsortImgMetaDataByEyeSpaceDepth(imgMetaData const& before, imgMetaData const& after){ return before.eye_z > after.eye_z; }

// ****************************************************************************
//  Method: avtSLIVRRayTracer constructor
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

avtSLIVRRayTracer::avtSLIVRRayTracer() : avtRayTracerBase()
{
    panPercentage[0] = 0;
    panPercentage[1] = 0;
    lighting = false;
    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0;
    lightPosition[3] = 1.0;
    materialProperties[0] = 0.4;
    materialProperties[1] = 0.75;
    materialProperties[2] = 0.0;
    materialProperties[3] = 15.0;
}


// ****************************************************************************
//  Method: avtSLIVRRayTracer destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSLIVRRayTracer::~avtSLIVRRayTracer()
{
    ;
}


// ****************************************************************************
//  Method: avtSLIVRRayTracer::Execute
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
//    Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9:
//    view changed to viewInfo.
//
// ****************************************************************************

void
avtSLIVRRayTracer::Execute()
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
    avtSLIVRSamplePointExtractor extractor(screen[0], screen[1], samplesPerRay);
    extractor.SetJittering(true);
    extractor.SetTransferFn(transferFn1D);
    extractor.SetInput(trans.GetOutput());

    //
    // Ray casting: SLIVR ~ Before Rendering
    //
    double dbounds[6];  // Extents of the volume in world coordinates
    vtkMatrix4x4 *pvm = vtkMatrix4x4::New();

    vtkImageData  *__opaqueImageVTK = NULL;
    unsigned char *__opaqueImageData = NULL;
    float         *__opaqueImageZB = NULL;

    int fullImageExtents[4];

    bool parallelOn = (imgComm.GetNumProcs() == 1)?false:true;

    //
    // Setup
    //

    //
    // Camera Settings
    //
    vtkCamera *sceneCam = vtkCamera::New();
    sceneCam->SetPosition(viewInfo.camera[0],viewInfo.camera[1],viewInfo.camera[2]);
    sceneCam->SetFocalPoint(viewInfo.focus[0],viewInfo.focus[1],viewInfo.focus[2]);
    sceneCam->SetViewUp(viewInfo.viewUp[0],viewInfo.viewUp[1],viewInfo.viewUp[2]);
    sceneCam->SetViewAngle(viewInfo.viewAngle);
    sceneCam->SetClippingRange(oldNearPlane, oldFarPlane);
    if (viewInfo.orthographic)
        sceneCam->ParallelProjectionOn();
    else
        sceneCam->ParallelProjectionOff();
    sceneCam->SetParallelScale(viewInfo.parallelScale);

    debug5 << "RT View settings: " << endl;
    debug5 << "camera: "       << viewInfo.camera[0]      << ", " << viewInfo.camera[1]     << ", " << viewInfo.camera[2] << std::endl;
    debug5 << "focus: "     << viewInfo.focus[0]          << ", " << viewInfo.focus[1]      << ", " << viewInfo.focus[2] << std::endl;
    debug5 << "viewUp: "    << viewInfo.viewUp[0]         << ", " << viewInfo.viewUp[1]     << ", " << viewInfo.viewUp[2] << std::endl;
    debug5 << "viewAngle: "  << viewInfo.viewAngle  << std::endl;
    debug5 << "eyeAngle: "  << viewInfo.eyeAngle  << std::endl;
    debug5 << "parallelScale: "  << viewInfo.parallelScale  << std::endl;
    debug5 << "setScale: "  << viewInfo.setScale  << std::endl;
    debug5 << "nearPlane: " << viewInfo.nearPlane   << std::endl;
    debug5 << "farPlane: " << viewInfo.farPlane     << std::endl;
    debug5 << "imagePan[0]: " << viewInfo.imagePan[0]     << std::endl;     // this is a freaking fraction!!!
    debug5 << "imagePan[1]: " << viewInfo.imagePan[1]     << std::endl;
    debug5 << "imageZoom: " << viewInfo.imageZoom     << std::endl;
    debug5 << "orthographic: " << viewInfo.orthographic     << std::endl;
    debug5 << "shear[0]: " << viewInfo.shear[0]     << std::endl;
    debug5 << "shear[1]: " << viewInfo.shear[1]     << std::endl;
    debug5 << "shear[2]: " << viewInfo.shear[2]     << std::endl;
    debug5 << "oldNearPlane: " << oldNearPlane  << std::endl;
    debug5 << "oldFarPlane: " <<oldFarPlane     << std::endl;
    debug5 << "aspect: " << aspect << std::endl << std::endl;

    double _clip[2];
    _clip[0]=oldNearPlane;  _clip[1]=oldFarPlane;

    panPercentage[0] = viewInfo.imagePan[0] * viewInfo.imageZoom;
    panPercentage[1] = viewInfo.imagePan[1] * viewInfo.imageZoom;


    // Scaling
    vtkMatrix4x4 *scaletrans = vtkMatrix4x4::New();
    scaletrans->Identity();
    scaletrans->SetElement(0, 0, scale[0]);
    scaletrans->SetElement(1, 1, scale[1]);
    scaletrans->SetElement(2, 2, scale[2]);

    // Zoom and pan portions
    vtkMatrix4x4 *imageZoomAndPan = vtkMatrix4x4::New();
    imageZoomAndPan->Identity();
    imageZoomAndPan->SetElement(0, 0, viewInfo.imageZoom);
    imageZoomAndPan->SetElement(1, 1, viewInfo.imageZoom);
    //imageZoomAndPan->SetElement(0, 3, 2*viewInfo.imagePan[0]*viewInfo.imageZoom);
    //imageZoomAndPan->SetElement(1, 3, 2*viewInfo.imagePan[1]*viewInfo.imageZoom);

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
    if (!viewInfo.orthographic)
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
    pantrans->SetElement(0, 3, 2*viewInfo.imagePan[0]);
    pantrans->SetElement(1, 3, 2*viewInfo.imagePan[1]);

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

    //
    // For curvilinear and unstructured meshes, it makes sense to convert the
    // cells to image space.  But for rectilinear meshes, it is not the
    // most efficient strategy.  So set some flags here that allow the
    // extractor to do the extraction in world space.
    //
    trans.SetPassThruRectilinearGrids(true);
    extractor.SetRectilinearGridsAreInWorldSpace(true, viewInfo, aspect);

    int timingVolToImg = 0;
    if (parallelOn)
        timingVolToImg = visitTimer->StartTimer();

    debug5 << "Raytracing setup done! " << std::endl;

    // Execute raytracer
    avtDataObject_p samples = extractor.GetOutput();

    debug5 << "Raytracing rendering done! " << std::endl;

    //
    // After Rendering
    //
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
        std::sort(allImgMetaData.begin(), allImgMetaData.end(), &SLIVRsortImgMetaDataByEyeSpaceDepth);


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
                                computeRay( viewInfo.camera, worldCoordinates, ray);
                                if ( intersect(dbounds, ray, viewInfo.camera, tMin, tMax) )
                                {
                                    double tIntersect = std::min( (worldCoordinates[0]-viewInfo.camera[0])/ray[0],
                                                        std::min( (worldCoordinates[1]-viewInfo.camera[1])/ray[1], (worldCoordinates[2]-viewInfo.camera[2])/ray[2] ) );

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
                                computeRay( viewInfo.camera, worldCoordinates, ray);
                                if ( intersect(dbounds, ray, viewInfo.camera, tMin, tMax) )
                                {
                                    double tIntersect = std::min( (worldCoordinates[0]-viewInfo.camera[0])/ray[0],
                                                        std::min( (worldCoordinates[1]-viewInfo.camera[1])/ray[1],
                                                                  (worldCoordinates[2]-viewInfo.camera[2])/ray[2] ) );

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
avtSLIVRRayTracer::blendImages(float *src, int dimsSrc[2], int posSrc[2], float *dst, int dimsDst[2], int posDst[2])
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
//  Method: avtSLIVRRayTracer::unProject
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
avtSLIVRRayTracer::unProject(int _x, int _y, float _z, double _worldCoordinates[3], int _width, int _height, vtkMatrix4x4 *invModelViewProj)
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
    {
        debug5 << "avtMassVoxelExtractor::unProject division by 0 error!" << endl;
    }

    worldCoordinates[0] = worldCoordinates[0]/worldCoordinates[3];
    worldCoordinates[1] = worldCoordinates[1]/worldCoordinates[3];
    worldCoordinates[2] = worldCoordinates[2]/worldCoordinates[3];
    worldCoordinates[3] = worldCoordinates[3]/worldCoordinates[3];

    _worldCoordinates[0] = worldCoordinates[0];
    _worldCoordinates[1] = worldCoordinates[1];
    _worldCoordinates[2] = worldCoordinates[2];
}



// ****************************************************************************
//  Method: avtSLIVRRayTracer::project
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
avtSLIVRRayTracer::project(double _worldCoordinates[3], int pos2D[2], int _width, int _height, vtkMatrix4x4 *modelViewProj)
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
//  Method: avtSLIVRRayTracer::project3Dto2D
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
avtSLIVRRayTracer::project3Dto2D(double _3Dextents[6], int width, int height, vtkMatrix4x4 *modelViewProj, int _2DExtents[4], double depthExtents[2])
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
//  Method: avtSLIVRRayTracer::checkInBounds
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
avtSLIVRRayTracer::checkInBounds(double volBounds[6], double coord[3])
{
    if (coord[0] > volBounds[0] && coord[0] < volBounds[1])
        if (coord[1] > volBounds[2] && coord[1] < volBounds[3])
            if (coord[2] > volBounds[4] && coord[2] < volBounds[5])
                return true;

    return false;
}


// ****************************************************************************
//  Method: avtSLIVRRayTracer::computeRay
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
avtSLIVRRayTracer::computeRay(double camera[3], double position[3], double ray[3])
{
    for (int i=0; i<3; i++)
        ray[i] = position[i] - camera[i];

    double mag = sqrt( ray[0]*ray[0] + ray[1]*ray[1] + ray[2]*ray[2] );

    for (int i=0; i<3; i++)
        ray[i] = ray[i]/mag;
}


// ****************************************************************************
//  Method: avtSLIVRRayTracer::intersect
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
avtSLIVRRayTracer::intersect(double bounds[6], double ray[3], double cameraPos[3], double &tMin, double &tMax)
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
