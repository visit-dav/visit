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
#include <snprintf.h>
#include <StackTimer.h>
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
        debug1 << "IceT: (rank, procs, tiles): (" << rank << ", " << nproc \
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
//  Method: lerp
//
//  Purpose: Linearly interpolates a value from one range to another.
//
//  Programmer: Tom Fogal
//  Creation:   July 17, 2008
//
// ****************************************************************************
template <typename in, typename out>
static inline out
lerp(in value, in imin, in imax, out omin, out omax)
{
    return omin + (value-imin) * (static_cast<float>(omax-omin) / (imax-imin));
}

// ****************************************************************************
//  Method: utofv
//
//  Purpose: Converts a vector of unsigned integers to a vector of floats.  The
//           returned buffer is dynamically allocated, and should be delete[]d
//           by the caller.
//
//  Programmer: Tom Fogal
//  Creation:   July 2, 2008
//
//  Modifications:
//
//    Tom Fogal, Thu Jul 17 10:27:43 EDT 2008
//    lerp the IceT buffer onto the range [0,1] as we convert.  This seems to
//    be what the rest of VisIt expects.
//
// ****************************************************************************
static float *
utofv(const unsigned int * const src, size_t n_elem)
{
    float *res = new float[n_elem];
    for(size_t i=0; i < n_elem; ++i) {
        res[i] = lerp(src[i], 0U,UINT_MAX, 0.0f,1.0f);
    }
    return res;
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
//  Modifications:
//
//    Tom Fogal, Thu Jul 17 14:51:09 EDT 2008
//    Configure every process to have a tile.  This lets us rely on every
//    process having an image, later.
//
// ****************************************************************************
void
IceTNetworkManager::TileLayout(size_t width, size_t height) const
{
    GLint n_proc;
    debug2 << "IceTNM: configuring " << width << "x" << height
           << " single tile display." << std::endl;

    ICET(icetResetTiles());
    ICET(icetGetIntegerv(ICET_NUM_PROCESSES, &n_proc));

    for(GLint proc=0; proc < n_proc; ++proc) {
        debug3 << "IceTNM: adding " << proc << "/" << n_proc-1 << std::endl;
        ICET(icetAddTile(0,0, width, height, proc));
    }
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
//    Tom Fogal, Fri Jul 11 19:53:03 PDT 2008
//    Added timer analogous to parent's overall render timer.
//
//    Tom Fogal, Fri Jul 18 17:32:31 EDT 2008
//    Query parent's implementation for rendering features.
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
    std::vector<avtPlot_p>& imageBasedPlots = viswinInfo.imageBasedPlots;

    TRY
    {
        this->StartTimer();
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

        CallInitializeProgressCallback(this->RenderingStages(windowID));

        // IceT mode is different from the standard network manager; we don't
        // need to create any compositor or anything: it's all done under the
        // hood.
        // Whether or not to do multipass rendering (opaque first, translucent
        // second) is all handled in the callback; from our perspective, we
        // just say draw, read back the image, and post-process it.

        ICET(icetEnable(ICET_CORRECT_COLORED_BACKGROUND));
        ICET(icetDrawFunc(render));
        ICET(icetDrawFrame());
        // The IceT documentation recommends synchronization after rendering.
        MPI_Barrier(VISIT_MPI_COMM);

        // Now that we're done rendering, we need to post process the image.
        debug3 << "IceTNM: Starting readback." << std::endl;
        avtDataObject_p dob;
        {
            bool needZB;
            needZB = (viswin->GetWindowMode() == WINMODE_3D ||
                      viswin->GetBackgroundMode() != 0)     &&
                      (this->r_mgmt.getZBuffer          ||
                       this->MultipassRendering(viswin) ||
                       this->Shadowing(windowID)        ||
                       this->DepthCueing(windowID)      ||
                       !(imageBasedPlots.empty()));
            avtImage_p img = this->Readback(viswin, needZB);
            CopyTo(dob, img);
        }

        // Now its essentially back to the same behavior as our parent:
        //  shadows
        //  depth cueing
        //  post processing
        //  creating a D.Obj.writer out of all this

        if (this->Shadowing(windowID))
        {
            this->RenderShadows(windowID, dob);
        }

        if (this->DepthCueing(windowID))
        {
            this->RenderDepthCues(windowID, dob);
        }

        //
        // If the engine is doing more than just 3D annotations,
        // post-process the composited image.
        //
        this->RenderPostProcess(imageBasedPlots, dob, windowID);

        avtDataObjectWriter_p writer;  // where the PPing output goes.
        writer = dob->InstantiateWriter();
        writer->SetInput(dob);

        this->RenderCleanup(windowID);

        CATCH_RETURN2(1, writer);
    }
    CATCHALL(...)
    {
        debug5 << "IceTNM::Render exception!" << std::endl;
        assert("Exception thrown, bailing out" == (const char*)0x0fa1afe1);
        RETHROW;
    }
    ENDTRY

    workingNet = origWorkingNet;
}

// ****************************************************************************
//  Method: RealRender
//
//  Purpose: Code which manages the `in OpenGL' portions of the render.
//
//  Programmer: Tom Fogal
//  Creation:   June 20, 2008
//
//  Modifications:
//
//    Tom Fogal, Mon Jun 30 16:17:43 EDT 2008
//    Support multipass rendering.
//
//    Tom Fogal, Sat Jul 26 23:15:21 EDT 2008
//    Don't bother copying back the image; there's no way to get that image
//    anyway.  If it doesn't render into the framebuffer, it might as well not
//    happen...
//
// ****************************************************************************

void
IceTNetworkManager::RealRender()
{
    avtImage_p dob = this->RenderGeometry();
    VisWindow *viswin =
        this->viswinMap.find(this->r_mgmt.windowID)->second.viswin;
    if(this->MultipassRendering(viswin))
    {
        avtDataObject_p i_as_dob;
        i_as_dob = this->RenderTranslucent(this->r_mgmt.windowID, dob);
    }
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
// ****************************************************************************
avtImage_p
IceTNetworkManager::RenderGeometry()
{
    VisWindow *viswin = viswinMap.find(this->r_mgmt.windowID)->second.viswin;
    if(this->MultipassRendering(viswin))
    {
        return NetworkManager::RenderGeometry();
    }
    CallProgressCallback("NetworkManager", "Render geometry", 0, 1);
        viswin->ScreenRender(this->r_mgmt.viewportedMode, true);
    CallProgressCallback("NetworkManager", "Render geometry", 0, 1);
}

// ****************************************************************************
//  Method: Readback
//
//  Purpose: Reads back the image buffer from IceT.
//
//  Programmer: Tom Fogal
//  Creation:   June 20, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue Jul  1 11:06:55 EDT 2008
//    Hack the number of scalars in the vtkImageData we create to be 4, to
//    match the kind of buffer IceT gives us.  This allows us to skip an
//    expensive GL_RGBA -> GL_RGB conversion.
//    Also, use a void*; don't know why it was a uchar* before ...
//
//    Tom Fogal, Wed Jul  2 11:05:07 EDT 2008
//    Readback and send/recv the Z buffer (unconditionally...).
//
//    Tom Fogal, Thu Jul 17 14:49:35 EDT 2008
//    Rely on the tiles being setup correctly; forget the whole `buddy' system.
//
//    Tom Fogal, Thu Jul 17 17:02:40 EDT 2008
//    Repurposed viewported argument for a boolean to grab Z.
//
// ****************************************************************************
avtImage_p
IceTNetworkManager::Readback(const VisWindow * const viswin,
                             bool readZ) const
{
    assert(viswin);

    GLboolean have_image;

    DEBUG_ONLY(
        ICET(icetGetBooleanv(ICET_COLOR_BUFFER_VALID, &have_image));
        assert(GL_TRUE == have_image);
    );

    int width=-42, height=-42;
    viswin->GetSize(width, height);
    assert(width > 0 && height > 0);

    GLubyte *pixels = NULL;
    GLuint *depth = NULL;

    // We have an image.  First read it back from IceT.
    pixels = icetGetColorBuffer();
    DEBUG_ONLY(ICET_CHECK_ERROR);
    depth = icetGetDepthBuffer();
    DEBUG_ONLY(ICET_CHECK_ERROR);

    this->VerifyColorFormat(); // Bail out if we don't get GL_RGBA data.
    assert(NULL != pixels);
    assert(NULL != depth);

    vtkImageData *image = avtImageRepresentation::NewImage(width, height);
    // NewImage assumes we want a 3-component ("GL_RGB") image, but IceT gives
    // us back data in a GL_RGBA format.  So we just reset the number of
    // components and reallocate the data; unfortunately this means we do an
    // allocate in NewImage and then immediately throw it away when doing an
    // allocate here.
    image->SetNumberOfScalarComponents(4);
    image->AllocateScalars();
    {
        void *img_pix = image->GetScalarPointer();
        memcpy(img_pix, pixels, width*height*4);
    }

    float *visit_depth_buffer = NULL;
    if(readZ) {
        debug1 << "converting depth values ..." << std::endl;
        visit_depth_buffer = utofv(depth, width*height);
    }
    avtSourceFromImage screenCapSrc(image, visit_depth_buffer);
    avtImage_p visit_img = screenCapSrc.GetTypedOutput();
    visit_img->Update(screenCapSrc.GetGeneralContract());
    visit_img->SetSource(NULL);
    image->Delete();
    delete[] visit_depth_buffer;

    debug3 << "Readback complete." << std::endl;

    return visit_img;
}

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
// ****************************************************************************
void
IceTNetworkManager::StopTimer(int windowID)
{
    char msg[1024];
    const VisWindow *viswin = this->viswinMap.find(windowID)->second.viswin;
    int rows,cols;
    viswin->GetSize(rows, cols);

    SNPRINTF(msg, 1023, "IceTNM::Render %d cells %d pixels",
             GetTotalGlobalCellCounts(windowID), rows*cols);
    visitTimer->StopTimer(this->r_mgmt.timer, msg);
    this->r_mgmt.timer = -1;
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
// ****************************************************************************
void
IceTNetworkManager::VerifyColorFormat() const
{
    GLint color_format;
    ICET(icetGetIntegerv(ICET_COLOR_FORMAT, &color_format));
    if(color_format != GL_RGBA)
    {
        const char *str;
        switch(color_format)
        {
            case GL_RGB: str = "GL_RGB"; break;
            case GL_BGR: str = "GL_BGR"; break;
            case GL_BGRA: str = "GL_BGRA"; break;
            default: str = "unexpected error case"; break;
        }
        EXCEPTION2(UnexpectedValueException, "GL_RGBA", std::string(str));
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
