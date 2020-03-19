// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_BASE_H
#define ENGINE_BASE_H
#include <engine_main_exports.h>

class Engine;

// ****************************************************************************
// Class: EngineBase
//
// Purpose:
//   Base class for engine objects.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 17:32:53 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ENGINE_MAIN_API EngineBase
{
public:
    EngineBase();
    virtual ~EngineBase();

    static Engine *GetEngine();
    static void    DeleteEngine();

    static void SetEngineCreationCallback(Engine *(*cb)(void *), void *cbdata);
private:
    static Engine *engine_instance;
    static Engine *(*engineCreateCB)(void *);
    static void   *engineCreateCBData;
};

#endif
