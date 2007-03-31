#ifndef QVIS_POSTABLE_WINDOW_OBSERVER_H
#define QVIS_POSTABLE_WINDOW_OBSERVER_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// ****************************************************************************
// Class: QvisPostableWindowObserver
//
// Purpose:
//   This is the base class for postable windows that observe state
//   objects and update themselves when the state objects change.
//   This class also has an Apply button in addition to Post and
//   Dismiss buttons.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:06:56 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 13:44:14 PST 2001
//   Added the "extra_" argument to the contructor.
//
//   Brad Whitlock, Wed May 2 11:32:41 PDT 2001
//   Replaced extra argument with buttonCombination.
//
//   Brad Whitlock, Thu Aug 23 8:31:23 PDT 2001
//   Added a private slot function to help with make default.
//
//   Brad Whitlock, Fri Feb 15 11:17:34 PDT 2002
//   Changed the protection on some of the methods.
//
//   Brad Whitlock, Fri Nov 7 16:10:58 PST 2003
//   I made it inherit from QvisPostableWindowSimpleObserver and I moved
//   some of the functionality there too.
//
// ****************************************************************************

class GUI_API QvisPostableWindowObserver : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisPostableWindowObserver(Subject *subj,
                               const char *caption = 0,
                               const char *shortName = 0,
                               QvisNotepadArea *notepad = 0,
                               int buttonCombo = AllExtraButtons,
                               bool stretch = true);
    virtual ~QvisPostableWindowObserver();

    virtual void SubjectRemoved(Subject *TheRemovedSubject);

public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents() = 0;
    Subject *subject;
};

#endif
