// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DELAYED_WINDOW_SIMPLEOBSERVER_H
#define QVIS_DELAYED_WINDOW_SIMPLEOBSERVER_H
#include <gui_exports.h>
#include <QvisDelayedWindow.h>
#include <SimpleObserver.h>

// ****************************************************************************
// Class: QvisDelayedWindowObserver
//
// Purpose:
//   This is the base class for a non-postable window that observes
//   multiple subjects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 15:53:55 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 12 12:16:08 PDT 2002
//   I added the ability for derived classes to query which subject caused
//   the call to the UpdateWindow method.
//
//   Brad Whitlock, Mon Sep 30 07:49:25 PDT 2002
//   I added window flags.
//
//   Brad Whitlock, Wed Apr  9 10:30:42 PDT 2008
//   Changed ctor args.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

class GUI_API QvisDelayedWindowSimpleObserver : public QvisDelayedWindow, public SimpleObserver
{
    Q_OBJECT
public:
    QvisDelayedWindowSimpleObserver(const QString &caption, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisDelayedWindowSimpleObserver();
    virtual void CreateWindowContents() = 0;
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
protected:
    Subject *SelectedSubject();
private:
    Subject *selectedSubject;
};

#endif
