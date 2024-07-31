// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_OSPRAY /* make sure VISIT_OSPRAY is defined */
# error "VISIT_OSPRAY is not defined but ospray is used"
#endif

#ifndef AVT_OSPRAY_COMMON_H
#define AVT_OSPRAY_COMMON_H

#include <ospray/ospray.h>
#include <string>
#include <vector>
#include <map>

namespace ospray {
namespace visit {
  
    /** 
     * Helper Functions
     */
    template<typename T> void ospray_rm(T& obj) {
        if (!obj) { ospRelease(obj); obj = NULL; }
    }
  
    /**
     * Abstraction of an object
     */
    template<typename T> struct Object {
        bool init;
        T    self;
        Object() : init(false), self(NULL) {}
        virtual ~Object() { ospray_rm(self); init = false; }
        T operator*() { return self; }
    };
  
    /**
     * Transfer Function Wrapper
     */
    struct TransferFunctionCore : public Object<OSPTransferFunction> {
    TransferFunctionCore() : Object<OSPTransferFunction>() {}
    };

    /**
     * Camera Wrapper
     */
    struct CameraCore : public Object<OSPCamera> {
        bool   orthographic;
        int    windowExts[4];
        int    screenSize[2];
        double pan[2]; // pan ratio [0, 1]
        double zoom;   // zoom factor
        CameraCore() : Object<OSPCamera>() {
            orthographic = false;
            windowExts[0] = windowExts[1] = 0;
            windowExts[2] = windowExts[3] = 0;
            screenSize[0] = screenSize[1] = 0;
            pan[0] = pan[1] = 0.0;
            zoom = 1.0;
        }
    };

    /**
     * Light Wrapper
     */
    struct LightCore : public Object<OSPLight> {
        bool isAmbient;
        LightCore() : Object<OSPLight>() {
            isAmbient = false;
        }
    };  

    /**
     * Renderer Wrapper
     */
    struct RendererCore : public Object<OSPRenderer> {
        OSPData                lightData;
        std::vector<LightCore> lightList;
    RendererCore() : Object<OSPRenderer>() {
            lightData = NULL;
        }
        ~RendererCore() { ospray_rm(lightData); }
    };

    /**
     * Model Wrapper
     */
    struct ModelCore : public Object<OSPModel> {
        ModelCore() : Object<OSPModel>() {}
    };

    /**
     * Volume Wrapper
     */
    struct VolumeCore : public Object<OSPVolume> {
        std::string volumeType;
        OSPDataType dataType;
        size_t      dataSize;
        const void* dataPtr;
        bool useGridAccelerator;
        VolumeCore() : Object<OSPVolume>() {
            volumeType = "";
            dataType = OSP_UCHAR; /* just give it a value */
            dataSize = 0;
            dataPtr  = NULL;
            useGridAccelerator = false;
        }
    };
  
    /**
     * Framebuffer Wrapper
     */
    struct FrameBufferCore : public Object<OSPFrameBuffer> {
        FrameBufferCore() : Object<OSPFrameBuffer>() {}
    };

    /**
     * Now we define a PatchCore
     */
    struct Patch {
        VolumeCore      volume;
        ModelCore       model;
        FrameBufferCore fb;
    };

    /**
     * And a ContextCore
     */
    struct ContextCore {
        // data
        std::string varname;
        std::map<int, Patch> patches;
        CameraCore           camera;
        RendererCore         renderer;
        TransferFunctionCore tfn;
        // flags
        bool oneSidedLighting;       /* renderer */
        bool shadowsEnabled;         /* renderer */
        bool aoTransparencyEnabled;  /* renderer */
        bool useGridAccelerator;     /*  volume  */
        bool adaptiveSampling;       /*  volume  */
        bool preIntegration;         /*  volume  */
        bool singleShade;            /*  volume  */
        bool gradientShadingEnabled; /*  volume  */
        // other parameters
        double Ks;
        double Ns;
        double samplingRate;
        int aoSamples;
        int spp;
        double scale[3];
        double gbbox[6];
        // (shared, dont delete here)
        const unsigned char *bgColorBuffer;  // backplatte color channel
        const float         *bgDepthBuffer;  // backplatte depth channel 
        int                  bgSize[2];      // channel buffer size
        ContextCore() {
            varname = "";
            oneSidedLighting       = false;
            shadowsEnabled         = false;
            aoTransparencyEnabled  = false;
            useGridAccelerator     = false;
            adaptiveSampling       = false;
            preIntegration         = false;
            singleShade            = false;
            gradientShadingEnabled = false;
            Ks = 1.0; Ns = 20;
            samplingRate = 3.0;
            aoSamples = 0;
            spp = 1;
            scale[0] = scale[1] = scale[2] = 1.f;
            gbbox[0] = gbbox[1] = gbbox[2] = 0.f;
            gbbox[3] = gbbox[4] = gbbox[5] = 0.f;
            bgSize[0] = bgSize[1] = 0;
        }
    };

};
};

// ***************************************************************************
//  Struct:  OSPVisItContext
//
//  Purpose:
//
//
//  Programmer: Qi WU
//  Creation:   
//
// ***************************************************************************

typedef ospray::visit::ContextCore OSPVisItContext;
namespace ospray {
    void InitOSP(int numThreads = 0);
    void Finalize();
    struct Context : public ospray::visit::ContextCore {
    public:
        void SetBackgroundBuffer(const unsigned char* color,
                                 const float* depth, const int size[2]);
        void SetSpecular(const double& k, const double& n) { Ks = k; Ns = n; }
        void SetScaleAndDataBounds(const double v[3], const double d[6])
        {
            scale[0] = v[0]; scale[1] = v[1]; scale[2] = v[2];
            gbbox[0] = d[0] * scale[0];
            gbbox[3] = d[1] * scale[0];
            gbbox[1] = d[2] * scale[1];
            gbbox[4] = d[3] * scale[1];
            gbbox[2] = d[4] * scale[2];
            gbbox[5] = d[5] * scale[2];
        }       
        void SetSamplingRate(const double& v) { samplingRate = v; }
        void SetAoSamples(const int v) { aoSamples = v; } 
        void SetSpp(const int v) { spp = v; }
        void SetOneSidedLighting(bool v) { oneSidedLighting = v; }
        void SetShadowsEnabled(bool v) { shadowsEnabled = v; }
        void SetAoTransparencyEnabled(bool v) { aoTransparencyEnabled = v; }
        void SetUseGridAccelerator(bool v) { useGridAccelerator = v; }
        void SetAdaptiveSampling(bool v) { adaptiveSampling = v; }
        void SetPreIntegration(bool v) { preIntegration = v; }
        void SetSingleShade(bool v) { singleShade = v; }
        void SetGradientShadingEnabled(bool v) { gradientShadingEnabled = v; }
        void SetVariableName(const std::string& str) { varname = str; }
        const std::string& GetVariableName() const { return varname; }
        void InitPatch(const int patchID);
        void SetupPatch(const int patchID, const int vtk_type,
                        const size_t data_size, const void* data_ptr,
                        const double *X, const double *Y, const double *Z, 
                        const int nX, const int nY, const int nZ,
                        const double dbox[6], const double cbox[6]);
        void RenderPatch(const int patchID,
                         const float xMin, const float xMax, 
                         const float yMin, const float yMax,
                         const int tile_w, const int tile_h,
                         float*& dest); 
    };
};

#endif//AVT_OSPRAY_COMMON_H

// ***************************************************************************
//
//
//
//  Extra Functions Defined here
//
//
//
// ***************************************************************************

#ifndef VISIT_OSPRAY_CONTEXT_ONLY

#ifndef AVT_OSPRAY_COMMON_EXTRA_H
#define AVT_OSPRAY_COMMON_EXTRA_H

#include <avtParallel.h>
#include <avtViewInfo.h>
#include <avtCallback.h>

#include <DebugStream.h>
#include <StackTimer.h>
#include <TimingsManager.h>
#include <ImproperUseException.h>

#include <vtkType.h>
#include <vtkMatrix4x4.h>

#include <ospray/ospray.h>
#include <ospray/visit/VisItModuleCommon.h>
#include <ospray/visit/VisItExtraLibraries.h>
#include <ospray/visit/VisItImageComposite.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdio.h>

#ifndef CLAMP
# define CLAMP(x, l, h) (x > l ? x < h ? x : h : l)
#endif

// ostreams customized for ospray
#ifdef ospout
#undef ospout
#endif
#define ospout                                                      \
    if (!ospray::visit::CheckVerbose() && !DebugStream::Level5()) ; \
    else (*ospray::osp_out)
#ifdef osperr
#undef osperr
#endif
#define osperr                                                      \
    if (!ospray::visit::CheckVerbose() && !DebugStream::Level1()) ; \
    else (*ospray::osp_err)
namespace ospray {
    extern std::ostream *osp_out;
    extern std::ostream *osp_err;
    //////////////////////////////////////////////////////
    //                                                  //
    // Those function has to be inline, otherwise we    //
    // need to link this library to other components    //
    // manually                                         //
    //                                                  //
    //////////////////////////////////////////////////////
};

// ***************************************************************************
//  Namespace:  ospray
//
//  Purpose:
//    
//
//  Programmer: Qi WU
//  Creation:   
//
// ***************************************************************************

namespace ospray {

    namespace visit {

        template<typename _CoreType, typename _OSPType> struct Manipulator {
        protected:
            typedef _CoreType CoreType;
            typedef _OSPType  OSPType;
            _CoreType *core;
        public:
            Manipulator(_CoreType& other) : core{&other} {}
            _OSPType   operator* () { return *(*core); }
            _CoreType* operator->() { return &(*core); }
        };
  
        /**
         * Transfer Function Wrapper
         */
        struct TransferFunction
            : public Manipulator<TransferFunctionCore, OSPTransferFunction>
        {
        public:
            TransferFunction(CoreType& other);
            void Set(const void *table, const unsigned int size,
                     const double datamin, const double datamax);      
        };

        /**
         * Camera Wrapper
         */
        struct Camera
            : public Manipulator<CameraCore, OSPCamera>
        {
        public:
            Camera(CameraCore& other);
            double GetWindowExts(const int i) const { 
                return core->windowExts[i]; 
            }
            void Set(const bool ortho,
                     const double camera_p[3], 
                     const double camera_f[3], 
                     const double camera_u[3], 
                     const double fovy, 
                     const double pan_ratio[2],
                     const double zoom_ratio,
                     const double near_clip,
                     const double canvas_size[2],
                     const int screen_size[2],
                     const int tile_extents[4]);
            void SetScreen(const double xMin, const double xMax,
                           const double yMin, const double yMax);
        };

        /**
         * Light Wrapper
         */
        struct Light
            : public Manipulator<LightCore, OSPLight>
        {
        public:
            Light(LightCore& other);
            void Set(const bool ambient, const double i, 
                     const double c, const double* d = NULL);
            void Set(const bool ambient, const double i, 
                     const double cr, const double cg, const double cb,
                     const double* d = NULL);
            void Set(const bool ambient, const double i, 
                     const double c[3], const double* d = NULL);
        };

        /**
         * Volume Wrapper
         */
        struct Volume 
            : public Manipulator<VolumeCore, OSPVolume>
        {
        public:
            Volume(VolumeCore& other);
            bool Init(const std::string volume_type, 
                      const OSPDataType data_type, 
                      const std::string data_char,
                      const size_t data_size, 
                      const void* data_ptr,
                      const bool use_grid_accelerator);
            void Set(const bool adaptiveSampling,
                     const bool preIntegration, 
                     const bool singleShade, 
                     const bool gradientShadingEnabled,
                     const double samplingRate, 
                     const double Ks, const double Ns,
                     const double *X, const double *Y, const double *Z, 
                     const int nX, const int nY, const int nZ,
                     const double dbox[6], const double cbox[6], 
                     const osp::vec3f& global_upper,
                     const osp::vec3f& global_lower,
                     const osp::vec3f& scale,
                     OSPTransferFunction tfn);
            void Set(const bool adaptiveSampling,
                     const bool preIntegration, 
                     const bool singleShade, 
                     const bool gradientShadingEnabled, 
                     const double samplingRate, 
                     const double Ks, const double Ns,
                     const double *X, const double *Y, const double *Z, 
                     const int nX, const int nY, const int nZ,
                     const double dbox[6], const double cbox[6], 
                     const osp::vec3f& global_upper,
                     const osp::vec3f& global_lower,
                     const osp::vec3f& scale,
                     TransferFunction tfn)
            {
                Set(adaptiveSampling,
                    preIntegration, singleShade, 
                    gradientShadingEnabled, samplingRate, 
                    Ks, Ns, X, Y, Z, nX, nY, nZ,
                    dbox, cbox, global_upper, global_lower, scale,
                    *tfn);    
            }

            static void ComputeGhostBounds(bool bound[6], 
                                           const unsigned char *ghosts, 
                                           const int gnX, 
                                           const int gnY, 
                                           const int gnZ);
        };

        /**
         * Model Wrapper
         */
        struct Model
            : public Manipulator<ModelCore, OSPModel>
        {
        public:
            Model(ModelCore& other);
            void Reset();
            void Init();
            void Set(OSPVolume osp_volume);
            void Set(Volume volume) { Set(*volume); }
        };

        /**
         * Renderer Wrapper
         */
        struct Renderer
            : public Manipulator<RendererCore, OSPRenderer>
        {
        public:
            Renderer(RendererCore& other);
            void  Init();
            void  ResetLights();
            Light AddLight();
            void  FinalizeLights();
            void  Set(const int aoSamples, const int spp, 
                      const bool oneSidedLighting,
                      const bool shadowsEnabled,
                      const bool aoTransparencyEnabled);
            void  Set(OSPCamera osp_camera);
            void  Set(Camera        camera) { Set(*camera); }
            void  Set(OSPModel   osp_world);
            void  Set(Model          world) { Set(*world);  }
        };


        /**
         * FrameBuffer Wrapper
         */
        struct FrameBuffer
            : public Manipulator<FrameBufferCore, OSPFrameBuffer>
        {
        public:
            FrameBuffer(FrameBufferCore& other);
            void Render(const int tile_w, const int tile_h,
                        const int tile_x, const int tile_y,
                        const int global_stride, 
                        const float* global_depth,
                        OSPRenderer renderer,
                        float*& dest);
            void Render(const int tile_w, const int tile_h,
                        const int tile_x, const int tile_y,
                        const int global_stride, 
                        const float* global_depth,
                        Renderer renderer,
                        float*& dest)
            {
                Render(tile_w, tile_h, tile_x, tile_y,
                       global_stride, global_depth,
                       *renderer, dest);
            }

        };
    };

    typedef ospray::visit::TransferFunction TransferFunction;
    typedef ospray::visit::Camera Camera;
    typedef ospray::visit::Renderer Renderer;
    typedef ospray::visit::Volume Volume;
    typedef ospray::visit::Model Model;
    typedef ospray::visit::FrameBuffer FrameBuffer;
    typedef ospray::visit::Patch Patch;
        
    // ***********************************************************************
    //  Struct:  ImgMetaData
    //
    //  Purpose:
    //    Holds information about patches but not the image 
    //
    //  Programmer:  
    //  Creation:   
    //
    // ***********************************************************************

    struct ImgMetaData
    {
        int procId;       // processor that produced the patch
        int patchNumber;  // id of the patch on that processor
        int destProcId;   // destination proc where this patch gets composited
        int inUse;        // whether the patch is composed locally or not
        int dims[2];      // height, width
        int screen_ll[2]; // (lower left)  position in the final image
        int screen_ur[2]; // (upper right)
        float avg_z;      // camera space depth of the patch (average)
        float eye_z;      // camera space z
        float clip_z;     // clip space z
    };

    // ***********************************************************************
    //  Struct:  ImgData
    //
    //  Purpose:
    //    Holds the image data generated
    //
    //  Programmer:  
    //  Creation:    
    //
    // ***********************************************************************
    
    struct ImgData
    {
        // acts as a key
        int procId;        // processor that produced the patch
        int patchNumber;   // id of the patch on that processor
        float *imagePatch; // the image data - RGBA
        ImgData() { imagePatch = NULL; }
        bool operator==(const ImgData &a) {
            return (patchNumber == a.patchNumber);
        }
    };

    // ***********************************************************************
    //
    //  Helper Functions
    //
    // ***********************************************************************
        
    void CheckMemoryHere(const std::string& message, 
                         std::string debugN = "debug5");
    void CheckMemoryHere(const std::string& message, 
                         std::ostream& out);

    typedef int timestamp;
    inline void CheckSectionStart(const std::string& c,
                                  const std::string& f,
                                  timestamp& timingDetail,
                                  const std::string& str) 
    {
        debug5 << c << "::" << f << " " << str << " Start" << std::endl;
        timingDetail = visitTimer->StartTimer();            
    }
    
    inline void CheckSectionStop(const std::string& c,
                                 const std::string& f, 
                                 timestamp& timingDetail,
                                 const std::string& str) 
    {
        visitTimer->StopTimer(timingDetail, 
                              (c + "::" + f + " " + str).c_str());
        ospray::CheckMemoryHere(("[" + c + "]" + " " + f + " " + str).c_str(),
                                "debug5");
        debug5 << c << "::" << f << " " << str << " Done" << std::endl;
    }

    inline void Exception(const std::string str)
    {
        std::cerr << str << std::endl;
        debug1    << str << std::endl;
        EXCEPTION1(ImproperUseException, str.c_str());
        avtCallback::SetRenderingException(str);
    }

    inline void Warning(const std::string str)
    {
        avtCallback::IssueWarning(str.c_str());
    }
    void CheckVolumeFormat(const int dt,
                           std::string& str_type,
                           OSPDataType& osp_type);

    void ComputeProjections(const avtViewInfo &view, 
                            const double      &aspect,
                            const double      &old_near_plane,
                            const double      &old_far_plane,
                            const double       data_scale[3],
                            const double       data_bound[6],
                            const int          screen_size[2],
                            vtkMatrix4x4 *model_to_screen_transform, 
                            vtkMatrix4x4 *screen_to_model_transform, 
                            vtkMatrix4x4 *screen_to_camera_transform,
                            double        canvas_size[2],
                            int           rendering_extents[4]);

    double ProjectWorldToScreen
        (const double worldCoord[3], 
         const int screenWidth, const int screenHeight,  
         const double panPercentage[2], const double imageZoom,
         vtkMatrix4x4 *mvp, int screenCoord[2]);
    
    void ProjectScreenToWorld
        (const int screenCoord[2], const double z,
         const int screenWidth, const int screenHeight, 
         const double panPercentage[2], const double imageZoom,
         vtkMatrix4x4 *imvp, double worldCoord[3]);

    void ProjectScreenToCamera
        (const int screenCoord[2], const double z,
         const int screenWidth, const int screenHeight,
         vtkMatrix4x4 *imvp, double cameraCoord[3]);

    inline void ProjectScreenToWorld
        (const int x, const int y, const double z,
         const int screenWidth, const int screenHeight, 
         const double panPercentage[2], const double imageZoom,
         vtkMatrix4x4 *imvp, double worldCoord[3]) 
    {
        int screen_coord[2] = {x, y};
        ProjectScreenToWorld(screen_coord, z, screenWidth, screenHeight, 
                             panPercentage, imageZoom, imvp, worldCoord);
    }

    void ProjectWorldToScreenCube
        (const double cube[6], const int screenWidth, const int screenHeight, 
         const double panPercentage[2], const double imageZoom, 
         vtkMatrix4x4 *mvp,int screenExtents[4], double depthExtents[2]);

    void CompositeBackground(int screen[2],
                             int compositedImageExtents[4],
                             int compositedImageWidth,
                             int compositedImageHeight,
                             float *compositedImageBuffer,
                             unsigned char *opaqueImageColor,
                             float         *opaqueImageDepth,
                             unsigned char *&imgFinal);
    
    void WriteArrayToPPM
        (std::string, const float *image, int dimX, int dimY);

    void WriteArrayToPPM
        (std::string, const unsigned char *image, int dimX, int dimY);

    void WriteArrayGrayToPPM
        (std::string, const float * image, int dimX, int dimY);
};

#endif//AVT_OSPRAY_COMMON_EXTRA_H

#endif//VISIT_OSPRAY_CONTEXT_ONLY

