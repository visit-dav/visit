// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef RPC_EXECUTOR_H
#define RPC_EXECUTOR_H
#include <Observer.h>


// ****************************************************************************
//  Class: RPCExecutor
//
//  Purpose:
//      A wrapper for implementing RPC functionality.
//
//  Note:
//      To use, simply implement the RPCExecutor<T>::Execute(T*) method
//      and attach one to a matching RPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
// ****************************************************************************
template <class T>
class RPCExecutor : public Observer
{
public:
    RPCExecutor(Subject *s) : Observer(s)
    {
    }
    virtual void Update(Subject *s)
    {
        Execute((T *)s);
    }
    void Execute(T *);
};

#endif
