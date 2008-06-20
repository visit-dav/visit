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
#include <avtParallel.h>
#include <DebugStream.h>
#include <Engine.h>

#include <mpi.h>
#include <GL/ice-t_mpi.h>

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

    DEBUG_ONLY(PR_ICET_MPI);

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
//           that a lone monitor is a `1x1 tiled display'.
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
//  Purpose: `Faux' render, or render entry point.  We can't do a real render
//           here because IceT wants to manage our render process, but we do
//           all the setup and then tell IceT to get started.
//           This is required to maintain API compatibility with our parent.
//
//  Arguments:
//
//  Programmer: Tom Fogal
//  Creation:   June 20, 2008
//
// ****************************************************************************
avtDataObjectWriter_p
IceTNetworkManager::Render(intVector networkIds, bool getZBuffer,
                           int annotMode, int windowID, bool leftEye)
{
    this->RenderSetup(networkIds, getZBuffer, annotMode, windowID, leftEye);

    ICET(icetDrawFunc(render));
    ICET(icetDrawFrame());

    // now readback.

    // various components which are the same as our base:
    //  shadows
    //  depth cueing
    //  post processing
    //  creating a D.Obj.writer out of all this
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
    debug1 << "woo icet asked us to render opaque geometry!" << std::endl;
    Engine *engy = Engine::Instance();
    IceTNetworkManager *net_mgr;

    net_mgr = dynamic_cast<IceTNetworkManager*>(engy->GetNetMgr());
    net_mgr->RealRender();
}
