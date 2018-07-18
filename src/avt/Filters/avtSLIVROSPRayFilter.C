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

#include "avtSLIVROSPRayFilter.h"

#include <avtMemory.h>
#include <avtParallel.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

#ifdef __unix__
# include <unistd.h>
#endif

// helper
namespace slivr {
    // output stream
    std::ostream *osp_out = &DebugStream::Stream5();
    std::ostream *osp_err = &DebugStream::Stream1();
};

double slivr::deg2rad (double degrees) {
    return degrees * 4.0 * atan (1.0) / 180.0;
}
double slivr::rad2deg (double radins) {
    return radins / 4.0 / atan (1.0) * 180.0;
}

#ifdef VISIT_OSPRAY
// other function
void 
OSPVisItVolume::Set(int type, void *ptr, double *X, double *Y, double *Z, 
                    int nX, int nY, int nZ,
                    double volumePBox[6], 
                    double volumeBBox[6], 
                    double mtl[4], float sr, bool shading)
{
    /* OSPRay Volume */
    specularKs    = (float)mtl[2];
    specularNs    = (float)mtl[3];
    enableShading = shading;
    samplingRate  = sr;
    // TODO: It seems if a volume is recovered from a session
    // ospray will crash during zooming ...
    // So we refresh volume everytime to fix the bug
    // which means we need to disable grid accelerator
    // to speed things up. Until I found the reason of crashing
    if (ptr != dataPtr) {
        ospout << "[ospray] update data" << std::endl;
    };
    if (true/*!finished*/) {
        // Because we initialized the volume each frame
        // we need to removed the old volume from model first
        volumeType = OSP_INVALID;
        InitVolume();
        SetVolume(type, ptr, X, Y, Z, nX, nY, nZ,
                  volumePBox, volumeBBox);
    }
    /* OSPRay Model */
    if (true/*!finished*/) {
        worldType = OSP_INVALID; 
        InitWorld();
        SetWorld();
    }
    /* update volume */
    finished = true;
}

// ospModel component
void OSPVisItVolume::InitWorld() {
    if (worldType == OSP_INVALID) {
        CleanWorld();
        worldType = OSP_VALID;
        world = ospNewModel();
    }
}
void OSPVisItVolume::SetWorld() {
    if (world != NULL) { 
        ospAddVolume(world, volume);
        ospCommit(world);
    }
}

// ospVolume component
void OSPVisItVolume::InitVolume(unsigned char type) {
    if (volumeType != type) { // only initialize once
        CleanVolume();
        volumeType = type;
        switch (type) {
        case (OSP_BLOCK_BRICKED_VOLUME):
            volume = ospNewVolume("block_bricked_volume"); 
            break;
        case (OSP_SHARED_STRUCTURED_VOLUME):
            volume = ospNewVolume("visit_shared_structured_volume"); 
            break;
        default:
            debug1 << "ERROR: ospray volume not initialized"
                   << std::endl;
            volumeType = OSP_INVALID;
            EXCEPTION1(VisItException, 
                       "ERROR: ospray volume not initialized");
        }
    }
}
void 
OSPVisItVolume::SetVolume(int type, void *ptr, 
                          double *X, double *Y, double *Z, 
                          int nX, int nY, int nZ,
                          double volumePBox[6], double volumeBBox[6]) 
{
    // calculate volume data type
    if (type == VTK_UNSIGNED_CHAR) {
        dataType = "uchar";
        voxelDataType = OSP_UCHAR;
    } else if (type == VTK_SHORT) {
        dataType = "short";
        voxelDataType = OSP_SHORT;
    } else if (type == VTK_UNSIGNED_SHORT) {
        dataType = "ushort";
        voxelDataType = OSP_USHORT;
    } else if (type == VTK_FLOAT) {
        dataType = "float";
        voxelDataType = OSP_FLOAT;
    } else if (type == VTK_DOUBLE) {
        dataType = "double";
        voxelDataType = OSP_DOUBLE;
    } else {
        debug1 << "ERROR: Unsupported ospray volume type" << std::endl;
        EXCEPTION1(VisItException, "ERROR: Unsupported ospray volume type");
    }
    ospout << "[ospray] data type " << dataType << std::endl;
    // assign data pointer
    dataPtr = ptr;
    // assign structure
    regionStart.x   = volumePBox[0];
    regionStart.y   = volumePBox[1];
    regionStart.z   = volumePBox[2];
    regionStop.x    = volumePBox[3];
    regionStop.y    = volumePBox[4];
    regionStop.z    = volumePBox[5];
    regionSize.x    = nX;
    regionSize.y    = nY;
    regionSize.z    = nZ;
    regionSpacing.x = (regionStop.x-regionStart.x)/((float)regionSize.x-1.0f);
    regionSpacing.y = (regionStop.y-regionStart.y)/((float)regionSize.y-1.0f);
    regionSpacing.z = (regionStop.z-regionStart.z)/((float)regionSize.z-1.0f);
    regionLowerClip.x = volumeBBox[0];
    regionLowerClip.y = volumeBBox[1];
    regionLowerClip.z = volumeBBox[2];
    regionUpperClip.x = volumeBBox[3];
    regionUpperClip.y = volumeBBox[4];
    regionUpperClip.z = volumeBBox[5];

    // other objects
    ospSetString(volume, "voxelType", dataType.c_str());
    ospSetObject(volume, "transferFunction", parent->transferfcn.transferfcn);

    // commit voxel data
    if (voxelData != NULL) { 
        debug1 << "ERROR: Found VoxelData to be non-empty "
               << "while creating new volume" << std::endl;
        EXCEPTION1(VisItException, 
                   "ERROR: Found VoxelData to be non-empty "
                   "while creating new volume");
    }
    voxelSize = nX * nY * nZ;
    voxelData = ospNewData(voxelSize, voxelDataType,
                           dataPtr, OSP_DATA_SHARED_BUFFER);
    ospSetData(volume, "voxelData", voxelData);

    // commit volume
    // -- no lighting by default
    ospout << "[ospray] setting specular value to " << specularKs << std::endl;
    osp::vec3f Ks; Ks.x = Ks.y = Ks.z = specularKs;
    ospSetVec3f(volume, "specular", Ks);
    ospSet1f(volume, "Ns", specularNs);
    ospSet1i(volume, "gradientShadingEnabled", (int)enableShading);
    // -- other properties
    osp::vec3f scaledBBoxLower;
    osp::vec3f scaledBBoxUpper;
    osp::vec3f scaledSpacing;
    osp::vec3f scaledOrigin;
    osp::vec3f scaledGlobalBBoxLower;
    osp::vec3f scaledGlobalBBoxUpper;
    scaledGlobalBBoxLower.x = parent->bounds[0] * regionScaling.x;
    scaledGlobalBBoxUpper.x = parent->bounds[1] * regionScaling.x;
    scaledGlobalBBoxLower.y = parent->bounds[2] * regionScaling.y;
    scaledGlobalBBoxUpper.y = parent->bounds[3] * regionScaling.y;
    scaledGlobalBBoxLower.z = parent->bounds[4] * regionScaling.z;
    scaledGlobalBBoxUpper.z = parent->bounds[5] * regionScaling.z;
    // -- x
    scaledBBoxLower.x = regionLowerClip.x * parent->regionScaling.x;
    scaledBBoxUpper.x = regionUpperClip.x * parent->regionScaling.x;
    scaledSpacing.x   = regionSpacing.x   * parent->regionScaling.x;
    scaledOrigin.x    = regionStart.x     * parent->regionScaling.x;
    // -- y
    scaledBBoxLower.y = regionLowerClip.y * parent->regionScaling.y;
    scaledBBoxUpper.y = regionUpperClip.y * parent->regionScaling.y;
    scaledSpacing.y   = regionSpacing.y   * parent->regionScaling.y;
    scaledOrigin.y    = regionStart.y     * parent->regionScaling.y;
    // -- z
    scaledBBoxLower.z = regionLowerClip.z * parent->regionScaling.z;
    scaledBBoxUpper.z = regionUpperClip.z * parent->regionScaling.z;
    scaledSpacing.z   = regionSpacing.z   * parent->regionScaling.z;
    scaledOrigin.z    = regionStart.z     * parent->regionScaling.z;
    // -- commit ospray
    ospSet1i(volume, "useGridAccelerator", 0);
    ospSetVec3f(volume, "volumeClippingBoxLower", scaledBBoxLower);
    ospSetVec3f(volume, "volumeClippingBoxUpper", scaledBBoxUpper);
    ospSetVec3f(volume, "gridSpacing", scaledSpacing);
    ospSetVec3f(volume, "gridOrigin",  scaledOrigin);
    ospSetVec3i(volume, "dimensions",  regionSize);
    ospSet1f(volume, "samplingRate", samplingRate);
    ospSet1i(volume, "adaptiveSampling", 0);
    ospSet1i(volume, "preIntegration", 1);
    ospSet1i(volume, "singleShade", 0);
    ospSetVec3f(volume, "volumeGlobalBoundingBoxLower", scaledGlobalBBoxLower);
    ospSetVec3f(volume, "volumeGlobalBoundingBoxUpper", scaledGlobalBBoxUpper);
    ospCommit(volume);
}

// ospFrameBuffer component     
void OSPVisItVolume::InitFB(unsigned int width, unsigned int height)
{
    // preparation
    osp::vec2i imageSize;
    imageSize.x = width;
    imageSize.y = height;
    // create max depth texture
    std::vector<float> maxDepth(width * height);
    //
    // The reason I use round(r * (N-1)) instead of floor(r * N) is that 
    // during the composition phase, there will be a wired offset between
    // rendered image and the background, which is about one pixel in size.
    // Using round(r * (N - 1)) can remove the problem
    //
    // const int Xs = 
    //  floor(parent->camera.imgS.x * parent->camera.size[0]);
    // const int Ys = 
    //  floor(parent->camera.imgS.y * parent->camera.size[1]);
    // const int Xs = 
    //  round(parent->camera.imgS.x * (parent->camera.size[0]-1));
    // const int Ys = 
    //  round(parent->camera.imgS.y * (parent->camera.size[1]-1));
    //
    // It seems this is the correct way of doing it
    //
    // It seems we need to also fix pan and zoom also
    //
    const int Xs = 
        std::min((int)round((parent->camera.r_xl + parent->camera.panx) * 
                            parent->camera.size[0]),
                 parent->camera.size[0]-1);
    const int Ys =
        std::min((int)round((parent->camera.r_yl + parent->camera.pany) * 
                            parent->camera.size[1]),
                 parent->camera.size[1]-1);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            maxDepth[i + j * width] = 
                parent->renderer.maxDepthBuffer
                [Xs + i + (Ys + j) * parent->renderer.maxDepthSize.x];
        }
    }
    framebufferBg = ospNewTexture2D(imageSize, OSP_TEXTURE_R32F, 
                                    maxDepth.data(),
                                    OSP_TEXTURE_FILTER_NEAREST);
    ospCommit(framebufferBg);
    ospSetObject(parent->renderer.renderer, "maxDepthTexture", framebufferBg);
    ospCommit(parent->renderer.renderer);
    ospRelease(framebufferBg);
    framebufferBg = NULL;
    // create framebuffer
    CleanFB();
    framebuffer = ospNewFrameBuffer(imageSize, 
                                    OSP_FB_RGBA32F,
                                    OSP_FB_COLOR);
}
void OSPVisItVolume::RenderFB() {
    ospRenderFrame(framebuffer, parent->renderer.renderer, OSP_FB_COLOR);
    framebufferData = (float*) ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
}
float* OSPVisItVolume::GetFBData() {
    return framebufferData;
}

// ****************************************************************************
//
// OSPLight
//
// ****************************************************************************
void OSPVisItLight::Init(const OSPRenderer& renderer)
{
    /* TODO check repeated initialization ? */
    Clean();
    aLight = ospNewLight(renderer, "ambient");
    dLight = ospNewLight(renderer, "distant");
    sLight = ospNewLight(renderer, "distant");
    ospCommit(aLight);
    ospCommit(dLight);
    ospCommit(sLight);
    OSPLight lights[3] = { aLight, dLight, sLight };
    lightdata = ospNewData(3, OSP_OBJECT, lights);
    ospCommit(lightdata);
}
void OSPVisItLight::Set(double materialProperties[4], double viewDirection[3])
{
    // light direction
    osp::vec3f lightDir;
    lightDir.x = (float)viewDirection[0];
    lightDir.y = (float)viewDirection[1];
    lightDir.z = (float)viewDirection[2];
    // ambient light
    ospSet1f(aLight, "intensity", (float)materialProperties[0]);
    ospSet1i(aLight, "isVisible", 0);
    ospCommit(aLight);
    // directional light
    ospSet1f(dLight, "intensity", (float)materialProperties[1]);
    ospSet1f(dLight, "angularDiameter", 0.53f);
    ospSet1i(dLight, "isVisible", 0);
    ospSetVec3f(dLight, "direction", lightDir);
    ospCommit(dLight);
    // sun light
    ospSet1f(sLight, "intensity", 1.5f /*TODO hard code it for now*/);
    ospSet1f(sLight, "angularDiameter", 0.53f);
    ospSet1i(sLight, "isVisible", 0);
    ospSetVec3f(sLight, "direction", lightDir);
    ospCommit(sLight);
}

// ****************************************************************************
//
// OSPRenderer
//
// ****************************************************************************
void OSPVisItRenderer::Init() 
{
    if (rendererType == INVALID) {
        Clean();
        rendererType = SCIVIS;
        renderer = ospNewRenderer("scivis");
        lights.Init(renderer);
    }
}
void OSPVisItRenderer::Set(double materialProperties[4],
                           double viewDirection[3],
                           bool flagUseShading) 
{    
    ospSet1f(renderer, "bgColor",   0.f);
    ospSet1i(renderer, "aoSamples", aoSamples);
    ospSet1i(renderer, "spp",       spp);
    ospSet1i(renderer, "oneSidedLighting",      flagOneSidedLighting);
    ospSet1i(renderer, "shadowsEnabled",        flagShadowsEnabled);
    ospSet1i(renderer, "aoTransparencyEnabled", flagAoTransparencyEnabled);
    if (flagUseShading)
    {
        ospout << "[ospray] use lighting w/ material " 
            << "ambient "    << materialProperties[0] << " "
            << "diffuse "    << materialProperties[1] << " "
            << "specular "   << materialProperties[2] << " "
            << "glossiness " << materialProperties[3] << std::endl;
        lights.Set(materialProperties, viewDirection);
        ospSetData(renderer, "lights", lights.lightdata);
        ospCommit(renderer);    
    }
    ospCommit(renderer);
}
void OSPVisItRenderer::SetCamera(const OSPCamera& camera)
{
    ospSetObject(renderer, "camera", camera);
    ospCommit(renderer);
}
void OSPVisItRenderer::SetModel(const OSPModel& world)
{
    ospSetObject(renderer, "model",  world);
    ospCommit(renderer);
}

// ****************************************************************************
//
// OSPCamera
//
// ****************************************************************************
void OSPVisItCamera::Init(State type) 
{
    if (cameraType != type) {
        Clean();
        cameraType = type;
        switch (cameraType) {
        case (PERSPECTIVE):
            camera = ospNewCamera("perspective");
            break;
        case (ORTHOGRAPHIC):
            camera = ospNewCamera("orthographic");
            break;
        default:
            cameraType = INVALID;
            EXCEPTION1(VisItException, "ERROR: wrong ospray camera type"); 
        }
    }
}
void OSPVisItCamera::Set(const double camp[3], 
                         const double camf[3], 
                         const double camu[3], 
                         const double camd[3],
                         const double sceneSize[2],
                         const double aspect, 
                         const double fovy, 
                         const double zoom_ratio, 
                         const double pan_ratio[2],
                         const int bufferExtents[4],
                         const int screenExtents[2]) 
{
    osp::vec3f camP, camD, camU;
    camP.x = camp[0]; camP.y = camp[1]; camP.z = camp[2];    
    camD.x = camd[0]; camD.y = camd[1]; camD.z = camd[2];
    camU.x = camu[0]; camU.y = camu[1]; camU.z = camu[2];
    panx = pan_ratio[0] * zoom_ratio;
    pany = pan_ratio[1] * zoom_ratio;
    size[0] = screenExtents[0];
    size[1] = screenExtents[1];
    zoom = zoom_ratio;
    ospSetVec3f(camera, "pos", camP);
    ospSetVec3f(camera, "dir", camD);
    ospSetVec3f(camera, "up",  camU);
    ospSet1f(camera, "aspect", aspect);
    if      (cameraType == PERSPECTIVE)  { ospSet1f(camera, "fovy", fovy); }
    else if (cameraType == ORTHOGRAPHIC) { 
        ospSet1f(camera, "height", sceneSize[1]); 
    }
    ospCommit(camera);
    this->SetScreen(bufferExtents[0], bufferExtents[1],
                    bufferExtents[2], bufferExtents[3]);
}
void OSPVisItCamera::SetScreen(float xMin, float xMax, float yMin, float yMax) 
{
    r_xl = xMin/size[0] - panx; 
    r_yl = yMin/size[1] - pany; 
    r_xu = xMax/size[0] - panx;
    r_yu = yMax/size[1] - pany; 
    imgS.x = (r_xl - 0.5f) / zoom + 0.5f;
    imgS.y = (r_yl - 0.5f) / zoom + 0.5f;
    imgE.x = (r_xu - 0.5f) / zoom + 0.5f;
    imgE.y = (r_yu - 0.5f) / zoom + 0.5f;
    ospSetVec2f(camera, "imageStart", imgS);
    ospSetVec2f(camera, "imageEnd",   imgE);
    ospCommit(camera);
}

// ****************************************************************************
//
// OSPTransferFunction
//
// ****************************************************************************
void OSPVisItTransferFunction::Init() 
{
    if (transferfcnType == INVALID) {
        Clean();
        transferfcnType = PIECEWISE_LINEAR;
        transferfcn = ospNewTransferFunction("piecewise_linear");
    }
}
void OSPVisItTransferFunction::Set(const OSPVisItColor *table,
                                   const unsigned int size, 
                                   const float datamin, 
                                   const float datamax) 
{
    std::vector<osp::vec3f> colors;
    std::vector<float>      opacities;
    for (int i = 0; i < size; ++i) {
        osp::vec3f color;
        color.x = table[i].R;
        color.y = table[i].G;
        color.z = table[i].B;
        colors.push_back(color);
        opacities.push_back(table[i].A);
    }
    OSPData colorData   = 
        ospNewData(colors.size(), OSP_FLOAT3, colors.data());
    OSPData opacityData = 
        ospNewData(opacities.size(), OSP_FLOAT, opacities.data());
    osp::vec2f range;
    range.x = datamin;
    range.y = datamax;
    ospSetData(transferfcn, "colors",      colorData);
    ospSetData(transferfcn, "opacities",   opacityData);
    ospSetVec2f(transferfcn, "valueRange", range);
    ospCommit(transferfcn);
    ospRelease(colorData);
    ospRelease(opacityData);
}


// ****************************************************************************
//  Struct:  OSPContext
//
//  Purpose:
//
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************

void OSPContext_ErrorFunc(OSPError, const char* msg) { 
    osperr << "#osp: (rank " << PAR_Rank() << ")" 
           << msg; 
}
void OSPContext_StatusFunc(const char* msg) { 
    osperr << "#osp: (rank " << PAR_Rank() << ")" 
           << msg; 
}
void OSPVisItContext::InitOSP(int numThreads) 
{ 
    OSPDevice device = ospGetCurrentDevice();
    if (device == NULL) 
    {
        // check hostname
#ifdef __unix__
        char hname[200];
        gethostname(hname, 200);
        ospout << "[ospray] on host >> " << hname << "<<" << std::endl;;
#endif
        // initialize ospray
        ospout << "[ospray] Initialize OSPRay";
        device = ospNewDevice();        
        // setup debug 
        if (DebugStream::Level5()) {
            ospout << " debug mode";
            ospDeviceSet1i(device, "debug", 0);
        }
        // setup number of threads (this can only be hard-coded)
        if (numThreads > 0) {
            ospout << " numThreads: " << numThreads;
            ospDeviceSet1i(device, "numThreads", numThreads);
        }
        ospout << std::endl;
        ospDeviceSetErrorFunc(device, OSPContext_ErrorFunc);
        ospDeviceSetStatusFunc(device, OSPContext_StatusFunc);
        ospDeviceCommit(device);
        ospSetCurrentDevice(device);
        // load ospray module
        OSPError err = ospLoadModule("visit");
        if (err != OSP_NO_ERROR) {
            osperr << "[Error] can't load visit module" << std::endl;
        }
    }
    initialized = true;
}

// We use this function to minimize interface
void OSPVisItContext::Render(float xMin, float xMax, float yMin, float yMax,
                             int imgWidth, int imgHeight,
                             float*& dest, OSPVisItVolume* volume) 
{
    int timing_SetSubCamera = visitTimer->StartTimer();
    camera.SetScreen(xMin, xMax, yMin, yMax);
    visitTimer->StopTimer(timing_SetSubCamera,
                          "[OSPRay] Calling OSPContext::SetSubCamera");

    int timing_SetModel = visitTimer->StartTimer();
    renderer.SetModel(volume->GetWorld());
    renderer.SetCamera(camera.camera);
    visitTimer->StopTimer(timing_SetModel,
                          "[OSPRay] Calling OSPContext::SetModel");

    int timing_InitFB = visitTimer->StartTimer();
    volume->InitFB(imgWidth, imgHeight);
    visitTimer->StopTimer(timing_InitFB,
                          "[OSPRay] Calling OSPContext::InitFB");

    int timing_RenderFB = visitTimer->StartTimer();
    volume->RenderFB();
    visitTimer->StopTimer(timing_RenderFB,
                          "[OSPRay] Calling OSPContext::RenderFB");

    int timing_stdcopy = visitTimer->StartTimer();
    std::copy(volume->GetFBData(), 
              volume->GetFBData() + (imgWidth * imgHeight) * 4, dest);
    visitTimer->StopTimer(timing_stdcopy, 
                          "[OSPRay] Calling OSPContext::std::copy");
}

void OSPVisItContext::InitPatch(int id) 
{
    if (volumes.size() < id) {
        debug1 << "ERROR: wrong patch index " << id << std::endl;
        EXCEPTION1(VisItException, "ERROR: wrong patch index"); 
        return;
    }
    if (volumes.size() == id) { 
        volumes.push_back(id); 
    }
    volumes[id].parent = this;
}
#endif//VISIT_OSPRAY


// ****************************************************************************
//  Namespace:  slivr
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************

void slivr::CheckMemoryHere(const std::string& message, std::string debugN)
{
    if (debugN.compare("ospout") == 0) {        
        slivr::CheckMemoryHere(message, *osp_out);
    }
    else if (debugN.compare("debug5") == 0) {
        if (DebugStream::Level5()) {
            slivr::CheckMemoryHere(message, DebugStream::Stream5());
        }       
    }
    else if (debugN.compare("debug4") == 0) {
        if (DebugStream::Level4()) {
            slivr::CheckMemoryHere(message, DebugStream::Stream4());
        }       
    }
    else if (debugN.compare("debug3") == 0) {
        if (DebugStream::Level3()) {
            slivr::CheckMemoryHere(message, DebugStream::Stream3());
        }       
    }
    else if (debugN.compare("debug2") == 0) {
        if (DebugStream::Level2()) {
            slivr::CheckMemoryHere(message, DebugStream::Stream2());
        }       
    }
    else if (debugN.compare("debug1") == 0) {
        if (DebugStream::Level1()) {
            slivr::CheckMemoryHere(message, DebugStream::Stream1());
        }       
    }
}

void slivr::CheckMemoryHere(const std::string& message, std::ostream& out)
{
    unsigned long m_size, m_rss;
    avtMemory::GetMemorySize(m_size, m_rss);
    out << message << std::endl << "\t"
        << " Rank " << PAR_Rank()
        << " Memory use begin " << m_size 
        << " rss " << m_rss/(1024*1024) << " (MB)"
        << std::endl;
}
