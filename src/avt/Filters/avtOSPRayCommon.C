/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

#include <avtOSPRayCommon.h>

#include <avtParallel.h>
#include <avtMemory.h>

#include <DebugStream.h>
#include <StackTimer.h>
#include <TimingsManager.h>
#include <ImproperUseException.h>

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>

#include <ospray/ospray.h>
#include <ospray/visit/VisItModuleCommon.h>
#include <ospray/visit/VisItExtraLibraries.h>
#include <ospray/visit/VisItImageComposite.h>
#include <ospray/ospcommon/vec.h>
#include <cmath>
#include <vector>

#ifdef __unix__
# include <unistd.h>
#endif

static bool CheckThreadedBlend_MetaData() {
    bool use = true;
    const char* env_use = std::getenv("OSPRAY_SERIAL_BLEND");
    if (env_use) { use = atoi(env_use) <= 0; }
    return use;
}

static bool UseThreadedBlend_MetaData = CheckThreadedBlend_MetaData();

std::ostream *ospray::osp_out = (ospray::visit::CheckVerbose()) ?
    &std::cout : &DebugStream::Stream5();
std::ostream *ospray::osp_err = (ospray::visit::CheckVerbose()) ?
    &std::cerr : &DebugStream::Stream1();

// ***************************************************************************
//
// OSPRay
//
// ***************************************************************************

using namespace ospcommon;

namespace ospray {
  namespace visit {
    
    template<typename T> 
    void ospray_check(const T& obj, const std::string s) {
      if (!obj) { 
        if (CheckVerbose()) {
          std::cerr << s << " is invalid" << std::endl; 
        }
        throw std::runtime_error(s + " is invalid");
      }
    }
    
    // =====================================================================//
    //
    // =====================================================================//
    struct Color { float R,G,B,A; };

    // =====================================================================//
    //
    // =====================================================================//
    TransferFunction::TransferFunction(TransferFunctionCore& other)
      : Manipulator<CoreType, OSPType>(other) {}
    void TransferFunction::Set(const void *_table,
                               const unsigned int size, 
                               const double datamin, 
                               const double datamax) 
    {
      // initialize it once
      if (!core->init) 
      {
        ospray_rm(core->self);
        core->self = ospNewTransferFunction("piecewise_linear");
        ospray_check(core->self, "transfer function");
        core->init = true;
      }
      // create OSP data
      const Color* table = reinterpret_cast<const Color*>(_table);
      std::vector<vec3f> cdata;
      std::vector<float> odata;
      for (unsigned int i = 0; i < size; ++i)
      {
        cdata.emplace_back(table[i].R, table[i].G, table[i].B);
        odata.emplace_back(table[i].A);
      }
      OSPData osp_cdata = ospNewData(cdata.size(), OSP_FLOAT3, cdata.data());
      OSPData osp_odata = ospNewData(odata.size(), OSP_FLOAT,  odata.data());
      ospray_check(osp_cdata,"TFN color data");
      ospray_check(osp_odata,"TFN opacity data");
      // commit
      ospSetData(core->self, "colors",    osp_cdata);
      ospSetData(core->self, "opacities", osp_odata);
      ospSet2f(core->self, "valueRange", datamin, datamax);
      ospCommit(core->self);
      // cleanup
      ospray_rm(osp_cdata);
      ospray_rm(osp_odata);
    }

    // =====================================================================//
    //
    // =====================================================================//
    Camera::Camera(CameraCore& other) 
      : Manipulator<CoreType, OSPType>(other) {}
    void Camera::Set(const bool ortho,
                     const double camera_p[3], 
                     const double camera_f[3], 
                     const double camera_u[3], 
                     const double fovy,
                     const double pan_ratio[2],
                     const double zoom_ratio,
                     const double near_clip,
                     const double canvas_size[2],
                     const int screen_size[2], /* overall screen size */
                     const int tile_extents[4] /*      tile size      */)
    {
      // create camera
      if (!core->init || (ortho != core->orthographic))
      {
        core->orthographic = ortho;
        ospray_rm(core->self);
        core->self = core->orthographic ? 
          ospNewCamera("orthographic") : 
          ospNewCamera("perspective");
        ospray_check(core->self, "camera");
        core->init = true;
      }
      // compute camera
      core->zoom = zoom_ratio;
      core->pan[0] = pan_ratio[0];
      core->pan[1] = pan_ratio[1];
      core->screenSize[0] = screen_size[0];
      core->screenSize[1] = screen_size[1];
      // commit
      ospSet1f(core->self, "nearClip", near_clip);
      ospSet3f(core->self, "pos", 
               camera_p[0], 
               camera_p[1], 
               camera_p[2]);
      ospSet3f(core->self, "dir", 
               camera_f[0] - camera_p[0], 
               camera_f[1] - camera_p[1], 
               camera_f[2] - camera_p[2]);
      ospSet3f(core->self, "up",
               camera_u[0],
               camera_u[1], 
               camera_u[2]);
      ospSet1f(core->self, "aspect", 
               static_cast<double>(screen_size[0]) /
               static_cast<double>(screen_size[1]));
      if (!ortho)
        ospSet1f(core->self, "fovy", fovy / core->zoom);
      else
        ospSet1f(core->self, "height", canvas_size[1]);
      SetScreen(tile_extents[0], tile_extents[1],
                tile_extents[2], tile_extents[3]);
    }
    void Camera::SetScreen(const double xMin, const double xMax,
                           const double yMin, const double yMax) 
    {
      core->windowExts[0] = std::max(static_cast<int>(std::round(xMin)), 0);
      core->windowExts[1] = std::min(static_cast<int>(std::round(xMax)), 
                                     core->screenSize[0]);
      core->windowExts[2] = std::max(static_cast<int>(std::round(yMin)), 0);
      core->windowExts[3] = std::min(static_cast<int>(std::round(yMax)), 
                                     core->screenSize[1]);
      const double r_xl = xMin/core->screenSize[0] - core->pan[0];
      const double r_yl = yMin/core->screenSize[1] - core->pan[1];
      const double r_xu = xMax/core->screenSize[0] - core->pan[0];
      const double r_yu = yMax/core->screenSize[1] - core->pan[1];
      ospSet2f(core->self, "imageStart", r_xl, r_yl);
      ospSet2f(core->self, "imageEnd",   r_xu, r_yu);
      ospCommit(core->self);
    }

    // =====================================================================//
    //
    // =====================================================================//
    Light::Light(LightCore& other) 
      : Manipulator<CoreType, OSPType>(other) {}
    void Light::Set(const bool ambient, const double i, 
                    const double c, const double* d)
    {
      Set(ambient, i, c, c, c, d);
    }
    void Light::Set(const bool ambient, const double i, 
                    const double cr, const double cg, const double cb,
                    const double* d)
    {
      const double c[3] = {cr, cg, cb};
      Set(ambient, i, c, d);
    }
    void Light::Set(const bool ambient, const double i, 
                    const double c[3], const double* d)
    {
      // create light
      if (!core->init || ambient != core->isAmbient) {
        ospray_rm(core->self);
        core->self = ospNewLight2("scivis", "distant");
        ospray_check(core->self, "light");
        ospSet1i(core->self, "isVisible", 0);
        ospSet1f(core->self, "angularDiameter", 0.53f);
        core->init = true;
      }
      // commit light
      ospSet1f(core->self, "intensity", i);
      ospSet3f(core->self, "color", c[0], c[1], c[2]);
      if (!ambient) {
        ospSet3f(core->self, "direction", d[0], d[1], d[2]);
      }
      ospCommit(core->self);
    }

    // =====================================================================//
    //
    // =====================================================================//
    Renderer::Renderer(RendererCore& other) 
      : Manipulator<CoreType, OSPType>(other) {}
    void Renderer::Init()
    {
      if (!core->init) {
        ospray_rm(core->self);
        core->self = ospNewRenderer("scivis");
        ospray_check(core->self, "SCIVIS Renderer");
        core->init = true;
      }
    }
    void Renderer::ResetLights()
    {
      if (!core->init) { Init(); }
      ospray_rm(core->lightData);
      core->lightList.clear();
    }
    Light Renderer::AddLight()
    {
      core->lightList.emplace_back();
      return Light(core->lightList.back());
    }
    void Renderer::FinalizeLights()
    {
      std::vector<OSPLight> osp_light_list;
      for (auto& l : core->lightList) { osp_light_list.emplace_back(*l); }
      ospray_rm(core->lightData);
      core->lightData = ospNewData(osp_light_list.size(), OSP_OBJECT, 
                                   osp_light_list.data());
      ospray_check(core->lightData, "light list");
    }
    void Renderer::Set(const int aoSamples, const int spp, 
                       const bool oneSidedLighting,
                       const bool shadowsEnabled,
                       const bool aoTransparencyEnabled)
    {
      if (!core->init) { Init(); }   
      ospSet1i(core->self, "aoSamples", aoSamples);
      ospSet1i(core->self, "spp", spp);
      ospSet1i(core->self, "oneSidedLighting", oneSidedLighting);
      ospSet1i(core->self, "shadowsEnabled", shadowsEnabled);
      ospSet1i(core->self, "aoTransparencyEnabled", aoTransparencyEnabled);
      ospSetData(core->self, "lights", core->lightData);
      ospCommit(core->self);
    }
    void Renderer::Set(OSPCamera osp_camera)
    {
      if (!core->init) { Init(); }
      ospSetObject(core->self, "camera", osp_camera);
      ospCommit(core->self);
    }
    void Renderer::Set(OSPModel osp_world)
    {
      if (!core->init) { Init(); }
      ospSetObject(core->self, "model", osp_world);
      ospCommit(core->self);
    }
    
    // =====================================================================//
    //
    // =====================================================================//
    Model::Model(ModelCore& other)
      : Manipulator<CoreType, OSPType>(other) {}
    void Model::Reset()
    {
      core->init = false;
    }
    void Model::Init()
    {
      if (!core->init) {
        ospray_rm(core->self);
        core->self = ospNewModel();
        ospray_check(core->self, "Model");
        core->init = true;
      }
    }
    void Model::Set(OSPVolume volume)
    {
      if (!core->init) { Init(); }
      ospAddVolume(core->self, volume);
      ospCommit(core->self);
    }

    // =====================================================================//
    //
    // =====================================================================//
    Volume::Volume(VolumeCore& other)
      : Manipulator<CoreType, OSPType>(other) {}
    bool Volume::Init(const std::string volume_type, 
                      const OSPDataType data_type, 
                      const std::string data_char,
                      const size_t data_size, 
                      const void* data_ptr,
                      const bool use_grid_accelerator)
    {
      if (!core->init || 
          volume_type != core->volumeType ||
          data_type   != core->dataType   ||
          data_size   != core->dataSize   ||
          data_ptr    != core->dataPtr    ||
          use_grid_accelerator != core->useGridAccelerator)
          
      {
        core->volumeType = volume_type;
        core->dataType = data_type;
        core->dataSize = data_size;
        core->dataPtr  = data_ptr;
        core->useGridAccelerator = use_grid_accelerator;
        ospray_rm(core->self);
        core->self = ospNewVolume(volume_type.c_str());
        ospray_check(core->self, volume_type);
        if (volume_type == "visit_shared_structured_volume" ||
            volume_type == "shared_structured_volume") 
        {
          OSPData osp_data = ospNewData(data_size, data_type,
                                        data_ptr, OSP_DATA_SHARED_BUFFER);
          ospSetString(core->self, "voxelType", data_char.c_str());
          ospSetData(core->self, "voxelData", osp_data);
          ospray_rm(osp_data);
        }
        // TODO: there is a bug inside the module_visit, therefore we cannot enable grid
        //       accelerator currently
        ospSet1i(core->self, "useGridAccelerator", /*(int)use_grid_accelerator*/false);
        core->init = true;
        return true;
      }
      return false;
    }
    void Volume::Set(const bool adaptiveSampling,
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
                     OSPTransferFunction tfn)
    {
      const vec3i dims(nX, nY, nZ);
      const vec3f data_lower(vec3f(dbox[0], dbox[1], dbox[2]) * (const vec3f&)scale);
      const vec3f data_upper(vec3f(dbox[3], dbox[4], dbox[5]) * (const vec3f&)scale);
      const vec3f clip_lower(vec3f(cbox[0], cbox[1], cbox[2]) * (const vec3f&)scale);
      const vec3f clip_upper(vec3f(cbox[3], cbox[4], cbox[5]) * (const vec3f&)scale);
      const vec3f spacing = (data_upper - data_lower)/((const vec3f)dims - 1.0f);
      ospSetVec3f(core->self, "volumeGlobalBoundingBoxLower", 
                  (const osp::vec3f&) global_upper);
      ospSetVec3f(core->self, "volumeGlobalBoundingBoxUpper",
                  (const osp::vec3f&) global_lower);
      ospSetVec3f(core->self, "volumeClippingBoxLower", (const osp::vec3f&)clip_lower);
      ospSetVec3f(core->self, "volumeClippingBoxUpper", (const osp::vec3f&)clip_upper);
      ospSetVec3f(core->self, "gridSpacing", (const osp::vec3f&)spacing);
      ospSetVec3f(core->self, "gridOrigin",  (const osp::vec3f&)data_lower);
      ospSetVec3i(core->self, "dimensions",  (const osp::vec3i&)dims);
      ospSet1f(core->self, "samplingRate", samplingRate);
      ospSet3f(core->self, "Ks", Ks, Ks, Ks);
      ospSet1f(core->self, "Ns", Ns);
      ospSet1i(core->self, "gradientShadingEnabled", (int)gradientShadingEnabled);
      ospSet1i(core->self, "adaptiveSampling", (int)adaptiveSampling);
      ospSet1i(core->self, "preIntegration", (int)preIntegration);
      ospSet1i(core->self, "singleShade", (int)singleShade);
      ospSetObject(core->self, "transferFunction", tfn);
      ospCommit(core->self);
    }  

  // =====================================================================//
  //
  // =====================================================================//
  FrameBuffer::FrameBuffer(FrameBufferCore& other)
    : Manipulator<CoreType, OSPType>(other) {}
  void FrameBuffer::Render(const int tile_w, const int tile_h,
                           const int tile_x, const int tile_y,
                           const int    global_stride, 
                           const float* global_depth,
                           OSPRenderer renderer,
                           float*& dest)
  {
    const vec2i fb_size(tile_w, tile_h);
    // prepare the maxDepthDexture
    {
      // The reason I use round(r * (N-1)) instead of floor(r * N) is that
      // during the composition phase, there will be a wired offset between
      // rendered image and the background, which is about one pixel in size.
      // Using round(r * (N - 1)) can remove the problem
      std::vector<float> local_depth(tile_w * tile_h);
      for (int i = 0; i < tile_w; ++i) {
        for (int j = 0; j < tile_h; ++j) {
          local_depth[i + j * tile_w] = 
            global_depth[tile_x + i + (tile_y + j) * global_stride];
        }
      }
      OSPTexture2D maxDepthTexture
        = ospNewTexture2D((const osp::vec2i&)fb_size, OSP_TEXTURE_R32F,
                          local_depth.data(), OSP_TEXTURE_FILTER_NEAREST);    
      ospCommit(maxDepthTexture);
      ospSetObject(renderer, "maxDepthTexture", maxDepthTexture);
      ospCommit(renderer);
      ospray_rm(maxDepthTexture);
    }    
    // do the rendering
    // ALWAYS create a new framebuffer
    {
      ospray_rm(core->self);
      core->self = 
        ospNewFrameBuffer((const osp::vec2i&)fb_size, OSP_FB_RGBA32F, OSP_FB_COLOR);
      ospray_check(core->self, "framebuffer");
      ospRenderFrame(core->self, renderer, OSP_FB_COLOR);
      const float* image = (float*)ospMapFrameBuffer(core->self, OSP_FB_COLOR);
      std::copy(image, image + (tile_w * tile_h) * 4, dest);
      ospUnmapFrameBuffer(image, core->self);
      ospray_rm(core->self);      
    }
  }

  
};
};

static void OSPContext_ErrorFunc(OSPError, const char* msg)
{ 
    osperr << "#osp: (rank " << PAR_Rank() << ")" << msg; 
}
static void OSPContext_StatusFunc(const char* msg)
{ 
    osperr << "#osp: (rank " << PAR_Rank() << ")" << msg; 
}

static bool ospray_initialized = false;

void ospray::InitOSP(int numThreads) 
{   
    if (!ospray_initialized) 
    {
#ifdef __unix__
        // check hostname
        char hname[200];
        gethostname(hname, 200);
        ospout << "[ospray] on host >> " << hname << "<<" << std::endl;;
#endif
        // load ospray device
        ospout << "[ospray] Initialize OSPRay" << std::endl;    
        OSPDevice device = ospGetCurrentDevice();
        // check if ospray has been initialized already
        if (!device) {
            ospout << "[ospray] device not found, creating one" << std::endl;
            device = ospNewDevice("default"); 
            if (DebugStream::Level5()) { 
                ospout << "[ospray] --> debug mode" << std::endl;
                ospDeviceSet1i(device, "debug", 0);
            }   
            if (numThreads > 0) {
                ospout << "[ospray] --> numThreads: " << numThreads 
                       << std::endl;
                ospDeviceSet1i(device, "numThreads", numThreads);
            }
            ospDeviceSetErrorFunc(device, OSPContext_ErrorFunc);
            ospDeviceSetStatusFunc(device, OSPContext_StatusFunc);
            ospDeviceCommit(device);
            ospSetCurrentDevice(device);
        }
        // load ospray module
        OSPError err = ospLoadModule("visit");
        if (err != OSP_NO_ERROR) {
            osperr << "[Error] can't load visit module" << std::endl;
        }
        ospray_initialized = true;
    }
}

void ospray::Finalize()
{
}

// ***************************************************************************
//
// OSPRay::Context
//
// ***************************************************************************

void ospray::Context::InitPatch(const int patchID)
{
    if (patches.find(patchID) == patches.end()) {
        patches[patchID] = ospray::Patch();
    }
}
void ospray::Context::SetBackgroundBuffer(const unsigned char* color, 
                                          const float* depth, 
                                          const int size[2])
{
    bgColorBuffer = color;
    bgDepthBuffer = depth;
    bgSize[0] = size[0];
    bgSize[1] = size[1];
}
void ospray::Context::SetupPatch(const int patchID,
                                 const int vtk_type,
                                 const size_t data_size, 
                                 const void* data_ptr,
                                 const double *X,
                                 const double *Y,
                                 const double *Z, 
                                 const int nX, const int nY, const int nZ,
                                 const double dbox[6], const double cbox[6])
    
{
    std::string str_type;
    OSPDataType osp_type;
    CheckVolumeFormat(vtk_type, str_type, osp_type);
    Volume volume(patches[patchID].volume);
    volume.Init("visit_shared_structured_volume",
                osp_type, str_type, data_size, data_ptr,
                useGridAccelerator);
    volume.Set(adaptiveSampling,
               preIntegration,
               singleShade,
               gradientShadingEnabled,
               samplingRate, 
               Ks, Ns,
               X, Y, Z, nX, nY, nZ,
               dbox, cbox,
               osp::vec3f{(float)gbbox[0],(float)gbbox[1],(float)gbbox[2]},
               osp::vec3f{(float)gbbox[3],(float)gbbox[4],(float)gbbox[5]},
               osp::vec3f{(float)scale[0],(float)scale[1],(float)scale[2]},
               tfn);    
    Model model(patches[patchID].model);
    model.Reset();
    model.Init();
    model.Set(patches[patchID].volume);
}
void ospray::Context::RenderPatch(const int patchID,
                                  const float xMin, const float xMax, 
                                  const float yMin, const float yMax,
                                  const int tile_w, const int tile_h,
                                  float*& dest)
{
    Camera      cam(camera);
    Renderer    ren(renderer);
    FrameBuffer fb(patches[patchID].fb);
    cam.SetScreen(xMin, xMax, yMin, yMax);
    ren.Set(patches[patchID].model);
    ren.Set(camera);
    fb.Render(tile_w, tile_h,
              cam.GetWindowExts(0),
              cam.GetWindowExts(2),
              bgSize[0],
              bgDepthBuffer,
              renderer, dest);
}

// ***************************************************************************
//
//  Extra Functions Defined here
//
// ***************************************************************************

void
ospray::CheckMemoryHere(const std::string& message, std::string debugN)
{
    if (debugN.compare("ospout") == 0) {        
        ospray::CheckMemoryHere(message, *osp_out);
    }
    else if (debugN.compare("debug5") == 0) {
        if (DebugStream::Level5()) {
            ospray::CheckMemoryHere(message, DebugStream::Stream5());
        }       
    }
    else if (debugN.compare("debug4") == 0) {
        if (DebugStream::Level4()) {
            ospray::CheckMemoryHere(message, DebugStream::Stream4());
        }       
    }
    else if (debugN.compare("debug3") == 0) {
        if (DebugStream::Level3()) {
            ospray::CheckMemoryHere(message, DebugStream::Stream3());
        }       
    }
    else if (debugN.compare("debug2") == 0) {
        if (DebugStream::Level2()) {
            ospray::CheckMemoryHere(message, DebugStream::Stream2());
        }       
    }
    else if (debugN.compare("debug1") == 0) {
        if (DebugStream::Level1()) {
            ospray::CheckMemoryHere(message, DebugStream::Stream1());
        }       
    }
}

void
ospray::CheckMemoryHere(const std::string& message, std::ostream& out)
{
    unsigned long m_size, m_rss;
    avtMemory::GetMemorySize(m_size, m_rss);
    out << message << std::endl << "\t"
        << " Rank " << PAR_Rank()
        << " Memory use begin " << m_size 
        << " rss " << m_rss/(1024*1024) << " (MB)"
        << std::endl;
}

void
ospray::CheckVolumeFormat(const int dt,
                          std::string& str_type,
                          OSPDataType& osp_type)
{
    if (dt == VTK_UNSIGNED_CHAR) {
        str_type = "uchar";
        osp_type = OSP_UCHAR;
    } else if (dt == VTK_SHORT) {
        str_type = "short";
        osp_type = OSP_SHORT;
    } else if (dt == VTK_UNSIGNED_SHORT) {
        str_type = "ushort";
        osp_type = OSP_USHORT;
    } else if (dt == VTK_FLOAT) {
        str_type = "float";
        osp_type = OSP_FLOAT;
    } else if (dt == VTK_DOUBLE) {
        str_type = "double";
        osp_type = OSP_DOUBLE;
    } else {
        ospray::Exception("Unsupported ospray volume type.");
    }
    ospout << "[ospray] data type " << str_type << std::endl;
}

void
ospray::ComputeProjections(const avtViewInfo &view, 
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
                           int           rendering_extents[4]) 
{
    //----------------------------------------------------------------------//
    // see avt/View/avtViewInfo::SetCameraFromView
    //----------------------------------------------------------------------//
    /* view.SetCameraFromView(vtkcamera); */
    vtkCamera *vtkcamera = vtkCamera::New();
    vtkcamera->SetViewAngle(view.viewAngle);
    vtkcamera->SetEyeAngle(view.eyeAngle);
    if (view.setScale)
    {
        vtkcamera->SetParallelScale(view.parallelScale);
    }
    vtkcamera->SetParallelProjection(view.orthographic ? 1 : 0);
    vtkcamera->SetViewShear(view.shear[0],view.shear[1],view.shear[2]);
    vtkcamera->SetFocalPoint(view.focus);
    vtkcamera->SetPosition(view.camera);
    vtkcamera->SetViewUp(view.viewUp);
    vtkcamera->SetFocalDisk(view.imageZoom);
    vtkcamera->Zoom(view.imageZoom);
    // here we have to use the old near/far planes
    vtkcamera->SetClippingRange(old_near_plane, old_far_plane);
    // here we dont want to move window center ?
    //vtkcamera->SetWindowCenter(2.0*view.imagePan[0], 2.0*view.imagePan[1]);
    //----------------------------------------------------------------------//
    // compute matrix
    //----------------------------------------------------------------------//
    vtkMatrix4x4 *matMVPS =
        vtkcamera->GetModelViewTransformMatrix();
    vtkMatrix4x4 *matProj = 
        vtkcamera->GetProjectionTransformMatrix(aspect, -1, 1);    
    vtkMatrix4x4::Multiply4x4(matProj, matMVPS,
                              model_to_screen_transform);
    vtkMatrix4x4::Invert(model_to_screen_transform,
                         screen_to_model_transform);
    vtkMatrix4x4::Invert(matProj,
                         screen_to_camera_transform);
    //----------------------------------------------------------------------//
    // compute canvas size (parallel scale)
    //----------------------------------------------------------------------//
    if (!view.orthographic)
    {
        canvas_size[0] = 2.0 * old_near_plane / matProj->GetElement(0, 0);
        canvas_size[1] = 2.0 * old_near_plane / matProj->GetElement(1, 1);
    }
    else
    {
        canvas_size[0] = 2.0 / matProj->GetElement(0, 0);
        canvas_size[1] = 2.0 / matProj->GetElement(1, 1);
    }
    //----------------------------------------------------------------------//
    // cleanup
    //----------------------------------------------------------------------//
    vtkcamera->Delete();
    //----------------------------------------------------------------------//
    // get the renderable region
    //----------------------------------------------------------------------//
    double depths[2];
    ospray::ProjectWorldToScreenCube(data_bound,
                                     screen_size[0], screen_size[1], 
                                     view.imagePan, view.imageZoom,
                                     model_to_screen_transform,
                                     rendering_extents, depths);
    rendering_extents[0] = std::max(rendering_extents[0], 0);
    rendering_extents[2] = std::max(rendering_extents[2], 0);
    rendering_extents[1] = std::min(1+rendering_extents[1], screen_size[0]);
    rendering_extents[3] = std::min(1+rendering_extents[3], screen_size[1]);
}

double
ospray::ProjectWorldToScreen(const double worldCoord[3], 
                             const int screenWidth, 
                             const int screenHeight,
                             const double panPercentage[2], 
                             const double imageZoom,
                             vtkMatrix4x4 *mvp, int screenCoord[2])
{
    // world space coordinate in homogeneous coordinate
    double worldHCoord[4] = {worldCoord[0],worldCoord[1],worldCoord[2],1.0};
    // world to clip space (-1 ~ 1)
    double clipHCoord[4];
    mvp->MultiplyPoint(worldHCoord, clipHCoord);
    // check error
    if (clipHCoord[3] == 0.0)
    {
        std::cerr << "world coordinates: (" 
                  << worldHCoord[0] << ", " 
                  << worldHCoord[1] << ", " 
                  << worldHCoord[2] << ", " 
                  << worldHCoord[3] << ")" << std::endl
                  << "clip space coordinate: ("
                  << clipHCoord[0] << ", " 
                  << clipHCoord[1] << ", " 
                  << clipHCoord[2] << ", "
                  << clipHCoord[3] << std::endl
                  << "Matrix: " << *mvp << std::endl;
        ospray::Exception("Zero Division During Projection.");
    }
    // screen coordinates (int integer)
    screenCoord[0] =
        round((clipHCoord[0] / clipHCoord[3] + 1) * screenWidth  * 0.5) +
        round(screenWidth  * panPercentage[0]);
    screenCoord[1] =
        round((clipHCoord[1] / clipHCoord[3] + 1) * screenHeight * 0.5) +
        round(screenHeight * panPercentage[1]); 
    // return point depth
    return clipHCoord[2]/clipHCoord[3];
}

void
ospray::ProjectScreenToWorld(const int screenCoord[2], const double z,
                             const int screenWidth, const int screenHeight, 
                             const double panPercentage[2], 
                             const double imageZoom,
                             vtkMatrix4x4 *imvp, double worldCoord[3])
{
    // remove panning
    const int x = screenCoord[0] - round(screenWidth*panPercentage[0]);
    const int y = screenCoord[1] - round(screenHeight*panPercentage[1]);   
    // do projection
    double worldHCoord[4] = {0,0,0,1};
    double clipHCoord[4] = {
        (x - screenWidth  / 2.0) / (screenWidth  / 2.0),
        (y - screenHeight / 2.0) / (screenHeight / 2.0), z, 1.0};
    imvp->MultiplyPoint(clipHCoord, worldHCoord);
    // check error
    if (worldHCoord[3] == 0) {
        std::cerr << "world coordinates: (" 
                  << worldHCoord[0] << ", " 
                  << worldHCoord[1] << ", " 
                  << worldHCoord[2] << ", " 
                  << worldHCoord[3] << ")" << std::endl
                  << "clip space coordinate: ("
                  << clipHCoord[0] << ", " 
                  << clipHCoord[1] << ", " 
                  << clipHCoord[2] << ", "
                  << clipHCoord[3] << std::endl
                  << "Matrix: " << *imvp << std::endl;
        ospray::Exception("Zero Division During Projection.");
    }    
    // normalize world space coordinate 
    worldCoord[0] = worldHCoord[0]/worldHCoord[3];
    worldCoord[1] = worldHCoord[1]/worldHCoord[3];
    worldCoord[2] = worldHCoord[2]/worldHCoord[3];
}

void
ospray::ProjectScreenToCamera(const int screenCoord[2], const double z,
                              const int screenWidth, const int screenHeight, 
                              vtkMatrix4x4 *imvp, double cameraCoord[3])
{
    // remove panning
    const int x = screenCoord[0];
    const int y = screenCoord[1];
    // do projection
    double cameraHCoord[4] = {0,0,0,1};
    double clipHCoord[4] = {
        (x - screenWidth /2.0)/(screenWidth /2.0),
        (y - screenHeight/2.0)/(screenHeight/2.0), z, 1.0};
    imvp->MultiplyPoint(clipHCoord, cameraHCoord);
    // check error
    if (cameraHCoord[3] == 0) {
        std::cerr << "world coordinates: (" 
                  << cameraHCoord[0] << ", " 
                  << cameraHCoord[1] << ", " 
                  << cameraHCoord[2] << ", " 
                  << cameraHCoord[3] << ")" << std::endl
                  << "clip space coordinate: ("
                  << clipHCoord[0] << ", " 
                  << clipHCoord[1] << ", " 
                  << clipHCoord[2] << ", "
                  << clipHCoord[3] << std::endl
                  << "Matrix: " << *imvp << std::endl;
        ospray::Exception("Zero Division During Projection.");
    }
    // normalize world space coordinate 
    cameraCoord[0] = cameraHCoord[0]/cameraHCoord[3];
    cameraCoord[1] = cameraHCoord[1]/cameraHCoord[3];
    cameraCoord[2] = cameraHCoord[2]/cameraHCoord[3];
}

void
ospray::ProjectWorldToScreenCube(const double cube[6],
                                 const int screenWidth, 
                                 const int screenHeight, 
                                 const double panPercentage[2], 
                                 const double imageZoom,
                                 vtkMatrix4x4 *mvp, 
                                 int screenExtents[4], 
                                 double depthExtents[2])
{
    int xMin = std::numeric_limits<int>::max();
    int xMax = std::numeric_limits<int>::min();
    int yMin = std::numeric_limits<int>::max();
    int yMax = std::numeric_limits<int>::min();
    double zMin = std::numeric_limits<double>::max();
    double zMax = std::numeric_limits<double>::min();

    float coordinates[8][3];
    coordinates[0][0] = cube[0];   
    coordinates[0][1] = cube[2];   
    coordinates[0][2] = cube[4];        

    coordinates[1][0] = cube[1];   
    coordinates[1][1] = cube[2];   
    coordinates[1][2] = cube[4];        

    coordinates[2][0] = cube[1];  
    coordinates[2][1] = cube[3];
    coordinates[2][2] = cube[4];        

    coordinates[3][0] = cube[0]; 
    coordinates[3][1] = cube[3]; 
    coordinates[3][2] = cube[4];

    coordinates[4][0] = cube[0];
    coordinates[4][1] = cube[2];
    coordinates[4][2] = cube[5];

    coordinates[5][0] = cube[1]; 
    coordinates[5][1] = cube[2]; 
    coordinates[5][2] = cube[5];        

    coordinates[6][0] = cube[1]; 
    coordinates[6][1] = cube[3];
    coordinates[6][2] = cube[5];

    coordinates[7][0] = cube[0]; 
    coordinates[7][1] = cube[3]; 
    coordinates[7][2] = cube[5];

    double worldCoord[3];
    int screenCoord[2]; double depth;
    for (int i=0; i<8; i++)
    {
        worldCoord[0] = coordinates[i][0];
        worldCoord[1] = coordinates[i][1];
        worldCoord[2] = coordinates[i][2];
        depth = ProjectWorldToScreen(worldCoord, screenWidth, screenHeight, 
                                     panPercentage, imageZoom, mvp,
                                     screenCoord);
        // clamp values
        screenCoord[0] = CLAMP(screenCoord[0], 0, screenWidth);
        screenCoord[1] = CLAMP(screenCoord[1], 0, screenHeight);
        screenExtents[0] = xMin = std::min(xMin, screenCoord[0]);
        screenExtents[1] = xMax = std::max(xMax, screenCoord[0]);
        screenExtents[2] = yMin = std::min(yMin, screenCoord[1]);
        screenExtents[3] = yMax = std::max(yMax, screenCoord[1]);
        depthExtents[0] = zMin = std::min(zMin, depth);
        depthExtents[1] = zMax = std::max(zMax, depth);
    }
}

void
ospray::CompositeBackground(int screen[2],
                            int compositedImageExtents[4],
                            int compositedImageWidth,
                            int compositedImageHeight,
                            float *compositedImageBuffer,
                            unsigned char *opaqueImageColor,
                            float         *opaqueImageDepth,
                            unsigned char *&imgFinal)
{
    if (UseThreadedBlend_MetaData) {
        visit::CompositeBackground(screen,
                                   compositedImageExtents,
                                   compositedImageWidth,
                                   compositedImageHeight,
                                   compositedImageBuffer,
                                   opaqueImageColor,
                                   opaqueImageDepth,
                                   imgFinal);
        return;
    } 
    for (int y = 0; y < screen[1]; y++)
    {
        for (int x = 0; x < screen[0]; x++)
        {
            int indexScreen     = y * screen[0] + x;
            int indexComposited =
                (y - compositedImageExtents[2]) * compositedImageWidth +
                (x - compositedImageExtents[0]);

            bool insideComposited = 
                ((x >= compositedImageExtents[0] && 
                  x < compositedImageExtents[1]) &&
                 (y >= compositedImageExtents[2] && 
                  y < compositedImageExtents[3]));

            if (insideComposited)
            {
                if (compositedImageBuffer[indexComposited*4 + 3] == 0)
                {
                    // No data from rendering here! - Good
                    imgFinal[indexScreen * 3 + 0] = 
                        opaqueImageColor[indexScreen * 3 + 0];
                    imgFinal[indexScreen * 3 + 1] = 
                        opaqueImageColor[indexScreen * 3 + 1];
                    imgFinal[indexScreen * 3 + 2] = 
                        opaqueImageColor[indexScreen * 3 + 2];
                }
                else
                {
                    // Volume in front
                    float alpha = 
                        (1.0 - compositedImageBuffer[indexComposited * 4 + 3]);
                    imgFinal[indexScreen * 3 + 0] = 
                        CLAMP(opaqueImageColor[indexScreen * 3 + 0] * alpha +
                              compositedImageBuffer[indexComposited * 4 + 0] *
                              255.f,
                              0.f, 255.f);
                    imgFinal[indexScreen * 3 + 1] = 
                        CLAMP(opaqueImageColor[indexScreen * 3 + 1] * alpha +
                              compositedImageBuffer[indexComposited * 4 + 1] *
                              255.f,
                              0.f, 255.f);
                    imgFinal[indexScreen * 3 + 2] =
                        CLAMP(opaqueImageColor[indexScreen * 3 + 2] * alpha +
                              compositedImageBuffer[indexComposited * 4 + 2] *
                              255.f,
                              0.f, 255.f);
                }
            }
            else
            {
                // Outside bounding box: Use the background : Good
                imgFinal[indexScreen * 3 + 0] = 
                    opaqueImageColor[indexScreen * 3 + 0];
                imgFinal[indexScreen * 3 + 1] =
                    opaqueImageColor[indexScreen * 3 + 1];
                imgFinal[indexScreen * 3 + 2] =
                    opaqueImageColor[indexScreen * 3 + 2];
            }
        }
    }
}

void
ospray::WriteArrayToPPM(std::string filename, const float * image,
                        int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x)*4;
            char color[3];
            float alpha = image[index + 3];
            color[0] = CLAMP(image[index + 0]*alpha, 0.0f, 1.0f) * 255;
            color[1] = CLAMP(image[index + 1]*alpha, 0.0f, 1.0f) * 255;
            color[2] = CLAMP(image[index + 2]*alpha, 0.0f, 1.0f) * 255;
            outputFile.write(color,3);
        }
    } 
    outputFile.close();
}

void
ospray::WriteArrayToPPM(std::string filename,  const unsigned char *image, 
                        int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        outputFile.write(reinterpret_cast<const char*>(&image[y * dimX * 3]), 
                         dimX * 3);
    } 
    outputFile.close();
}

void
ospray::WriteArrayGrayToPPM(std::string filename, const float* image, 
                            int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x);
            char var = CLAMP(image[index], 0.f, 1.f) * 255;
            char color[3];
            color[0] = var;
            color[1] = var;
            color[2] = var;
            outputFile.write(color,3);
        }
    } 
    outputFile.close();
}
