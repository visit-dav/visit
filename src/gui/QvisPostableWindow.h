// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_POSTABLE_WINDOW
#define QVIS_POSTABLE_WINDOW
#include <gui_exports.h>
#include <QString>
#include <QvisWindowBase.h>

class DataNode;
class QvisNotepadArea;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class QTextEdit;
class QvisHelpWindow;

// ****************************************************************************
// Class: QvisPostableWindow
//
// Purpose:
//   This is a base class for postable windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:28:28 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:40:41 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Wed May 2 11:09:42 PDT 2001
//   Removed interpreter member.
//
//   Brad Whitlock, Tue Sep 25 12:28:53 PDT 2001
//   Made post() and unpost() virtual.
//
//   Brad Whitlock, Fri Feb 15 11:16:41 PDT 2002
//   Made CreateEntireWindow public.
//
//   Brad Whitlock, Fri Sep 5 15:50:16 PST 2003
//   Added postWhenShown static member.
//
//   Brad Whitlock, Mon Nov 14 10:42:26 PDT 2005
//   Added postEnabled static member.
//
//   Brad Whitlock, Wed Apr  9 10:50:05 PDT 2008
//   QString for caption and shortName.
//
//   Brad Whitlock, Fri May 30 09:59:10 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Jul 23 16:14:30 PDT 2009
//   I added SetAddStretch and SetDismissEnabled.
//
//   Cyrus Harrison, Thu May  6 16:32:07 PDT 2010
//   Added 'avoid_scroll' argument. This avoids creating scrollbars if they do
//   not already exist.
//
// ****************************************************************************

class GUI_API QvisPostableWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisPostableWindow(const QString &captionString = QString(),
                       const QString &shortName = QString(),
                       QvisNotepadArea *n = 0);
    virtual ~QvisPostableWindow();
    virtual void CreateWindowContents() = 0;
    QWidget *GetCentralWidget();
    const QString &GetShortCaption();
    bool posted();
    virtual void CreateEntireWindow();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);

    static void  SetPostEnabled(bool);
    void         SetDismissEnabled(bool);
    void         SetAddStretch(bool);
protected:
    virtual void UpdateWindow(bool doAll);
public slots:
    virtual void raise();
    virtual void show();
    virtual void hide();
    virtual void post() { post(false); } ///TODO: check on this fix for overloaded function
    virtual void post(bool avoid_scroll);
    virtual void unpost();
    virtual void help();
protected:
    bool               isCreated;
    bool               isPosted;
    bool               addLayoutStretch;
    QString            shortCaption;
    QWidget            *central;
    QVBoxLayout        *topLayout;
    QPushButton        *postButton;
    QPushButton        *dismissButton;
    QPushButton        *helpButton;
    QvisNotepadArea    *notepad;

    static bool         postWhenShown;
    static bool         postEnabled;
};

#endif
