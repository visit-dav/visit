#include "enginewrapper.h"
#include <Engine.h>
#include <LoadBalancer.h>

#include <iostream.h>

void *get_engine()
{
    cerr << "In enginewrapper.C::load_engine\n";
    LoadBalancer::ForceStatic();
    Engine *engine = Engine::Instance();
    return (void*)engine;
}

void initialize(void *e, int argc, char *argv[])
{
    Engine *engine = (Engine*)(e);
    engine->Initialize(&argc, &argv);
}

void connect_to_viewer(void *e, int argc, char *argv[])
{
    Engine *engine = (Engine*)(e);
    bool success = engine->ConnectViewer(&argc, &argv);
    if (!success)
    {
        fprintf(stderr, "Error connecting to viewer!\n");
    }
    else
    {
        engine->SetUpViewerInterface(&argc, &argv);
    }
}

int get_descriptor(void *e)
{
    Engine *engine = (Engine*)(e);
    return engine->GetInputSocket();
}

void process_input(void *e)
{
    Engine *engine = (Engine*)(e);
    engine->ProcessInput();
    //while (engine->NeedsRead() && engine->ProcessInput())
    //   /* do nothing */;
}
