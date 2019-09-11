// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "Synchronizer.h"

// ****************************************************************************
// Method: Synchronizer::Synchronizer
//
// Purpose:
//   Constructor.
//
// Programmer: Eric Brugger
// Creation:   Fri May  1 08:06:28 PDT 2009
//
// Modifications:
//
// ****************************************************************************

Synchronizer::Synchronizer(Subject *s, Subject *ds) : QObject(), Observer(s)
{
    syncCount = 100;
    sync = (SyncAttributes *)s;
    delayedSync = (SyncAttributes *)ds;
}

// ****************************************************************************
// Method: Synchronizer::~Synchronizer
//
// Purpose:
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Fri May  1 08:06:28 PDT 2009
//
// Modifications:
//
// ****************************************************************************

Synchronizer::~Synchronizer()
{
}

// ****************************************************************************
// Method: Synchronizer::PostSynchronize
//
// Purpose:
//   Causes the viewer to get an update event.  This routine should be called
//   after doing some viewer action that requires waiting for the viewer to
//   complete it.
//
// Programmer: Eric Brugger
// Creation:   Fri May  1 08:06:28 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void
Synchronizer::PostSynchronize()
{
    delayedSync->SetSyncTag(syncCount++);
    delayedSync->Notify();
}

// ****************************************************************************
// Method: Synchronizer::Update
//
// Purpose:
//   This update method, which is called when the viewer is finished
//   processing its work.  This causes the embedded application to regain
//   control and issue more viewer actions.
//
// Programmer: Eric Brugger
// Creation:   Fri May  1 08:06:28 PDT 2009
//
// Modifications:
//
// ****************************************************************************

void Synchronizer::Update(Subject *)
{
    emit synchronized();
}
