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

#include <GL/ice-t_mpi.h>
#include <mpi.h>
#include <vtkImageData.h>

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

static void SendImageToRenderNodes(int, int, bool, GLubyte * const,
                                   GLuint * const);

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
// ****************************************************************************
void
IceTNetworkManager::TileLayout(size_t width, size_t height) const
{
    debug2 << "IceTNM: configuring " << width << "x" << height
           << " single tile display." << std::endl;

    ICET(icetResetTiles());
    const GLint this_mpi_rank_gets_an_image = 0;
    ICET(icetAddTile(0,0, width, height, this_mpi_rank_gets_an_image));
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
//    Tom Fogal, Mon Jul 28 14:45:09 EDT 2008
//    Do Z test earlier, and request IceT buffers based on Z test.
//
//    Tom Fogal, Sun Aug  3 23:04:20 EDT 2008
//    Use MemoMultipass; this fixes a bug which occurs when IceT calls our
//    render function multiple times on a subset of nodes.
//
//    Hank Childs, Thu Jan 15 11:07:53 CST 2009
//    Changed GetSize call to GetCaptureRegion, since that works for 2D.
//
//    Brad Whitlock, Mon Mar  2 16:38:53 PST 2009
//    I made the routine return an avtDataObject_p.
//
//    Hank Childs, Sat Mar 21 10:34:16 PST 2009
//    Fix compilation error.
//
// ****************************************************************************

avtDataObject_p
IceTNetworkManager::Render(bool, intVector networkIds, bool getZBuffer,
                           int annotMode, int windowID, bool leftEye)
{
    int t0 = visitTimer->StartTimer();
    DataNetwork *origWorkingNet = workingNet;
    avtDataObject_p retval;

    EngineVisWinInfo &viswinInfo = viswinMap[windowID];
    viswinInfo.markedForDeletion = false;
    VisWindow *viswin = viswinInfo.viswin;
    std::vector<avtPlot_p>& imageBasedPlots = viswinInfo.imageBasedPlots;

    TRY
    {
        this->StartTimer();
        this->RenderSetup(networkIds, getZBuffer, annotMode, windowID, leftEye);
        bool needZB = !imageBasedPlots.empty() ||
                      this->Shadowing(windowID)  ||
                      this->DepthCueing(windowID);

        // Confusingly, we need to set the input to be *opposite* of what VisIt
        // wants.  This is due to (IMHO) poor naming in the IceT case; on the
        // input side:
        //     ICET_DEPTH_BUFFER_BIT set:     do Z-testing
        //     ICET_DEPTH_BUFFER_BIT not set: do Z-based compositing.
        // On the output side:
        //     ICET_DEPTH_BUFFER_BIT set:     readback of Z buffer is allowed
        //     ICET_DEPTH_BUFFER_BIT not set: readback of Z does not work.
        // In VisIt's case, we calculated a `need Z buffer' predicate based
        // around the idea that we need the Z buffer to do Z-compositing.
        // However, IceT \emph{always} needs the Z buffer internally -- the
        // flag only differentiates between `compositing' methodologies
        // (painter-style or `over' operator) on input.
        GLenum inputs = ICET_COLOR_BUFFER_BIT;
        GLenum outputs = ICET_COLOR_BUFFER_BIT;
        // Scratch all that, I guess.  That might be the correct way to go
        // about things in the long run, but IceT only gives us back half an
        // image if we don't set the depth buffer bit.  The compositing is a
        // bit wrong, but there's not much else we can do..
        // Consider removing the `hack' if a workaround is found.
        if(/*hack*/true/*hack*/ || !this->MemoMultipass(viswin))
        {
            inputs |= ICET_DEPTH_BUFFER_BIT;
        }
        if(needZB)
        {
            outputs |= ICET_DEPTH_BUFFER_BIT;
        }
        ICET(icetInputOutputBuffers(inputs, outputs));

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

        // scalable threshold test (the 0.5 is to add some hysteresus to avoid 
        // the misfortune of oscillating switching of modes around the
        // threshold)
        int scalableThreshold = GetScalableThreshold(windowID);
        if (GetTotalGlobalCellCounts(windowID) < 0.5 * scalableThreshold)
        {
            this->RenderCleanup(windowID);
            avtDataObject_p dobj = NULL;
            CATCH_RETURN2(1, dobj);
        }

        debug5 << "Rendering " << viswin->GetNumPrimitives()
               << " primitives.  Balanced speedup = "
               << RenderBalance(viswin->GetNumPrimitives()) << "x" << endl;

        int width, height, width_start, height_start;
        // This basically gets the width and the height.
        // The distinction is for 2D rendering, where we only want the
        // width and the height of the viewport.
        viswin->GetCaptureRegion(width_start, height_start, width, height,
                                 this->r_mgmt.viewportedMode);
        
        this->TileLayout(width, height);

        CallInitializeProgressCallback(this->RenderingStages(windowID));

        // IceT mode is different from the standard network manager; we don't
        // need to create any compositor or anything: it's all done under the
        // hood.
        // Whether or not to do multipass rendering (opaque first, translucent
        // second) is all handled in the callback; from our perspective, we
        // just say draw, read back the image, and post-process it.

        ICET(icetDrawFunc(render));
        ICET(icetDrawFrame());

        // Now that we're done rendering, we need to post process the image.
        debug3 << "IceTNM: Starting readback." << std::endl;
        avtDataObject_p dob;
        {
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

        retval = dob;

        this->RenderCleanup(windowID);
    }
    CATCHALL(...)
    {
        RETHROW;
    }
    ENDTRY

    workingNet = origWorkingNet;
    visitTimer->StopTimer(t0, "Ice-T Render");
    return retval;
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
//    Tom Fogal, Mon Aug  4 17:47:25 EDT 2008
//    Remove the DataObject; since we override the method, it's always NULL
//    (and was never used anyway).
//
// ****************************************************************************

void
IceTNetworkManager::RealRender()
{
    avtImage_p dob = this->RenderGeometry();
    VisWindow *viswin =
        this->viswinMap.find(this->r_mgmt.windowID)->second.viswin;
    if(this->MemoMultipass(viswin))
    {
        this->RenderTranslucent(this->r_mgmt.windowID, dob);
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
//    Tom Fogal, Mon Jul 28 14:57:01 EDT 2008
//    Need to return NULL in the single-pass case!
//
// ****************************************************************************
avtImage_p
IceTNetworkManager::RenderGeometry()
{
    VisWindow *viswin = viswinMap.find(this->r_mgmt.windowID)->second.viswin;
    if(this->MemoMultipass(viswin))
    {
        return NetworkManager::RenderGeometry();
    }
    CallProgressCallback("IceTNetworkManager", "Render geometry", 0, 1);
        viswin->ScreenRender(this->r_mgmt.viewportedMode, true);
    CallProgressCallback("IceTNetworkManager", "Render geometry", 0, 1);
    return NULL;
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
// ****************************************************************************
avtDataObject_p
IceTNetworkManager::RenderTranslucent(int windowID, const avtImage_p& input)
{
    VisWindow *viswin = viswinMap.find(windowID)->second.viswin;
    CallProgressCallback("IceTNetworkManager", "Transparent rendering", 0, 1);
    {
        StackTimer second_pass("Second-pass screen capture for SR");

        //
        // We have to disable any gradient background before
        // rendering, as those will overwrite the first pass
        //
        AnnotationAttributes::BackgroundMode bm = viswin->GetBackgroundMode();
        viswin->SetBackgroundMode(AnnotationAttributes::Solid);

        viswin->ScreenRender(
            this->r_mgmt.viewportedMode,
            true,   // Z buffer
            false,  // opaque geometry
            true,   // translucent geometry
            input   // image to composite with
        );

        // Restore the background mode for next time
        viswin->SetBackgroundMode(bm);
    }
    CallProgressCallback("IceTNetworkManager", "Transparent rendering", 1, 1);

    //
    // In this implementation, the user should never use the return value --
    // read it back from IceT instead!
    //
    return NULL;
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
//    Tom Fogal, Thu Jul 17 17:02:40 EDT 2008
//    Repurposed viewported argument for a boolean to grab Z.
//
//    Tom Fogal, Mon Jul 28 14:44:28 EDT 2008
//    Don't ask IceT for Z if we're not going to use it anyway.
//
//    Tom Fogal, Mon Sep  1 14:21:46 EDT 2008
//    Removed asserts / dependence on NDEBUG.
//
//    Hank Childs, Mon Dec 29 18:24:05 CST 2008
//    Make an image have 3 components, not 4, since 3 is better supported
//    throughout VisIt (including saving TIFFs).
//
//    Hank Childs, Thu Jan 15 11:07:53 CST 2009
//    Changed GetSize call to GetCaptureRegion, since that works for 2D.
//
//    Hank Childs, Fri Feb  6 15:47:17 CST 2009
//    Fix memory leak.
//
// ****************************************************************************
avtImage_p
IceTNetworkManager::Readback(VisWindow * const viswin,
                             bool readZ) const
{
    GLboolean have_image;

    ICET(icetGetBooleanv(ICET_COLOR_BUFFER_VALID, &have_image));

    int width=-42, height=-42, width_start, height_start;
    // This basically gets the width and the height.
    // The distinction is for 2D rendering, where we only want the
    // width and the height of the viewport.
    viswin->GetCaptureRegion(width_start, height_start, width, height,
                             this->r_mgmt.viewportedMode);

    GLubyte *pixels = NULL;
    GLuint *depth = NULL;

    if(readZ && have_image == GL_TRUE)
    {
        depth = icetGetDepthBuffer();
        DEBUG_ONLY(ICET_CHECK_ERROR);
    }
    // We can't delete pointers IceT gives us.  However if we're a receiving
    // node, we'll dynamically allocate our buffers and thus need to deallocate
    // them.
    bool dynamic = false;

    if(have_image == GL_TRUE)
    {
        // We have an image.  First read it back from IceT.
        pixels = icetGetColorBuffer();
        DEBUG_ONLY(ICET_CHECK_ERROR);

        this->VerifyColorFormat(); // Bail out if we don't get GL_RGBA data.
    } else {
        // We don't have an image -- we need to receive it from our buddy.
        // Purpose of static pixel_ptr ... if I delete this memory too soon (i.e. along
        // with "depth"), then there is a crash ... it is being used after the function
        // exits.  So just wait until the next render to free it.
        static GLubyte *pixel_ptr = NULL;
        if (pixel_ptr != NULL)
           delete [] pixel_ptr;
        pixel_ptr = new GLubyte[4*width*height];
        pixels = pixel_ptr;
        depth = new GLuint[width*height];

        dynamic = true;
    }
    SendImageToRenderNodes(width, height, readZ, pixels, depth);

    vtkImageData *image = avtImageRepresentation::NewImage(width, height);
    // NewImage assumes we want a 3-component ("GL_RGB") image, but IceT gives
    // us back data in a GL_RGBA format.  So we just reset the number of
    // components and reallocate the data; unfortunately this means we do an
    // allocate in NewImage and then immediately throw it away when doing an
    // allocate here.
    image->SetNumberOfScalarComponents(3);
    image->AllocateScalars();
    {
        unsigned char *img_pix = (unsigned char *) image->GetScalarPointer();
        const int numPix = width*height;
        for (int i = 0 ; i < numPix ; i++)
        {
            *img_pix++ = *pixels++;
            *img_pix++ = *pixels++;
            *img_pix++ = *pixels++;
            pixels++; // Alpha
        }
        //memcpy(img_pix, pixels, width*height*4);
    }
    float *visit_depth_buffer = NULL;

    if(readZ)
    {
        debug4 << "Converting depth values ..." << std::endl;
        visit_depth_buffer = utofv(depth, width*height);
    }

    avtSourceFromImage screenCapSrc(image, visit_depth_buffer);
    avtImage_p visit_img = screenCapSrc.GetTypedOutput();
    visit_img->Update(screenCapSrc.GetGeneralContract());
    visit_img->SetSource(NULL);
    image->Delete();
    delete[] visit_depth_buffer;
    if(dynamic)
    {
        delete[] depth;
    }

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
//  Modifications:
//
//    Hank Childs, Thu Jan 15 11:07:53 CST 2009
//    Changed GetSize call to GetCaptureRegion, since that works for 2D.
//
// ****************************************************************************
void
IceTNetworkManager::StopTimer(int windowID)
{
    char msg[1024];
    VisWindow *viswin = this->viswinMap.find(windowID)->second.viswin;
    GLubyte *pixels = NULL;
    int rows,cols, width_start, height_start;
    // This basically gets the width and the height.
    // The distinction is for 2D rendering, where we only want the
    // width and the height of the viewport.
    viswin->GetCaptureRegion(width_start, height_start, rows,cols,
                             this->r_mgmt.viewportedMode);

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
// ****************************************************************************
static void
SendImageToRenderNodes(int width, int height, bool Z,
                       GLubyte * const pixels,
                       GLuint * const depth)
{
    GLint n_tiles, n_procs, rank;
    GLboolean have_image;

    ICET(icetGetIntegerv(ICET_NUM_TILES, &n_tiles));
    ICET(icetGetIntegerv(ICET_NUM_PROCESSES, &n_procs));
    ICET(icetGetIntegerv(ICET_RANK, &rank));
    ICET(icetGetBooleanv(ICET_COLOR_BUFFER_VALID, &have_image));

    //              4: assuming GL_RGBA.
    MPI_Bcast(pixels, 4*width*height, MPI_BYTE, 0, VISIT_MPI_COMM);
    if(Z) {
        MPI_Bcast(depth, width*height, MPI_UNSIGNED, 0, VISIT_MPI_COMM);
    }
}
