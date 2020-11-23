// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <QMap>

// Forward declarations.
class EngineList;
class QComboBox;
class QGroupBox;
class QLabel;
class QProgressBar;
class QPushButton;
class StatusAttributes;

// ****************************************************************************
// Class: QvisEngineWindow
//
// Purpose:
//   This class implements a window that presents the list of engines to the
//   user so the engines' statuses can be watched and engines can be killed.
//
// Notes:      This window obseves multiple subjects.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:27:54 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:12:08 PDT 2001
//    Separated the single status/progress bar into one which reports
//    total status and one which reports current stage progress.
//
//    Brad Whitlock, Wed Nov 27 13:47:30 PST 2002
//    I added widgets to show more engine information.
//
//    Brad Whitlock, Wed Feb 25 09:22:29 PDT 2004
//    I added a "Clear cache" button.
//
//    Jeremy Meredith, Tue Mar 30 09:32:41 PST 2004
//    I added support for simulations.
//
//    Brad Whitlock, Wed Apr  9 11:00:33 PDT 2008
//    QString for caption, shortName.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Brad Whitlock, Mon Oct 10 12:40:06 PDT 2011
//    I added more information.
//
//    Alister Maguire, Thu Nov 12 09:58:35 PST 2020
//    Removed interruptEngine and interruptEngineButton. We decided this
//    is very unlikely to ever be implemented.
//
// ****************************************************************************

class GUI_API QvisEngineWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

public:
    QvisEngineWindow(EngineList *engineList,
                     const QString &caption = QString(),
                     const QString &shortName = QString(),
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisEngineWindow();
    virtual void CreateWindowContents();
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectStatusAttributes(StatusAttributes *s);
private:
    void UpdateWindow(bool doAll);
    void UpdateStatusArea();
    void UpdateInformation(int index);
    void AddStatusEntry(const QString &key);
    void RemoveStatusEntry(const QString &key);
    void UpdateStatusEntry(const QString &key);
private slots:
    void closeEngine();
    void selectEngine(int index);
    void clearCache();
private:
    QMap<QString, StatusAttributes*>   statusMap;
    EngineList       *engines;
    StatusAttributes *statusAtts;
    Subject          *caller;
    QString           activeEngine;
    
    QLabel           *engineLabel;
    QComboBox        *engineCombo;
    QGroupBox        *engineInfo;
    QLabel           *engineNN;
    QLabel           *engineNP;
    QLabel           *engineNP_GPU;
    QLabel           *engineLB;
    QLabel           *engineLB_Scheme;
    QLabel           *totalStatusLabel;
    QProgressBar     *totalProgressBar;
    QLabel           *stageStatusLabel;
    QProgressBar     *stageProgressBar;
    QPushButton      *closeEngineButton;
    QPushButton      *clearCacheButton;
};

#endif
