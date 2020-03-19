// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "Synchronizer.h"

Synchronizer::Synchronizer(Subject *s, Subject *ds) : QObject(), Observer(s)
{
    syncCount = 100;
    sync = (SyncAttributes *)s;
    delayedSync = (SyncAttributes *)ds;
}

Synchronizer::~Synchronizer()
{
}

void
Synchronizer::PostSynchronize()
{
    delayedSync->SetSyncTag(syncCount++);
    delayedSync->Notify();
}

void Synchronizer::Update(Subject *)
{
    emit synchronized();
}
