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
// ****************************************************************************

class GUI_API QvisDelayedWindowSimpleObserver : public QvisDelayedWindow, public SimpleObserver
{
    Q_OBJECT
public:
    QvisDelayedWindowSimpleObserver(const char *caption = 0, WFlags f = 0);
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
