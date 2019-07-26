// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <EngineBase.h>
#include <Engine.h>

//Static members.
Engine *EngineBase::engine_instance = NULL;
Engine *(*EngineBase::engineCreateCB)(void *) = NULL;
void   *EngineBase::engineCreateCBData = NULL;

// ****************************************************************************
// Method: EngineBase::EngineBase
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 17:33:39 PDT 2014
//
// Modifications:
//
// ****************************************************************************

EngineBase::EngineBase()
{
}

// ****************************************************************************
// Method: EngineBase::~EngineBase
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 17:33:39 PDT 2014
//
// Modifications:
//
// ****************************************************************************

EngineBase::~EngineBase()
{
}

// ****************************************************************************
// Method: EngineBase::GetEngine
//
// Purpose:
//   Factory method for returning the engine instance.
//
// Returns:    The engine pointer.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 17:34:10 PDT 2014
//
// Modifications:
//
// ****************************************************************************

Engine *
EngineBase::GetEngine()
{
    if(engine_instance == NULL)
    {
        if(engineCreateCB != NULL)
            engine_instance = (*engineCreateCB)(engineCreateCBData);
        else
            engine_instance = new Engine;
    }

    return engine_instance;
}

// ****************************************************************************
// Method: EngineBase::DeleteEngine
//
// Purpose:
//   Delete the engine.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 17:40:53 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
EngineBase::DeleteEngine()
{
    if(engine_instance != NULL)
    {
        delete engine_instance;
        engine_instance = NULL;
    }
}

// ****************************************************************************
// Method: EngineBase::SetEngineCreationCallback
//
// Purpose:
//   Set an option engine creation callback function.
//
// Arguments:
//   cb     : The callback function to create an engine instance.
//   cbdata : The callback data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 17:34:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
EngineBase::SetEngineCreationCallback(Engine *(*cb)(void *), void *cbdata)
{
    engineCreateCB = cb;
    engineCreateCBData = cbdata;
}

