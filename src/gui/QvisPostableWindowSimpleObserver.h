// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Fri Nov 7 16:12:55 PST 2003
//   I added much of QvisPostableWindowObserver's extra functionality.
//
//   Brad Whitlock, Wed Apr  9 10:49:01 PDT 2008
//   QString for caption and shortName.
//
//   Jeremy Meredith, Fri Jan  2 17:05:57 EST 2009
//   Added Load/Save button support.
//
// ****************************************************************************

class GUI_API QvisPostableWindowSimpleObserver : public QvisPostableWindow, public SimpleObserver
{
    Q_OBJECT
public:
    static const int NoExtraButtons;
    static const int ApplyButton;
    static const int MakeDefaultButton;
    static const int ResetButton;
    static const int SaveButton;
    static const int LoadButton;
    static const int AllExtraButtons;
    static const int AllExtraButtonsAndLoadSave;

    QvisPostableWindowSimpleObserver(const QString &caption = QString(),
                                     const QString &shortName = QString(),
                                     QvisNotepadArea *n = 0,
                                     int buttonCombo = AllExtraButtons,
                                     bool stretch = true);
    virtual ~QvisPostableWindowSimpleObserver();

    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    virtual void CreateEntireWindow();

public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
    virtual void loadSubject();
    virtual void saveSubject();
protected slots:
    void makeDefaultHelper();
protected:
    virtual void CreateWindowContents() = 0;
    Subject     *SelectedSubject();

    Subject *selectedSubject;
    int      buttonCombination;
    bool     stretchWindow;
};

#endif
