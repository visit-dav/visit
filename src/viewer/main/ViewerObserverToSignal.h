// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             ViewerRPCObserver.h                           //
// ************************************************************************* //

#ifndef VIEWER_OBSERVER_TO_SIGNAL_H
#define VIEWER_OBSERVER_TO_SIGNAL_H
#include <viewer_exports.h>
#include <QObject>
#include <Observer.h>

// ****************************************************************************
//  Class: ViewerObserverToSignal
//
//  Purpose:
//    ViewerObserverToSignal observes state objects or RPC's and emits a Qt
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
//    Brad Whitlock, Wed Feb 14 10:58:34 PDT 2007
//    I renamed the class to ViewerObserverToSignal.
//
//    Brad Whitlock, Thu Apr 22 15:47:45 PST 2010
//    I made the destructor virtual.
//
// ****************************************************************************

class VIEWER_API ViewerObserverToSignal : public QObject, public Observer
{
    Q_OBJECT
public:
    ViewerObserverToSignal(Subject *);
    virtual ~ViewerObserverToSignal();
    virtual void Update(Subject *);
signals:
    void execute();
};

#endif
