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

#ifndef AVT_OSPRAY_FILTER_H
#define AVT_OSPRAY_FILTER_H

#include <DebugStream.h>
#include <TimingsManager.h>
#include <ImproperUseException.h>

#include <vtkType.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#ifdef VISIT_OSPRAY
# include "ospray/ospray.h"
# include "ospray/visit/VisItImageComposite.h"
# define OSP_PERSPECTIVE              1
# define OSP_ORTHOGRAPHIC             2
# define OSP_BLOCK_BRICKED_VOLUME     3
# define OSP_SHARED_STRUCTURED_VOLUME 4
# define OSP_INVALID                  5
# define OSP_VALID                    6
#else
#define OSPRAY_ERROR_MSG \
    "OSPRay is currently not built, please recompile VisIt with OSPRay support."
#endif

// ****************************************************************************
// Debug ostreams customized for ospray
// ****************************************************************************
namespace slivr 
{
    extern std::ostream *osp_out;
    extern std::ostream *osp_err;
};

// ****************************************************************************
// Those function has to be inline, otherwise we need to link this library 
// to other components manually
// ****************************************************************************
namespace slivr 
{
    // ************************************************************************
    //
    // Detect verbose from environmental variable
    //
    // ************************************************************************
    inline bool InitVerbose() 
    {
#ifndef VISIT_OSPRAY
        return false;
#else
        // ********************************************************************
        //
        // OSPRay defines following environmental variables
        //
        // OSPRAY_THREADS
        // OSPRAY_SET_AFFINITY
        // OSPRAY_DEBUG
        // OSPRAY_LOG_LEVEL
        // OSPRAY_LOG_OUTPUT
        //
        // We define one more environmental variable here
        //
        // OSPRAY_VERBOSE
        //
        // ********************************************************************
        const char* env_verbose   = std::getenv("OSPRAY_VERBOSE");
        const char* env_debug     = std::getenv("OSPRAY_DEBUG");
        const char* env_log_level = std::getenv("OSPRAY_LOG_LEVEL");    
        bool verbose = false;
        if (env_verbose) {
            if (atoi(env_verbose) > 0) { verbose = true; }
        }
        if (env_debug) {
            if (atoi(env_debug) > 0) { verbose = true; }
        }
        if (env_log_level) {
            if (atoi(env_log_level) > 0) { verbose = true; }
        }
        if (verbose) {
            slivr::osp_out = &std::cout;
            slivr::osp_err = &std::cerr;
            return true;
        } else {
            return false;
        }
#endif
    }

    // ************************************************************************
    //
    // Detect sample per pixel from environmental variable
    //
    // ************************************************************************
    inline int InitOSPRaySpp() {
#ifndef VISIT_OSPRAY
        return 1;
#else
        int spp = 1;
        const char* env_spp = std::getenv("OSPRAY_SPP");
        if (env_spp) {
            if (atoi(env_spp) > 0) { 
                spp = atoi(env_spp); 
            }
        }       
        return spp;
#endif
    }
    inline bool CheckVerbose() // initialize OSPRAY_VERBOSE
    {
        static bool OSPRAY_VERBOSE = slivr::InitVerbose();
        return OSPRAY_VERBOSE;
    }
    inline int CheckOSPRaySpp()
    {
        static int spp = InitOSPRaySpp();
        return spp;
    }
};

// ****************************************************************************
//
// Over-write ostream marcos
//
// ****************************************************************************
#define ospout \
    if (!slivr::CheckVerbose() && !DebugStream::Level5()) ; \
    else (*slivr::osp_out)
#define osperr \
    if (!slivr::CheckVerbose() && !DebugStream::Level1()) ; \
    else (*slivr::osp_err)

// ****************************************************************************
//  Struct:  OSPVisItVolume
//
//  Purpose:
//    
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************
class OSPVisItContext;
class OSPVisItVolume 
{
 public:
    void Set(int type, void *ptr, 
             double *X, double *Y, double *Z, 
             int nX, int nY, int nZ, 
             double volumePBox[6], double volumeBBox[6],
             double mtl[4], float sr, bool shading)
#ifdef VISIT_OSPRAY
        ;
#else
    {};
#endif

#ifdef VISIT_OSPRAY
 private:
    friend class OSPVisItContext;
 private:
    OSPVisItContext *parent;

    // objects owned by the struct
    // -- ospray model ---
    OSPModel            world;
    unsigned char       worldType;
    // --- ospray framebuffer ---
    OSPFrameBuffer      framebuffer;
    float              *framebufferData;
    OSPTexture2D        framebufferBg;
    // --- ospray volume ---
    OSPVolume           volume;
    unsigned char       volumeType;
    // --- ospray data ---
    OSPDataType         voxelDataType;
    OSPData             voxelData;
    size_t              voxelSize;
    void*               dataPtr;
    std::string         dataType;

    // metadata for volume
    int                 patchId;       // volume patch id
    bool                finished;      // check if this volume is initialized
    bool                enableShading;
    bool                enableDVR;     // Distributed Volume Renderer
    float               specularKs;
    float               specularNs;
    float               samplingRate;

    // geometric parameters for volume
    osp::vec3i          regionSize;
    osp::vec3f          regionStart;
    osp::vec3f          regionStop;
    osp::vec3f          regionSpacing;
    osp::vec3f          regionUpperClip;
    osp::vec3f          regionLowerClip;
    osp::vec3f          regionScaling;
    
 public:
    // constructor
    OSPVisItVolume(int id) {
        // objects owned by the struct
        world           = NULL;
        worldType       = OSP_INVALID;
        framebuffer     = NULL;
        framebufferData = NULL;
        framebufferBg   = NULL;
        volume          = NULL;
        volumeType      = OSP_INVALID;
        voxelDataType   = OSP_VOID_PTR;
        voxelData       = NULL;
        voxelSize       = 0;
        dataPtr         = NULL;
        dataType        = "";
        // metadata for volume
        patchId = id;    
        finished      = false; 
        enableShading = false;
        enableDVR     = false;
        specularKs    = 1.0f;
        specularNs    = 15.0f;
        samplingRate  = 3.0f;
        // geometric parameters for volume
        regionSize.x  = regionSize.y  = regionSize.z  = 0;
        regionStart.x = regionStart.y = regionStart.z = 0.0f;
        regionStop.x  = regionStop.y  = regionStop.z  = 0.0f;
        regionSpacing.x   = regionSpacing.y   = regionSpacing.z   = 0.0f;
        regionUpperClip.x = regionUpperClip.y = regionUpperClip.z = 0.0f;
        regionLowerClip.x = regionLowerClip.y = regionLowerClip.z = 0.0f;
        regionScaling.x   = regionScaling.y   = regionScaling.z   = 1.0f;
    }

    // destructor
    ~OSPVisItVolume() { Clean(); }    
    void Clean() {
        CleanFB();
        CleanVolume();  
        CleanWorld();
    }
    
    // other function
    bool GetDVRFlag() { return enableDVR; }
    void SetDVRFlag(bool mode) { enableDVR = mode; }
    bool GetFinishedFlag() { return finished; }
    void SetFinishedFlag(bool f) { finished = f; } 

    // ospModel component
    OSPModel GetWorld() { return world; }
    void InitWorld();
    void SetWorld();
    void CleanWorld() {
        if (world != NULL) {        
            ospRelease(world);
            world = NULL;
        }
        worldType = OSP_INVALID;
    }
        
    // ospVolume component
    void InitVolume(unsigned char type = OSP_SHARED_STRUCTURED_VOLUME); 
    OSPVolume GetVolume() { return volume; }
    void SetVolume(int type, void *ptr,
                   double *X, double *Y, double *Z, 
                   int nX, int nY, int nZ,
                   double volumePBox[6], 
                   double volumeBBox[6]);
    void CleanVolume() {        
        if (volume != NULL) { ospRelease(volume); volume = NULL; }
        if (voxelData != NULL) { 
            ospRelease(voxelData);
            voxelData = NULL; 
        }
        volumeType = OSP_INVALID;
    }

    // framebuffer component     
    void InitFB(unsigned int width, unsigned int height);
    void RenderFB();
    float* GetFBData();
    void CleanFB() {
        if (framebufferData != NULL) { 
            ospUnmapFrameBuffer(framebufferData, framebuffer); 
            framebufferData = NULL;
        }
        if (framebuffer != NULL) { 
            ospFreeFrameBuffer(framebuffer);        
            framebuffer = NULL;
        }
        if (framebufferBg != NULL) {
            ospRelease(framebufferBg);      
            framebufferBg = NULL;
        }
    }
#endif//VISIT_OSPRAY
};


// ****************************************************************************
//  Struct:  OSPVisItLight
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************

#ifdef VISIT_OSPRAY
struct OSPVisItLight
{
    OSPLight aLight;
    OSPLight dLight;
    OSPLight sLight; // constant sun light
    OSPData  lightdata;
    OSPVisItLight() {
        aLight = NULL;
        dLight = NULL;
        sLight = NULL;
        lightdata = NULL;
    }
    ~OSPVisItLight() { Clean(); }
    void Clean() {/* TODO should we delete them? */}
    void Init(const OSPRenderer& renderer);
    void Set(double materialProperties[4], double viewDirection[3]);
};
#endif//VISIT_OSPRAY


// ****************************************************************************
//  Struct:  OSPVisItRenderer
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************

struct OSPVisItRenderer
{
public:
    enum State {
        INVALID, /* TODO do we need this actually ? */
        SCIVIS,
    } rendererType;

#ifdef VISIT_OSPRAY
public:
    OSPRenderer renderer;
    OSPVisItLight lights;
    // properties
    int aoSamples;
    int spp; //!< samples per pixel
    bool flagOneSidedLighting;
    bool flagShadowsEnabled;
    bool flagAoTransparencyEnabled;
    float       *maxDepthBuffer;  // depth buffer for the background (never delete)
    osp::vec2i   maxDepthSize;    // buffer extents (minX, maxX, minY, max)  
public:
    OSPVisItRenderer() {
        renderer = NULL;
        rendererType = INVALID;
        aoSamples = 0;
        spp = slivr::CheckOSPRaySpp();
        flagOneSidedLighting = false;
        flagShadowsEnabled = false;
        flagAoTransparencyEnabled = false;
    }
    ~OSPVisItRenderer() { Clean(); }
    void Clean() {
        if (renderer != NULL) {
            lights.Clean();
            ospRelease(renderer);
            renderer = NULL;
            rendererType = INVALID;
        }
    }
    void SetCamera(const OSPCamera& camera);
    void SetModel(const OSPModel& world);
#endif

    void Init()
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

    void Set(double materialProperties[4], double viewDirection[3], bool)
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

};


// ****************************************************************************
//  Struct:  OSPVisItCamera
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************

struct OSPVisItCamera
{
public:
    enum State {
        INVALID,
        PERSPECTIVE,
        ORTHOGRAPHIC,
    } cameraType;

    void Init(State type)
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

    void Set(const double camp[3], 
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
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

    void SetScreen(float xMin, float xMax, float yMin, float yMax)
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

#ifdef VISIT_OSPRAY
public:
    OSPCamera camera;
    float panx; // this is a ratio [0, 1]
    float pany; // this is a ratio [0, 1]
    float r_xl; 
    float r_yl;
    float r_xu;
    float r_yu;
    float zoom; 
    int   size[2];
    osp::vec2f imgS, imgE;
public:
    OSPVisItCamera() {
        camera = NULL;
        cameraType = INVALID;
        panx = 0.0f;
        pany = 0.0f;
        zoom = 1.0f;
        size[0] = size[1] = 0.0f;
        imgS.x = 0.f;
        imgS.y = 0.f;
        imgE.x = 0.f;
        imgE.y = 0.f;
    }
    ~OSPVisItCamera() { Clean(); }
    void Clean() {
        if (camera != NULL) {
            ospRelease(camera);
            camera = NULL;
            cameraType = INVALID;
        }
    }
#endif//VISIT_OSPRAY
};


// ****************************************************************************
//  Struct:  OSPVisItColor
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************

struct OSPVisItColor { float R,G,B, A; };

// ****************************************************************************
//  Struct:  OSPVisItTransferFunction
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************

struct OSPVisItTransferFunction
{
public:
    enum State { INVALID, PIECEWISE_LINEAR, } transferfcnType;

#ifdef VISIT_OSPRAY
public:
    OSPTransferFunction  transferfcn;
public:
    OSPVisItTransferFunction() {
        transferfcn = NULL;
        transferfcnType = INVALID;
    }
    void Clean() {
        if (transferfcn != NULL) {
            ospRelease(transferfcn);
            transferfcn = NULL;
            transferfcnType = INVALID;
        }
    }
#endif

    void Init()
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

    void Set(const OSPVisItColor* table,
             const unsigned int size, 
             const float datamin,
             const float datamax)
#ifdef VISIT_OSPRAY
        ;
#else 
    {};
#endif

};


// ****************************************************************************
//  Struct:  OSPVisItContext
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************

class OSPVisItContext
{
public:
    std::string var;
#ifdef VISIT_OSPRAY
private:
    friend class OSPVisItVolume;

private:
    // ************************************************************************
    // class parameters
    // ************************************************************************
    osp::vec3f regionScaling;
    double     bounds[6];
    // ************************************************************************
    // ospray mode
    // ************************************************************************
    bool initialized;
#endif//VISIT_OSPRAY

 public:
    // ************************************************************************
    // Members
    // ************************************************************************   
    OSPVisItRenderer renderer;
    OSPVisItCamera   camera;
    OSPVisItTransferFunction transferfcn;
    std::vector<OSPVisItVolume> volumes;

    // ************************************************************************
    // We expose this in header because iy will be called in other components
    // where we dont have direct library linkage
    // ************************************************************************
    OSPVisItContext() 
    {
#ifdef VISIT_OSPRAY
        regionScaling.x = regionScaling.y = regionScaling.z = 1.0f;
        initialized = false;
#else
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
#endif//VISIT_OSPRAY
    }

    // ************************************************************************
    // We expose this in header because iy will be called in other components
    // where we dont have direct library linkage
    // ************************************************************************
    ~OSPVisItContext() {        
#ifdef VISIT_OSPRAY
        // clean stuffs
        volumes.clear();
        renderer.Clean();
        camera.Clean();
        transferfcn.Clean();
#endif//VISIT_OSPRAY
    }

    // ************************************************************************
    // helper
    // ************************************************************************
    void Render(float xMin, float xMax, float yMin, float yMax,
                int imgWidth, int imgHeight, 
                float*& dest, OSPVisItVolume* volume)
#ifdef VISIT_OSPRAY
        ;
#else 
    { EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG); }; 
#endif//VISIT_OSPRAY

    // ************************************************************************
    // parameters
    // ************************************************************************
    void SetDataBounds(double dbounds[6]) {
#ifdef VISIT_OSPRAY
        for (int i = 0; i < 6; ++i) { bounds[i] = dbounds[i]; }
#else
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
#endif//VISIT_OSPRAY
    }
    void SetBgBuffer(float* depth, int extents[4]) {
#ifdef VISIT_OSPRAY
        renderer.maxDepthBuffer = depth;
        renderer.maxDepthSize.x = extents[1] - extents[0];
        renderer.maxDepthSize.y = extents[3] - extents[2];
#else
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
#endif//VISIT_OSPRAY
    }
    void SetScaling(double s[3]) { 
#ifdef VISIT_OSPRAY
        regionScaling.x = (float)s[0];
        regionScaling.y = (float)s[1];
        regionScaling.z = (float)s[2]; 
#else
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
#endif//VISIT_OSPRAY
    }

    // ************************************************************************
    // patch 
    // ************************************************************************
#ifdef VISIT_OSPRAY
    void InitOSP(int numThreads = 0);
    void InitPatch(int id);
#else
    void InitOSP(int numThreads = 0) {
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
    };
    void InitPatch(int id) {
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
    };
#endif//VISIT_OSPRAY

    // ************************************************************************
    // patch 
    // ************************************************************************
    OSPVisItVolume* GetPatch(int id) {
#ifdef VISIT_OSPRAY
        return &volumes[id]; 
#else
        EXCEPTION1(ImproperUseException, OSPRAY_ERROR_MSG);
#endif//VISIT_OSPRAY
    }

};


// ****************************************************************************
//  Namespace:  slivr
//
//  Purpose:
//    
//
//  Programmer: Qi WU
//  Creation:   
//
// ****************************************************************************
namespace slivr
{
    double deg2rad (double degrees);
    double rad2deg (double radins);
    void CheckMemoryHere(const std::string& message, 
                         std::string debugN = "debug5");
    void CheckMemoryHere(const std::string& message, 
                         std::ostream& out);
    inline void CheckSectionStart(const std::string& c, const std::string& f,
                                  int& timingDetail, const std::string& str) {
        debug5 << c << "::" << f << " " << str << " Start" << std::endl;
        timingDetail = visitTimer->StartTimer();            
    }
    
    inline void CheckSectionStop(const std::string& c, const std::string& f,
                                 int& timingDetail, const std::string& str) {
        visitTimer->StopTimer(timingDetail, (c + "::" + f + " " + str).c_str());
        slivr::CheckMemoryHere(("[" + c + "]" + " " + f + " " + str).c_str(), 
                               "ospout");
        debug5 << c << "::" << f << " " << str << " Done" << std::endl;
    }
};

#endif //AVT_OSPRAY_FILTER_H
