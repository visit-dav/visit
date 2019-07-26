// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <QObject>
#include <Observer.h>
#include <SyncAttributes.h>

// ****************************************************************************
// Class: Synchronizer
//
// Purpose:
//   A class for synchronizing communication between an embedded viewer
//   application and the viewer.
//
// Programmer: Eric Brugger
// Creation:   Fri May  1 08:02:37 PDT 2009
//
// Modifications:
//
// ****************************************************************************

class Synchronizer : public QObject, public Observer
{
   Q_OBJECT
public:
   Synchronizer(Subject *s, Subject *ds);
   virtual ~Synchronizer();

   void PostSynchronize();

signals:
   void synchronized();
private:
   virtual void        Update(Subject *s);

   int                 syncCount;
   SyncAttributes     *sync, *delayedSync;
};

#endif
