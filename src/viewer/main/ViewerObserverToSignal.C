// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             ViewerObserverToSignal.C                      //
// ************************************************************************* //

#include <ViewerObserverToSignal.h>

// ****************************************************************************
//  Method: ViewerObserverToSignal constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Oct 27 14:47:26 PST 2000
//    I removed the code to this function since it is no longer needed.
//
//    Brad Whitlock, Mon Sep 17 11:11:59 PDT 2001
//    I changed the argument from ViewerRPC to Subject.
//
// ****************************************************************************

ViewerObserverToSignal::ViewerObserverToSignal(Subject *subj) : Observer(subj)
{
}

// ****************************************************************************
//  Method: ViewerObserverToSignal destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerObserverToSignal::~ViewerObserverToSignal()
{
}

// ****************************************************************************
//  Method: ViewerObserverToSignal::Update
//
//  Purpose:
//      Execute an RPC in the subject.
//
//  Arguments:
//      s       The subject which contains the rpc information.
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//      Eric Brugger, Wed Aug 16 12:19:04 PDT 2000
//      Modify the RPC callbacks to take a VisApplication as a second
//      argument.
//
//      Brad Whitlock, Fri Oct 27 14:45:29 PST 2000
//      I changed the code so it emits a Qt signal instead of calling a
//      callback function in the ViewerSubject.
//
//      Brad Whitlock, Mon Sep 17 11:11:11 PDT 2001
//      I generalized the function so it the subject does not have to be
//      a viewerRPC object.
//
// ****************************************************************************

void ViewerObserverToSignal::Update(Subject *)
{
    // Tell the ViewerSubject to execute an RPC.
    emit execute();
}
