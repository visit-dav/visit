/*****************************************************************************
*
* Copyright (c) 2000 - 2016, Lawrence Livermore National Security, LLC
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
//                            IceTNetworkManager.C                           //
// ************************************************************************* //

#include "IceTNetworkManager.h"
#include <avtCallback.h>
#include <avtDebugDumpOptions.h>
#include <avtParallel.h>
#include <avtSoftwareShader.h>
#include <avtSourceFromImage.h>
#include <avtTransparencyActor.h>
#include <DebugStream.h>
#include <DataNetwork.h>
#include <Engine.h>
#include <snprintf.h>
#include <StackTimer.h>
#include <UnexpectedValueException.h>
#include <VisWindow.h>

#include <IceT.h>
#include <IceTMPI.h>
#include <mpi.h>
#include <vtkImageData.h>

//Rank IceT send the composited image to
#define ICET_COLLECTION_RANK 0
// ****************************************************************************
// Debugging help.

#ifdef DEBUG_ICET
#   define DEBUG_ONLY(x) x
#else
#   define DEBUG_ONLY(x) /* nothing. */
#endif

#define ICET_CHECK_ERROR                                                    \
    do {                                                                    \
        GLenum err = icetGetError();                                        \
        const char *es = NULL;                                              \
        switch(err) {                                                       \
            case ICET_NO_ERROR: es = "no error"; break;                     \
            case ICET_SANITY_CHECK_FAIL: es = "sanity check failed"; break; \
            case ICET_INVALID_ENUM: es = "invalid enum"; break;             \
            case ICET_BAD_CAST: es = "bad cast"; break;                     \
            case ICET_OUT_OF_MEMORY: es = "out of memory"; break;           \
            case ICET_INVALID_OPERATION: es = "invalid operation"; break;   \
            case ICET_INVALID_VALUE: es = "invalid value"; break;           \
        }                                                                   \
        if(err != ICET_NO_ERROR) {                                          \
            debug1 << "[ICET_ERROR] " << es << std::endl;                   \
        }                                                                   \
    } while(0)

#define PR_ICET_MPI                                                        \
    do {                                                                   \
        GLint rank, nproc, ntiles;                                         \
        ICET(icetGetIntegerv(ICET_RANK, &rank));                           \
        ICET(icetGetIntegerv(ICET_NUM_PROCESSES, &nproc));                 \
        ICET(icetGetIntegerv(ICET_NUM_TILES, &ntiles));                    \
        debug1 << "IceT: (rank, procs, tiles): (" << rank << ", " << nproc \
                 << ", " << ntiles << ")" << std::endl;                      \
      } while(0)

#ifdef DEBUG_ICET
#   define ICET(stmt)     \
      do {                  \
          stmt;             \
          ICET_CHECK_ERROR; \
      } while(0)
#else
#   define ICET(stmt) stmt
#endif

// ****************************************************************************
//  Function: SendImageToRenderNodes
//
//  Purpose: IceT distinguishes between `tile nodes' and `render nodes'.  All
//           nodes assist in image composition, but final images are only
//           collected on tile nodes.
//           Unfortunately various post-rendering algorithms in VisIt do not
//           make this distinction, which in IceT parlance means we assume all
//           nodes are tile nodes.  This method should send out the image to
//           all other nodes, so we can still utilize IceT by blurring that
//           distinction so that the rest of VisIt never knows.
//
//  Programmer: Tom Fogal
//  Creation:   August 7, 2008
//
//  Modifications:
//    Matt Larsen, Mon May 9, 2016 15:34:01 PDT
//    Removed un-used variables and changed types from GL to normal
// ****************************************************************************
static void
SendImageToRenderNodes(int width, int height, bool Z,
                       unsigned char * const pixels,
                       float * const depth)
{
    MPI_Bcast(pixels, 
              4*width*height, 
              MPI_BYTE, 
              ICET_COLLECTION_RANK, 
              VISIT_MPI_COMM);
    if(Z) 
    {
        MPI_Bcast(depth, 
                  width*height, 
                  MPI_FLOAT, 
                  ICET_COLLECTION_RANK, 
                  VISIT_MPI_COMM);
    }
}

  // ****************************************************************************
  //  Method: IceTNetworkManager default constructor
  //
  //  Programmer: Tom Fogal
  //  Creation:   June 17, 2008
  //
  //  Modifications:
  //
  //    Tom Fogal, Wed May 18 11:57:34 MDT 2011
  //    Initialize 'renderings'.
  //
  //    Matt Larsen, Mon May 9, 2016 08:15:54 PDT 2016
  //    Updated some IceT calls
  // ****************************************************************************
  IceTNetworkManager::IceTNetworkManager(void): NetworkManager(), renderings(0)
  {
      this->comm = icetCreateMPICommunicator(VISIT_MPI_COMM);
      DEBUG_ONLY(ICET_CHECK_ERROR);
      this->context = icetCreateContext(comm);
      DEBUG_ONLY(ICET_CHECK_ERROR);

      ICET(icetSetContext(this->context));

      DEBUG_ONLY(ICET(icetDiagnostics(ICET_DIAG_FULL)));
  
      ICET(icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE));
      ICET(icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT));

      DEBUG_ONLY(PR_ICET_MPI);
  }

  // ****************************************************************************
  //  Method: IceTNetworkManager destructor
  //
  //  Programmer: Tom Fogal
  //  Creation:   June 17, 2008
  //
  // ****************************************************************************
  IceTNetworkManager::~IceTNetworkManager(void)
  {
      ICET(icetDestroyContext(this->context));
      ICET(icetDestroyMPICommunicator(this->comm));
  }

// ****************************************************************************
//  Method: IceTNetworkManager TileLayout
//
//  Purpose: Configures IceT for the tiled system we'll be rendering too (note
//           that a lone monitor is a `1x1 tiled display').
//
//  Programmer: Tom Fogal
//  Creation:   June 17, 2008
//
//  Modifications: 
//
//    Matt Larsen, Mon May 9, 2016 08:15:54 PDT
//    Modified to use collection rank
// ****************************************************************************
void
IceTNetworkManager::TileLayout(size_t width, size_t height) const
{
    debug2 << "IceTNM: configuring " << width << "x" << height
           << " single tile display." << std::endl;

    ICET(icetResetTiles());
    ICET(icetAddTile(0,0, width, height, ICET_COLLECTION_RANK));
}

// ****************************************************************************
//  Method: IceTNetworkManager IceTSetup
//
//  Purpose: Setup and configure IceT based on the current rendering state. 
//           Moved code into this fuction as part of updating and refactoring.
//
//  Programmer: Matt Larsen
//  Creation:   May 9th, 2016
//
// ****************************************************************************
void
IceTNetworkManager::IceTSetup()
{
    EngineVisWinInfo &viswinInfo = viswinMap[renderState.windowID];
    VisWindow *viswin = viswinInfo.viswin;

    // If there is a backdrop image, we need to tell IceT so that it can
    // composite correctly.
    if(viswin->GetBackgroundMode() != AnnotationAttributes::Solid)
    {
        ICET(icetEnable(ICET_CORRECT_COLORED_BACKGROUND));
    }
    else
    {
        ICET(icetDisable(ICET_CORRECT_COLORED_BACKGROUND));
    }

    debug5 << "Rendering " << viswin->GetNumPrimitives()
           << " primitives." << endl;

    int width, height, width_start, height_start;
    // This basically gets the width and the height.
    // The distinction is for 2D rendering, where we only want the
    // width and the height of the viewport.
    viswin->GetCaptureRegion(width_start, height_start, width, height,
                             renderState.viewportedMode);
    this->TileLayout(width, height);

    //reset IceT to the defualt state
    ICET(icetStrategy(ICET_STRATEGY_SEQUENTIAL)); 
    ICET(icetSingleImageStrategy(ICET_SINGLE_IMAGE_STRATEGY_AUTOMATIC));

    ICET(icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE));
    ICET(icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT));
    ICET(icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER));
    ICET(icetEnable(ICET_COMPOSITE_ONE_BUFFER));
    bool isSupportedBackground =  (renderState.window->GetBackgroundMode() == 
                                    AnnotationAttributes::Solid);
    if(renderState.getZBuffer || !isSupportedBackground)
    {
        //By default IceT will not return the zbuffer to
        //the final image. We must call this to "enable"
        //the zbuffer
        ICET(icetDisable(ICET_COMPOSITE_ONE_BUFFER));
    }


    CallInitializeProgressCallback(this->RenderingStages());

    // IceT sometimes omits large parts of Curve plots when using the
    // REDUCE strategy. Use a different compositing strategy for Curve
    // plots to avoid the problem.
    if(viswin->GetWindowMode() == WINMODE_CURVE)
    {
        //It appears as if the curve plots do not have zbuffer information
        //when rendered. Thus, we have to switch the compositing mode to 
        //blend, otherwise the results are blown away. With the tests
        //so far, only rank 0 has any image and this needs to be tested.
        ICET(icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE));
        ICET(icetCompositeMode(ICET_COMPOSITE_MODE_BLEND));
    }
}

// ****************************************************************************
//  Method: IceTNetworkManager UnpackBuffer
//
//  Purpose: Function for converting from RGB to RGBA. IceT always needs RGBA
//           pixel data and the image represention does not always have an
//           alpha channel
//
//  Programmer: Matt Larsen
//  Creation:   May 9th, 2016
//
// ****************************************************************************
void
IceTNetworkManager::UnpackBuffer(const unsigned char *rgb,
                                 const int &numPixels)
{ 
    if(numPixels*4 != rgba.size()) 
        rgba.resize(numPixels*4);
    
    for(int i = 0; i < numPixels; ++i)
    {
         rgba[i*4+0] = rgb[i*3+0];
         rgba[i*4+1] = rgb[i*3+1];
         rgba[i*4+2] = rgb[i*3+2];
         rgba[i*4+3] = 255;
    }
}
// ****************************************************************************
//  Method: IceTNetworkManager PackBuffer
//
//  Purpose: Function for converting from RGB to RGBA. IceT always needs RGBA
//           pixel data and the image represention does not always have an
//           alpha channel
//
//  Programmer: Matt Larsen
//  Creation:   May 9th, 2016
//
// ****************************************************************************

void
IceTNetworkManager::PackBuffer(unsigned char *rgb,
                               const int &numPixels)
{ 
    for(int i = 0; i < numPixels; ++i)
    {
         rgb[i*3+0] = rgba[i*4+0];
         rgb[i*3+1] = rgba[i*4+1];
         rgb[i*3+2] = rgba[i*4+2];
    }
}
// ****************************************************************************
//  Method: NetworkManager::RenderInternal
//
//  Purpose: do the actual rendering and compositing work. this was
//          originally lumped together with setup/tear down. I factored
//          it out so that the setup/tear down was not done twice when
//          IceTNetworkManager called it.
//
//  Programmer:  Burlen Loring
//  Creation:    Thu Sep  3 10:26:48 PDT 2015
//
//  Modifications:
//    Matt Larsen Fri May 6 08:22:21 PDT 2016 
//    Matching refactor with main Network manager to share as much code as 
//    possible.
//
// ****************************************************************************

avtDataObject_p
IceTNetworkManager::RenderInternal()
{
    CallInitializeProgressCallback(RenderingStages());

    // ************************************************************
    // pass 1a : opaque (and translucent geometry if serial)
    // ************************************************************
    avtImage_p pass = RenderGeometry();

    // ************************************************************
    // pass 1b : shadow mapping
    // ************************************************************
    if (renderState.shadowMap)
        NetworkManager::RenderShadows(pass);

    // ************************************************************
    // pass 1c : depth cues
    // ************************************************************
    if (renderState.depthCues)
        NetworkManager::RenderDepthCues(pass);


    // ************************************************************
    // pass 2 : translucent geometry if parallel
    // ************************************************************
    if (renderState.transparencyInPass2)
        pass = NetworkManager::RenderTranslucent(pass);

    // ************************************************************
    // pass 3 : 2d overlays
    // ************************************************************
    RenderPostProcess(pass);

    avtDataObject_p output;
    CopyTo(output, pass);

    return output;
}


// ****************************************************************************
//  Method: RenderGeometry
//
//  Purpose: Renders the geometry for a scene; this is always the opaque
//           objects, and may or may not include translucent objects (depending
//           on the current multipass rendering settings).
//           We override this method because we can avoid a readback in the
//           one-pass case (we'll read it back from IceT later anyway).
//
//  Programmer: Tom Fogal
//  Creation:   July 26, 2008
//
//  Modifications:
//
//    Tom Fogal, Mon Jul 28 14:57:01 EDT 2008
//    Need to return NULL in the single-pass case!
//
//    Burlen Loring, Tue Sep  1 14:26:30 PDT 2015
//    sync up with network manager(base class) order compositing refactor
//
//    Matt Larsen, Tue May 10 08:26:54 PDT 2015
//    Refactoring and updating to newer version of IceT. Fix: broadcasting
//    image to all ranks only when needed. Fix: now works with translucency
//    in second pass. Fix: reduce the amount of peak memory usage by
//    eliminating extra image copies.
// ****************************************************************************
avtImage_p
IceTNetworkManager::RenderGeometry()
{
    StackTimer t0("IceTNetworkManager::RenderGeometry");
    CallInitializeProgressCallback(this->RenderingStages());
    
    
    if (renderState.transparencyInPass1)
        return NetworkManager::RenderGeometry();

    this->IceTSetup();
   
    //figure out if we are using a zbuffer composite
    IceTEnum depthFormat;
    ICET(icetGetEnumv(ICET_DEPTH_FORMAT,&depthFormat));
    bool usingZ = depthFormat == ICET_IMAGE_DEPTH_FLOAT; 
    
    VisWindow *viswin = renderState.window; 
    
    //find out if we are expecting an alpha channel in the image
    //and should return one.
    bool needsAlpha = renderState.orderComposite;
    
   
    double bgColorBackup[3] = {0.0};
    AnnotationAttributes::BackgroundMode bgMode = AnnotationAttributes::Solid;
    
    //Setup some state for translucency
    if(needsAlpha)
    {
        viswin->EnableAlphaChannel();

        bgMode = viswin->GetBackgroundMode();
        viswin->SetBackgroundMode(AnnotationAttributes::Solid);

        memcpy(bgColorBackup, viswin->GetBackgroundColor(), 3*sizeof(double));
        viswin->SetBackgroundColor(0.0, 0.0, 0.0);
    }
    
    avtImage_p img;
    
    CallProgressCallback("IceTNetworkManager", "Render geometry", 0, 1);
    viswin->ScreenRender(renderState.viewportedMode,
        /*disbale fg=*/true, /*opaque on=*/true,
        /*translucent on=*/false,
        /*disable bg=*/false, /*input image=*/NULL);

    img = viswin->ScreenReadBack(renderState.viewportedMode,
                      /*read z=*/usingZ, /*read a=*/needsAlpha);

    CallProgressCallback("IceTNetworkManager", "Render geometry", 0, 1);
    
    
    //restore some state for transluceny
    if(needsAlpha)
    {
        viswin->DisableAlphaChannel();
        viswin->SetBackgroundMode(bgMode);
        viswin->SetBackgroundColor(bgColorBackup[0], 
                                   bgColorBackup[1], 
                                   bgColorBackup[2]);
    }
    
    int height = -1;
    int width = -1;

    img->GetImage().GetSize(&width, &height);
    int numChannels = img->GetImage().GetNumberOfColorChannels();
    debug5 <<"IceTNetworkManager: Image has " << numChannels << "channels\n";
    const int num_pixels = width * height; 
    unsigned char *pixels = NULL;
    float *zbuffer = NULL;
    
    // We need to put RGB values into a RGBA format
    // IcetCompositeImage uses a shallow copy
    // of the pre-rendered image, so we cannot delete
    // the pointers. We will just keep them around, and 
    // delete them at the end.
        
    unsigned char *imgPixels = img->GetImage().GetRGBBuffer(); 
    if(!needsAlpha)
    {
        //this is a rgb buffer and we need to unpack it
        this->UnpackBuffer(imgPixels,num_pixels);
        pixels = &rgba[0];
    }
    else
    { 
        pixels = imgPixels;   
    }
    
    if(usingZ)
    {
        zbuffer = img->GetImage().GetZBuffer(); 
    }
       
    //extract the background for IceT
    float bgColor[4];
    if(renderState.window->GetBackgroundMode() == AnnotationAttributes::Solid)
    {
        const double *bgDouble = renderState.window->GetBackgroundColor();
        for(int i = 0; i < 3; ++i) bgColor[i] = bgDouble[i];
        bgColor[3] = 1.f;
    }
    else 
    {  
        for(int i = 0; i < 4; ++i) bgColor[i] = 1.f;
        bgColor[3] = 0.f;
    }
    
    IceTImage compositedImg = ICET(icetCompositeImage(pixels,
                                                      zbuffer,
                             /*active pixel viewport*/NULL, 
                                        /*projMatrix*/NULL,
                                   /*modelViewMatrix*/NULL,
                                                      bgColor));
                                                 
    

    //in the current config, only one rank gets the composited image
    int rank;
    ICET(icetGetIntegerv(ICET_RANK, &rank));
    bool haveImage = rank == ICET_COLLECTION_RANK;  

    //Check to see if we need the depth buffer back
    //This should only set if we need it, otherwise,
    //Icet will complain about the depth format
    IceTBoolean noDepthBuffer;
    ICET(icetGetBooleanv(ICET_COMPOSITE_ONE_BUFFER, &noDepthBuffer));
    bool returnDepth = renderState.getZBuffer;
    if(haveImage)
    {
        // IceT does not support gradient or image based 
        // backgrounds.This is a *hack* to get IceT to retun a
        // the propper background. There are probably cases
        // where this breaks.
        if(renderState.window->GetBackgroundMode() 
            != AnnotationAttributes::Solid 
            && usingZ)
        {
            unsigned char * comp_pixels = NULL;
            float * comp_depth = NULL;
            comp_pixels = icetImageGetColorub(compositedImg);
            comp_depth = icetImageGetDepthf(compositedImg);
            for(int i = 0; i < num_pixels; ++i)
            { 
              //background is at depth 1.f
              if(comp_depth[i] != 1.f)
              {
                pixels[i*4+0] = comp_pixels[i*4+0];
                pixels[i*4+1] = comp_pixels[i*4+1];
                pixels[i*4+2] = comp_pixels[i*4+2];
                pixels[i*4+3] = comp_pixels[i*4+3];
                
              }
              if(returnDepth) zbuffer[i] = comp_depth[i];
            }
        }
        else
        {
            //Copy back the final image so we can send it to all processors.
            //Still unclear why this needs to happen, but I will continue the
            //tradition.
            ICET(icetImageCopyColorub(compositedImg,
                                      pixels,
                                      ICET_IMAGE_COLOR_RGBA_UBYTE));
        }
        if(returnDepth)
        {
            ICET(icetImageCopyDepthf(compositedImg,
                                     zbuffer,
                                     ICET_IMAGE_DEPTH_FLOAT));
        }
    }
    
    //Need to distribute the composited image back to all processes.
    //Note: there is a way to leave the composited peices on all
    //procs and reduce the gather comm.
    if(renderState.allReducePass1)
        SendImageToRenderNodes(width, height, returnDepth, &rgba[0], zbuffer);

    //Now that all nodes have the image, we need pack it
    //backing into rgb format if that is what we received.
    if(!needsAlpha)
    {
        this->PackBuffer(imgPixels, num_pixels);
    }
    if(avtDebugDumpOptions::DumpEnabled())
        this->DumpImage(img, "icet-render-geom");


    this->renderings++;
 
    return img;
}

// ****************************************************************************
//  Method: RenderTranslucent
//
//  Purpose: Renders translucent geometry within a VisWindow.
//           Expects that opaque geometry has already been rendered.  In the
//           IceT case, our work is a lot simpler because we don't need to
//           read the image back from the framebuffer (we'll read it back from
//           IceT later anyway).
//
//  Programmer: Tom Fogal
//  Creation:   August 4, 2008
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 14:26:30 PDT 2015
//    sync up with network manager(base class) order compositing refactor
//
//    Burlen Loring, Thu Oct  8 16:02:32 PDT 2015
//    fix a warning.
//
//    Burlen Loring, Sat Oct 17 07:54:42 PDT 2015
//    temporarily remove the unused methods because they are creating
//    compiler warnings.
//
// ****************************************************************************

/*avtImage_p
IceTNetworkManager::RenderTranslucent(int windowID, const avtImage_p& input)
{
    (void) windowID;

    CallProgressCallback("IceTNetworkManager", "Transparent rendering", 0, 1);
    VisWindow *viswin = renderState.window;
    {
        StackTimer second_pass("Second-pass screen capture for SR");

        // We have to disable any gradient background before
        // rendering, as those will overwrite the first pass
        AnnotationAttributes::BackgroundMode bm = viswin->GetBackgroundMode();
        viswin->SetBackgroundMode(AnnotationAttributes::Solid);

        viswin->ScreenRender(
            renderState.viewportedMode,
            /need z=/true, /opaque on=/false,
            /translucent on=/true, /disable bg=/false,
            input);

        // Restore the background mode for next time
        viswin->SetBackgroundMode(bm);
    }
    CallProgressCallback("IceTNetworkManager", "Transparent rendering", 1, 1);

    // In this implementation, the user should never use the return value --
    // read it back from IceT instead!
    return NULL;
}*/

// ****************************************************************************
//  Method: StopTimer
//
//  Purpose: Time the IceT rendering process.
//           IceT includes its own timing code that we might consider using at
//           some point ...
//
//  Programmer: Tom Fogal
//  Creation:   July 14, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jan 15 11:07:53 CST 2009
//    Changed GetSize call to GetCaptureRegion, since that works for 2D.
//
//    Tom Fogal, Fri May 29 20:37:59 MDT 2009
//    Remove an unused variable.
//
//    Burlen Loring, Tue Sep  1 14:26:30 PDT 2015
//    sync up with network manager(base class) order compositing refactor
//
// ****************************************************************************
void
IceTNetworkManager::StopTimer()
{
    char msg[1024];
    VisWindow *viswin = renderState.window;
    int rows,cols, width_start, height_start;
    // This basically gets the width and the height.
    // The distinction is for 2D rendering, where we only want the
    // width and the height of the viewport.
    viswin->GetCaptureRegion(width_start, height_start, rows,cols,
                             renderState.viewportedMode);

    SNPRINTF(msg, 1023, "IceTNM::Render %lld cells %d pixels",
             renderState.cellCountTotal, rows*cols);
    visitTimer->StopTimer(renderState.timer, msg);
    renderState.timer = -1;
}

// ****************************************************************************
//  Method: FormatDebugImage
//
//  Purpose: Figure out a name for our debug image.  This is complicated in the
//           IceT case, because IceT can call our render function as much as it
//           wants.
//
//  Programmer: Tom Fogal
//  Creation:   May 18, 2011
//
// ****************************************************************************
void IceTNetworkManager::FormatDebugImage(char* out, size_t len,
                                          const char* prefix) const
{
  SNPRINTF(out, len, "%s-%03d-%03u", prefix, PAR_Rank(), this->renderings);
}

// ****************************************************************************
//  Method: VerifyColorFormat
//
//  Purpose: We currently expect GL_RGBA from IceT.  If it gives us something
//           different, we should bail out.
//
//  Programmer: Tom Fogal
//  Creation:   July 1, 2008
//
//  Modifications:
//
//    Matt Larsen, Tues May 10, 2016 09:15:03 PDT
//    Updated to newest version of IceT
//
// ****************************************************************************
void
IceTNetworkManager::VerifyColorFormat(IceTImage image) const
{
    IceTEnum color_format;
    ICET(color_format = icetImageGetColorFormat(image));
    if(color_format != ICET_IMAGE_COLOR_RGBA_UBYTE)
    {
        const char *str;
        switch(color_format)
        {
            case ICET_IMAGE_COLOR_RGBA_FLOAT: str = "ICET_RGBA_FLOAT"; break;
            case ICET_IMAGE_COLOR_NONE: str = "ICET_NONE"; break;
            default: str = "unexpected error case"; break;
        }
        EXCEPTION2(UnexpectedValueException, "ICET_RGBA", std::string(str));
    }
    //TODO:mark
    
}

// ****************************************************************************
//  Method: DebugIceTTimings
//
//  Purpose: To print interal IceT timing for debugging.
//
//  Programmer: Matt Larsen
//  Creation:   May 10, 2016
//
// ****************************************************************************
void
IceTNetworkManager::DebugIceTTimings() const
{
    int rank;
    ICET(icetGetIntegerv(ICET_RANK, &rank));
    if(rank == 0)
    {
        //Time spent reading and copying buffer data
        double bufferReadTime = 0;
        ICET(icetGetDoublev(ICET_BUFFER_READ_TIME, &bufferReadTime));
        //Time spent writing to buffers
        double bufferWriteTime = 0;
        ICET(icetGetDoublev(ICET_BUFFER_WRITE_TIME, &bufferWriteTime));
        //Time spent blending or z-compositing
        double blendTime = 0;
        ICET(icetGetDoublev(ICET_BLEND_TIME, &blendTime));
        //Total composite time
        double totCompTime = 0;
        ICET(icetGetDoublev(ICET_COMPOSITE_TIME, &totCompTime));
        //Time spent in drawing callback
        double callBackTime = 0;
        ICET(icetGetDoublev(ICET_BUFFER_READ_TIME, &callBackTime));
        //total bytes sent last composite
        int bytesSent = 0;
        ICET(icetGetIntegerv(ICET_BYTES_SENT, &bytesSent));
        
        debug3<<"-----ICET stats for rank "<<rank<<" -------\n"
              <<"Total Composite Time       : "<<totCompTime<<"\n"
              <<"Call Back Time             : "<<callBackTime<<"\n"
              <<"Blend time                 : "<<blendTime<<"\n"
              <<"Buffer Write Time          : "<<bufferWriteTime<<"\n"
              <<"Buffer Read Time           : "<<bufferReadTime<<"\n"
              <<"Total Bytes sent           : "<<bytesSent<<"\n"
              <<"--------------------------------------\n";
    }
}
