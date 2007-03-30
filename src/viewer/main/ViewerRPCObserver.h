// ************************************************************************* //
//                             ViewerRPCObserver.h                           //
// ************************************************************************* //

#ifndef VIEWER_RPC_OBSERVER_H
#define VIEWER_RPC_OBSERVER_H
#include <viewer_exports.h>
#include <qobject.h>
#include <Observer.h>

// ****************************************************************************
//  Class: ViewerRPCObserver
//
//  Purpose:
//    ViewerRPCObserver observes state objects or RPC's and emits a Qt
//    signal when the subjects are updated.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Oct 27 14:39:39 PST 2000
//    I changed the code to emit a Qt signal instead of calling a callback.
//
//    Brad Whitlock, Mon Sep 17 11:14:12 PDT 2001
//    I generalized the object so it can observe other kinds of subjects.
//
// ****************************************************************************

class VIEWER_API ViewerRPCObserver : public QObject, public Observer
{
    Q_OBJECT
public:
    ViewerRPCObserver(Subject *);
    ~ViewerRPCObserver();
    virtual void Update(Subject *);
signals:
    void executeRPC();
};

#endif
