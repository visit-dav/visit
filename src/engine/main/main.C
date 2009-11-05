/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <visit-config.h>

#include <Engine.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <avtParallel.h>

#include <cstring>

// ****************************************************************************
//  BEGIN HACK, because AIX won't link the engine against GL unless it
//  needs to resolve symbols directly from this source.
//  GL dependencies exist in libplotter, libvisit_vtk and libvtkRendering
//  When these extended dependencies are removed and the engine no
//  longer needs to link against GL, this hack can be removed.
#ifdef AIX
#include <GL/gl.h>
void foogl(void)
{
   GLfloat mad[] = {0.1, 0.5, 0.8, 1.0};
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mad);
}

// HACK: Needed to force linking of libz on AIX
#ifdef HAVE_LIBZ
#include <zlib.h>
void fooz(void)
{
   zlibVersion();
}
#endif

#endif
//
// END HACK
// ****************************************************************************



// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for a simple "engine" that runs in
//   parallel and gets state information from the viewer and prints
//   the information that changed to the console.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 15:20:19 PST 2000
//
// Modifications:
//
//    Jeremy Meredith, Wed Aug  9 14:41:24 PDT 2000
//    Switched out plotAtts for plotRPC.
//
//    Jeremy Meredith, Wed Aug  9 14:41:24 PDT 2000
//    Cleaned up the way RPCs are handled.
//    Switched out quitAtts for quitRPC.
//
//    Jeremy Meredith, Thu Sep  7 13:06:10 PDT 2000
//    Added the new RPC types for doing network-style computation.
//
//    Jeremy Meredith, Fri Sep 15 16:12:56 PDT 2000
//    Added slice RPC, fixed a bug with the previous ones.
//
//    Jeremy Meredith, Thu Sep 21 22:15:06 PDT 2000
//    Made it work in parallel again.
//
//    Kathleen Bonnell, Thu Oct 12 12:50:27 PDT 2000
//    Added OnionPeelRPC.
//
//    Brad Whitlock, Fri Oct 6 11:42:56 PDT 2000
//    Removed the SocketConnections. I also added code to set the
//    destination format from the connection back to the viewer.
//
//    Hank Childs, Thu Oct 26 10:13:55 PDT 2000
//    Added initialization of exceptions.
//
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Kathleen Bonnell, Fri Nov 17 16:33:40 PST 2000 
//    Added MatPlotRPC.
//
//    Kathleen Bonnell, Fri Dec  1 14:51:50 PST 2000 
//    Added FilledBoundaryRPC.
//
//    Jeremy Meredith, Tue Dec 12 13:50:03 PST 2000
//    Added MaterialSelectRPC.
//
//    Hank Childs, Thu Jan 11 16:45:11 PST 2001
//    Added RangeVolumePlotRPC, IsoSurfaceVolumePlotRPC.
//
//    Kathleen Bonnell, Thu Feb 22 15:08:32 PST 2001 
//    Added ContourPlotRPC.
//
//    Jeremy Meredith, Sun Mar  4 16:50:49 PST 2001
//    Ripped out all plot and operator RPCs.
//    Created two new ones:  ApplyOperator and MakePlot.
//    Added manual initialization of (a new) PluginManager.
//
//    Kathleen Bonnell, Thu Mar  8 09:01:10 PST 2001 
//    Added registration of surface plot.
//
//    Brad Whitlock, Thu Mar 15 14:35:22 PST 2001
//    Modified how input connections are supplied to the xfer object. Changed
//    calls to the event loops.
//
//    Jeremy Meredith, Tue Mar 20 12:21:07 PST 2001
//    Removed registration of all plots since we are using the new
//    PlotPluginManager.
//
//    Hank Childs, Tue Apr 24 15:25:37 PDT 2001
//    Added initialization of VTK modules.
//
//    Brad Whitlock, Wed Apr 25 17:09:37 PST 2001
//    Added code to catch IncompatibleVersionException. The handlers set the
//    noFatalExceptions variable to false which causes the engine to terminate.
//
//    Jeremy Meredith, Thu May 10 15:00:40 PDT 2001
//    Added initialization of PlotPluginManager.
//
//    Kathleen Bonnell, Mon May  7 15:58:13 PDT 2001 
//    Added registration of Erase operator. 
//    
//    Hank Childs, Sun Jun 17 18:06:53 PDT 2001
//    Removed reference to avtLoadBalancer, initialized LoadBalancer.
//
//    Hank Childs, Wed Jun 20 18:15:44 PDT 2001
//    Initialize avt filters.
//
//    Jeremy Meredith, Tue Jul  3 15:10:52 PDT 2001
//    Added xfer parameter to EngineAbortCallback.
//
//    Hank Childs, Fri Jul 20 08:09:53 PDT 2001
//    Remove MaterialSelect.
//
//    Jeremy Meredith, Tue Jul 24 14:09:27 PDT 2001
//    Removed FacelistFilter.
//
//    Jeremy Meredith, Thu Jul 26 03:36:21 PDT 2001
//    Added support for the new (real) operator plugin manager.
//
//    Hank Childs, Mon Aug 13 15:14:50 PDT 2001
//    Changed location of progress/abort callbacks from avtFilter to
//    avtDataObjectSource.
//
//    Jeremy Meredith, Thu Sep 20 01:00:59 PDT 2001
//    Added registration of the progress callback with the LoadBalancer.
//
//    Jeremy Meredith, Fri Sep 21 14:45:14 PDT 2001
//    Added registration of the abort callback with the LoadBalancer.
//
//    Jeremy Meredith, Fri Sep 28 13:41:27 PDT 2001
//    Added load of plugins since they are not loaded anymore until
//    explicity told to do so.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Fri Nov  9 10:34:08 PST 2001
//    Added UseNetworkRPC and SetWindowAttsRPC.
//
//    Hank Childs, Thu Nov 29 16:22:37 PST 2001
//    Added UpdatePlotAttsRPC.
//
//    Kathleen Bonnell, Tue Nov 20 12:35:54 PST 2001 
//    Added PickRPC StartPickRPC.
//
//    Jeremy Meredith, Wed Jan 16 10:09:45 PST 2002
//    Do initialization of plugin managers with parallel flag.
//
//    Sean Ahern, Thu Mar 21 13:18:09 PST 2002
//    Added ApplyUnaryOperatorRPC.
//
//    Sean Ahern, Fri Apr 19 14:02:34 PDT 2002
//    Removed ApplyUnaryOperatorRPC.  Added ApplyNamedFunctionRPC.
//
//    Brad Whitlock, Tue Jul 30 13:13:42 PST 2002
//    I added ClearCacheRPC.
//
//    Jeremy Meredith, Wed Aug 21 12:51:28 PDT 2002
//    I renamed some plot/operator plugin manager methods for refactoring.
//
//    Jeremy Meredith, Thu Aug 22 14:31:44 PDT 2002
//    Added database plugins.
//
//    Hank Childs, Mon Sep 30 14:26:55 PDT 2002
//    Made the network manager be allocated off the heap.  That way we can
//    control whether or not we decide to clean up the memory associated with
//    it when we exit the program (it's faster not to).
//
//    Kathleen Bonnell, Mon Sep 16 14:28:09 PDT 2002  
//    Added QueryRPC, ReleaseDataRPC.
//
//    Brad Whitlock, Mon Sep 30 09:02:35 PDT 2002
//    The code to connect to the viewer became more complex so I moved it
//    to a new function. I made the new function return a bool that tells
//    whether or not the connection to the viewer was a success. The return
//    value is used to jump over the initializing of a lot of objects so the
//    engine can terminate faster.
//
//    Mark C. Miller, Mon Nov 11 14:45:16 PST 2002
//    Added a call to ForceMesa during initialization.
//
//    Brad Whitlock, Tue Dec 10 14:33:13 PST 2002
//    I added OpenDatabaseRPC.
//
//    Jeremy Meredith, Fri Feb 28 12:21:01 PST 2003
//    Renamed LoadPlugins to LoadPluginsNow.   There is now a corresponding
//    LoadPluginsOnDemand, and I made the Plot and Operator plugin managers
//    use that method instead.  At the moment, all Database plugins need
//    to be open for the avtDatabaseFactory to determine which one to use
//    when opening a file, but I expect this to change shortly.
//
//    Brad Whitlock, Tue Mar 25 13:13:53 PST 2003
//    I added a new rpc that lets us define virtual database files.
//
//    Jeremy Meredith, Wed May  7 15:49:53 PDT 2003
//    Force static load balancing.... for now.
//
//    Jeremy Meredith, Thu Jul 10 11:35:03 PDT 2003
//    Moved most of the stuff in here into a new Engine.C class.  This main
//    routine was re-written to take advantage of it.
//
//    Hank Childs, Tue Jun  1 13:58:57 PDT 2004
//    Added call to Init::Finalize.
//
//    Hank Childs, Sun Mar  6 08:56:30 PST 2005
//    Remove call to LoadBalancer::ForceStatic.  This is now the default.
//
//    Jeremy Meredith, Tue May 17 11:20:51 PDT 2005
//    Allow disabling of signal handlers for the engine library.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added code to initialize timings manager asap
//    Removed delete of visitTimer since that is handled in
//    TimingsManager::Finalize
//
//    Hank Childs, Sun Nov 16 21:49:29 CST 2008
//    Add a print statement when the engine couldn't connect to the viewer, so
//    we have a better idea when the engine is giving up.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    // Start timings asap to get info on initialization activity
    TimingsManager::Initialize("");
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-timing")==0 || strcmp(argv[i], "-timings")==0)
            visitTimer->Enable();
    }

    Engine *engine = Engine::Instance();

    // Do some pre-connect initialization
    engine->Initialize(&argc, &argv, true);

    // Try to connect to the viewer
    if (engine->ConnectViewer(&argc, &argv))
    {
        // Do the post-connect initialization
        engine->SetUpViewerInterface(&argc, &argv);

        // Begin the engine's event processing loop.
#ifdef PARALLEL
        engine->PAR_EventLoop();
#else
        engine->EventLoop();
#endif
    }
    else
    {
        // Connect failed
        if (PAR_Rank() == 0)
        {
             debug1 << "The engine could not connect to the viewer due to a"
                  << "networking problem.  The engine is exiting" << endl;
             cerr << "The engine could not connect to the viewer due to a"
                  << "networking problem.  The engine is exiting" << endl;
        }
    }

    debug1 << "ENGINE exited." << endl;
    engine->Finalize();

#ifdef DEBUG_MEMORY_LEAKS
    delete engine;
#endif

#ifdef PARALLEL
    PAR_Exit();
#endif
    return 0;
}
