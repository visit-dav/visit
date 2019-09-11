// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_RPC_EXECUTOR_H
#define ENGINE_RPC_EXECUTOR_H
#include <Observer.h>
#include <EngineBase.h>

// ****************************************************************************
//  Class: EngineRPCExecutor
//
//  Purpose:
//      A wrapper for implementing RPC functionality.
//
//  Note:
//      To use, simply implement the EngineRPCExecutor<T>::Execute(T*) method
//      and attach one to a matching RPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
// ****************************************************************************
template <class T>
class EngineRPCExecutor : public Observer, public EngineBase
{
public:
    EngineRPCExecutor(Subject *s) : Observer(s), EngineBase()
    {
    }
    virtual void Update(Subject *s)
    {
        Execute((T *)s);
    }
    void Execute(T *);
};

#endif
