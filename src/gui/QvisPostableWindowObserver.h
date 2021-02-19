// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Wed Apr  9 10:48:01 PDT 2008
//   QString for caption and shortName.
//
//   Jeremy Meredith, Fri Jan  2 17:12:56 EST 2009
//   Added support for Load/Save on this window's sole subject.
//
// ****************************************************************************

class GUI_API QvisPostableWindowObserver : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisPostableWindowObserver(Subject *subj,
                               const QString &caption = QString(),
                               const QString &shortName = QString(),
                               QvisNotepadArea *notepad = 0,
                               int buttonCombo = AllExtraButtonsAndLoadSave,
                               bool stretch = true);
    virtual ~QvisPostableWindowObserver();

    virtual void SubjectRemoved(Subject *TheRemovedSubject);

public slots:
    virtual void apply();
    virtual void loadSubject();
    virtual void saveSubject();
protected:
    virtual void CreateWindowContents() = 0;
    Subject *subject;
};

#endif
