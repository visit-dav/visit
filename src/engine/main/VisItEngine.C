#include "VisItEngine.h"

#include <Engine.h>
#include <LostConnectionException.h>
#include <LoadBalancer.h>
#include <MPIXfer.h>
#include <VisItException.h>

#include <visitstream.h>

// ****************************************************************************
//  Library:  VisItEngine
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
// ****************************************************************************

void *get_engine()
{
    Engine *engine = Engine::Instance();
    return (void*)engine;
}

int initialize(void *e, int argc, char *argv[])
{
    Engine *engine = (Engine*)(e);
    engine->Initialize(&argc, &argv);
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
        return 1;
    }
}

int get_descriptor(void *e)
{
    Engine *engine = (Engine*)(e);
    return engine->GetInputSocket();
}

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
    CATCHALL (...)
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

void disconnect()
{
    Engine::Disconnect();
}

void set_slave_process_callback(void(*spic)())
{
#ifdef PARALLEL
    MPIXfer::SetSlaveProcessInstructionCallback(spic);
#endif
}

//  Needed for some reason on some platforms.
int main()
{
}
