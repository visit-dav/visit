/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <avtParallel.h>
#include <avtSoftwareShader.h>
#include <avtSourceFromImage.h>
#include <DebugStream.h>
#include <Engine.h>
#include <UnexpectedValueException.h>
#include <VisWindow.h>

#include <cassert>
#include <GL/ice-t_mpi.h>
#include <mpi.h>
#include <vtkImageData.h>

// ****************************************************************************
// Debugging help.

#ifdef NDEBUG
#   define DEBUG_ONLY(x) /* nothing. */
#else
#   define DEBUG_ONLY(x) x
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
        debug1 << "icet: (rank, procs, tiles): (" << rank << ", " << nproc \
               << ", " << ntiles << ")" << std::endl;                      \
    } while(0)

#ifdef NDEBUG
#   define ICET(x) x
#else
#   define ICET(x)        \
    do {                  \
        x;                \
        ICET_CHECK_ERROR; \
    } while(0)
#endif

// ****************************************************************************
//  Method: gl_rgba_to_gl_rgb
//
//  Purpose: converts GL_RGBA format data to GL_RGB data.  IceT gives us the
//           former; VisIt expects the latter.
//
//  Programmer: Tom Fogal
//  Creation:   June, 2007
//
// ****************************************************************************
static void
gl_rgba_to_gl_rgb(const GLubyte *src, GLubyte *dest, int width, int height)
{
    debug1 << "GL_RGBA (" << width*height*4 << " bytes) -> GL_RGB ("
           << width*height*3 << " bytes)" << std::endl;

    DEBUG_ONLY(bzero(dest, width * height * 3)); /* clear it. */
    for(int i=0; i < width*height; i++)
    {
        memcpy(dest, src, 3);
        dest += 3;
        src += 4;
    }
    /* move the pointer back to its base. */
    dest -= width * height * 3;
}

// IceT render callback.
// IceT needs to control the render; it calls the user render function as
// needed (and multiple times, potentially) -- much like GLUT does rendering.
// So we define a callback function which perform the rendering by grabbing
// our instance (we're a singleton!) and calling the appropriate rendering
// method there (``RealRender'').
extern "C" void render();

// ****************************************************************************
//  Method: IceTNetworkManager default constructor
//
//  Programmer: Tom Fogal
//  Creation:   June 17, 2008
//
// ****************************************************************************
IceTNetworkManager::IceTNetworkManager(void): NetworkManager()
{
    this->comm = icetCreateMPICommunicator(VISIT_MPI_COMM);
    DEBUG_ONLY(ICET_CHECK_ERROR);
    this->context = icetCreateContext(comm);
    DEBUG_ONLY(ICET_CHECK_ERROR);

    ICET(icetSetContext(this->context));

    DEBUG_ONLY(ICET(icetDiagnostics(ICET_DIAG_FULL)));

    ICET(icetStrategy(ICET_STRATEGY_REDUCE));
    ICET(icetDrawFunc(render));

    ICET(icetDisable(ICET_DISPLAY));
    ICET(icetInputOutputBuffers(
            ICET_COLOR_BUFFER_BIT | ICET_DEPTH_BUFFER_BIT, /* inputs */
            ICET_COLOR_BUFFER_BIT | ICET_DEPTH_BUFFER_BIT  /* outputs */
        ));

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
//  Method: IceTNetworkManager destructor
//
//  Purpose: Configures IceT for the tiled system we'll be rendering too (note
//           that a lone monitor is a `1x1 tiled display').
//
//  Programmer: Tom Fogal
//  Creation:   June 17, 2008
//
// ****************************************************************************
void
IceTNetworkManager::TileLayout(size_t width, size_t height) const
{
    debug2 << "IceTNM: configuring " << width << "x" << height
           << " single tile display." << std::endl;

    ICET(icetResetTiles());
    static const int img_stored_on_this_mpi_proc = 0;
    ICET(icetAddTile(0,0, width, height, img_stored_on_this_mpi_proc));
}

// ****************************************************************************
//  Method: Render
//
//  Purpose: Render entry point.  We can't do a real render here because IceT
//           wants to manage our render process.  Here we setup the networks,
//           tell IceT to render, and then read back IceT's result.
//
//  Arguments:
//
//  Programmer: Tom Fogal
//  Creation:   June 20, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue Jun 24 14:32:17 EDT 2008
//    Added depth cueing back in.
//
// ****************************************************************************
avtDataObjectWriter_p
IceTNetworkManager::Render(intVector networkIds, bool getZBuffer,
                           int annotMode, int windowID, bool leftEye)
{
    DataNetwork *origWorkingNet = workingNet;

    EngineVisWinInfo &viswinInfo = viswinMap[windowID];
    viswinInfo.markedForDeletion = false;
    VisWindow *viswin = viswinInfo.viswin;
    WindowAttributes &windowAttributes = viswinInfo.windowAttributes;
    std::vector<avtPlot_p>& imageBasedPlots =
        viswinMap.find(windowID)->second.imageBasedPlots;

    TRY
    {
        this->RenderSetup(networkIds, getZBuffer, annotMode, windowID, leftEye);

        // scalable threshold test (the 0.5 is to add some hysteresus to avoid 
        // the misfortune of oscillating switching of modes around the
        // threshold)
        int scalableThreshold = GetScalableThreshold(windowID);
        if (GetTotalGlobalCellCounts(windowID) < 0.5 * scalableThreshold)
        {
            this->RenderCleanup(windowID);
            CATCH_RETURN2(1, this->CreateNullDataWriter(windowID));
        }

        debug5 << "Rendering " << viswin->GetNumPrimitives()
               << " primitives.  Balanced speedup = "
               << RenderBalance(viswin->GetNumPrimitives()) << "x" << endl;

        int width, height;
        viswin->GetSize(width, height);
        this->TileLayout(width, height);

        //
        // Determine if we need to go for two passes
        //
        bool doShadows = windowAttributes.GetRenderAtts().GetDoShadowing();
        bool doDepthCueing =
            windowAttributes.GetRenderAtts().GetDoDepthCueing();

        // Shadows and depth cueing don't make sense in a non-3D render anyway.
        if(viswin->GetWindowMode() != WINMODE_3D)
        {
            doShadows = false;
            doDepthCueing = false;
        }

        CallInitializeProgressCallback(this->RenderingStages(windowID));

        this->r_mgmt.viewportedMode =
            (this->r_mgmt.annotMode != 1) ||
            (viswin->GetWindowMode() == WINMODE_2D) ||
            (viswin->GetWindowMode() == WINMODE_CURVE) ||
            (viswin->GetWindowMode() == WINMODE_AXISARRAY);

        // IceT mode is different from the standard network manager; we don't
        // need to create any compositor or anything: it's all done under the
        // hood.
        // Whether or not to do multipass rendering (opaque first, translucent
        // second) is all handled in the callback; from our perspective, we
        // just say draw, read back the image, and post-process it.

        ICET(icetDrawFunc(render));
        ICET(icetDrawFrame());
        // The IceT documentation recommends synchronization after rendering.
        MPI_Barrier(VISIT_MPI_COMM);

        // Now we're done rendering, we need to post process the image.
        // However, the image is only there / valid on processor/rank 0!
        // Make sure we don't try to grab it on other processors.
        GLint rank;
        ICET(icetGetIntegerv(ICET_RANK, &rank));
        avtDataObjectWriter_p writer;  // where the PPing output goes.
        if (0 == rank)
        {
            debug3 << "IceTNM: Starting a readback because I am processor 0."
                   << std::endl;
            avtImage_p img = this->Readback(viswin, this->r_mgmt.viewportedMode);

            // Now its essentially back to the same behavior as our parent:
            //  shadows
            //  depth cueing
            //  post processing
            //  creating a D.Obj.writer out of all this

            if (doShadows)
            {
                avtDataObject_p dob;
                CopyTo(dob, img);
                this->RenderShadows(windowID, dob);
                CopyTo(img, dob);
            }

            if (doDepthCueing)
            {
                avtDataObject_p dob;
                CopyTo(dob, img);
                this->RenderDepthCues(windowID, dob);
                CopyTo(img, dob);
            }

            //
            // If the engine is doing more than just 3D annotations,
            // post-process the composited image.
            //
            avtDataObject_p dob;
            CopyTo(dob, img);

            this->RenderPostProcess(imageBasedPlots, dob, windowID);
            CopyTo(img, dob);

            writer = img->InstantiateWriter();
            writer->SetInput(dob);
        }
        else
        {
            // We're not being inaccurate here; we really don't have any
            // relevant data at this point.  IceT only leaves final composited
            // images on tile nodes; the buffers on non-display nodes are
            // invalid.
            debug3 << "IceTNM: telling VisIt I have no data because I "
                   << "am not processor 0." << std::endl;
            avtNullData_p nullData = new avtNullData(NULL,AVT_NULL_IMAGE_MSG);
            avtDataObject_p dummyDob;
            CopyTo(dummyDob, nullData);
            writer = dummyDob->InstantiateWriter();
            writer->SetInput(dummyDob);
            return writer;
        }

        this->RenderCleanup(windowID);

        CATCH_RETURN2(1, writer);
    }
    CATCHALL(...)
    {
        debug5 << "IceTNM::Render exception!" << std::endl;
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RealRender
//
//  Purpose: Code which manages the `in OpenGL' portions of the render.
//
//  Programmer: Tom Fogal
//  Creation:   June 20, 2008
//
// ****************************************************************************
void
IceTNetworkManager::RealRender()
{
    this->RenderGeometry();
    // and translucent ..
}

// ****************************************************************************
//  Method: Readback
//
//  Purpose: Reads back the image buffer from IceT.
//
//  Programmer: Tom Fogal
//  Creation:   June 20, 2008
//
// ****************************************************************************
avtImage_p
IceTNetworkManager::Readback(const VisWindow *viswin, bool viewported) const
{
    assert(viswin);

    {
        GLboolean cbuf;
        ICET(icetGetBooleanv(ICET_COLOR_BUFFER_VALID, &cbuf));
        if(GL_FALSE == cbuf)
        {
            debug1 << "IceTNM: Readback of empty color buffer!" << std::endl;
            return NULL;
        }
    }

    int width=-42, height=-42;
    viswin->GetSize(width, height);
    assert(width > 0 && height > 0);

    GLubyte *pixels = icetGetColorBuffer();
    DEBUG_ONLY(ICET_CHECK_ERROR);

    {
        GLint color_format;
        ICET(icetGetIntegerv(ICET_COLOR_FORMAT, &color_format));
        if(color_format != GL_RGBA) {
            const char *str;
            switch(color_format) {
                case GL_RGB: str = "GL_RGB"; break;
                case GL_BGR: str = "GL_BGR"; break;
                case GL_BGRA: str = "GL_BGRA"; break;
                default: str = "unexpected error case"; break;
            }
            EXCEPTION2(UnexpectedValueException, "GL_RGBA", std::string(str));
        }
    }

    vtkImageData *image = avtImageRepresentation::NewImage(width, height);
    {
        // We have the pixel data, but it's GL_RGBA!  Our caller is expecting
        // GL_RGB, so we need to convert the data.
        unsigned char *img_pix = static_cast<unsigned char *>
                                            (image->GetScalarPointer(0,0,0));
        gl_rgba_to_gl_rgb(pixels, img_pix, width, height);
    }

    avtSourceFromImage screenCapSrc(image, NULL);
    avtImage_p visit_img = screenCapSrc.GetTypedOutput();
    visit_img->Update(screenCapSrc.GetGeneralContract());
    visit_img->SetSource(NULL);
    image->Delete();

    return visit_img;
}

// ****************************************************************************
//  Method: Shadows
//
//  Purpose: Adds shadows to a scene.  Only meant to be called by a tile node.
//
//  Programmer: Tom Fogal
//  Creation:   June 25, 2008
//
// ****************************************************************************
void
IceTNetworkManager::RenderShadows(int windowID,
                                  avtDataObject_p& input_as_dob) const
{
    VisWindow *viswin = viswinMap.find(windowID)->second.viswin;

    CallProgressCallback("NetworkManager", "Creating shadows",0,1);
    avtView3D cur_view = viswin->GetView3D();

    //
    // Figure out which direction the light is pointing.
    //
    const LightList *light_list = viswin->GetLightList();
    const LightAttributes &la = light_list->GetLight0();
    double light_dir[3];
    bool canShade = avtSoftwareShader::GetLightDirection(la, cur_view,
                                                         light_dir);

    /// TJF -- note to self -- need to find another way to do shadows!
    debug1 << "Shadowing with the IceT renderer is not currently supported!"
           << std::endl;
}

// ****************************************************************************
//  Method: RenderPostProcess
//
//  Purpose: Post-Rendering plots to apply.  These are typically 2D overlays.
//
//  Programmer: Tom Fogal
//  Creation:   June 25, 2008
//
// ****************************************************************************
void
IceTNetworkManager::RenderPostProcess(std::vector<avtPlot_p>& image_plots,
                                      avtDataObject_p& input_as_dob,
                                      int windowID) const
{
    const WindowAttributes &windowAttributes =
        viswinMap.find(windowID)->second.windowAttributes;
    VisWindow *viswin = viswinMap.find(windowID)->second.viswin;

    if(!image_plots.empty())
    {
        avtImage_p compositedImage;
        CopyTo(compositedImage, input_as_dob);

        for(std::vector<avtPlot_p>::iterator plot = image_plots.begin();
            plot != image_plots.end();
            ++plot)
        {
            avtImage_p newImage = (*plot)->ImageExecute(compositedImage,
                                                        windowAttributes);
            compositedImage = newImage;
        }
        CopyTo(input_as_dob, compositedImage);
    }
}

// ****************************************************************************
//  Method: render
//
//  Purpose: IceT render callback.  IceT operates like GLUT: it controls the
//           rendering process.  This is the method we'll give to IceT which
//           tells it to render.
//           Note this function MUST have C linkage!
//
//  Programmer: Tom Fogal
//  Creation:   June 19, 2008
//
// ****************************************************************************
extern "C" void
render()
{
    debug2 << "IceT has invoked our render function." << std::endl;
    Engine *engy = Engine::Instance();
    IceTNetworkManager *net_mgr;

    net_mgr = dynamic_cast<IceTNetworkManager*>(engy->GetNetMgr());
    net_mgr->RealRender();
}
