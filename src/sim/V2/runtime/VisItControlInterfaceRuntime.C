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

#include "VisItControlInterfaceRuntime.h"
#include <VisItInterfaceTypes_V2.h>

#include <Engine.h>
#include <LostConnectionException.h>
#include <LoadBalancer.h>
#ifdef PARALLEL
#include <MPIXfer.h>
#endif
#include <TimingsManager.h>
#include <VisItException.h>

#include <visitstream.h>

extern void DataCallbacksCleanup(void);

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
//
// ****************************************************************************

void *simv2_get_engine()
{
    // Make sure the timer is initialized. In visit this is normally
    // done in the main function but for the simulation it's done here.
    if ( visitTimer == NULL)
        TimingsManager::Initialize( "Simulation");
    Engine *engine = Engine::Instance();
    engine->EnableSimulationPlugins();
    return (void*)engine;
}

int simv2_initialize(void *e, int argc, char *argv[])
{
    Engine *engine = (Engine*)(e);
    engine->Initialize(&argc, &argv, false);
    return 1;
}

int simv2_connect_viewer(void *e, int argc, char *argv[])
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

int simv2_get_descriptor(void *e)
{
    Engine *engine = (Engine*)(e);
    return engine->GetInputSocket();
}

#include <DebugStream.h>

int simv2_process_input(void *e)
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
        debug1 << "Caught a VisIt exception: " << e.Message() << endl;
    }
    CATCHALL
    {
        // Unknown processing error!  Ignoring....
        CATCH_RETURN2(1, 1);
    }
    ENDTRY

    return 1;
}

void simv2_time_step_changed(void *e)
{
    Engine *engine = (Engine*)(e);
    engine->SimulationTimeStepChanged();
}

void simv2_execute_command(void *e, const char *command)
{
    if(command != NULL)
    {
        Engine *engine = (Engine*)(e);       
        engine->SimulationInitiateCommand(command);
    }
}

void simv2_disconnect()
{
    Engine::DisconnectSimulation();

    DataCallbacksCleanup();
}

void simv2_set_slave_process_callback(void(*spic)())
{
#ifdef PARALLEL
    MPIXfer::SetSlaveProcessInstructionCallback(spic);
#endif
}

void simv2_set_command_callback(void *e,void(*sc)(const char*,const char*,void*),
    void *scdata)
{
    Engine *engine = (Engine*)(e);
    engine->SetSimulationCommandCallback(sc, scdata);
}

int
simv2_save_window(void *e, const char *filename, int w, int h, int format)
{
    Engine *engine = (Engine*)(e);

    SaveWindowAttributes::FileFormat fmt;
    if(format == VISIT_IMAGEFORMAT_BMP)
        fmt = SaveWindowAttributes::BMP;
    else if(format == VISIT_IMAGEFORMAT_JPEG)
        fmt = SaveWindowAttributes::JPEG;
    else if(format == VISIT_IMAGEFORMAT_PNG)
        fmt = SaveWindowAttributes::PNG;
    else if(format == VISIT_IMAGEFORMAT_POVRAY)
        fmt = SaveWindowAttributes::POVRAY;
    else if(format == VISIT_IMAGEFORMAT_PPM)
        fmt = SaveWindowAttributes::PPM;
    else if(format == VISIT_IMAGEFORMAT_RGB)
        fmt = SaveWindowAttributes::RGB;
    else
        fmt = SaveWindowAttributes::TIFF;

    int ret = engine->SaveWindow(filename, w, h, fmt);
    return ret ? VISIT_OKAY : VISIT_ERROR;
}

void
simv2_debug_logs(int level, const char *msg)
{
    if(level == 1)
        debug1 << msg;
    else if(level == 2)
        debug2 << msg;
    else if(level == 3)
        debug3 << msg;
    else if(level == 4)
        debug4 << msg;
    else if(level == 5)
        debug5 << msg;
}
