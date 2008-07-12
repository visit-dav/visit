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
//  Method: utofv
//
//  Purpose: Converts a vector of unsigned integers to a vector of floats.  The
//           returned buffer is dynamically allocated, and should be delete[]d
//           by the caller.
//
//  Programmer: Tom Fogal
//  Creation:   July 2, 2008
//
// ****************************************************************************
static float *
utofv(const unsigned int * const src, size_t n_elem)
{
    size_t i;
    float *res = new float[n_elem];
    for(i=0; i < n_elem; ++i) {
        res[i] = static_cast<float>(src[i]);
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
//    Tom Fogal, Fri Jul 11 19:53:03 PDT 2008
//    Added timer analogous to parent's overall render timer.
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
        StackTimer t_total("Total time for IceTNetworkManager::Render");
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

        this->r_mgmt.viewportedMode =
            (this->r_mgmt.annotMode != 1) ||
            (viswin->GetWindowMode() == WINMODE_2D) ||
            (viswin->GetWindowMode() == WINMODE_CURVE) ||
            (viswin->GetWindowMode() == WINMODE_AXISARRAY);

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

        // Now we're done rendering, we need to post process the image.
        GLint rank;
        ICET(icetGetIntegerv(ICET_RANK, &rank));

        debug3 << "IceTNM: Starting readback." << std::endl;
        avtDataObject_p dob;
        {
            avtImage_p img = this->Readback(viswin, this->r_mgmt.viewportedMode);
            CopyTo(dob, img);
        }

        // Now its essentially back to the same behavior as our parent:
        //  shadows
        //  depth cueing
        //  post processing
        //  creating a D.Obj.writer out of all this

        if (doShadows)
        {
            this->RenderShadows(windowID, dob);
        }

        if (doDepthCueing)
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
// ****************************************************************************

void
IceTNetworkManager::RealRender()
{
    avtImage_p dob = this->RenderGeometry();
    VisWindow *viswin =
        this->viswinMap.find(this->r_mgmt.windowID)->second.viswin;
    if(this->MultipassRendering(viswin)) {
        avtDataObject_p i_as_dob;
        i_as_dob = this->RenderTranslucent(this->r_mgmt.windowID, dob);
        CopyTo(dob, i_as_dob);
    }
}

// ****************************************************************************
//  Method: Readback
//
//  Purpose: Reads back the image buffer from IceT.
//           Unfortunately for us, many post processing algorithms in VisIt
//           assume they'll have an image available.  In IceT, this is only
//           true if the node is also a tile node.  Most of the complication
//           (and the MPI calls) in this method comes from making sure *all*
//           nodes have images, whether or not they are driving a tile.
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
// ****************************************************************************
avtImage_p
IceTNetworkManager::Readback(const VisWindow * const viswin,
                             bool viewported) const
{
    assert(viswin);

    GLboolean have_image;
    GLint n_tiles, n_procs, rank;

    ICET(icetGetBooleanv(ICET_COLOR_BUFFER_VALID, &have_image));
    ICET(icetGetIntegerv(ICET_NUM_TILES, &n_tiles));
    ICET(icetGetIntegerv(ICET_NUM_PROCESSES, &n_procs));
    ICET(icetGetIntegerv(ICET_RANK, &rank));

    int width=-42, height=-42;
    viswin->GetSize(width, height);
    assert(width > 0 && height > 0);

    GLubyte *pixels = NULL;
    GLuint *depth = NULL;

    // We can't delete pointers IceT gives us.  However if we're a receiving
    // node, we'll dynamically allocate our buffers and thus need to deallocate
    // them.
    bool dynamic = false;

    if(GL_TRUE == have_image)
    {
        // We have an image.  First read it back from IceT.
        pixels = icetGetColorBuffer();
        DEBUG_ONLY(ICET_CHECK_ERROR);
        depth = icetGetDepthBuffer();
        DEBUG_ONLY(ICET_CHECK_ERROR);

        this->VerifyColorFormat(); // Bail out if we don't get GL_RGBA data.
        assert(NULL != depth);

        for(GLint buddy=rank+1; buddy < n_procs; ++buddy)
        {
            // Send to machines with ranks a multiple of my own
            if((buddy % n_tiles) == rank)
            {
                // 4: assuming GL_RGBA.
                debug2 << "Processor " << rank << " sending to " << buddy
                       << std::endl;
                MPI_Send(pixels, 4*width*height, MPI_BYTE, buddy, 1,
                         VISIT_MPI_COMM);
                MPI_Send(depth, width*height, MPI_UNSIGNED, buddy, 2,
                         VISIT_MPI_COMM);
            }
        }
    }
    else
    {
        // We don't have an image -- we need to receive it from our buddy.
        GLint source = (rank % n_tiles);
        debug2 << "Processor " << rank << " waiting for data from " << source
               << std::endl;
        pixels = new GLubyte[4*width*height];
        depth = new GLuint[width*height];
        dynamic = true;
        MPI_Recv(pixels, 4*width*height, MPI_BYTE, source, 1, VISIT_MPI_COMM,
                 MPI_STATUS_IGNORE);
        debug2 << "Received image!" << std::endl;
        MPI_Recv(depth, width*height, MPI_UNSIGNED, source, 2, VISIT_MPI_COMM,
                 MPI_STATUS_IGNORE);
    }
    debug2 << "Finished pushing buffers out." << std::endl;

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

    float *visit_depth_buffer = utofv(depth, width*height);
    avtSourceFromImage screenCapSrc(image, visit_depth_buffer);
    avtImage_p visit_img;
    visit_img = screenCapSrc.GetTypedOutput();
    visit_img->Update(screenCapSrc.GetGeneralContract());
    visit_img->SetSource(NULL);
    image->Delete();
    delete[] visit_depth_buffer;
    if(dynamic)
    {
        delete[] pixels;
        delete[] depth;
    }

    return visit_img;
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
