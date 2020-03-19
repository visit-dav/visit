// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_RECENT_PATH_REMOVAL_WINDOW_H
#define QVIS_RECENT_PATH_REMOVAL_WINDOW_H
#include <gui_exports.h>
#include <QvisDelayedWindowObserver.h>
#include <QualifiedFilename.h>

// Forward declarations
class QGroupBox;
class QListWidget;
class QPushButton;

// ****************************************************************************
// Class: QvisRecentPathRemovalWindow
//
// Purpose:
//   Removes paths from the recently visited path list in the file selection
//   window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 14:57:48 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:43:34 PDT 2008
//   Make captionString use QString.
//
// ****************************************************************************

class GUI_API QvisRecentPathRemovalWindow : public QvisDelayedWindowObserver
{
    Q_OBJECT
public:
    QvisRecentPathRemovalWindow(Subject *s, const QString &captionString);
    virtual ~QvisRecentPathRemovalWindow();
public slots:
    virtual void show();
protected:
    virtual void CreateWindowContents();
    virtual void UpdateWindow(bool doAll);
    void UpdateWidgets();
private slots:
    void removePaths();
    void removeAllPaths();
    void invertSelection();
    void applyDismiss();
    void handleCancel();
private:
    QGroupBox   *removalControlsGroup;
    QListWidget    *removalListBox;
    QPushButton *removeButton;
    QPushButton *removeAllButton;
    QPushButton *invertSelectionButton;

    QPushButton *okButton;
    QPushButton *cancelButton;

    QualifiedFilenameVector paths;
};

#endif
