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

#include "VisItEngineV1.h"
#include "TimingsManager.h"

#include <Engine.h>
#include <LostConnectionException.h>
#include <LoadBalancer.h>
#ifdef PARALLEL
#include <MPIXfer.h>
#endif
#include <VisItException.h>

#include <visitstream.h>

// ****************************************************************************
//  Library:  VisItEngine, Version 1
//
//  Purpose:
//    Wrapper for simulations to control an engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Nov  1 17:19:02 PST 2004
//    Added parallel simulation support.
//
//    Hank Childs, Fri Jan 28 13:40:20 PST 2005
//    Use exception macros.
//
//    Hank Childs, Sun Mar  6 08:42:50 PST 2005
//    Removed ForceStatic call.  That is now the default.
//
//    Jeremy Meredith, Fri Mar 18 08:36:54 PST 2005
//    Added simulation command control.
//
//    Jeremy Meredith, Mon Apr 25 10:00:06 PDT 2005
//    Added versioning.
//
//    Jeremy Meredith, Wed May 11 09:17:44 PDT 2005
//    Forced the RESTRICTED load balancer mode.
//
//    Jeremy Meredith, Wed May 25 13:25:45 PDT 2005
//    Disabled our own signal handlers.
//
//    Shelly Prevost, Tue Sep 12 15:59:51 PDT 2006
//    I initialize the timer if it isn't already initialized by now.
//
//    Brad Whitlock, Thu Jan 25 15:08:59 PST 2007
//    Added new functions.
//
//    Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL
//
//    Brad Whitlock, Tue Feb 16 15:29:39 PST 2010
//    Enable Simulation plugins.
//
// ****************************************************************************

void *get_engine()
{
    // Make sure the timer is initialized. In visit this is normally
    // done in the main function but for the simulation it's done here.
    if ( visitTimer == NULL)
        TimingsManager::Initialize( "Simulation");
    Engine *engine = Engine::Instance();
    engine->EnableSimulationPlugins();
    return (void*)engine;
}

int initialize(void *e, int argc, char *argv[])
{
    Engine *engine = (Engine*)(e);
    engine->Initialize(&argc, &argv, false);
    return 1;
}

int connect_to_viewer(void *e, int argc, char *argv[])
{
    Engine *engine = (Engine*)(e);
    bool success = engine->ConnectViewer(&argc, &argv);
    if (!success)
    {
        return 0;
    }
    else
    {
        engine->SetUpViewerInterface(&argc, &argv);
        LoadBalancer::SetScheme(LOAD_BALANCE_RESTRICTED);
        return 1;
    }
}

int get_descriptor(void *e)
{
    Engine *engine = (Engine*)(e);
    return engine->GetInputSocket();
}

#include <DebugStream.h>

int process_input(void *e)
{
    Engine *engine = (Engine*)(e);

    TRY {
#ifdef PARALLEL
        engine->PAR_ProcessInput();
#else
        engine->ProcessInput();
#endif
    }
    CATCH (LostConnectionException)
    {
        // Lost connection to the viewer!
        CATCH_RETURN2(1, 0);
    }
    CATCH2(VisItException, e)
    {
        debug1 << "Caught a damn VisIt exception: " << e.Message() << endl;
    }
    CATCHALL
    {
        // Unknown processing error!  Ignoring....
        CATCH_RETURN2(1, 1);
    }
    ENDTRY

    return 1;
}

void time_step_changed(void *e)
{
    Engine *engine = (Engine*)(e);
    engine->SimulationTimeStepChanged();
}

void update_plots(void *e)
{
    Engine *engine = (Engine*)(e);
    engine->SimulationInitiateCommand("UpdatePlots");
}

void execute_command(void *e, const char *command)
{
    if(command != NULL)
    {
        Engine *engine = (Engine*)(e);
        std::string cmd("Interpret:");
        cmd += command;
        
        engine->SimulationInitiateCommand(cmd);
    }
}

void disconnect()
{
    Engine::DisconnectSimulation();
}

void set_slave_process_callback(void(*spic)())
{
#ifdef PARALLEL
    MPIXfer::SetSlaveProcessInstructionCallback(spic);
#endif
}

// This is needed to keep the old command callbacks still working even though the
// engine's command callback signature has changed.
static void
internal_command_callback_bridge(const char *cmd, const char *args, void *cbdata)
{
    void(*sc)(const char*,int,float,const char*) = (void(*)(const char*,int,float,const char*))cbdata;
    if(sc != NULL)
        (*sc)(cmd, 0, 0.f, args);
}

void set_command_callback(void *e,void(*sc)(const char*,int,float,const char*))
{
    Engine *engine = (Engine*)(e);
    engine->SetSimulationCommandCallback(internal_command_callback_bridge, (void *)sc);
}

//  Needed for some reason on some platforms.
int main()
{
}
