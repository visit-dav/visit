#ifndef QVIS_POSTABLE_WINDOW_SIMPLEOBSERVER_H
#define QVIS_POSTABLE_WINDOW_SIMPLEOBSERVER_H
#include <gui_exports.h>
#include <QvisPostableWindow.h>
#include <SimpleObserver.h>

// ****************************************************************************
// Class: QvisPostableWindowSimpleObserver
//
// Purpose:
//   This is the base class for a postable window that observes multiple
//   subjects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 11:46:00 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:26:46 PDT 2002
//   Made CreateEntireWindow public.
//
//   Jeremy Meredith, Wed May  8 10:47:14 PDT 2002
//   Added stretch.
//
//   Brad Whitlock, Mon Sep 9 10:58:32 PDT 2002
//   I made the apply button optional.
//
// ****************************************************************************

class GUI_API QvisPostableWindowSimpleObserver : public QvisPostableWindow, public SimpleObserver
{
    Q_OBJECT
public:
    QvisPostableWindowSimpleObserver(const char *caption = 0,
                                     const char *shortName = 0,
                                     QvisNotepadArea *n = 0,
                                     bool stretch = true);
    virtual ~QvisPostableWindowSimpleObserver();
    virtual void CreateWindowContents() = 0;
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    Subject *SelectedSubject();
    virtual void CreateEntireWindow();

protected:
    void setApplyButton(bool val) { applyButton = val; }
    bool getStretchWindow() const { return stretchWindow; };
private:
    Subject *selectedSubject;
    bool     stretchWindow;
    bool     applyButton;
};

#endif
